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

#if defined(WRENCH_LITTLEFS_FILE_IO) || defined(WRENCH_SPIFFS_FILE_IO)
#include <new>
#include <FS.h>
#ifdef WRENCH_LITTLEFS_FILE_IO
#include <LittleFS.h>
#define FILE_OBJ LittleFS
#endif

#ifdef WRENCH_SPIFFS_FILE_IO
#include <Spiffs.h>
#define FILE_OBJ Spiffs
#endif

//------------------------------------------------------------------------------
enum WRFSOpenModes
{
	LFS_READ   = 0x1,
	LFS_RDWR   = 0x2,
	LFS_APPEND = 0x4,
};

//------------------------------------------------------------------------------
enum WRFSSeek
{
	LFS_SET = 0,
	LFS_CUR,
	LFS_END,
};

//------------------------------------------------------------------------------
struct WRFSFile
{
	File file;
	WRFSFile* next;
};

static WRFSFile* g_OpenFiles =0;

//------------------------------------------------------------------------------
void wr_destroyFile( WRFSFile* file )
{
	file->~WRFSFile();
	g_free( file );
}

//------------------------------------------------------------------------------
void wr_stdout( const char* data, const int size )
{
}

//------------------------------------------------------------------------------
WRFSFile* wr_safeGetFile( const void* p )
{
	for( WRFSFile* safe = g_OpenFiles; safe; safe = safe->next )
	{
		if( safe == p )
		{
			return safe;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn != 1 )
	{
		return;
	}
	
	WRValue* arg = stackTop - 1;
	WRValue::MallocStrScoped fileName( *arg );

	File file = FILE_OBJ.open( fileName );
	if ( file && !file.isDirectory() )
	{
		stackTop->va = c->getSVA( file.size(), SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !stackTop->va )
		{
			return;
		}
#endif
		stackTop->p2 = INIT_AS_ARRAY;

		if ( file.readBytes( stackTop->va->m_SCdata, file.size() ) != file.size() )
		{
			stackTop->init();
		}

		file.close();
	}
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn != 2 )
	{
		return;
	}

	WRValue::MallocStrScoped fileName( *(stackTop - 2) );
	
	unsigned int len;
	const char* data = (char*)((stackTop - 1)->array(&len));
	if ( !data )
	{
		return;
	}

	File file = FILE_OBJ.open( fileName, FILE_WRITE );
	if ( !file )
	{
		return;
	}

	stackTop->i = file.write( (uint8_t *)data, len );

	file.close();
}

//------------------------------------------------------------------------------
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		WRValue::MallocStrScoped fileName( *(stackTop - 1) );
		if ( fileName )
		{
			FILE_OBJ.remove( fileName );
		}
	} 
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( !argn )
	{
		return;
	}
	
	WRValue* args = stackTop - argn;

	int mode = LFS_RDWR;
	if ( argn > 1 )
	{
		mode = args[1].asInt();
	}

	WRValue::MallocStrScoped fileName( *args );
	if ( fileName )
	{
		void* memory = g_malloc( sizeof(WRFSFile) );

	#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if(!memory) { g_mallocFailed = true; return; }
	#endif

		WRFSFile* entry = new(memory) WRFSFile;
		
		if ( mode & LFS_APPEND )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_APPEND );
		}
		else if ( mode & LFS_READ )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_READ );
		}
		else if ( mode & LFS_RDWR )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_WRITE );
		}
		else
		{
			wr_destroyFile( entry );
			return;
		}

		if ( !entry->file )
		{
			wr_destroyFile( entry );
			return;
		}

		entry->next = g_OpenFiles;
		g_OpenFiles = entry;
		stackTop->r1 = entry;
	}
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* args = stackTop - argn;
	if ( !argn || !args->r1 )
	{
		return;
	}

	WRFSFile* prev = 0;
	WRFSFile* cur = g_OpenFiles;
	while( cur )
	{
		if ( cur == args->r1 )
		{
			cur->file.close();
			
			if ( !prev )
			{
				g_OpenFiles = g_OpenFiles->next;
			}
			else
			{
				prev->next = cur->next;
			}
			
			wr_destroyFile( cur );
			break;
		}

		prev = cur;
		cur = cur->next;
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

	WRFSFile* fd = wr_safeGetFile( args[0].r1 );
	if ( !fd )
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

	int result = fd->file.readBytes( stackTop->va->m_SCdata, toRead );
	
	stackTop->va->m_size = (result > 0) ? result : 0;
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	WRFSFile* fd = wr_safeGetFile( args[0].r1 );
	if ( !fd )
	{
		return;
	}

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
			stackTop->ui = fd->file.write( data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
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

		stackTop->ui = fd->file.write( (const uint8_t *)data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
	}
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		WRFSFile* fd = wr_safeGetFile( args[0].r1 );
		if ( !fd )
		{
			return;
		}
		
		int offset = args[1].asInt();
		int whence = argn > 2 ? args[2].asInt() : LFS_SET;

		if ( whence == LFS_END )
		{
			offset += (int)fd->file.size();
		}
		else if ( whence == LFS_CUR )
		{
			offset += (int)fd->file.position();
		}
		
		fd->file.seek( (unsigned long)offset );
	}
}

//------------------------------------------------------------------------------
void wr_ioFSync(WRValue* stackTop, const int argn, WRContext* c)
{
	if ( argn == 1 )
	{
		WRFSFile* fd = wr_safeGetFile( (stackTop - 1)->r1 );
		if ( fd )
		{
			fd->file.flush();
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioCleanupFunction( WRState* w, void* param )
{
	while( g_OpenFiles )
	{
		WRFSFile* next = g_OpenFiles->next;
		g_OpenFiles->file.close();
		wr_destroyFile( g_OpenFiles );
		g_OpenFiles = next;
	}
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	wr_registerLibraryConstant( w, "io::O_RDONLY", (int32_t)LFS_READ );
	wr_registerLibraryConstant( w, "io::O_RDWR", (int32_t)LFS_RDWR );
	wr_registerLibraryConstant( w, "io::O_APPEND", (int32_t)LFS_APPEND );
	wr_registerLibraryConstant( w, "io::O_CREAT", (int32_t)LFS_RDWR );

	wr_registerLibraryConstant( w, "io::SEEK_SET", (int32_t)LFS_SET );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", (int32_t)LFS_CUR );
	wr_registerLibraryConstant( w, "io::SEEK_END", (int32_t)LFS_END );

	FILE_OBJ.begin();

	wr_addLibraryCleanupFunction( w, wr_ioCleanupFunction, 0 );

}

//------------------------------------------------------------------------------
void wr_loadIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::readFile", wr_read_file );
	wr_registerLibraryFunction( w, "io::writeFile", wr_write_file );
	wr_registerLibraryFunction( w, "io::deleteFile", wr_delete_file );
	wr_registerLibraryFunction( w, "io::getline", wr_getline );
	wr_registerLibraryFunction( w, "io::open", wr_ioOpen );
	wr_registerLibraryFunction( w, "io::close", wr_ioClose );
	wr_registerLibraryFunction( w, "io::read", wr_ioRead );
	wr_registerLibraryFunction( w, "io::write", wr_ioWrite );
	wr_registerLibraryFunction( w, "io::seek", wr_ioSeek );
	wr_registerLibraryFunction( w, "io::fsync", wr_ioFSync );

	wr_ioPushConstants( w );
}

#undef FILE_OBJ

#endif
