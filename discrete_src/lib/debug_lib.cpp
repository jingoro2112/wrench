
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

int wr_sprintfEx( char* outbuf,
				  const unsigned int outsize,
				  const char* fmt,
				  const unsigned int fmtsize,
				  WRValue* args,
				  const int argn );

//------------------------------------------------------------------------------
void wr_debugPrintEx( WRValue* stackTop, const int argn, WRContext* c, const char* append )
{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	if( argn >= 1 && c->debugInterface )
	{
		WRValue* args = stackTop - argn;
		unsigned int inlen;
		char* inbuf = args[0].asMallocString( &inlen );
		if ( !inbuf )
		{
			return;
		}

		char outbuf[200];
		int size = wr_sprintfEx( outbuf, 199, inbuf, inlen, args + 1, argn - 1);
			
		WrenchPacket* P = WrenchPacket::alloc( WRD_DebugOut, size, (uint8_t*)outbuf );
			
		g_free( inbuf );

		c->debugInterface->I->m_comm->send( P );

		g_free( P );
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
