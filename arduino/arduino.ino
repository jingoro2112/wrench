#include <Arduino.h>
#include "wrench.h"

void log( WRState* w, WRValue* argv, int argn, WRValue& retVal, void* usr )
{
	char buf[512];
	for( int i=0; i<argn; ++i )
	{
		Serial.print( wr_valueToString(argv[i], buf) );
	}
}


const char* wrenchCode = 

"log( \"Hello World!\\n\" ); "
"for( i=0; i<10; i++ )       "
"{                           "
"    log( i );               "
"}                           ";
 

void setup()
{
	Serial.begin( 115200 );

	delay( 2000 );

	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "log", log ); // bind this function to the state so it can call it back

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;
	
	int err = wr_compile( wrenchCode, strlen(wrenchCode), &outBytes, &outLen ); // compile it
	if ( err )
	{
		Serial.print( "error: " );
		Serial.print( err );
		Serial.print( "\n" );
	}
	else
	{
		wr_run( w, outBytes, outLen ); // load and run the code!

		delete[] outBytes; // clean up 
	}

	wr_destroyState( w );
}

void loop()
{}
