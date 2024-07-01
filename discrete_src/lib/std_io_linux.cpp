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

#ifdef WRENCH_LINUX_FILE_IO

#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

//------------------------------------------------------------------------------
void wr_stdout( const char* data, const int size )
{
	if ( write( STDOUT_FILENO, data, size ) > 0 )
	{
		fflush( stdout );
	}
}

//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn == 1 )
	{
		WRValue* arg = stackTop - 1;
		const char* fileName = (const char*)arg->array();

		struct stat sbuf;
		int ret = stat( fileName, &sbuf );

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
				if ( fread( stackTop->va->m_Cdata, sbuf.st_size, 1, infil ) != 1 )
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
	stackTop->init();

	if ( argn == 2 )
	{
		WRValue* arg1 = stackTop - 2;
		unsigned int len;
		const char* data = (char*)((stackTop - 1)->array(&len));
		if ( !data )
		{
			return;
		}

		const char* fileName = (const char*)arg1->array();
		if ( !fileName )
		{
			return;
		}

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
		char* name = (stackTop - 1)->asMallocString();
		unlink( name );
		g_free( name );
	}
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
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
	stackTop->init();
	stackTop->i = -1;

	if ( argn )
	{
		WRValue* args = stackTop - argn;

		const char* fileName = (const char*)args->array();
		if ( fileName )
		{
			int mode = (argn > 1) ? args[1].asInt() : O_RDWR | O_CREAT;
			stackTop->i = open( fileName, mode, 0666 );
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
		close( (stackTop - argn)->i );
	}
}

//------------------------------------------------------------------------------
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	
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
		
	int result = read( args[0].asInt(), stackTop->va->m_Cdata, toRead );
	stackTop->va->m_size = (result > 0) ? result : 0;
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	
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
				stackTop->ui = write( fd, data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
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

			stackTop->ui = write( fd, data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		int fd = args[0].asInt();
		int offset = args[1].asInt();
		int whence = argn > 2 ? args[2].asInt() : SEEK_SET;

		stackTop->ui = lseek( fd, offset, whence );
	}
}

//------------------------------------------------------------------------------
void wr_ioFSync(WRValue* stackTop, const int argn, WRContext* c)
{
	if (argn)
	{
		stackTop->i = fsync( (stackTop - argn)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	WRValue C;

	wr_registerLibraryConstant( w, "io::O_RDONLY", wr_makeInt(&C, O_RDONLY) );
	wr_registerLibraryConstant( w, "io::O_RDWR", wr_makeInt(&C, O_RDWR) );
	wr_registerLibraryConstant( w, "io::O_APPEND", wr_makeInt(&C, O_APPEND) );
	wr_registerLibraryConstant( w, "io::O_CREAT", wr_makeInt(&C, O_CREAT) );
	wr_registerLibraryConstant( w, "io::O_TRUNC", wr_makeInt(&C, O_TRUNC) );
	wr_registerLibraryConstant( w, "io::O_EXCL", wr_makeInt(&C, O_EXCL) );

	wr_registerLibraryConstant( w, "io::SEEK_SET", wr_makeInt(&C, SEEK_SET) );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", wr_makeInt(&C, SEEK_CUR) );
	wr_registerLibraryConstant( w, "io::SEEK_END", wr_makeInt(&C, SEEK_END) );
}

#endif
