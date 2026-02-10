/*******************************************************************************
Copyright (c) 2026 Curt Hartung -- curt.hartung@gmail.com

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

//------------------------------------------------------------------------------
HANDLE wr_serialOpen( const char* name )
{
	WRstr portName = name ? name : "";
	if ( portName.size() >= 4
		 && !strncmp( portName.c_str(), "COM", 3 )
		 && *portName.c_str(3) >= '0' && *portName.c_str(3) <= '9'
		 && strncmp( portName.c_str(), "\\\\.\\", 4 ) )
	{
		portName = "\\\\.\\";
		portName += name;
	}

	HANDLE h = CreateFileA( portName.c_str(),         // Specify port device "COMx"
							GENERIC_READ | GENERIC_WRITE, // Specify mode that opens device.
							0,                            // The device isn't shared.
							NULL,                         // Default security.
							OPEN_EXISTING,                // Open existing device.
							0,
							NULL );

	if ( h == WR_BAD_SOCKET )
	{
		return WR_BAD_SOCKET;
	}

	SetupComm( h, 4096, 4096 );
	PurgeComm( h, PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR );

	DCB dcb;
	memset( &dcb, 0, sizeof(dcb) );
	dcb.DCBlength = sizeof(dcb);
	if ( !GetCommState( h, &dcb ) )
	{
		CloseHandle( h );
		return WR_BAD_SOCKET;
	}

	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;

	if ( !SetCommState( h, &dcb ) )
	{
		CloseHandle( h );
		return WR_BAD_SOCKET;
	}

	COMMTIMEOUTS timeouts;
	memset( &timeouts, 0, sizeof(timeouts) );
	timeouts.ReadIntervalTimeout = 1;
	timeouts.ReadTotalTimeoutConstant = 5;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	timeouts.WriteTotalTimeoutConstant = 5;
	timeouts.WriteTotalTimeoutMultiplier = 1;
	SetCommTimeouts( h, &timeouts );

	return h;
}

//------------------------------------------------------------------------------
int wr_serialSend( HANDLE comm, const char* data, const int size )
{
	DWORD bytes = 0;
	if ( WriteFile(comm, data, size, &bytes, NULL) == FALSE )
	{
		return -1;
	}

	return (int)bytes;
}

//------------------------------------------------------------------------------
int wr_serialReceive( HANDLE comm, char* data, const int max )
{
	DWORD bytes = 0;
	if ( ReadFile(comm, data, max, &bytes, NULL) == FALSE )
	{
		return -1;
	} 
	
	return (int)bytes;
}

//------------------------------------------------------------------------------
int wr_serialPeek( HANDLE comm )
{
	COMSTAT st;
	DWORD err = 0;
	if ( !ClearCommError( comm, &err, &st ) )
	{
		return -1;
	}
	return (int)st.cbInQue;
}

//------------------------------------------------------------------------------
void wr_serialClose( HANDLE comm )
{
	CloseHandle( comm );
}
