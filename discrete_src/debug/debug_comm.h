#ifndef _DEBUG_COMM_H
#define _DEBUG_COMM_H
/*******************************************************************************
Copyright (c) 2026 Curt Hartung -- curt.hartung@gmail.com

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
class WrenchDebugCommInterface
{
public:
	// block until exactly 'bytes' are sent or return false 
	virtual bool send( WrenchPacket* packet );
	virtual WrenchPacket* receive( const int timeoutMilliseconds =0 );

	virtual bool sendEx( const uint8_t* data, const int bytes ) { return false; }
	virtual bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds ) { return false; }
	virtual uint32_t peekEx( const int timeoutMilliseconds ) { return 0; }

	virtual ~WrenchDebugCommInterface() {}
};

//------------------------------------------------------------------------------
class WrenchDebugLoopbackInterface;

class WrenchDebugLoopbackEndpoint : public WrenchDebugCommInterface
{
public:
	WrenchDebugLoopbackEndpoint(SimpleLL<WrenchPacket*>& writeQueue,
		SimpleLL<WrenchPacket*>& readQueue)
		: m_writeQueue(writeQueue)
		, m_readQueue(readQueue)
		, m_server(nullptr)
	{
	}

	bool send(WrenchPacket* packet) override
	{
		WrenchPacket* p = (*m_writeQueue.addHead() = WrenchPacket::alloc(*packet));
		if (m_server) m_server->tick();
		return (p != nullptr);
	}

	WrenchPacket* receive(const int timeoutMilliseconds = 0) override
	{
		WrenchPacket* p = nullptr;
		m_readQueue.popTail(&p);
		return p;
	}

	void setServer(WRDebugServerInterface* server) { m_server = server; }

private:
	SimpleLL<WrenchPacket*>& m_writeQueue;
	SimpleLL<WrenchPacket*>& m_readQueue;
	WRDebugServerInterface* m_server;
};

class WrenchDebugLoopbackInterface : public WrenchDebugCommInterface
{
public:
	WrenchDebugLoopbackInterface()
		: m_clientEndpoint(m_c2s, m_s2c)
		, m_serverEndpoint(m_s2c, m_c2s)
	{
	}

	WrenchDebugCommInterface* clientInterface() { return &m_clientEndpoint; }

	WrenchDebugCommInterface* serverInterface() { return &m_serverEndpoint; }

	void setServer(WRDebugServerInterface* server)
	{
		m_clientEndpoint.setServer(server);
	}

private:
	SimpleLL<WrenchPacket*> m_c2s;
	SimpleLL<WrenchPacket*> m_s2c;
	WrenchDebugLoopbackEndpoint m_clientEndpoint;
	WrenchDebugLoopbackEndpoint m_serverEndpoint;
};

//------------------------------------------------------------------------------
class WrenchDebugTcpInterface : public WrenchDebugCommInterface
{
public:

	bool serve( const int port );
	bool accept();
	bool connect( const char* address, const int port );
	
	bool sendEx( const uint8_t* data, const int bytes );
	bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds );
	uint32_t peekEx( const int timeoutMilliseconds );
	
	WrenchDebugTcpInterface();
	
private:
	int m_socket;
};


//------------------------------------------------------------------------------
class WrenchDebugSerialInterface : public WrenchDebugCommInterface
{
public:

	bool open( const char* name );
	
	bool sendEx( const uint8_t* data, const int bytes );
	bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds );
	uint32_t peekEx( const int timeoutMilliseconds );
	
	WrenchDebugSerialInterface();
	
private:
	HANDLE m_interface;
};

#endif

#endif
