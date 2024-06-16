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

const int wr_blankSize=10;
const unsigned char wr_blankCode[]=
{
	0x00, 0x00, 0x01, 0x02, 0xEE, 0x13, 0x25, 0xE1, 0xA4, 0x5A, // 10
};

//------------------------------------------------------------------------------
// hide this from the wrench.h header to keep it cleaner
void WRDebugClientInterface::init()
{
	I->m_scratchState = wr_newState();
	uint8_t* out;
	int outSize;
	wr_compile("", 0, &out, &outSize);
	I->m_scratchContext = wr_newContext(I->m_scratchState, out, outSize, true);
	I->m_scratchContext->allocatedMemoryHint = 0;
}

//------------------------------------------------------------------------------
WRDebugClientInterface::WRDebugClientInterface( bool (*receiveFunction)( char* data, const int length ),
												bool (*sendFunction)( const char* data, const int length ),
												int (*dataAvailableFunction)() )
{
	I = new WRDebugClientInterfacePrivate;
	I->m_receiveFunction = receiveFunction;
	I->m_sendFunction = sendFunction;
	I->m_dataAvailableFunction = dataAvailableFunction;

	init();
}

//------------------------------------------------------------------------------
WRDebugClientInterface::WRDebugClientInterface( WRDebugServerInterface* localServer )
{
	I = new WRDebugClientInterfacePrivate;
	I->m_localServer = localServer;
	
	init();
}

//------------------------------------------------------------------------------
WRDebugClientInterface::~WRDebugClientInterface()
{
	wr_destroyState( I->m_scratchState );
	delete I;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::load( const char* byteCode, const int size )
{
	I->m_scratchContext->gc(0);

	WrenchPacket* packet = WrenchPacket::alloc( WRD_Load, size );
	memcpy( packet->payload(), byteCode, size );
	
	free( I->transmit(packet) );

	delete[] I->m_sourceBlock;
	I->m_sourceBlock = 0;
	I->m_functions->clear();
	I->m_callstackDirty = true;
	I->m_symbolsLoaded = false;
	I->populateSymbols();
}


//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::trapRunOutput( WrenchPacket const& packet )
{
	for(;;)
	{
		WrenchPacket *reply = transmit( WrenchPacket::alloc(packet) );
		if ( reply->_type == WRD_DebugOut )
		{
			printf( "%s", (char*)reply->payload() );
			free( reply );
			continue;
		}
		else
		{
			free( reply );
			break;
		}
	}
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::run( const int toLine ) 
{
	I->m_scratchContext->gc(0);
	I->m_callstackDirty = true;

	WrenchPacket packet( WRD_Run );
	packet.param1 = toLine;

	I->trapRunOutput( packet );
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getSourceCode( const char** data, int* len )
{
	if ( !I->m_sourceBlock )
	{
		WrenchPacketScoped r( I->transmit(WrenchPacketScoped(WRD_RequestSourceBlock)) );
		
		if ( !r.packet || r.packet->_type != WRD_ReplySource )
		{
			return false;
		}
		
		delete[] I->m_sourceBlock;
		I->m_sourceBlockLen = r.packet->payloadSize();
		I->m_sourceBlock = new char[ r.packet->payloadSize() + 1 ];
		
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
	WrenchPacketScoped r( I->transmit(WrenchPacketScoped(WRD_RequestSourceHash)) );
	if ( !r.packet || r.packet->_type != WRD_ReplySourceHash )
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

	WrenchPacketScoped p( WRD_RequestValue );
	p.packet->param1 = index;
	p.packet->param2 = depth;

	WrenchPacketScoped r( I->transmit(p.packet) );

	if ( !r.packet || r.packet->_type != WRD_ReplyValue )
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
SimpleLL<WrenchCallStackEntry>* WRDebugClientInterface::getCallstack()
{
	if ( !I->m_callstackDirty )
	{
		return I->m_callStack;
	}

	I->m_callStack->clear();

	WrenchPacketScoped r( I->transmit(WrenchPacketScoped(WRD_RequestCallstack)) );
	if ( !r.packet || (r.packet->_type != WRD_ReplyCallstack) )
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
	I->trapRunOutput( packet );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::stepOver( const int steps )
{
	I->m_callstackDirty = true;
	WrenchPacket packet( WRD_RequestStepOver );
	I->trapRunOutput( packet );
}

//------------------------------------------------------------------------------
WRP_ProcState WRDebugClientInterface::getProcState()
{
	return I->m_procState;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::setBreakpoint( const int lineNumber )
{
	WrenchPacketScoped b(WRD_RequestSetBreakpoint);
	b.packet->param1 = lineNumber;
	free( I->transmit(b) );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::clearBreakpoint( const int lineNumber )
{
	WrenchPacketScoped b(WRD_RequestClearBreakpoint);
	b.packet->param1 = lineNumber;
	free (I->transmit(b) );
}

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::WRDebugClientInterfacePrivate()
{
	init();
}

//------------------------------------------------------------------------------
void wr_FunctionClearFunc( WrenchFunction& F )
{
	delete F.vars;
}
							  
//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::init()
{
	memset( this, 0, sizeof(*this) );
	m_packetQ = new SimpleLL<WrenchPacket*>( wr_PacketClearFunc );
	m_functions = new SimpleLL<WrenchFunction>( wr_FunctionClearFunc );
	m_callStack = new SimpleLL<WrenchCallStackEntry>();
}

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::~WRDebugClientInterfacePrivate()
{
	delete[] m_sourceBlock;

	delete m_packetQ;
	delete m_functions;
	delete m_callStack;
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugClientInterfacePrivate::transmit( WrenchPacket* packet )
{
	WrenchPacket* reply = 0;
	if ( m_localServer )
	{
		reply = m_localServer->I->processPacket( packet );
	}
	else
	{
		uint32_t size = packet->xlate();
		
		if ( m_sendFunction((char*)packet, size) )
		{
			WrenchPacket r;
			if ( m_receiveFunction( (char*)&r, sizeof(WrenchPacket)) )
			{
				r.xlate();
				reply = (WrenchPacket*)g_malloc( r.size );
				memcpy( (char*)reply, (char*)&r, sizeof(WrenchPacket) );
				
				if ( r.payloadSize() )
				{
					if ( !m_receiveFunction((char*)reply->payload(), reply->payloadSize()) )
					{
						g_free( reply );
						reply = 0;
					}
				}
			}
		}

		if ( reply == 0 )
		{
			// there was an error, drain the q
			while( m_dataAvailableFunction() )
			{
				char c;
				m_receiveFunction( &c, 1 );
			}
		}

	}

	return reply;
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::populateSymbols()
{
	if ( m_symbolsLoaded )
	{
		return;
	}
	
	m_functions->clear();

	WrenchPacketScoped r( transmit(WrenchPacketScoped(WRD_RequestSymbolBlock)) );

	if ( !r.packet || r.packet->_type != WRD_ReplySymbolBlock )
	{
		return;
	}

	m_symbolsLoaded = true;
	
	const unsigned char *block = (const unsigned char *)r.packet->payload();
	
	int functionCount = READ_16_FROM_PC( block );
	int pos = 2;

	for( int f=0; f<functionCount; ++f )
	{
		uint8_t count = READ_8_FROM_PC( block + pos );
		++pos;

		WrenchFunction* func = m_functions->addTail(); // so first is always "global"

		func->arguments = READ_8_FROM_PC( block + pos++ );
		sscanf( (const char *)(block + pos), "%63s", func->name );

		while( block[pos++] ); // skip to next null

		func->vars = new SimpleLL<WrenchSymbol>();
		for( uint8_t v=0; v<count; ++v )
		{
			WrenchSymbol* s = func->vars->addTail();
			sscanf( (const char*)(block + pos), "%63s", s->label );
			while( block[pos++] );
		}
	}
}

#endif
