#include <wrench.h>
#include <string.h>
#include <stdio.h>

void print( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[1024];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf, 1024) );
	}
}


const char* wrenchCode = 
						"function someFunction( data )   "
						"{                               "
						"    print( data.val, \"\\n\" );   "
						"    data.ai[3] += data.val;     "
						"    print( data.ai[3], \"\\n\" ); "
						"}                               ";



int main( int argn, char** argv )
{
	WRValue userData;
	int usrInt[10];

	wr_makeUserData( &userData );
	wr_addUserIntArray( &userData, "ai", usrInt, 10 );

	WRValue userval;
	wr_makeInt( &userval, 22 );
	wr_addUserValue( &userData, "val", &userval );

	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "print", print ); // bind a function

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;

	int err = wr_compile( wrenchCode, strlen(wrenchCode), &outBytes, &outLen ); // compile it
	if ( err == 0 )
	{
		WRContext* context = wr_run( w, outBytes ); // load and run the code!

		usrInt[3] = 55;

		wr_callFunction( w, context, "someFunction", &userData, 1 ); // now call the function that it loaded

		delete[] outBytes; // clean up.. don't do this until all calls are made, wrench does NOT copy bytecode into it's own space
	}
	
	wr_destroyState( w );

	return 0;
}
