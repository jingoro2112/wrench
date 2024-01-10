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

#if defined(WRENCH_WIN32_FILE_IO) \
	|| defined(WRENCH_LINUX_FILE_IO) \
	|| defined(WRENCH_SPIFFS_FILE_IO) \
	|| defined(WRENCH_LITTLEFS_FILE_IO) \
	|| defined(WRENCH_CUSTOM_FILE_IO)

//------------------------------------------------------------------------------
void wr_loadIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::readFile", wr_read_file );
	wr_registerLibraryFunction( w, "io::writeFile", wr_write_file );
	wr_registerLibraryFunction( w, "io::getline", wr_getline );
	
	wr_registerLibraryFunction( w, "time::clock", wr_clock );
	wr_registerLibraryFunction( w, "time::ms", wr_clock );

	wr_registerLibraryFunction( w, "io::open", wr_ioOpen );//( name, flags, mode ); // returning a file handle 'fd' ; 'mode' specifies unix file access permissions.
	wr_registerLibraryFunction( w, "io::close", wr_ioClose );//( fd );
	wr_registerLibraryFunction( w, "io::read", wr_ioRead ); //( fd, data[], max_cnt );
	wr_registerLibraryFunction( w, "io::write", wr_ioWrite );//( fd, data[], cnt ); // don't know whether it's possible to give an array (by reference) as arg so that it's writable - or not?
	wr_registerLibraryFunction( w, "io::seek", wr_ioSeek ); //( fd, offset, whence );

	wr_ioPushConstants( w );
}

#else

void wr_loadIOLib( WRState* w )
{
}

#endif
