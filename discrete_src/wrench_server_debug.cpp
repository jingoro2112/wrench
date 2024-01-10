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

//------------------------------------------------------------------------------
WRDebugServerInterface::WRDebugServerInterface( WRState* w,
												const unsigned char* bytes,
												const int len,
												bool (*receiveFunction)( char* data, const size_t length, const int timeoutMilliseconds ),
												size_t (*writeFunction)( const char* data, const size_t length )  )
{
	memset( this, 0, sizeof(*this) );

	m_lineBreaks = new SimpleLL<LineBreak>();
	m_packetQ = new SimpleLL<WrenchPacket>();
	
	m_receiveFunction = receiveFunction;
	m_writeFunction = writeFunction;
	m_w = w;

	loadBytes( bytes, len );
}

//------------------------------------------------------------------------------
WRDebugServerInterface::~WRDebugServerInterface()
{
	delete m_packetQ;
	delete m_lineBreaks;

	free( m_localBytes );
}

//------------------------------------------------------------------------------
WRContext* WRDebugServerInterface::loadBytes( const unsigned char* bytes, const int len )
{
	if ( m_context )
	{
		wr_destroyContext( m_context );
		m_context = 0;
	}

	if ( !bytes || !len )
	{
		return 0;
	}

	m_context = wr_newContext( m_w, bytes, len );
	m_firstCall = true;

	if ( !m_context )
	{
		return 0;
	}

	int debugBlockOffset = 4 // EOF hash
						   + (m_context->globals ? (4 + m_context->globals*4) : 0) // globals block plus it's CRC
						   + 28; // debug block plus it's crc

	if ( debugBlockOffset < m_context->bottomSize )
	{
		const unsigned char* data = m_context->bottom + (m_context->bottomSize - debugBlockOffset);
		uint32_t hash = wr_hash_read8( data, 24 );
		
		//flags = READ_32_FROM_PC( data );
		data += 4; // flags

		m_embeddedSourceSize = READ_32_FROM_PC( data );
		data += 4;

		m_embeddedSourceHash = READ_32_FROM_PC( data );
		data += 4;

		uint32_t offset = READ_32_FROM_PC( data );
		data += 4;
		m_embeddedSource = offset ? m_context->bottom + offset : 0;
		
		m_symbolBlockSize = READ_32_FROM_PC( data );
		data += 4;

		offset = READ_32_FROM_PC( data );
		data += 4;
		
		m_symbolBlock = offset ? m_context->bottom + offset : 0;

		if ( hash != (uint32_t)READ_32_FROM_PC( data ) )
		{
			return 0;
		}
	}
		
	return m_context;
}

/*
//------------------------------------------------------------------------------
void WRDebugInterface::setBreakpoint( const int lineNumber )
{
	unsigned char command[5];
	command[0] = SetBreak;
	wr_pack32( (int32_t)lineNumber, command + 1 );

	transmit( command, 5 );
}

//------------------------------------------------------------------------------
bool WRDebugInterface::getSourceCode( WRContext* context,
									  const char** data,
									  int* len )
{
	WrenchDebugBlock block;
	if ( !context || !data || !block.populate(context) || !block.sourceCodeSize )
	{
		return false;
	}

	*data = (const char *)(context->bottom + block.sourceCodeOffset);
	if ( len )
	{
		*len = block.sourceCodeSize;
	}

	return block.sourceCodeSize > 0;
}

//------------------------------------------------------------------------------
uint32_t WRDebugInterface::getSourceCodeHash( WRContext* context )
{
	WrenchDebugBlock block;
	return (context && block.populate(context)) ? block.sourceCodeCRC : 0;
}

//------------------------------------------------------------------------------
void WRDebugInterface::injectDataFromWrench( const char* data, const int len )
{
	switch( data[0] )
	{
		case SetBreak:
		{
			int line = READ_32_FROM_PC( data + 1 );
			int first = -1;
			for( int i=0; i<wr_maxLineNumberBreaks; ++i )
			{
				if ( m_lineNumberBreaks[i] == 0 && first == -1 )
				{
					first = i;
				}

				if ( m_lineNumberBreaks[i] == line )
				{
					break; // already set
				}

				if ( first != -1 )
				{
					m_lineNumberBreaks[first] = line;
				}
			}
			break;
		}

		default: break;
	}
}
*/


//------------------------------------------------------------------------------
void WRDebugServerInterface::codewordEncountered( uint16_t codeword, WRValue* stackTop )
{
	int type = codeword & TypeMask;
	
	if ( type == LineNumber )
	{
		m_onLine = codeword & PayloadMask;

		for( LineBreak* L = m_lineBreaks->first(); L ; L = m_lineBreaks->next() )
		{
			if ( L->line == m_onLine )
			{
				m_brk = true;
				break;
			}
		}

		if ( m_lineSteps && !--m_lineSteps )
		{
			m_brk = true;
		}

		m_brk = true;

	}
	else if ( type == FunctionCall )
	{
		m_onFunction = codeword & PayloadMask;

		printf( "Calling[%d] @ line[%d] stack[%d]\n", m_onFunction, m_onLine, (int)(stackTop - m_context->w->stack) );
	}
	else if ( type == Returned )
	{
		printf( "returning @ stack[%d]\n", (int)(stackTop - m_context->w->stack) );
	}
}

//------------------------------------------------------------------------------
void WRDebugServerInterface::processPacket( WrenchPacket* packet )
{
	packet->xlate();

	WrenchPacket* reply = m_packetQ->addTail();
	
	switch( packet->type )
	{
		case Run:
		{
			if ( !m_context )
			{
				reply->type = Err;
			}
			else if ( m_firstCall )
			{ 
				m_firstCall = false;
				wr_executeContext( m_context );
			}
			else
			{
				WRFunction f;
				f.offset = 0;
				wr_callFunction( m_context, &f, 0, 0 ); // signal "continue"
			}

			reply->type = Halted;
			break;
		}

		case Load:
		{
			int32_t size = packet->payloadSize;
			if ( size )
			{
				free( m_localBytes );
				m_localBytes = packet->payload; // memory was "malloced" before being sent to us
				m_localBytesLen = size;
				loadBytes( (unsigned char*)m_localBytes, m_localBytesLen );
			}
			else if ( m_context )
			{
				loadBytes( m_context->bytes, m_context->bytesLen );
			}
			
			break;
		}

		case RequestSourceBlock:
		{
			if ( !m_context )
			{
				reply->type = Err;
			}
			else if ( !m_embeddedSourceSize )
			{
				reply->type = ReplyUnavailable;
			}
			else
			{
				reply->type = ReplySource;
				reply->payload = (char *)malloc( m_embeddedSourceSize );
				for( int i=0; i<m_embeddedSourceSize; ++i )
				{
					reply->payload[i] = (unsigned char)READ_8_FROM_PC( m_embeddedSource + i );
				}
			}
			
			break;
		}
		
		case RequestSourceHash:
		{
			if ( !m_context )
			{
				reply->type = Err;
			}
			else
			{
				reply->type = ReplySourceHash;
				WrenchPacketGenericPayload *p = reply->genericPayload();
				p->hash = m_embeddedSourceHash;
				p->xlate();
			}
			break;
		}

		default:
		{
			break;
		}
	}
}
