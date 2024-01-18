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
WRDebugClientInterface::WRDebugClientInterface( size_t (*receiveFunction)( char* data, const size_t length, const int timeoutMilliseconds ),
												size_t (*writeFunction)( const char* data, const size_t length ) )
{
	init();
	
	m_receiveFunction = receiveFunction;
	m_writeFunction = writeFunction;
}

//------------------------------------------------------------------------------
WRDebugClientInterface::WRDebugClientInterface( WRDebugServerInterface* localServer )
{
	init();
	
	m_localServer = localServer;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::init()
{
	memset( this, 0, sizeof(*this) );

	m_packetQ = new SimpleLL<WrenchPacket>();
	m_globals = new SimpleLL<WrenchSymbol>();
	m_functions = new SimpleLL<WrenchFunction>();
	m_packet = new WrenchPacket;
}

//------------------------------------------------------------------------------
WRDebugClientInterface::~WRDebugClientInterface()
{
	delete m_packetQ;
	delete m_globals;
	delete m_functions;
	delete m_packet;
	
	delete[] m_sourceBlock;
}

/*
//------------------------------------------------------------------------------
const WRDebugMessage& WRDebugClientInterface::status()
{
	if ( m_localServer )
	{
		return m_localServer->m_msg;
	}
	else if ( m_writeFunction && m_receiveFunction )
	{
		WrenchPacket packet;
		packet.type = RequestStatus;

		char data[1 + (4 * 2)];
		m_msg.type = Err;
		
		if ( m_writeFunction((char *)&packet, sizeof(WrenchPacket)) == sizeof(WrenchPacket)
			 && m_receiveFunction(data, 4*2, -1) )
		{
			m_msg.type = data[0];
			m_msg.line = READ_32_FROM_PC( (const unsigned char *)(data + 1) );
			m_msg.function = READ_32_FROM_PC( (const unsigned char *)(data + 5) );
		}
	}
	else
	{
		m_msg.type = Err;
	}
			
	return m_msg;
}
*/
//------------------------------------------------------------------------------
void WRDebugClientInterface::load( const char* byteCode, const int size )
{
	WrenchPacket packet;
	packet.type = Load;
	wr_pack32( size, (unsigned char *)&packet.payloadSize );
	if ( size )
	{
		packet.payload = (char*)malloc( size );
		memcpy( packet.payload, byteCode, size );
	}

	transmit( packet );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::run()
{
	WrenchPacket packet;
	packet.type = Run;
	
	transmit( packet );
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getSourceCode( const char** data, int* len )
{
	WrenchPacket packet;
	packet.type = RequestSourceBlock;
	
	if ( !transmit(packet) )
	{
		return false;
	}

	WrenchPacket* P = receive();
	if ( !P || (P->type != ReplySource) )
	{
		return false;
	}

	m_sourceBlock = new char[ P->payloadSize ];
	memcpy( m_sourceBlock, P->payload, P->payloadSize );
	*len = P->payloadSize;
	*data = m_sourceBlock;
	return true;
}

//------------------------------------------------------------------------------
uint32_t WRDebugClientInterface::getSourceCodeHash()
{
	WrenchPacket packet;
	packet.type = RequestSourceBlock;

	if ( !transmit(packet) )
	{
		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugClientInterface::receive( const int timeoutMilliseconds )
{
	if ( m_localServer )
	{
		WrenchPacket* p = m_localServer->m_packetQ->head();
		if ( p )
		{
			*m_packet = *p;
			m_localServer->m_packetQ->popHead();
			return m_packet;
		}
	}
	else
	{
		m_packet->clear();
		
		size_t read = m_receiveFunction( (char *)m_packet, sizeof(WrenchPacket), timeoutMilliseconds );
		if ( read == sizeof(WrenchPacket) )
		{
			m_packet->xlate();
			if ( m_packet->payloadSize )
			{
				m_packet->payload = (char *)malloc( m_packet->payloadSize );
				if ( !m_receiveFunction(m_packet->allocate(m_packet->payloadSize), m_packet->payloadSize, timeoutMilliseconds) )
				{
					m_packet->clear();
					return 0;
				}
			}
	
			return m_packet;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::transmit( WrenchPacket& packet )
{
	packet.xlate();

	if ( m_localServer )
	{
		m_localServer->processPacket( &packet );
		// memory will be free'd as if it came in off the wire
		return true;
	}
	else if ( m_writeFunction )
	{
		size_t xmit = 5 + packet.payloadSize;
		bool ret = m_writeFunction( (char*)&packet, xmit ) == xmit;

		// must free any allocated memory if it was allocated
		free( packet.payload );
		
		return ret;
	}

	return false;

}

//------------------------------------------------------------------------------
void WRDebugClientInterface::loadSourceBlock()
{
	if ( m_sourceBlockHash )
	{
		return;
	}
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::loadSymbols()
{
	if ( m_symbolsLoaded )
	{
		return;
	}
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::populateSymbols( const char* block, const int size )
{
	m_globals->clear();
	m_functions->clear();

	int pos = 0;
	int globals = READ_16_FROM_PC( (const unsigned char *)block );
	pos += 2;
	int functions = READ_16_FROM_PC( (const unsigned char *)(block + pos) );
	pos += 2;

	for( int g=0; g<globals; ++g )
	{
		WrenchSymbol* s = m_globals->addTail();
		sscanf( block + pos, "%63s", s->label );
		while( block[pos++] );
	}

	for( int f=0; f<functions; ++f )
	{
		WrenchFunction* func = m_functions->addTail();
		sscanf( block + pos, "%63s", func->label );
		while( block[pos++] );

		int locals = (int)block[pos++];
		
		for( int l=0; l<locals; ++l )
		{
			WrenchSymbol* s = func->locals.addTail();
			sscanf( block + pos, "%63s", s->label );
			while( block[pos++] );
		}
	}
}
