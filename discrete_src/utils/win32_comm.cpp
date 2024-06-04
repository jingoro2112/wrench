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

#ifdef WRENCH_WIN32_SERIAL

#include "wrench.h"
#include <windows.h>
#include <atlstr.h>
static HANDLE s_comm = INVALID_HANDLE_VALUE;

//------------------------------------------------------------------------------
bool wr_serialOpen( const char* name )
{
	CloseHandle( s_comm );
	s_comm = INVALID_HANDLE_VALUE;

	s_comm = CreateFile( CString(name),					 // Specify port device "COMx"
						 GENERIC_READ | GENERIC_WRITE,   // Specify mode that open device.
						 0,                              // the devide isn't shared.
						 NULL,                           // the object gets a default security.
						 OPEN_EXISTING,                  // Specify which action to take on file. 
						 0,                              // default.
						 NULL );
	
	return s_comm != INVALID_HANDLE_VALUE;
}

//------------------------------------------------------------------------------
bool wr_serialSend( const char* data, const int size )
{
	DWORD bytes = 0;
	if ( (WriteFile(s_comm, data, size, &bytes, NULL) == FALSE) || (bytes != size) )
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
bool wr_serialReceive( char* data, const int expected )
{
	if ( s_comm == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	DWORD bytes = 0;
	if (!ReadFile(s_comm, data, expected, &bytes, NULL))
	{
		return false;
	} 
	
	return bytes == expected;
}

//------------------------------------------------------------------------------
int wr_serialBytesAvailable()
{
	DWORD avail = 0;
	if ( s_comm != INVALID_HANDLE_VALUE )
	{
		PeekNamedPipe( s_comm, 0, 0, 0, &avail, 0 );
	}
	return (int)avail;
}

#endif
