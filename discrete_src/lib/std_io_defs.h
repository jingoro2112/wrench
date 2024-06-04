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

#ifndef _STD_IO_DEFS_H
#define _STD_IO_DEFS_H
/*------------------------------------------------------------------------------*/

struct WRValue;
struct WRContext;
struct WRState;

void wr_read_file( WRValue* stackTop, const int argn, WRContext* c );
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c );
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c );

void wr_getline( WRValue* stackTop, const int argn, WRContext* c );

void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioFSync( WRValue* stackTop, const int argn, WRContext* c );

void wr_ioPushConstants( WRState* w );

#endif
