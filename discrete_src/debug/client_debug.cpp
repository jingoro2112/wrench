/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include "wrench.h"

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
// hide this from the wrench.h header to keep it cleaner
void WRDebugClientInterface::init()
{
	I->m_scratchState = wr_newState();
	uint8_t* out;
	int outSize;
	wr_compile("", 0, &out, &outSize);
	I->m_scratchContext = wr_newContext( I->m_scratchState, out, outSize, true );
	I->m_scratchContext->allocatedMemoryHint = 0;
}

//------------------------------------------------------------------------------
WRDebugClientInterface::WRDebugClientInterface( WrenchDebugCommInterface* comm )
{
	I = new WRDebugClientInterfacePrivate;
	I->m_parent = this;
	I->m_comm = comm;

	init();
}

//------------------------------------------------------------------------------
WRDebugClientInterface::~WRDebugClientInterface()
{
	wr_destroyState( I->m_scratchState );
	delete I;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::load( const uint8_t* byteCode, const int size )
{
	I->m_scratchContext->gc(0);

	I->m_comm->send( WrenchPacketScoped(WRD_Load, size, byteCode) );

	// invalidate source block;
	g_free( I->m_sourceBlock );
	I->m_sourceBlock = 0;
	I->m_functions->clear();
	I->m_callstackDirty = true;
	I->m_symbolsLoaded = false;
	I->populateSymbols();
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::run( const int toLine ) 
{
	I->m_scratchContext->gc(0);
	I->m_callstackDirty = true;

	WrenchPacket packet( WRD_Run );
	packet.param1 = toLine;
	I->m_comm->send( &packet );

	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getSourceCode( const char** data, int* len )
{
	if ( !I->m_sourceBlock )
	{
		I->m_comm->send( WrenchPacket(WRD_RequestSourceBlock) );

		WrenchPacketScoped r( I->getPacket() );
		
		if ( !r || r.packet->_type != WRD_ReplySource )
		{
			return false;
		}
		
		I->m_sourceBlockLen = r.packet->payloadSize();
		I->m_sourceBlock = (char*)g_malloc( r.packet->payloadSize() + 1 );
		
		memcpy( I->m_sourceBlock, r.packet->payload(), r.packet->payloadSize() );
		I->m_sourceBlock[ r.packet->payloadSize() ] = 0;
	}
	
	*data = I->m_sourceBlock;
	*len = I->m_sourceBlockLen;
	
	return true;
}

//------------------------------------------------------------------------------
uint32_t WRDebugClientInterface::getSourceCodeHash()
{
	I->m_comm->send( WrenchPacket(WRD_RequestSourceHash) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || r.packet->_type != WRD_ReplySourceHash )
	{
		return 0;
	}

	return r.packet->param1;
}

//------------------------------------------------------------------------------
SimpleLL<WrenchFunction>& WRDebugClientInterface::getFunctions()
{
	I->populateSymbols();
	return *I->m_functions;
}

//------------------------------------------------------------------------------
const char* WRDebugClientInterface::getFunctionLabel( const int index )
{
	SimpleLL<WrenchFunction>& funcs = getFunctions();
	WrenchFunction *f = funcs[index];
	return f ? f->name : 0;
}

//------------------------------------------------------------------------------
WRValue* WRDebugClientInterface::getValue( WRValue& value, const int index, const int depth )
{
	value.init();

	WrenchPacket p( WRD_RequestValue );
	p.param1 = index;
	p.param2 = depth;

	I->m_comm->send( &p );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || r.packet->_type != WRD_ReplyValue )
	{
		return 0;
	}

	WRValueSerializer serializer( (char *)r.packet->payload(), r.packet->payloadSize() );

	wr_deserializeEx( value, serializer, I->m_scratchContext );

	return &value;
}

//------------------------------------------------------------------------------
const char* WRDebugClientInterface::getValueLabel( const int index, const int depth )
{
	WrenchFunction* func = (*I->m_functions)[depth];
	if (!func)
	{
		return 0;
	}
		
	int i = 0;
	for( WrenchSymbol* sym = func->vars->first(); sym; sym = func->vars->next() )
	{
		if ( i++ == index )
		{
			return sym->label;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getStackDump( char* out, const unsigned int maxOut )
{
	I->m_comm->send( WrenchPacket(WRD_RequestStackDump) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || (r.packet->_type != WRD_ReplyStackDump) )
	{
		return false;
	}

	strncpy( out, (const char*)r.packet->payload(), maxOut < r.packet->payloadSize() ? maxOut : r.packet->payloadSize() );
	
	return true;
}

//------------------------------------------------------------------------------
SimpleLL<WrenchCallStackEntry>* WRDebugClientInterface::getCallstack()
{
	if ( !I->m_callstackDirty )
	{
		return I->m_callStack;
	}

	I->m_callStack->clear();

	I->m_comm->send( WrenchPacket(WRD_RequestCallstack) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || (r.packet->_type != WRD_ReplyCallstack) )
	{
		return 0;
	}

	WrenchCallStackEntry* entries = (WrenchCallStackEntry *)r.packet->payload();

	I->m_procState = (WRP_ProcState)r.packet->param2;

	uint32_t count = r.packet->param1;
	for( uint32_t i=0; i<count; ++i )
	{
		entries[i].onLine = wr_x32( entries[i].onLine );
		entries[i].locals = wr_x16( entries[i].locals );
		*I->m_callStack->addTail() = entries[i];
	}

	I->m_callstackDirty = false;
	return I->m_callStack;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::stepInto( const int steps )
{
	I->m_callstackDirty = true;
	WrenchPacket packet( WRD_RequestStepInto );
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::stepOver( const int steps )
{
	I->m_callstackDirty = true;
	WrenchPacket packet( WRD_RequestStepOver );
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
WRP_ProcState WRDebugClientInterface::getProcState()
{
	return I->m_procState;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::setDebugEmitCallback( void (*print)( const char* debugText ) )
{
	I->outputDebug = print;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::setBreakpoint( const int lineNumber )
{
	WrenchPacket packet( WRD_RequestSetBreakpoint );
	packet.param1 = lineNumber;
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::clearBreakpoint( const int lineNumber )
{
	WrenchPacket packet( WRD_RequestClearBreakpoint );
	packet.param1 = lineNumber;
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

#endif
