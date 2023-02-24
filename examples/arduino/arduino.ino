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


const int basic_bytecodeSize=53;
const unsigned char basic_bytecode[]=
{
	0x00, 0x01, 0x06, 0x00, 0x0D, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, // 16
	0x21, 0x0A, 0x08, 0x01, 0x16, 0x37, 0x8A, 0x88, 0xD2, 0x00, 0x00, 0x02, 0x0A, 0x7D, 0x00, 0x0F, // 32
	0x09, 0x2B, 0x00, 0x08, 0x01, 0x16, 0x37, 0x8A, 0x88, 0x9D, 0x00, 0x44, 0xEF, 0x09, 0x01, 0x12, // 48
	0x13, 0xF9, 0x10, 0x8F, 0x3B, // 53
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

	WRState* w = wr_newState(20); // create the state with 20 stack entries

	wr_registerFunction( w, "print", print ); // bind a function

	wr_run( w, basic_bytecode, basic_bytecodeSize ); // load and run the code!

	wr_destroyState( w );
}

void loop()
{}
