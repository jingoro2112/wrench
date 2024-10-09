#ifndef _STD_TCP_H
#define _STD_TCP_H
/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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

#if (defined(WRENCH_WIN32_TCP) || defined(WRENCH_LINUX_TCP))

// Win32 IO Includes
#ifdef WRENCH_WIN32_TCP
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Wsock32.lib" )

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <time.h>
#include <io.h>
#endif

// Linux IO includes
#ifdef WRENCH_LINUX_TCP
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#endif

struct WRValue;
struct WRContext;

int wr_bindTCPEx( const int port );
int wr_acceptTCPEx( const int socket, sockaddr* info );
int wr_connectTCPEx( const char* address, const int port );

int wr_receiveTCPEx( const int socket, char* data, const int toRead, const int timeoutMilliseconds =0 );
int wr_peekTCPEx( const int socket, const int timeoutMilliseconds =0 );
int wr_sendTCPEx( const int socket, const char* data, const int toSend );
void wr_closeTCPEx( const int socket );


#endif

#endif
