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

#if defined(WRENCH_WIN32_TCP) \
	|| defined(WRENCH_LINUX_TCP)

//------------------------------------------------------------------------------
const unsigned int c_legalMasks[33] = 
{
	0x00000000, // 0
	0x80000000,
	0xC0000000,
	0xE0000000,
	0xF0000000,
	0xF8000000,
	0xFC000000,
	0xFE000000,
	0xFF000000, // /8
	0xFF800000, // /9
	0xFFC00000,
	0xFFE00000,
	0xFFF00000,
	0xFFF80000,
	0xFFFC0000,
	0xFFFE0000,
	0xFFFF0000, // /16
	0xFFFF8000, // /17
	0xFFFFC000,
	0xFFFFE000,
	0xFFFFF000,
	0xFFFFF800,
	0xFFFFFC00,
	0xFFFFFE00,   
	0xFFFFFF00, // 24
	0xFFFFFF80, // 25
	0xFFFFFFC0,
	0xFFFFFFE0,
	0xFFFFFFF0,
	0xFFFFFFF8,
	0xFFFFFFFC,
	0xFFFFFFFE,
	0xFFFFFFFF, // 32
};

// 'isdigit' has crazy amounts of bloat, this is substsantial gain
// (even though we chuck single-eval)
#define COMM_isdigit(c) ( (c)>='0' && (c)<='9' )

//------------------------------------------------------------------------------
// very quickly determine if the input string is an ip address, and if
// so, what it is. NOTE: Intolerant of whitespace
static int isIp( const char *data, unsigned int *mask =0 )
{
	if ( !data )
	{
		return 0;
	}

	int pos = 0;
	unsigned int running = 0;
	int address = 0;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running;
	running = 0;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running << 8;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running << 16;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( running > 255 )
	{
		return 0;
	}

	address |= running << 24;

	if ( mask )
	{
		*mask = 0;
		if ( !data[pos] )
		{
			return address;
		}

		if ( data[pos++] != '/' )
		{
			return 0;
		}

		if ( !data[pos] )
		{
			return address;
		}

		if ( !COMM_isdigit(data[pos]) )
		{
			return 0;
		}

		*mask = data[pos++] - '0';
		if ( !data[pos] )
		{
			*mask = c_legalMasks[*mask];
			*mask = htonl(*mask);
			return address;
		}

		if ( !COMM_isdigit(data[pos]) )
		{
			return 0;
		}

		*mask *= 10;
		*mask += data[pos++] - '0';

		if ( *mask > 32 || data[pos] )
		{
			return 0;
		}

		*mask = htonl(c_legalMasks[*mask]);
	}
	else if ( data[pos] ) // more characters? must be exactly an IP address, no go
	{
		return 0;
	}

	return address;
}

//------------------------------------------------------------------------------
static char* ipFromAddress( const char* hostname, sockaddr_in* location, char* s )
{
	memset( location, 0, sizeof(sockaddr_in) );

	*((int *)&location->sin_addr) = isIp( hostname );
	if ( !*((int *)&location->sin_addr) )
	{
		struct addrinfo *result;

		if ( getaddrinfo(hostname, NULL, NULL, &result) )
		{
			return 0;
		}

		location->sin_addr = ((sockaddr_in*)(result->ai_addr))->sin_addr;

		freeaddrinfo(result);
	}

	return (char*)inet_ntop(AF_INET, &(((struct sockaddr_in *)location)->sin_addr), s, 20 );
}

//------------------------------------------------------------------------------
int wr_bindTCPEx( const int port )
{
	sockaddr_in location;
	memset( &location, 0, sizeof(location) );

	location.sin_family = AF_INET;
	location.sin_addr.s_addr = htonl( INADDR_ANY );
	location.sin_port = htons( (short)port );

	int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( s == -1 )
	{
		return -1;
	}

	if ( (::bind(s, reinterpret_cast<sockaddr *>(&location), sizeof(location)) == -1)
		 || (::listen(s, 5) == -1) )
	{
		wr_closeTCPEx( s );
		return -1;
	}

	return s;
}

//------------------------------------------------------------------------------
void wr_bindTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
		stackTop->i = wr_bindTCPEx( (stackTop - argn)->asInt() );
	}
}

//------------------------------------------------------------------------------
int wr_acceptTCPEx( const int socket, sockaddr* info )
{
	socklen_t length = sizeof( sockaddr );
	memset( info, 0, length );

	return accept( socket, info, &length );
}

//------------------------------------------------------------------------------
void wr_acceptTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn < 1 )
	{
		return;
	}

	sockaddr info;
	stackTop->i = wr_acceptTCPEx( (stackTop - argn)->asInt(), &info );
}

//------------------------------------------------------------------------------
int wr_connectTCPEx( const char* address, const int port )
{
	sockaddr_in location;
	memset( &location, 0, sizeof(location) );

	char ret[21];
	ipFromAddress( address, &location, ret );
	
	location.sin_family = AF_INET;
	location.sin_port = htons( port );

	int sock;
	if ( (sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 )
	{
		return -1;
	}

	if ( ::connect( sock, reinterpret_cast<sockaddr *>(&location), sizeof(location)) == -1 )
	{
		wr_closeTCPEx( sock );
		return -1;
	}

	return sock;
}

//------------------------------------------------------------------------------
void wr_connectTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	char* address = args[0].asMallocString();
	if ( !address || !address[0] )
	{
		return;
	}

	stackTop->i = wr_connectTCPEx( address, args[1].asInt() );
	g_free( address );
}

//------------------------------------------------------------------------------
void wr_closeTCPEx( const int socket )
{
#ifdef WRENCH_WIN32_TCP
	_close( socket );
#endif
#ifdef WRENCH_LINUX_TCP
	close( socket );
#endif
}

//------------------------------------------------------------------------------
void wr_closeTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( !argn )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	wr_closeTCPEx( args[0].asInt() );
}


//------------------------------------------------------------------------------
int wr_sendTCPEx( const int socket, const char* data, const int toSend )
{
	return send( socket, data, toSend, 0 );
}

//------------------------------------------------------------------------------
void wr_sendTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	unsigned int len;
	char* data = args[0].asMallocString( &len );
	if ( data )
	{
		stackTop->i = wr_sendTCPEx( args[0].asInt(), data, len );
	}

	g_free( data );
}

//------------------------------------------------------------------------------
int wr_receiveTCPEx( const int socket, char* data, const int toRead, const int timeoutMilliseconds )
{
#ifdef WRENCH_WIN32_TCP
	DWORD timeout = timeoutMilliseconds;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) );
	return _read( socket, data, toRead );
#endif
#ifdef WRENCH_LINUX_TCP
	struct timeval timeout;
	timeout.tv_sec = timeoutMilliseconds;
	timeout.tv_usec = 0;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	return read( socket, data, toRead );
#endif
}

//------------------------------------------------------------------------------
void wr_receiveTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	int toRead = 8192;
	if ( argn > 1 )
	{
		toRead = args[1].asInt();
	}

	stackTop->va = c->getSVA( toRead, SV_CHAR, false );
	stackTop->p2 = INIT_AS_ARRAY;

	int ret = wr_receiveTCPEx( args[0].asInt(), (char*)stackTop->va->m_Cdata, toRead );

	if ( !ret )
	{
		stackTop->init();
	}
	else
	{
		stackTop->va->m_size = ret;
	}
}

//------------------------------------------------------------------------------
int wr_peekTCPEx( const int socket, const int timeoutMilliseconds )
{
#ifdef WRENCH_WIN32_TCP

	DWORD timeout = timeoutMilliseconds;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) );
	int retval = recv( socket, 0, 65000, MSG_PEEK );
	
	return retval == SOCKET_ERROR ? -1 : retval;

#endif
#ifdef WRENCH_LINUX_TCP

	int retval = 0;

	short poll_events = POLLIN;

	pollfd readfd;
	readfd.fd = socket;
	readfd.events = poll_events;
	readfd.revents = 0;
	retval = ::poll( &readfd, 1, timeoutMilliseconds );

	return retval;

#endif
}

//------------------------------------------------------------------------------
void wr_peekTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 1 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	stackTop->i = wr_peekTCPEx( args[0].asInt(), argn > 1 ? args[1].asInt() : 0 );
}

//------------------------------------------------------------------------------
void wr_loadTCPLib( WRState* w )
{
	wr_registerLibraryFunction( w, "tcp::bind", wr_bindTCP ); // (port) ret: socket
	wr_registerLibraryFunction( w, "tcp::accept", wr_acceptTCP ); // (socket) ret: socket
	wr_registerLibraryFunction( w, "tcp::connect", wr_connectTCP ); // (address, port) ret: socket
	wr_registerLibraryFunction( w, "tcp::close", wr_closeTCP ); // (socket)
	wr_registerLibraryFunction( w, "tcp::send", wr_sendTCP ); // (socket, data) ret: data sent
	wr_registerLibraryFunction( w, "tcp::receive", wr_receiveTCP ); // (socket) ret: data received
	wr_registerLibraryFunction( w, "tcp::peek", wr_peekTCP ); // (socket) ret: data available

#ifdef WRENCH_WIN32_TCP
	WSADATA wsaData;
	WSAStartup( MAKEWORD(2, 2), &wsaData );
#endif
}


#else

//------------------------------------------------------------------------------
void wr_loadTCPLib( WRState* w )
{
}

#endif