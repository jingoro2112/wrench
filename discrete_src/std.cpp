/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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

// standard functions that sort of come up a lot

int32_t wr_Seed;

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len )
{
	// in-place implementation of murmer
	uint32_t hash = 0x811C9DC5;
	const unsigned char* data = (const unsigned char *)dat;

	for( int i=0; i<len; ++i )
	{
		hash ^= (uint32_t)data[i];
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hashStr( const char* dat )
{
	uint32_t hash = 0x811C9DC5;
	const char* data = dat;
	while ( *data )
	{
		hash ^= (uint32_t)(*data++);
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
void wr_std_rand( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->p2 = INIT_AS_INT;

		int32_t	k = wr_Seed / 127773;
		wr_Seed = 16807 * ( wr_Seed - k * 127773 ) - 2836 * k;
		stackTop->i = (uint32_t)wr_Seed % (uint32_t)(stackTop - 1)->asInt();
	}
}

//------------------------------------------------------------------------------
void wr_std_srand( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		wr_Seed = (stackTop - 1)->asInt();
	}
}

//------------------------------------------------------------------------------
void wr_loadStdLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::rand", wr_std_rand );
	wr_registerLibraryFunction( w, "std::srand", wr_std_srand );
}

//------------------------------------------------------------------------------
void wr_loadAllLibs( WRState* w )
{
	wr_loadMathLib( w );
	wr_loadStdLib( w );
	wr_loadFileLib( w );
	wr_loadStringLib( w );
}

