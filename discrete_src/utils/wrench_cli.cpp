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

#define STR_FILE_OPERATIONS
#include <wrench.h>

#include "utils/simple_args.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int runTests( int number =0 );
void testGlobalValues( WRState* w );
void setup();
void eventMain();
void testImport();
void testHalt();
void testTimeSlices();
void testScheduler();
void testStackOverflow();
//void testYield2();

//------------------------------------------------------------------------------
void blobToHeader( WRstr const& blob, WRstr const& variableName, WRstr& header )
{
	header.format( "#ifndef _%s_H\n"
				   "#define _%s_H\n"
				   "\n"
				   "/******* wrench bytcode automatically generated header *******/\n\n"
				   "const int %s_bytecodeSize=%d;\n"
				   "const unsigned char %s_bytecode[]=\n"
				   "{\n",
				   variableName.c_str(),
				   variableName.c_str(),
				   variableName.c_str(),
				   (int)blob.size(),
				   variableName.c_str() );

	unsigned int p=0;
	while( p < blob.size() )
	{
		header += "\t";
		for( int i=0; i<16 && p<blob.size() ; i++ )
		{
			header.appendFormat( "0x%02X, ", (unsigned char)blob[p++] );
		}

		header.appendFormat( "// %d\n", p );
	}

	header += "};\n\n"
			  "#endif\n";
}

//------------------------------------------------------------------------------
void blobToAssemblyInc( WRstr const& blob, WRstr const& variableName, WRstr& header )
{
	header.format( "\n;******* wrench bytcode automatically generated include file *******\n\n"
				   ".export %s_bytecode\n"
				   "%s_bytecode: .export %s_bytecode\n",
				   variableName.c_str(),
				   variableName.c_str(),
				   variableName.c_str() );

	unsigned int p=0;
	while( p < blob.size() )
	{
		header += "\t.byte ";
		for( int i=0; i<16 && p<blob.size() ; i++ )
		{
			header.appendFormat( "$%02X, ", (unsigned char)blob[p++] );
		}

		header.shave( 2 );
		header.appendFormat( " ; %d\n", p );
	}

	header += "\n\n";
}

//------------------------------------------------------------------------------
// smush all these files together to make wrench.cpp, in this order!
const char* sourceOrder[]=
{
	"/utils/utils.h",
	"/utils/serializer.h",
	"/utils/simple_ll.h",
	"/utils/simple_args.h",
	"/vm/gc_object.h",
	"/vm/vm.h",
	"/utils/opcode.h",
	"/cc/str.h",
	"/cc/opcode_stream.h",
	"/cc/cc.h",
	"/debug/debug.h",
	"/utils/debugger.h",
	"/lib/std_io_defs.h",
	"/cc/cc.cpp",
	"/cc/link.cpp",
	"/cc/expression.cpp",
	"/cc/optimizer.cpp",
	"/cc/token.cpp",
	"/vm/gc.cpp",
	"/vm/vm.cpp",
	"/utils/utils.cpp",
	"/utils/serializer.cpp",
	"/debug/client_debug.cpp",
	"/debug/server_debug.cpp",
	"/debug/disassemble.cpp",
	"/debug/packet.cpp",
	"/debug/server_interface_private.cpp",
	"/vm/scheduler.cpp",
	"/vm/operations.cpp",
	"/vm/index.cpp",
	"/lib/std.cpp",
	"/lib/std_io.cpp",
	"/lib/std_io_linux.cpp",
	"/lib/std_io_win32.cpp",
	"/lib/std_io_little_spiffs.cpp",
	"/lib/std_string.cpp",
	"/lib/std_math.cpp",
	"/lib/std_msg.cpp",
	"/lib/std_sys.cpp",
	"/lib/std_serialize.cpp",
	"/lib/debug_lib.cpp",
	"/lib/esp32_lib.cpp",
	"/lib/arduino_lib.cpp",
	"/utils/arduino_comm.cpp",
	"/utils/win32_comm.cpp",
	"/utils/linux_comm.cpp",
	""
};

//------------------------------------------------------------------------------
int usage()
{
	printf( "version: %d.%d\n"
			"usage: wrench <command> [options]\n"
			"where command is:\n"
			"\n"
			"v                              show version + help\n"
			"\n"
			"c [infile] [out as bytecode]   compile infile and output as raw bytecode\n"
			"\n"
			"ch [infile] [out file] [name]  compile infile and output as a header file\n"
			"                               of name \"out file\"\n"
			"                               suitable for '#include'\n"
			"                               the exported constants will be:\n"
			"                               const char* [name]_bytecode;\n"
			"                               const int [name]_bytecodeSize;\n"
			
			"p [infile]                     print disassembly of file\n"
/*
			"ca [infile] [out file] [name]  compile infile and output as an inc file\n"
			"                               for assembly of name \"out file\"\n"
			"                               the exported constants will be:\n"
			"                               [name]_bytecode and\n"
			"                               [name]_bytecodeSize = $xxxx\n"
*/
			"\n"
#ifdef WRENCH_INCLUDE_DEBUG_CODE
			"d"
			"\n"
#endif
			"t                              run internal tests\n"
			"\n"
			"release [fromdir] [todir]      collect the discrete source files together\n"
			"                               and output [todir]/wrench.[cpp/h]\n"
			"\n"
			"rb [binary file to execute]    execute the file as if its bytecode\n"
			"r  [source file to execute]    compile and execute execute the file\n"
			"                               as if its source code\n"
			"\n"
			"options:\n"
			"-debug                         add debug code\n"
			"-debugsource                   add debug code and include a copy of the source code\n" 
			"-noglobals                     no global hashes, this saves 4 bytes per global\n"
			"                               and prevents wr_getGlobalRef() from working\n"
			"-nostrict                     do NOT require 'var' for variables\n"
		
			"\n", (int)WRENCH_VERSION_MAJOR, (int)WRENCH_VERSION_MINOR );

	return -1;
}

//------------------------------------------------------------------------------
static void println( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char outbuf[64000];
		argv[i].asString( outbuf, 64000 );
		printf( "%s", outbuf );
	}
	printf( "\n" );

	retVal.i = 20; // for argument-return testing
}

//------------------------------------------------------------------------------
static void print( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char outbuf[64000];
		argv[i].asString( outbuf, 64000 );
		printf( "%s", outbuf );
	}
}

struct Test
{
	int i;
};

WRState* gw = 0;
uint32_t flags = 0;

//------------------------------------------------------------------------------
int main( int argn, char* argv[] )
{
	assert( sizeof(WRValue) == 2*sizeof(void*) );
	assert( sizeof(float) == 4 );
	assert( sizeof(unsigned char) == 1 );

	if ( argn <= 1 )
	{
		return usage();
	}
	
	WRstr command = SimpleArgs::get( argn, argv, 1 );

	flags = (SimpleArgs::get(argn, argv, "-nostrict") ? WR_NON_STRICT_VAR : 0)
			| (SimpleArgs::get(argn, argv, "-debug") ? WR_EMBED_DEBUG_CODE : 0)
			| (SimpleArgs::get(argn, argv, "-debugsource") ? WR_EMBED_SOURCE_CODE : 0);

	if ( SimpleArgs::get(argn, argv, "t") )
	{
		runTests( (argn >= 3) ? atoi(argv[2]) : 0 );
		printf("\n");
	}
	else if ( SimpleArgs::get(argn, argv, "p") )
	{
		// this is ALPHA and doesn't tell you much yet :)
		
		WRstr infile;
		infile.fileToBuffer( SimpleArgs::get(argn, argv, -1) );

		unsigned char* out;
		int outLen;
		int err = wr_compile( infile, infile.size(), &out, &outLen, 0, flags );
		
		if ( err )
		{
			printf( "compile error [%s]\n", c_errStrings[err] );
			return -1;
		}

		char* listing = 0;
		wr_disassemble( out, outLen, &listing );
		printf("%s\n", listing );
		g_free( listing );
		
	}
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	else if ( SimpleArgs::get(argn, argv, "d") )
	{
		WrenchDebugClient client;
		char port[64];
		if ( SimpleArgs::get( argn, argv, "-port", port, 64) )
		{
			client.enter( SimpleArgs::get(argn, argv, -1), port );
		}
		else
		{
			client.enter( SimpleArgs::get(argn, argv, -1) );
		}
	}
#endif
	else if ( SimpleArgs::get(argn, argv, "r") )
	{
#ifndef WRENCH_WITHOUT_COMPILER
		WRstr infile;
		if ( !infile.fileToBuffer(SimpleArgs::get(argn, argv, -1)) )
		{
			printf( "Could not open source file [%s]\n", SimpleArgs::get(argn, argv, -1) );
			return usage();
		}

		unsigned char* out;
		int outLen;
		
		int err = wr_compile( infile, infile.size(), &out, &outLen, 0, flags );
		
//		WRstr str;
//		wr_asciiDump( out, outLen, str );
//		printf( "Code Image: %d:\n%s\n", outLen, str.c_str() );
		
		if ( err )
		{
			printf( "compile error [%s]\n", c_errStrings[err] );
			return -1;
		}

		gw = wr_newState( 128 );
		wr_loadAllLibs(gw);
		wr_registerFunction( gw, "println", println );
		wr_registerFunction( gw, "print", print );

		wr_run( gw, out, outLen );

		g_free( out );

		if ( wr_getLastError(gw) )
		{
			printf( "err: %s\n", c_errStrings[(int)wr_getLastError(gw)] );
		}

		wr_destroyState( gw );
#else
		printf( "compiler not included in this build\n" );
		return usage();
#endif
	}
	else if ( SimpleArgs::get(argn, argv, "rb") )
	{
		WRstr bytes;
		if ( !bytes.fileToBuffer(SimpleArgs::get(argn, argv, -1)) )
		{
			printf( "Could not open bytecode [%s]\n", SimpleArgs::get(argn, argv, -1) );
			return usage();
		}

		gw = wr_newState( 128 );
		wr_loadAllLibs(gw);
		wr_registerFunction( gw, "println", println );
		wr_registerFunction( gw, "print", print );

		wr_run( gw, (const unsigned char *)bytes.c_str(), bytes.size() );
		if ( wr_getLastError( gw ) )
		{
#ifndef WRENCH_WITHOUT_COMPILER
			printf( "err: %s\n", c_errStrings[(int)wr_getLastError(gw)] );
#else
			printf( "err: %d\n", (int)wr_getLastError(gw) );
#endif
		}

		wr_destroyState( gw );
	}
	else if ( SimpleArgs::get(argn, argv, "c") )
	{
#ifndef WRENCH_WITHOUT_COMPILER
		if ( argn < 4 )
		{
			printf( "[%d]\n", argn );
			return usage();
		}

		unsigned char* out;
		int outLen;

		WRstr code;
		if ( !code.fileToBuffer(SimpleArgs::get(argn, argv, -2)) )
		{
			printf( "Could not open [%s]\n", SimpleArgs::get(argn, argv, -2) );
			return usage();
		}

		int err = wr_compile( code, code.size(), &out, &outLen, 0, flags );

		if ( err )
		{
			printf( "compile error [%s]\n", c_errStrings[err] );
			return 0;
		}

		WRstr str;
		str.set( (const char*)out, outLen );
		
//		printf( "%d:\n%s\n", outLen, str.c_str() );

		WRstr outname( SimpleArgs::get(argn, argv, -1) );
		
		if ( !str.bufferToFile(outname) )
		{
			printf( "could not write to [%s]\n", outname.c_str() );
		}

		printf( "%s -> %s\n", SimpleArgs::get(argn, argv, -2), outname.c_str() );

		g_free( out );
#else
		printf( "compiler not included in this build\n" );
		return usage();
#endif
	}
	else if ( SimpleArgs::get(argn, argv, "ch") )
	{
#ifndef WRENCH_WITHOUT_COMPILER
		if ( argn < 4 )
		{
			printf( "[%d]\n", argn );
			return usage();
		}
		
		unsigned char* out;
		int outLen;

		WRstr code;
		if ( !code.fileToBuffer(SimpleArgs::get(argn, argv, -3)) )
		{
			printf( "Could not open [%s]\n", SimpleArgs::get(argn, argv, -3) );
			return usage();
		}
		
		int err = wr_compile( code, code.size(), &out, &outLen, 0, flags );
		
		if ( err )
		{
			printf( "compile error [%s]\n", c_errStrings[err] );
			return 0;
		}

		WRstr str;
		wr_asciiDump( out, outLen, str );
		printf( "%d:\n%s\n", outLen, str.c_str() );
		
		WRstr outname( SimpleArgs::get(argn, argv, -2) );
		
		blobToHeader( WRstr((char *)out, outLen), SimpleArgs::get(argn, argv, -1), code );
		
		if ( !code.bufferToFile(outname) )
		{
			printf( "could not write to [%s]\n", outname.c_str() );
		}
		else
		{
			printf( "%s -> %s as %s\n", SimpleArgs::get(argn, argv, -3), outname.c_str(), SimpleArgs::get(argn, argv, -1) );
		}

		g_free( out );
#else
		printf( "compiler not included in this build\n" );
		return usage();
#endif
	}
	else if ( SimpleArgs::get(argn, argv, "release") )
	{
		WRstr version;
		version.format( "%d.%d.%d", WRENCH_VERSION_MAJOR, WRENCH_VERSION_MINOR, WRENCH_VERSION_BUILD );
		version.bufferToFile( "version.txt" );

		WRstr out = "#include \"wrench.h\"\n";
		WRstr name;		
		WRstr read;
		for( int s=0; sourceOrder[s][0]; ++s )
		{
			name = SimpleArgs::get(argn, argv, -2);
			name += sourceOrder[s];
			if ( !read.fileToBuffer(name) )
			{
				printf( "error reading source [%s]\n", name.c_str() );
				return -1;
			}
			out += read;
		}

		name = SimpleArgs::get(argn, argv, -1);
		name += "/wrench.cpp";
		out.bufferToFile( name );

		name = SimpleArgs::get(argn, argv, -2);
		name += "/wrench.h";
		if ( !read.fileToBuffer(name) )
		{
			printf( "error 2 reading [%s]\n", name.c_str() );
			return -1;
		}
		out = "#define WRENCH_COMBINED\n";
		out += read;
		name = SimpleArgs::get(argn, argv, -1);
		name += "/wrench.h";
		out.bufferToFile( name );
	}
	else
	{
		return usage();
	}

	return 0;
}

#ifndef WRENCH_WITHOUT_COMPILER
//------------------------------------------------------------------------------
void emit( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char buf[256];
		((WRstr*)usr)->appendFormat( "%s", argv[i].asString(buf, 256) );
	}

	retVal.i = 20; // for argument-return testing
}

//------------------------------------------------------------------------------
void emitln( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char buf[256];
		((WRstr*)usr)->appendFormat( "%s\n", argv[i].asString(buf, 256) );
	}

	retVal.i = 20; // for argument-return testing
}

//------------------------------------------------------------------------------
void checkIsRawArray( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	int len = -1;
	assert( argv[0].isRawArray(&len) );
	assert( !argv[0].isWrenchArray() );
	assert( !argv[0].isHashTable() );
	assert( !argv[0].isString() );
	if (len != argv[1].asInt()) { assert(0); }
}

//------------------------------------------------------------------------------
void checkIsWrenchArray( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	int len = -1;
	assert( !argv[0].isRawArray() );
	assert( argv[0].isWrenchArray(&len) );
	assert( !argv[0].isHashTable() );
	assert( !argv[0].isString() );
	if (len != argv[1].asInt())	{ assert(0); }
}

//------------------------------------------------------------------------------
void checkIsString( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	int len = -1;
	assert( !argv[0].isRawArray() );
	assert( !argv[0].isWrenchArray() );
	assert( !argv[0].isHashTable() );
	assert( argv[0].isString(&len) );
	if (len != argv[1].asInt()) { assert(0); }
}

//------------------------------------------------------------------------------
void checkIsHashTable( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	int len = -1;
	assert( !argv->isRawArray() );
	assert( !argv->isWrenchArray() );
	assert( argv->isHashTable(&len) );
	assert( !argv->isString() );
	if (len != argv[1].asInt()) { assert(0); }
}

//------------------------------------------------------------------------------
void checkIter( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
#if (__cplusplus <= 199711L)
	for ( WRValue::Iterator it = argv->begin(); it != argv->end(); ++it )
	{
		WRIteratorEntry const& E = *it;
#else
	for( WRIteratorEntry const& E : *argv )
	{
#endif
		if ( E.value->getHash() != argv[E.index+1].getHash() ) { assert(0); }
	}
}

#endif

//------------------------------------------------------------------------------
int runTests( int number )
{
	int err = 0;

#ifndef WRENCH_WITHOUT_COMPILER
	WRstr code;
	WRstr codeName;

	WRValue container;
	wr_makeContainer( &container );

	WRValue integer;
	wr_makeInt( &integer, 0 );
	wr_addValueToContainer( &container, "integer", &integer );

	char someArray[10] = "hello";
	wr_addArrayToContainer( &container, "name", someArray, 10 );

	char* someBigArray = (char *)g_malloc( 0x1FFFFF );
	someBigArray[0] = 10;
	someBigArray[10000] = 20;
	someBigArray[100000] = 30;
	someBigArray[0x1FFFFE] = 40;

	char byte = (char)0x99;
	char byte2 = (char)0x99;
	wr_addArrayToContainer( &container, "b", &byte, 1 );
	wr_addArrayToContainer( &container, "c", &byte2, 1 );
		
	wr_addArrayToContainer( &container, "big", someBigArray, 0x1FFFFF );

	FILE* tfile = fopen( "test_files.txt", "r" );
	char buf[256];
	int fileNumber = 0;
	char errMsg[256];

	unsigned char* out;
	int outLen;

	WRState* w = wr_newState( 128 );

	wr_loadAllLibs( w );

	
	while( fgets(buf, 255, tfile) && (err==0) )
	{
		if ( !number || (number == fileNumber) )
		{
			WRstr expect;
			codeName = buf;
			codeName.trim();

			if ( code.fileToBuffer(codeName) )
			{
				unsigned int t = 0;

				for( ; (t < code.size()) && (code[t] != '~'); ++t );

				t += 2;

				for( ; t<code.size() && (code[t] != '~'); ++t )
				{
					expect += code[t];
				}

				printf( "test [%d][%s]: ", fileNumber, codeName.c_str() );

				wr_compile( code, code.size(), &out, &outLen, errMsg, WR_NON_STRICT_VAR|WR_INCLUDE_GLOBALS );
				
				if ( err )
				{
					printf( "compile error [%s]\n", c_errStrings[err] );
					return -1;
				}

				WRstr logger;
				wr_registerFunction( w, "print", emit, &logger );
				wr_registerFunction( w, "println", emitln, &logger );
				
				wr_registerFunction( w, "checkIsWrenchArray", checkIsWrenchArray );
				wr_registerFunction( w, "checkIsRawArray", checkIsRawArray );
				wr_registerFunction( w, "checkIsString", checkIsString );
				wr_registerFunction( w, "checkIsHashTable", checkIsHashTable );
				wr_registerFunction( w, "checkIter", checkIter );

				wr_destroyContext( 0 ); // test that this works

				WRContext* context = 0;
				wr_destroyContext( context );
				context = wr_run( w, out, outLen );

				err = wr_getLastError( w );

				int args;
				WRValue* firstArg;
				WRValue* returnValue;
				while( wr_getYieldInfo(context, &args, &firstArg, &returnValue) )
				{
					if ( args > 0 )
					{
						*returnValue = *firstArg;
					}
					wr_continue( context );
				}

				if ( context && !(err = wr_getLastError(w)) )
				{
					integer.i = 2456;
					someArray[1] = 'e';

					WRValue* V = wr_callFunction( context, "userCheck", &container, 1 );
					if ( V && V->i == 77 )
					{
						assert( integer.i == 56789 );
						assert( someArray[1] == 'c' );
					}

					char testString[12] = "test string";
					WRValue val;
					wr_makeString( context, &val, testString, 11 );

					assert( val.isString() );
					
					wr_callFunction( context, "stringCheck", &val, 1 );
					wr_callFunction( context, "stringCheck", &val, 1 );
					wr_callFunction( context, "stringCheck", &val, 1 );
					wr_callFunction( context, "stringCheck", &val, 1 );

					V = wr_callFunction( context, "arrayCheck" );
					if ( V )
					{
						assert( V->isWrenchArray() );
						char someString[256] = { 0 };
						someString[0] = someString[0]; // kill warning
						assert( WRstr(V->indexArray(context, 0, false)->asString(someString)) == "some string" );
						assert( V->indexArray(context, 1, false)->asFloat() == 0.0f );
						assert( V->indexArray(context, 2, false)->asInt() == 0);
						assert( !V->indexArray(context, 3, false)  );
						assert( V->indexArray(context, 3, true)->asInt() == 0  );
					}

					WRValue s;
					wr_makeString( context, &s, "test string" );
					wr_callFunction( context, "stringCheck", &s, 1 );
				}

				if ( err )
				{
					printf( "execute error [%s]\n", c_errStrings[err] );
				}
				else if ( logger != expect )
				{
					printf( "error: expected\n"
							"-----------------------------\n"
							"%s\n"
							"saw:-------------------------\n"
							"%s"
							"\n"
							"-----------------------------\n",
							expect.c_str(),
							logger.c_str() );
				}
				else
				{
					printf( "PASS\n" );
				}

				g_free( out );
				wr_destroyContext( context );
			}
			else if ( fileNumber != 0 )
			{
				printf( "test [%d][%s]: FILE NOT FOUND\n", fileNumber, codeName.c_str() );
			}
		}
		
		fileNumber++;
	}

	testTimeSlices();
	testImport();
	testHalt();
	testScheduler();
	testStackOverflow();

	setup();

	wr_destroyContainer( &container );

	testGlobalValues( w );

	wr_destroyState( w );

	g_free( someBigArray );
	fclose( tfile );
#endif
	return err;
}

//------------------------------------------------------------------------------
void testGlobalValues( WRState* w )
{
	char globalTestCode[] = "var global_one = 10;\n"
							"var global_two = 20;\n"
							"var global_three = 30;\n"
							"var global_four = 40;\n"
							
							"function test1() { global_two = 25; }\n"
							"function test2() { return global_two == 35; }\n"
							"function test3() { global_three = 5.3; }\n"
							"function test4() { return global_three == 45; }\n"
							"function test5() { return global_three == 4.5; }\n"
							
							"function test6() { return global_four[7] == 8; }\n"
							"function test7() { return global_four[17] == 18; }\n"
							"function test8() { return global_four[17] == 19.3; }\n"
							"function test9() { return global_four[2] = 200; }\n"
							"function test10() { return global_four[32] = 3200; }\n";

	unsigned char* out;
	int outLen;
	char errMsg[256];
	int err = wr_compile( globalTestCode, (int)strlen(globalTestCode), &out, &outLen, errMsg, WR_INCLUDE_GLOBALS|WR_NON_STRICT_VAR );
	if ( err )
	{
		assert(0);
		printf("used");
	}

	WRContext* gc = wr_run( w, out, outLen );

	WRValue* g = wr_getGlobalRef( gc, "does_not_exist" );
	if ( g )
	{
		assert(0);
		printf("used");
	}

	g = wr_getGlobalRef( gc, "global_one" );
	assert( g && g->i == 10 );
	g = wr_getGlobalRef( gc, "::global_two" );
	assert( g && g->i == 20 );
	g = wr_getGlobalRef( gc, "global_three" );
	assert( g && g->i == 30 );
	g = wr_getGlobalRef( gc, "::global_four" );
	assert( g && g->i == 40 );

	WrenchValue wv1( gc, "global_one" );
	assert( wv1.isValid()
			&& *wv1.Int() == 10
			&& *(int *)wv1 == 10 );

	WrenchValue wv2( gc, "global_two" );
	wr_callFunction( gc, "test1" );
	assert( *wv2.Int() == 25 );

	*wv2.Int() = 35;
	assert( wr_callFunction(gc, "test2")->i );

	WrenchValue wv3( gc, "global_three" );
	assert( *(int *)wv3 == 30 );
	wr_callFunction(gc, "test3");
	assert( *(float *)wv3 == 5.3f );

	*(int *)wv3 = 45;
	assert( wr_callFunction(gc, "test4")->i );
	*(float *)wv3 = 4.5f;
	assert( wr_callFunction(gc, "test5")->i );

	*(float *)wv3 = 4.6f;
	assert( wr_callFunction(gc, "test5")->i == 0 );

	WrenchValue wv4( gc, "global_four" );
	
	assert( *(int *)wv4 == 40 );

	wr_makeInt( wv4.asArrayMember(7), 8 );
	assert( wr_callFunction(gc, "test6")->i );

	wr_makeInt( wv4.asArrayMember(17), 18 );
	assert( wr_callFunction(gc, "test7")->i );

	wv4.asArrayMember(17)->setFloat( 19.3f );
	assert( wr_callFunction(gc, "test8")->i );

	wv4.asArrayMember(2)->setInt( 200 );
	assert( wr_callFunction(gc, "test9") );

	wv4.asArrayMember(32)->setInt( 3200 );
	assert( wr_callFunction(gc, "test10") );

	g_free( out );
}

//------------------------------------------------------------------------------
void testStackOverflow()
{
#ifdef WRENCH_PROTECT_STACK_FROM_OVERFLOW
	const char* recurse = "function oops() { var a = 100; oops(); }\n"
						  "oops();\n";
	
	WRState* w = wr_newState( 10 );
	
	uint8_t* out;
	int outLen;
	wr_compile( recurse, strlen(recurse), &out, &outLen );

	assert( wr_run(w, out, outLen, true) == 0 );
	assert( w->err == WR_ERR_stack_overflow );

	wr_destroyState( w );
#endif	
}

//------------------------------------------------------------------------------
void testScheduler()
{
#ifdef WRENCH_TIME_SLICES
	const char* loop1 = "for(;;) { println(\"1\"); }";
	const char* loop2 = "for(;;) { println(\"2\"); }";
	const char* loop3 = "println(\"once\");";
	const char* loop4 = "for(;;) { println(\"4\"); }";
	const char* loop5 = "for(;;) { println(\"5\"); }";

	WrenchScheduler scheduler( 8 );

	WRstr logger;
	wr_registerFunction( scheduler.state(), "println", emitln, &logger );

	uint8_t* out;
	int outLen;

	wr_compile( loop1, strlen(loop1), &out, &outLen );
	scheduler.addThread( out, outLen, 10, true );
	scheduler.tick(10);

	wr_compile( loop2, strlen(loop2), &out, &outLen );
	scheduler.addThread( out, outLen, 10, true );
	scheduler.tick(10);

	wr_compile( loop3, strlen(loop3), &out, &outLen );
	scheduler.addThread( out, outLen, 10, true );
	scheduler.tick(10);

	wr_compile( loop4, strlen(loop4), &out, &outLen );
	scheduler.addThread( out, outLen, 10, true );
	scheduler.tick(10);

	wr_compile( loop5, strlen(loop5), &out, &outLen );
	scheduler.addThread( out, outLen, 10, true );
	scheduler.tick(10);

	//printf( "%s\n", logger.c_str() );

#endif
}

//------------------------------------------------------------------------------
void testTimeSlices()
{
#ifdef WRENCH_TIME_SLICES
	const char* loop = "for(;;) {}";

	WRState* w = wr_newState( 16 );

	wr_setInstructionsPerSlice( w, 10 );

	unsigned char* out;
	int outlen;
	wr_compile( loop, strlen(loop), &out, &outlen );
	WRContext* context = wr_run(w, out, outlen, true);
	for( int y=0; y<10; ++y )
	{
		assert( !wr_continue(context) );
		assert( wr_getYieldInfo(context) );
	}
	
	wr_destroyState( w );
#endif
}

//------------------------------------------------------------------------------
void testHalt()
{
	const char* haltMe = "sys::halt( 101 );";
	const char* haltMe2 = "sys::halt( 1 );";

	WRState* w = wr_newState( 16 );
	wr_loadAllLibs(w);

	unsigned char* out;
	int outlen;
	wr_compile( haltMe, strlen(haltMe), &out, &outlen );
	wr_run( w, out, outlen, true );
	assert( w->err == 101 );

	wr_compile( haltMe2, strlen(haltMe2), &out, &outlen );
	wr_run( w, out, outlen, true );
	assert( w->err == WR_ERR_USER_err_out_of_range );

	wr_destroyState( w );
}

//------------------------------------------------------------------------------
WRstr g_importBuf;
static void importln( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		unsigned int len = 0;
		char* str = argv[i].asMallocString( &len );
		g_importBuf += str;
		g_free( str );
	}
	g_importBuf += "\n";
}

/*
const char exampleYield[]=
" // Example Wrench Script\n"
" //   All scripts will 'override' a `run` function	\n"
"													\n"
"function run( values_hashtable )					\n"
"{													\n"
"   for(;;)											\n"
"   {												\n"
"      values_hashtable[\"random_number\"] += 1;	\n"
"   }												\n"
" }													\n";

#include <time.h>
//------------------------------------------------------------------------------
void testYield2()
{
	WRState* state = wr_newState(); // create a state (only need to do this once)

	// make sure we only execute 10 instructions at any time
	wr_setInstructionsPerSlice( state, 10 );

	// compile a script
	unsigned char* out;
	int outLen;
	int err = wr_compile( exampleYield, strlen(exampleYield), &out, &outLen );
	if ( err )
	{
		// oops! something wrong with the code:
		printf( "err compiling [%s]\n", c_errStrings[err] );
		wr_destroyState( state );
		return;
	}

	// now that the script is compiled into bytecode (stored in
	// out/outLen) we need to "run" it, this loads it into a context.
	// for the case of your example, it won't do anything but load the
	// function signature and return, since nothing actually happens in
	// the code in global scope.
	
	WRContext* context = wr_run( state, out, outLen );
	if ( !context )
	{
		// must have been an error processing the script?
		printf( "err compiling [%s]\n", c_errStrings[(int)wr_getLastError(state)] );
		wr_destroyState( state );
		return;
	}

	time_t future = time(0) + 5;

	while( time(0) < future ) // run for 5 seconds
	{
		// the VM will automatically continue on yield, this can be
		// called multiple times.. need to make this clear in the docs..
		WRValue* retval = wr_callFunction( context, "run" ); 
		
		if ( retval )
		{
			// the function returned, so it completed, do something
			// with it...

			// and we're out
			break;
		}
		
		if ( !wr_getYieldInfo(context) )
		{
			// something went wrong! this is not yielded
			printf( "non-yield [%s]\n", c_errStrings[(int)wr_getLastError(state)] );
			break;
		}
		else
		{
			printf( "function forced to yield\n" );
		}
	}

	wr_destroyState( state );
}
*/

const char* importMe = "export function imported(a) \n"
					   "{                    \n"
					   "    return a + 0xAE;    \n"
					   "}                    \n"
					   "export struct S1(n) { var a = 10; var b = 15; var c=n; };\n"
					   "\n"
					   "struct S0 { var c = 5; };\n";


const char* baseMe = "yield();\n"
					 "sys::importCompile( \"export struct S2 { var b = 20; };\" );\n"
					 "println( imported(1) );\n"
					 "imported(1);\n"
					 "var s1 = new S1;\n"
					 "println( s1.a );\n"
					 "var s2 = new S2;\n"
					 "println( s2.b );\n"
					 "var s0 = new S1;\n"
					 "s0 = new S1[]{};\n"
					 "s0 = new S1[]{{}};\n"
					 "s0 = new S1(99);\n"
					 "println(s0.d);\n"
					 "s0 = new S1{c = 201};\n"
					 "println(s0.a);\n"
					 "println(s0.b);\n"
					 "println(s0.c);\n"
					 "s0 = new S1{55,66};\n"
					 "println(s0.a);\n"
					 "println(s0.b);\n"
					 "println(s0.c);\n"
					 "var s0 = new S0;\n"
					 ;

//------------------------------------------------------------------------------
void testImport()
{
	WRState* w = wr_newState( 128 );
	wr_loadAllLibs(w);

	g_importBuf.clear();
	wr_registerFunction( w, "println", importln );

	unsigned char* out1;
	int out1len;
	unsigned char* out2;
	int out2len;

//	WRstr base;
//	base.fileToBuffer( "test.c" );
//	wr_compile( base, base.size(), &out1, &out1len );

//	wr_compile( "export struct S2 { var b = 20; };", strlen("export struct S2 { var b = 20; };"), &out1, &out1len );
//	g_free( out1 );
	
	wr_compile( baseMe, strlen(baseMe), &out1, &out1len );
	wr_compile( importMe, strlen(importMe), &out2, &out2len );

	WRContext* context = wr_run( w, out1, out1len, true );

	wr_import( context, out2, out2len, true );

	wr_continue( context );

	assert( w->err == WR_ERR_struct_not_exported );

	assert( g_importBuf ==
			"175\n"
			"10\n"
			"20\n"
			"0\n"
			"10\n"
			"15\n"
			"201\n"
			"55\n"
			"66\n"
			"0\n" );
	
	wr_destroyState( w );
}


// COMPLETE EXAMPLE MUST WORK

#include "wrench.h"

const int Pbasic_bytecodeSize=54;
const unsigned char Pbasic_bytecode[]=
{
	0x01, 0x00, 0x00, 0x04, 0x0D, 0x00, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, // 16
	0x64, 0x21, 0x0A, 0x06, 0x01, 0x88, 0x8A, 0x37, 0x16, 0xCF, 0x00, 0x00, 0xAF, 0x0A, 0x70, 0x00, // 32
	0x0F, 0x09, 0x20, 0x00, 0x06, 0x01, 0x88, 0x8A, 0x37, 0x16, 0x90, 0x00, 0x37, 0xEF, 0x09, 0x02, // 48
	0xEF, 0x14, 0x94, 0xF8, 0x8F, 0x5A, // 54
};

void logBlank( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr ) { }


void log2( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[512];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf, 512) );
	}
}

const char* wrenchCode = "print( \"Hello World!\\n\" ); "
						 "for( var i=0; i<10; i++ )     "
						 "{                             "
						 "    log( i );                 "
						 "}                             ";

void setup()
{
//	Serial.begin( 115200 );
//	delay( 2000 ); // wait for link to come up for sample

	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "print", logBlank ); // bind a function

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;

	int err = wr_compile( wrenchCode, (int)strlen(wrenchCode), &outBytes, &outLen );
	
				assert( err == 0 );
	
	if ( err == 0 )
	{
		wr_run( w, outBytes, outLen, true ); // load and run the code!
	}

				assert( wr_run(w, Pbasic_bytecode, Pbasic_bytecodeSize) );

	wr_destroyState( w );
}

void loop()
{}



