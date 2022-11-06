#include <Arduino.h>
#include "wrench.h"

/**********************************************************************

print( "Hello World!\n" );

for( i=0; i<10; i++ )       
{
	print( i );
}

***********************************************

to compile:

$ wrench ch basic.w basic.h basic

*/


const unsigned char basic_bytecode[]=
{
	0x00, 0x01, 0x01, 0x05, 0x00, 0x0D, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, // 16
	0x64, 0x21, 0x0A, 0x07, 0x01, 0x16, 0x37, 0x8A, 0x88, 0xCD, 0x00, 0x00, 0xAD, 0x0A, 0x6E, 0x00, // 32
	0x0F, 0x09, 0x1E, 0x00, 0x07, 0x01, 0x16, 0x37, 0x8A, 0x88, 0x8E, 0x00, 0x35, 0xEF, 0x09, 0x03, // 48
	0x13, // 49
};


void print( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[512];
	for( int i=0; i<argn; ++i )
	{
		Serial.print( argv[i].asString(buf, 512) );
	}
}


void setup()
{
	Serial.begin( 115200 );

	delay( 2000 ); // wait for link to come up for sample

	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "print", print ); // bind a function

	wr_run( w, basic_bytecode ); // load and run the code!

	wr_destroyState( w );
}

void loop()
{}
