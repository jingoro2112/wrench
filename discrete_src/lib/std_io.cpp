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
	|| defined(WRENCH_LITTLEFS_FILE_IO)

#if defined(WRENCH_WIN32_FILE_IO) || defined(WRENCH_LINUX_FILE_IO)

//------------------------------------------------------------------------------
void wr_stdout( const char* data, const int size )
{
#if defined(WRENCH_WIN32_FILE_IO)
	if ( _write( _fileno(stdout), data, size ) > 0 )
#endif
#if defined(WRENCH_LINUX_FILE_IO)
	if ( write( STDOUT_FILENO, data, size ) > 0 )
#endif
	{
		fflush( stdout );
	}
}

//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		WRValue* arg = stackTop - 1;
		WRValue::MallocStrScoped fileName( *arg );

#if defined(WRENCH_WIN32_FILE_IO)
		struct _stat sbuf;
		int ret = _stat( fileName, &sbuf );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		struct stat sbuf;
		int ret = stat( fileName, &sbuf );
#endif
		
		if ( ret == 0 )
		{
			FILE *infil = fopen( fileName, "rb" );
			if ( infil )
			{
				stackTop->va = c->getSVA( (int)sbuf.st_size, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
				if ( !stackTop->va )
				{
					return;
				}
#endif
				stackTop->p2 = INIT_AS_ARRAY;
				if ( fread(stackTop->va->m_Cdata, sbuf.st_size, 1, infil) != 1 )
				{
					stackTop->init();
					return;
				}
			}

			fclose( infil );
		}
	}
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 2 )
	{
		WRValue* arg1 = stackTop - 2;
		unsigned int len;
		const char* data = (char*)((stackTop - 1)->array(&len));
		if ( !data )
		{
			return;
		}

		WRValue::MallocStrScoped fileName( *arg1 );

		FILE *outfil = fopen( fileName, "wb" );
		if ( !outfil )
		{
			return;
		}

		stackTop->i = (int)fwrite( data, len, 1, outfil );
		fclose( outfil );
	}
}

//------------------------------------------------------------------------------
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		const char* filename = WRValue::MallocStrScoped(*(stackTop - 1));
		if ( filename )
		{
#if defined(WRENCH_WIN32_FILE_IO)
			_unlink( filename );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			unlink( filename );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
	char buf[256];
	int pos = 0;
	for (;;)
	{
		int in = fgetc( stdin );

		if ( in == EOF || in == '\n' || in == '\r' || pos >= 256 )
		{ 
			stackTop->va = c->getSVA( pos, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !stackTop->va )
			{
				return;
			}
#endif
			stackTop->p2 = INIT_AS_ARRAY;
			memcpy( stackTop->va->m_Cdata, buf, pos );
			break;
		}

		buf[pos++] = in;
	}
}

//------------------------------------------------------------------------------
void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->i = -1;

	if ( argn )
	{
		WRValue* args = stackTop - argn;

		WRValue::MallocStrScoped fileName( *args );

		if ( fileName )
		{
			int mode = (argn > 1) ? args[1].asInt() : O_RDWR | O_CREAT;
			
#if defined(WRENCH_WIN32_FILE_IO)
			stackTop->i = _open( fileName, mode | O_BINARY, _S_IREAD | _S_IWRITE /*0600*/ );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			stackTop->i = open( fileName, mode, 0666 );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->i = _close( (stackTop - argn)->asInt() );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->i = close( (stackTop - argn)->asInt() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn != 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	int toRead = args[1].asInt();
	if ( toRead <= 0 )
	{
		return;
	}

	stackTop->va = c->getSVA( toRead, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;


#if defined(WRENCH_WIN32_FILE_IO)
	int result = _read( args[0].asInt(), stackTop->va->m_Cdata, toRead );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
	int result = read( args[0].asInt(), stackTop->va->m_Cdata, toRead );
#endif

	stackTop->va->m_size = (result > 0) ? result : 0;
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		int fd = args[0].asInt();
		WRValue& data = args[1].deref();

		if ( IS_ARRAY(data.xtype) )
		{
			uint32_t size = data.va->m_size;
			if ( argn > 2 )
			{
				size = args[2].asInt();
			}

			if ( data.va->m_type == SV_CHAR )
			{
#if defined(WRENCH_WIN32_FILE_IO)
				stackTop->i = _write( fd, data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
				stackTop->i = write( fd, data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
#endif
			}
			else if ( data.va->m_type == SV_VALUE )
			{
				// .. does this even make sense?
			}
		}
		else if ( IS_RAW_ARRAY(data.xtype) )
		{
			uint32_t size = EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2);
			if ( argn > 2 )
			{
				size = args[2].asInt();
			}

#if defined(WRENCH_WIN32_FILE_IO)
			stackTop->i = _write( fd, data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			stackTop->i = write( fd, data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		int fd = args[0].asInt();
		int offset = args[1].asInt();
		int whence = argn > 2 ? args[2].asInt() : SEEK_SET;

#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->ui = _lseek( fd, offset, whence );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->ui = lseek( fd, offset, whence );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioFSync(WRValue* stackTop, const int argn, WRContext* c)
{
	if (argn)
	{
#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->i = _commit( (stackTop - argn)->asInt() );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->i = fsync( (stackTop - argn)->asInt() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	wr_registerLibraryConstant( w, "io::O_RDONLY", (int32_t)O_RDONLY );
	wr_registerLibraryConstant( w, "io::O_RDWR", (int32_t)O_RDWR );
	wr_registerLibraryConstant( w, "io::O_APPEND", (int32_t)O_APPEND );
	wr_registerLibraryConstant( w, "io::O_CREAT", (int32_t)O_CREAT );
	wr_registerLibraryConstant( w, "io::O_TRUNC", (int32_t)O_TRUNC );
	wr_registerLibraryConstant( w, "io::O_EXCL", (int32_t)O_EXCL );

	wr_registerLibraryConstant( w, "io::SEEK_SET", (int32_t)SEEK_SET );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", (int32_t)SEEK_CUR );
	wr_registerLibraryConstant( w, "io::SEEK_END", (int32_t)SEEK_END );
}

#endif

//------------------------------------------------------------------------------
void wr_loadIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::readFile", wr_read_file ); // (name) returns array which is file
	wr_registerLibraryFunction( w, "io::writeFile", wr_write_file ); // (name, array) writes array
	wr_registerLibraryFunction( w, "io::deleteFile", wr_delete_file ); // (name)
	wr_registerLibraryFunction( w, "io::getline", wr_getline ); // get a line of text from input
	wr_registerLibraryFunction( w, "io::open", wr_ioOpen ); //( name, flags, mode ); // returning a file handle 'fd' ; 'mode' specifies unix file access permissions.
	wr_registerLibraryFunction( w, "io::close", wr_ioClose ); //( fd );
	wr_registerLibraryFunction( w, "io::read", wr_ioRead );  //( fd, data, max_count );
	wr_registerLibraryFunction( w, "io::write", wr_ioWrite ); //( fd, data, count );
	wr_registerLibraryFunction( w, "io::seek", wr_ioSeek );  //( fd, offset, whence );
	wr_registerLibraryFunction( w, "io::fsync", wr_ioFSync ); //( fd );

	wr_ioPushConstants( w );
}

#else

void wr_loadIOLib( WRState* w )
{
}

#endif
