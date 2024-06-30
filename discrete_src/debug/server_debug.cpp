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
WRDebugServerInterface::WRDebugServerInterface( WRState* w )
{
	memset( this, 0, sizeof(*this) );
	I = (WRDebugServerInterfacePrivate *)g_malloc( sizeof(WRDebugServerInterfacePrivate) );
	new (I) WRDebugServerInterfacePrivate( this );

	I->m_w = w;
}

//------------------------------------------------------------------------------
WRDebugServerInterface::WRDebugServerInterface( WRState* w,
												bool (*receiveFunction)( char* data, const int length ),
												bool (*sendFunction)( const char* data, const int length ),
												int (*dataAvailableFunction)() )
{
	memset( this, 0, sizeof(*this) );
	I = (WRDebugServerInterfacePrivate *)g_malloc( sizeof(WRDebugServerInterfacePrivate) );
	new (I) WRDebugServerInterfacePrivate( this );

	I->m_w = w;

	I->m_receiveFunction = receiveFunction;
	I->m_sendFunction = sendFunction;
	I->m_dataAvailableFunction = dataAvailableFunction;
}

//------------------------------------------------------------------------------
WRDebugServerInterface::~WRDebugServerInterface()
{
	I->~WRDebugServerInterfacePrivate();
	g_free( I );
}

#endif
