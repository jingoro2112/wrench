#ifndef _STD_SERIAL_H
#define _STD_SERIAL_H
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

#if defined(WRENCH_WIN32_SERIAL) || defined(WRENCH_LINUX_SERIAL) || defined(WRENCH_ARDUINO_SERIAL)

#ifdef WRENCH_WIN32_SERIAL
#include <windows.h>
#define WR_BAD_SOCKET INVALID_HANDLE_VALUE
#endif

#ifdef WRENCH_LINUX_SERIAL
#define HANDLE int
#define WR_BAD_SOCKET -1
#endif

#ifdef WRENCH_ARDUINO_SERIAL
#include <Arduino.h>
#define HANDLE HardwareSerial&
#endif

HANDLE wr_serialOpen( const char* name );
void wr_serialClose( HANDLE name );
int wr_serialSend( HANDLE comm, const char* data, const int size );
int wr_serialReceive( HANDLE comm, char* data, const int max );
int wr_serialPeek( HANDLE comm );

#endif

#endif
