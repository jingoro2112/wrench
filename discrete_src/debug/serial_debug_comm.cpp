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

#if (defined(WRENCH_WIN32_SERIAL) || defined(WRENCH_LINUX_SERIAL) || defined(WRENCH_ARDUINO_SERIAL)) && defined(WRENCH_INCLUDE_DEBUG_CODE)

//------------------------------------------------------------------------------
WrenchDebugSerialInterface::WrenchDebugSerialInterface()
{
#ifndef WRENCH_ARDUINO_SERIAL
	m_interface = WR_BAD_SOCKET;
#endif
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::open( const char* name )
{
	m_interface = wr_serialOpen( name );
	return m_interface != WR_BAD_SOCKET;
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::sendEx( const uint8_t* data, const int bytes )
{
	int soFar = 0;
	while( soFar < bytes )
	{
		int ret = wr_serialSend( m_interface, (const char*)data + soFar, bytes - soFar );
		if ( ret == -1 )
		{
			wr_serialClose( m_interface );
			m_interface = WR_BAD_SOCKET;
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds )
{
	int soFar = 0;

	while( soFar < bytes )
	{
		int ret = wr_serialReceive( m_interface, (char*)(data + soFar), bytes - soFar );
		if ( ret == -1 )
		{
			wr_serialClose( m_interface );
			m_interface = WR_BAD_SOCKET;
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WrenchDebugSerialInterface::peekEx( const int timeoutMilliseconds )
{
	int ret = wr_serialPeek( m_interface );
	if ( ret < 0 )
	{
		wr_serialClose( m_interface );
		m_interface = WR_BAD_SOCKET;
		return 0;
	}

	return (uint32_t)ret;
}

#endif
