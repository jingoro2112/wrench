#include <Arduino.h>
#include "wrench.h"

/**********************************************************************

print( "Hello World!\n" );

for( var i=0; i<10; i++ )       
{
	print( i );
}

***********************************************

to compile:

$ wrench ch basic.w basic.h basic

*/

const int basic_bytecodeSize=54;
const unsigned char basic_bytecode[]=
{
        0x00, 0x01, 0x00, 0x04, 0x0D, 0x00, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, // 16
        0x64, 0x21, 0x0A, 0x06, 0x01, 0x88, 0x8A, 0x37, 0x16, 0xCE, 0x00, 0x00, 0xAE, 0x0A, 0x6F, 0x00, // 32
        0x0F, 0x09, 0x1F, 0x00, 0x06, 0x01, 0x88, 0x8A, 0x37, 0x16, 0x8F, 0x00, 0x36, 0xEF, 0x09, 0x02, // 48
        0xEE, 0x13, 0xF4, 0x02, 0x11, 0x5E, // 54
};

void print( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
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

	Serial.print( "\r\n" );

	WRState* w = wr_newState(20); // create the state with 20 stack entries

	wr_registerFunction( w, "print", print ); // bind a function

	wr_run( w, basic_bytecode, basic_bytecodeSize ); // load and run the code!

	wr_destroyState( w );
}

void loop()
{}
