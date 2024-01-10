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

#ifdef WRENCH_LITTLEFS_FILE_IO


//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_clock( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_milliseconds(WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	WRValue C;

	wr_registerLibraryConstant( w, "io::O_RDONLY", wr_makeInt(&C, O_RDONLY) ); //( fd, offset, whence );
	wr_registerLibraryConstant( w, "io::O_RDWR", wr_makeInt(&C, O_RDWR) ); //( fd, offset, whence );
	wr_registerLibraryConstant( w, "io::O_APPEND", wr_makeInt(&C, O_APPEND) ); //( fd, offset, whence );
	wr_registerLibraryConstant( w, "io::O_CREAT", wr_makeInt(&C, O_CREAT) ); //( fd, offset, whence );

	wr_registerLibraryConstant( w, "io::SEEK_SET", wr_makeInt(&C, SEEK_SET) ); //( fd, offset, whence );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", wr_makeInt(&C, SEEK_CUR) ); //( fd, offset, whence );
	wr_registerLibraryConstant( w, "io::SEEK_END", wr_makeInt(&C, SEEK_END) ); //( fd, offset, whence );
}

#endif
