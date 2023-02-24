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

// standard functions that sort of come up a lot

int32_t wr_Seed;

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len )
{
	// fnv-1
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
int wr_itoa( int i, char* string, size_t len )
{
	char buf[12];
	size_t pos = 0;
	int val;
	if ( i < 0 )
	{
		string[pos++] = '-';
		val = -i;
	}
	else
	{
		val = i;
	}

	int digit = 0;
	do
	{
		buf[digit++] = (val % 10) + '0';
	} while( val /= 10 );

	--digit;

	for( ; digit>=0 && pos < len; --digit )
	{
		string[pos++] = buf[digit];
	}
	string[pos] = 0;
	return pos;
}

//------------------------------------------------------------------------------
int wr_ftoa( float f, char* string, size_t len )
{
	size_t pos = 0;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		string[pos++] = '-';
	}

	if ( pos > len )
	{
		string[0] = 0;
		return 0;
	}

	f += 5.f / (float)10e6; // round value to 5 places

	int i = (int)f;

	if ( i )
	{
		f -= i;
		pos += wr_itoa( i, string + pos, len - pos );
	}
	else
	{
		string[pos++] = '0';
	}

	string[pos++] = '.';

	for( int p=0; pos < len && p<5; ++p ) // convert non-integer to 5 digits of precision
	{
		f *= 10.0;
		char c = (char)f;
		string[pos++] = '0' + c;
		f -= c;
	}

	for( --pos; pos > 0 && string[pos] == '0' && string[pos] != '.' ; --pos ); // knock off trailing zeros and decimal if appropriate

	if ( string[pos] != '.' )
	{
		++pos;
	}

	string[pos] = 0;
	return pos;
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
	wr_loadMessageLib( w );
}

