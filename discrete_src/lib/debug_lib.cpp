
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

#include "wrench.h"

int wr_sprintfEx( char* outbuf, const char* fmt, WRValue* args, const int argn );

//------------------------------------------------------------------------------
void wr_debugPrintEx( WRValue* stackTop, const int argn, WRContext* c, const char* append )
{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	if( argn >= 1 && c->debugInterface )
	{
		WRValue* args = stackTop - argn;

		char inbuf[512];

		WrenchPacket* P = WrenchPacket::alloc( WRD_DebugOut, 520 );

		int size = wr_sprintfEx( (char*)P->payload(), args[0].asString(inbuf), args + 1, argn - 1);

		for( int a=0; append && append[a]; ++a )
		{
			*(P->payload() + size) = append[a];
			++size;
		}

		*(P->payload() + size++) = 0;
		P->setPayloadSize( size );

		if ( c->debugInterface->I->m_sendFunction )
		{
			c->debugInterface->I->m_sendFunction( (char *)P, P->xlate() );
		}
		else
		{
			printf("%s", P->payload() );
		}
	}
#endif
}

//------------------------------------------------------------------------------
void wr_debugPrint( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_debugPrintEx( stackTop, argn, c, 0 );
}

//------------------------------------------------------------------------------
void wr_debugPrintln( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_debugPrintEx( stackTop, argn, c, "\r\n" );
}

//------------------------------------------------------------------------------
void wr_loadDebugLib( WRState* w )
{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	wr_registerLibraryFunction( w, "debug::print", wr_debugPrint );
	wr_registerLibraryFunction( w, "debug::println", wr_debugPrintln );
#endif
}
