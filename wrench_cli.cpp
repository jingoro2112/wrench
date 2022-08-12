#include <stdio.h>

#define STR_FILE_OPERATIONS
#include <wrench.h>
#include "discrete_src/str.h"

int runTests( int number =0 );
int testRembrandt();

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
const char* sourceOrder[]=
{
	"/utils.h",
	"/vm.h",
	"/opcode.h",
	"/str.h",
	"/cc.h",
	"/cc.cpp",
	"/vm.cpp",
	"/operations.cpp",
	"/std.cpp",
	""
};

//------------------------------------------------------------------------------
int usage()
{
	printf( "Wrench v%d\n"
		   "usage: wrench <command> [options]\n"
			"where command is:\n"
			"\n"
			"cb [infile] [out as bytecode]  compile infile and output as raw bytecode\n"
			"\n"
			"ch [infile] [out file] [name]  compile infile and output as a header file\n"
			"                               of name \"out file\"\n"
			"                               suitable for '#include'\n"
			"                               the exported constants will be:\n"
			"                               const char* [name]_bytecode;\n"
			"                               const int [name]_bytecodeSize;\n"
			"\n"
			"t                              run internal tests\n"
			"\n"
			"release [fromdir] [todir]      collect the discrete source files together\n"
			"                               and output [todir]/wrench.[cpp/h]\n"
			"\n"
			"xb [binary file to execute]    execute the file as if its bytecode\n"
			"xs [source file to execute]    compile and execute execute the file\n"
			"                               as if its source code\n"
			"\n",
			WRENCH_VERSION
		  );
	
	return -1;
}

//------------------------------------------------------------------------------
void getIndexed( WRState* w, void* usr, WRValue* index, WRValue* out )
{
	out->type = WR_INT;
	out->i = 10;
	
}

//------------------------------------------------------------------------------
static void log( WRState* s, WRValue* argv, int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char buf[256];
		printf( "%s", wr_valueToString(argv[i], buf) );
	}
	printf( "\n" );
}

//------------------------------------------------------------------------------
int main( int argn, char* argv[] )
{
	if ( argn <= 1 )
	{
		return usage();
	}

	WRstr command(argv[1]);

	if ( command == "t" )
	{
		runTests( (argn >= 3) ? atoi(argv[2]) : 0 );
	}
	else if ( command == "r" )
	{
		testRembrandt();
	}
	else if ( (command == "xb" || command == "xs") && argn == 3 )
	{
		WRstr bytes;
		if ( command == "xb" )
		{
			if ( !bytes.fileToBuffer(argv[2]) )
			{
				printf( "Could not open bytecode [%s]\n", argv[2] );
				return usage();
			}
		}
		else
		{
			WRstr infile;
			if ( !infile.fileToBuffer(argv[2]) )
			{
				printf( "Could not open source file [%s]\n", argv[2] );
				return usage();
			}


			unsigned char* out;
			int outLen;

			int err = wr_compile( infile, infile.size(), &out, &outLen );
			if ( err )
			{
				printf( "compile error [%d]\n", err );
				return -1;
			}

			bytes.set( (char *)out, outLen );
			delete[] out;
		}

		WRState* w = wr_newState();
		wr_registerFunction( w, "log", log );

		int err = wr_run( w, (const unsigned char *)bytes.c_str(), bytes.size() );
		if ( err )
		{
			printf( "err: %d\n", err );
		}
	}
	else if ( command == "cb" || command == "ch" )
	{
		unsigned char* out;
		int outLen;

		WRstr code;
		if ( !code.fileToBuffer(argv[2]) )
		{
			printf( "Could not open [%s]\n", argv[2] );
			return usage();
		}

		int err = wr_compile( code, code.size(), &out, &outLen );
		if ( err )
		{
			printf( "compile error [%d]\n", err );
			return 0;
		}

		WRstr outname( argv[3] );
		if ( command == "cb" && argn == 4)
		{
			code.set( (char *)out, outLen );
		}
		else if ( command == "ch" && argn == 5 )
		{
			blobToHeader( WRstr((char *)out, outLen), argv[4], code );
		}

		if ( !code.bufferToFile(outname) )
		{
			printf( "could not write to [%s]\n", outname.c_str() );
		}
		else
		{
			printf( "%s -> %s\n", argv[2], outname.c_str() );
		}

		delete[] out;
	}
	else if ( argn == 4 && WRstr(argv[1]) == "release" )
	{
		WRstr out = "#include \"wrench.h\"\n";
		WRstr name;		
		WRstr read;
		for( int s=0; sourceOrder[s][0]; ++s )
		{
			name = argv[2];
			name += sourceOrder[s];
			if ( !read.fileToBuffer(name) )
			{
				printf( "error reading [%s]\n", name.c_str() );
				return -1;
			}
			out += read;
		}

		name = argv[3];
		name += "/wrench.cpp";
		out.bufferToFile( name );

		name = argv[2];
		name += "/wrench.h";
		if ( !read.fileToBuffer(name) )
		{
			printf( "error reading [%s]\n", name.c_str() );
			return -1;
		}
		out = "#define WRENCH_COMBINED\n";
		out += read;
		name = argv[3];
		name += "/wrench.h";
		out.bufferToFile( name );
	}
	else
	{
		return usage();
	}

	return 0;
}

//------------------------------------------------------------------------------
static void emit( WRState* s, WRValue* argv, int argn, WRValue& retVal, void* usr )
{
	if ( argn >= 1 )
	{
		char buf[256];
		((WRstr*)usr)->appendFormat( "%s\n", wr_valueToString(*argv, buf) );
	}
}

//------------------------------------------------------------------------------
unsigned int cliTestLoader( int offset, unsigned char** block, void* usr )
{
	*block = (unsigned char *)usr + offset;
	return 16;
}

//------------------------------------------------------------------------------
int runTests( int number )
{
	WRstr code;
	WRstr codeName;

	WRValue userData;
	WRValue charArray;
	WRValue intArray;
	WRValue floatArray;
	unsigned char userChar[10];
	int usrInt[10];
	float usrFloat[10];
	wr_makeUserData( &userData );
	wr_makeCharArray( &charArray, 10, userChar, "ac", &userData );
	wr_makeIntArray( &intArray, 10, usrInt, "ai", &userData );
	wr_makeFloatArray( &floatArray, 10, usrFloat, "af", &userData );

/*
	user.ac[10]
	user.ai[10]
	user.af[10]
*/
	FILE* tfile = fopen( "test_files.txt", "r" );
	char buf[256];
	int fileNumber = 0;
	WRError err = WR_ERR_None;

	WRState* w = wr_newState();

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

				unsigned char* out;
				int outLen;

				err = wr_compile( code, code.size(), &out, &outLen );
				if ( err )
				{
					printf( "compile error [%d]\n", err );
					return -1;
				}
				
				WRstr logger;
				wr_registerFunction( w, "log", emit, &logger );


#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				int context = wr_run( w, out, outLen );
#else
				int context = wr_run( w, cliTestLoader, out );
#endif

				if ( !wr_getLastError(w) )
				{
					wr_callFunction( w, context, "userCheck", &userData, 1 );
				}
				
				if ( err )
				{
					printf( "execute error [%d]\n", err );
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
							expect.c_str(), logger.c_str() );
				}
				else
				{
					printf( "PASS\n" );
				}

				delete[] out;
			}
			else if ( fileNumber != 0 )
			{
				printf( "test [%d][%s]: FILE NOT FOUND\n", fileNumber, codeName.c_str() );
			}
		}

		fileNumber++;
	}

	wr_destroyState( w );

	wr_destroyValue( &userData );
	wr_destroyValue( &charArray );
	wr_destroyValue( &intArray );
	wr_destroyValue( &floatArray );

	fclose( tfile );
	return err;
}


//------------------------------------------------------------------------------
#include "rembrandt/effects.h"
#include "rembrandt/wrench_effects.h"
AddressableChannel addrChannel[NUM_EFFECT_CHANNELS];
RGBChannel rgbChannel[NUM_RGB_CHANNELS];
EEPROMConstants constants;
WRValue clock;


//------------------------------------------------------------------------------
int testRembrandt()
{
	unsigned char* out;
	int outLen;
	WRstr infile;
	infile.fileToBuffer( "arduino_effects.c" );
	wr_compile( infile, infile.size(), &out, &outLen );

	WRState* w = wr_newState();
	
	setupWrench();
	for( int c=0; c<NUM_EFFECT_CHANNELS; ++c )
	{
		addrChannel[c].threadId = wr_run( w, wrenchEffects_bytecode, wrenchEffects_bytecodeSize );
		printf( "%d\n", addrChannel[c].threadId );
	}

	for( int c=0; c<NUM_RGB_CHANNELS; ++c )
	{
		rgbChannel[c].threadId = wr_run( w, wrenchEffects_bytecode, wrenchEffects_bytecodeSize );
		printf( "%d\n", rgbChannel[c].threadId );
	}

	for(;;)// clock.i=0; clock.i<100000; ++clock.i )
	{
		for( int c=0; c<NUM_EFFECT_CHANNELS; ++c )
		{
			if ( --addrChannel[c].ticksToNextCall.i < 0 )
			{
				addrChannel[c].ticksToNextCall.i = 0;
				wr_callFunction( w, addrChannel[c].threadId, "tick_addr", &addrChannel[c].userData, 1 );
			}
		}

		for( int c=0; c<NUM_RGB_CHANNELS; ++c )
		{
			if ( --rgbChannel[c].ticksToNextCall.i < 0 )
			{
				rgbChannel[c].ticksToNextCall.i = 0;
				wr_callFunction( w, rgbChannel[c].threadId, "tick_rgb", &rgbChannel[c].userData, 1 );
			}
		}
		
//		printf( "%d\n", r[5].i );
	}

	return 0;
}