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

#ifdef WRENCH_LINUX_SERIAL

#include "wrench.h"

//------------------------------------------------------------------------------
HANDLE wr_serialOpen( const char* name )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialSend( HANDLE comm, const char* data, const int size )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialReceive( HANDLE comm, char* data, const int expected )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialPeek( HANDLE comm )
{
	return 0;
}

//------------------------------------------------------------------------------
void wr_serialClose( HANDLE comm )
{
	close( comm );
}

#endif
