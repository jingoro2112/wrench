#include <wrench.h>
#include <string.h>
#include <stdio.h>

void print( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[1024];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf, 1024) );
	}
}

const char* wrenchCode = 
"print( \"Hello World!\\n\" );"
"for( var i=0; i<10; i++ )    "
"{                            "
"    print( i );              "
"}                            "
"print(\"\\n\");              ";



int main( int argn, char** argv )
{
	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "print", print ); // bind a function

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;

	int err = wr_compile( wrenchCode, strlen(wrenchCode), &outBytes, &outLen ); // compile it
	if ( err == 0 )
	{
		wr_run( w, outBytes, outLen, true ); // load and run the code!
	}

	wr_destroyState( w );

	return 0;
}
