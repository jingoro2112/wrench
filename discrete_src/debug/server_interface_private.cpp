/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

MIT License

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
WRDebugServerInterfacePrivate::WRDebugServerInterfacePrivate( WRDebugServerInterface* parent )
{
	memset( (char*)this, 0, sizeof(*this) );
	m_parent = parent;


	m_lineBreaks = (SimpleLL<int>*)g_malloc(sizeof(SimpleLL<int>));
	new (m_lineBreaks)  SimpleLL<int>();

	m_callStack = (SimpleLL<WrenchCallStackEntry>*)g_malloc(sizeof(SimpleLL<WrenchCallStackEntry>));
	new (m_callStack)  SimpleLL<WrenchCallStackEntry>();

}

//------------------------------------------------------------------------------
WRDebugServerInterfacePrivate::~WRDebugServerInterfacePrivate()
{
	g_free( m_externalCodeBlock ); // MIGHT have been allocated

	m_lineBreaks->~SimpleLL<int>();
	g_free( m_lineBreaks );

	m_callStack->~SimpleLL<WrenchCallStackEntry>();
	g_free( m_callStack );
}

//------------------------------------------------------------------------------
void WRDebugServerInterface::tick()
{
	WrenchPacket* reply = 0;
	bool err = false;
	
	while( I->m_dataAvailableFunction() >= (int)sizeof(WrenchPacket) )
	{
		if ( err )
		{
			// on err, drain the q and restart
			while( I->m_dataAvailableFunction() )
			{
				I->m_receiveFunction( (char *)&I->m_lastPacket, 1 );
			}
			err = false;
			continue;
		}
		
		if ( !I->m_receiveFunction( (char *)&I->m_lastPacket, sizeof(WrenchPacket)) )
		{
			err = true;
			continue;
		}

		I->m_lastPacket.xlate();

		if ( I->m_lastPacket.payloadSize() == 0 )
		{
			reply = I->processPacket( &I->m_lastPacket );
		}
		else
		{
			WrenchPacket *p = WrenchPacket::alloc( I->m_lastPacket );
			if ( I->m_receiveFunction( (char*)p->payload(), p->payloadSize() ) )
			{
				reply = I->processPacket( p );
			}
			else
			{
				err = true;
				reply = 0;
			}

			g_free( p );
		}

		if ( reply )
		{
			uint32_t size = reply->size;
			
			reply->xlate();
			
			I->m_sendFunction( (char*)reply, size );
			g_free( reply );
		}
	}
}

//------------------------------------------------------------------------------
WRContext* WRDebugServerInterface::loadBytes( const uint8_t* bytes, const int len )
{
	if ( I->m_context )
	{
		wr_destroyContext( I->m_context );
		I->m_context = 0;
	}

	if ( !bytes || !len || !I->m_w )
	{
		return 0;
	}

	WRContext* ret = wr_newContext( I->m_w, bytes, len );
	I->m_context = ret;

	if ( !ret )
	{
		return 0;
	}

	ret->debugInterface = this;
	
	I->m_steppingOverReturnVector = 0;
	I->m_lineSteps = 0;
	I->m_stepOverDepth = -1;

	I->m_lineBreaks->clear();

	I->m_stopOnLine = 0;
	I->m_firstCall = true;
	I->m_stepOut = false;
	I->m_halted = false;
 	
	const uint8_t* code = bytes + 2;
	
	I->m_compilerFlags = READ_8_FROM_PC( code++ );

	// skip over function signatures
	code += I->m_context->numLocalFunctions * WR_FUNCTION_CORE_SIZE;

	if ( I->m_compilerFlags & WR_INCLUDE_GLOBALS )
	{
		code += ret->globals * sizeof(uint32_t); // these are lazily loaded, just skip over them for now
	}

	if ( I->m_compilerFlags & WR_EMBED_DEBUG_CODE || I->m_compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		I->m_embeddedSourceHash = READ_32_FROM_PC( code );
		code += 4;

		I->m_symbolBlockLen = READ_16_FROM_PC( code );
		code += 2;

		I->m_symbolBlock = code;
		code += I->m_symbolBlockLen;
	}

	if ( I->m_compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		I->m_embeddedSourceLen = READ_32_FROM_PC( code );
		I->m_embeddedSource = code + 4;
	}

	return ret;
}

//------------------------------------------------------------------------------
bool WRDebugServerInterfacePrivate::codewordEncountered( const uint8_t* pc, uint16_t codeword, WRValue* stackTop )
{
	unsigned int type = codeword & WRD_TypeMask;

	if ( type == WRD_LineNumber )
	{
		int line = codeword & WRD_PayloadMask;
		m_callStack->tail()->onLine = line;

		for( int* L = m_lineBreaks->first() ; L ; L = m_lineBreaks->next() )
		{
			if ( *L == line )
			{
				return true;
			}
		}

		if ( m_stepOverDepth <= 0
			 && m_lineSteps
			 && !--m_lineSteps )
		{
			m_stopOnLine = 0;
			m_stepOverDepth = -1;
			return true;
		}

		if ( line == m_stopOnLine )
		{
			m_stopOnLine = 0;
			return true;
		}
	}
	else if ( type == WRD_FunctionCall )
	{
		if ( (codeword & WRD_PayloadMask) == WRD_GlobalStopFunction )
		{
			m_halted = true;
			return false;
		}
		
		if ( (codeword & WRD_PayloadMask) == WRD_ExternalFunction )
		{
			// no need for bookkeeping, just skip it
			return false;
		}
		
		if ( m_stepOverDepth >= 0 ) // track stepping over/into functions
		{
			++m_stepOverDepth;
		}

		WrenchCallStackEntry* from = m_callStack->tail();
		WrenchCallStackEntry* entry = m_callStack->addTail();
		entry->onLine = -1; // don't know yet!
		entry->fromUnitIndex = from->thisUnitIndex;
		entry->thisUnitIndex = codeword & WRD_PayloadMask;
//		entry->stackOffset = stackTop - m_w->stack;

		WRFunction* func = m_context->localFunctions + (entry->thisUnitIndex - 1); // unit '0' is the global unit
		entry->arguments = func->arguments;
		entry->locals = func->frameSpaceNeeded;
		
		//printf( "Calling[%d] @ line[%d] stack[%d]\n", m_status.onFunction, m_status.onLine, (int)(stackTop - m_context->w->stack) );
	}
	else if ( type == WRD_Return )
	{
		m_callStack->popHead();

		if ( m_stepOverDepth >= 0 )
		{
			--m_stepOverDepth;
		}
	}
	
	return false;
}

//------------------------------------------------------------------------------
void WRDebugServerInterfacePrivate::clearBreakpoint( const int bp )
{
	for( int* L = m_lineBreaks->first(); L ; L = m_lineBreaks->next() )
	{
		if ( *L == bp )
		{
			m_lineBreaks->popItem( L );
			return;
		}
	}
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugServerInterfacePrivate::processPacket( WrenchPacket* packet )
{
	WrenchPacket* reply = 0;
	
	switch( packet->_type )
	{
		case WRD_RequestStepOver:
		{
			m_lineSteps = 1;
			m_stepOverDepth = 0;
			goto WRDRun;
		}

		case WRD_RequestStepInto:
		{
			m_lineSteps = 1;
			m_stepOverDepth = -1;
			goto WRDRun;
		}
		
		case WRD_Run:
		{
WRDRun:
			if ( !m_context || m_halted )
			{
				reply = WrenchPacket::alloc( WRD_Err );
				break;
			}

			m_stopOnLine = packet->param1;

			if ( m_firstCall )
			{ 
				m_firstCall = false;
				
				m_callStack->clear();
				
				WrenchCallStackEntry* stack = m_callStack->addTail();
				memset( (char*)stack, 0, sizeof(WrenchCallStackEntry) );
				stack->locals = m_context->globals;
				stack->thisUnitIndex = 0;

				wr_executeContext( m_context );
			}
			else
			{
				wr_continue( m_context );
			}

			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_Load:
		{
			uint32_t size = packet->payloadSize();
			if ( size )
			{
				if ( m_externalCodeBlockSize < size )
				{
					g_free( m_externalCodeBlock );
					m_externalCodeBlock = (uint8_t*)g_malloc( size );
					m_externalCodeBlockSize = size;
				}
				
				memcpy( m_externalCodeBlock, packet->payload(), size );
				
				m_parent->loadBytes( m_externalCodeBlock, m_externalCodeBlockSize );
			}
			else
			{
				m_parent->loadBytes( m_context->bottom, m_context->bottomSize );
			}

			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_RequestSourceBlock:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else if ( !m_embeddedSourceLen )
			{
				reply = WrenchPacket::alloc( WRD_ReplyUnavailable );
			}
			else
			{
				uint32_t size = sizeof(WrenchPacket) + m_embeddedSourceLen + 1;
				reply = WrenchPacket::alloc( WRD_ReplySource, size );
				uint32_t i=0;
				for( ; i< m_embeddedSourceLen; ++i )
				{
					*reply->payload( i ) = (uint8_t)READ_8_FROM_PC( m_embeddedSource + i );
				}
				*reply->payload( i ) = 0;
			}
			
			break;
		}

		case WRD_RequestSymbolBlock:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else if ( !m_symbolBlockLen )
			{
				reply = WrenchPacket::alloc( WRD_ReplyUnavailable );
			}
			else
			{
				uint32_t size = sizeof(WrenchPacket) + m_symbolBlockLen;
				reply = WrenchPacket::alloc( WRD_ReplySymbolBlock, size );
				uint32_t i=0;
				for( ; i < m_symbolBlockLen; ++i )
				{
					*reply->payload( i ) = (uint8_t)READ_8_FROM_PC( m_symbolBlock + i );
				}
			}				
			
			break;
		}

		case WRD_RequestSourceHash:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else
			{
				reply = WrenchPacket::alloc( WRD_ReplySourceHash );
				reply->param1 = m_embeddedSourceHash;
			}
			break;
		}

		case WRD_RequestValue:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else
			{
				int index = packet->param1;
				int depth = packet->param2;

				WRValueSerializer serializer;

				if (depth == 0) // globals are stored separately
				{
					wr_serializeEx( serializer, ((WRValue *)(m_context + 1))[index] );
				}
				else
				{
					WrenchCallStackEntry* frame = (*m_callStack)[depth];
					
					if ( !frame )
					{
						reply = WrenchPacket::alloc( WRD_Err );
					}
					else
					{
						WRValue* stackFrame = m_context->stack + (frame->stackOffset - frame->arguments);
						
						wr_serializeEx( serializer, *(stackFrame + index) );
					}
				}
				
				if ( !reply )
				{
					reply = WrenchPacket::alloc( WRD_ReplyValue, serializer.size() );
					memcpy( reply->payload(), serializer.data(), serializer.size() );
				}
			}
				
			break;
		}

		case WRD_RequestCallstack:
		{
			if ( !m_context || m_halted )
			{
				reply = WrenchPacket::alloc( WRD_ReplyCallstack );
				reply->param1 = 0;
				reply->param2 = (int32_t)(m_context ? WRP_Complete : (m_firstCall ? WRP_Loaded : WRP_Unloaded) );
			}
			else
			{
				uint32_t count = m_callStack->count();

				reply = WrenchPacket::alloc( WRD_ReplyCallstack, sizeof(WrenchCallStackEntry) * count );
				reply->param1 = count;

				reply->param2 = (int32_t)( m_firstCall ? WRP_Loaded : WRP_Running );
				
				WrenchCallStackEntry* pack = (WrenchCallStackEntry *)reply->payload();

				for( WrenchCallStackEntry* E = m_callStack->first(); E; E = m_callStack->next() )
				{
					*pack = *E;
					pack->onLine = wr_x32( pack->onLine );
					pack->locals = wr_x16( pack->locals );
					++pack;
				}
			}
			
			break;
		}

		case WRD_RequestStackDump:
		{
			WRstr dump( "none\n" );
			if ( m_context )
			{
				wr_stackDump( m_context->stack, m_context->yield_stackTop, dump );
			}
				
			reply = WrenchPacket::alloc( WRD_ReplyStackDump, dump.size() );
			memcpy( reply->payload(), dump.c_str(), dump.size() );

			break;
		}

		case WRD_RequestSetBreakpoint:
		{
			clearBreakpoint( packet->param1 );
			*m_lineBreaks->addTail() = packet->param1;
			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_RequestClearBreakpoint:
		{
			clearBreakpoint( packet->param1 );
			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		default:
		{
			reply = WrenchPacket::alloc( WRD_Err );
			break;
		}
	}

	return reply;
}

#endif
