/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

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

#if (defined(WRENCH_WIN32_TCP) || defined(WRENCH_LINUX_TCP)) && defined(WRENCH_INCLUDE_DEBUG_CODE)

//------------------------------------------------------------------------------
WrenchDebugTcpInterface::WrenchDebugTcpInterface()
{
	m_socket = -1;

#ifdef WRENCH_WIN32_TCP
	WSADATA wsaData;
	WSAStartup( MAKEWORD(2, 2), &wsaData );
#endif	
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::serve( const int port )
{
	wr_closeTCPEx( m_socket );
	m_socket = wr_bindTCPEx( port );
	return m_socket != -1;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::accept()
{
	sockaddr info;
	int socket = wr_acceptTCPEx( m_socket, &info );
	if ( socket == -1 )
	{
		return false;
	}

	wr_closeTCPEx( m_socket );
	m_socket = socket;
	
	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::connect( const char* address, const int port )
{
	wr_closeTCPEx( m_socket );
	return (m_socket = wr_connectTCPEx( address, port )) != -1;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::sendEx( const uint8_t* data, const int bytes )
{
	int soFar = 0;
	while( soFar < bytes )
	{
		int ret = wr_sendTCPEx( m_socket, (const char*)data + soFar, bytes - soFar );
		if ( ret == -1 )
		{
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds )
{
	int soFar = 0;

	while( soFar < bytes )
	{
		int ret = wr_receiveTCPEx( m_socket, (char*)(data + soFar), bytes - soFar, timeoutMilliseconds );
		if ( ret == -1 )
		{
			return false;
		}
		
		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WrenchDebugTcpInterface::peekEx( const int timeoutMilliseconds )
{
	int ret = wr_peekTCPEx( m_socket, timeoutMilliseconds );
	if ( ret < 0 )
	{
		wr_closeTCPEx( m_socket );
		m_socket = -1;
		return 0;
	}
			
	return ret;
}

#endif
