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

WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership );

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen )
{
	WRstr listing;
	
	WRState* w = wr_newState();
	
	WRContext* context = wr_createContext( w, bytecode, len, false );
	if ( !context )
	{
		listing = "err: context failed\n";
	}

	listing.format( "%d globals\n", context->globals );
	listing.appendFormat( "%d units\n", context->numLocalFunctions );
	for( int i=0; i<context->numLocalFunctions; ++i )
	{
		listing.appendFormat( "unit %d[0x%08X] offset[0x%04X] arguments[%d]\n",
							  i,
							  context->localFunctions[i].hash,
							  context->localFunctions[i].functionOffset,
							  context->localFunctions[i].arguments );
	}

	listing.release( out, outLen );
}
