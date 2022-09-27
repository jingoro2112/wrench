#include <wrench.h>
#include <string.h>
#include <stdio.h>

void log( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[1024];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf) );
	}
}


const char* wrenchCode = 
"log( \"Hello World!\\n\" ); "
"for( i=0; i<10; i++ )       "
"{                           "
"    log( i );               "
"}                           "
"log(\"\\n\");               ";



int main( int argn, char** argv )
{
	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "log", log ); // bind a function

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;

	int err = wr_compile( wrenchCode, strlen(wrenchCode), &outBytes, &outLen ); // compile it
	if ( err == 0 )
	{
		wr_run( w, outBytes ); // load and run the code!
		delete[] outBytes; // clean up 
	}

	wr_destroyState( w );

	return 0;
}
