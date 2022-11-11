/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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

#include <Arduino.h>
#include <Wire.h>

//------------------------------------------------------------------------------
void wr_std_delay( WRValue* stackTop, const int argn, WRContext* c)
{
	// std::delay( milliseconds )
	if( argn == 1)
	{
		delay( (unsigned long)stackTop[-1].asInt() ); // a signed int will work up to 25 days
	}
}

//------------------------------------------------------------------------------
void wr_io_pinMode( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::pinMode( pin, <0 input, 1 output> )
	if( argn == 2)
	{
		pinMode( stackTop[-2].asInt(), (stackTop[-1].asInt() == 0) ? INPUT : OUTPUT );
	}
}

//------------------------------------------------------------------------------
void wr_io_digitalWrite( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::digitalWrite( pin, value )
	if( argn == 2)
	{
		digitalWrite(stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_io_analogRead( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::analogRead( pin )
	if ( argn == 1)
	{
		stackTop->i = analogRead( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_lcd_begin( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::begin( columns, rows )
	// returns status
	if( argn == 2)
	{
//		stackTop->i = lcd.begin( stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_lcd_setCursor( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::setCursor( column, row )
	if( argn == 2 )
	{
//		lcd.setCursor( stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_lcd_print( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::print( arg )
	// returns number of chars printed
	if( argn == 1 )
	{
		char buf[61];
//		stackTop->i = lcd.print( stackTop[-1].asString(buf, sizeof(buf)-1));
	}
}

//------------------------------------------------------------------------------
void wr_wire_begin( WRValue* stackTop, const int argn, WRContext* c)
{
	Wire.begin();
}

//------------------------------------------------------------------------------
void wr_wire_beginTransmission( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 1)
	{
		Wire.beginTransmission( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_write( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 1)
	{
		Wire.write( stackTop[-1].asInt() );
	}
	else if( argn == 2)
	{
		// To be added !
	}
}

//------------------------------------------------------------------------------
void wr_wire_endTransmission( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 0)
	{
		stackTop->i = Wire.endTransmission();
	}
	else if( argn == 1)
	{
		stackTop->i = Wire.endTransmission( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_requestFrom( WRValue* stackTop, const int argn, WRContext* c)
{
	// wire::requestFrom( address, bytes )
	if( argn == 2)
	{
		stackTop->i = Wire.requestFrom( stackTop[-2].asInt(), stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_available( WRValue* stackTop, const int argn, WRContext* c)
{
	stackTop->i = Wire.available();
}

//------------------------------------------------------------------------------
void wr_wire_read( WRValue* stackTop, const int argn, WRContext* c)
{
	stackTop->i = Wire.read();
}

//------------------------------------------------------------------------------
void wr_loadArduinoWireLib( WRState* w )
{
	wr_registerLibraryFunction( w, "wire::begin", wr_wire_begin);
	wr_registerLibraryFunction( w, "wire::beginTransmission", wr_wire_beginTransmission);
	wr_registerLibraryFunction( w, "wire::write", wr_wire_write);
	wr_registerLibraryFunction( w, "wire::endTransmission", wr_wire_endTransmission);
	wr_registerLibraryFunction( w, "wire::requestFrom", wr_wire_requestFrom);
	wr_registerLibraryFunction( w, "wire::available", wr_wire_available);
	wr_registerLibraryFunction( w, "wire::read", wr_wire_read);
}

//------------------------------------------------------------------------------
void wr_loadArduinoSTDLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::delay", wr_std_delay );
}

//------------------------------------------------------------------------------
void wr_loadArduinoIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::pinMode", wr_io_pinMode );
	wr_registerLibraryFunction( w, "io::digitalWrite", wr_io_digitalWrite );
	wr_registerLibraryFunction( w, "io::analogRead", wr_io_analogRead );
}

//------------------------------------------------------------------------------
void wr_loadArduinoLCDLib( WRState* w )
{
	wr_registerLibraryFunction( w, "lcd::begin", wr_lcd_begin );
	wr_registerLibraryFunction( w, "lcd::setCursor", wr_lcd_setCursor );
	wr_registerLibraryFunction( w, "lcd::print", wr_lcd_print );
}

//------------------------------------------------------------------------------
void wr_loadArduinoLib( WRState* w )
{
	wr_loadArduinoLCDLib( w );
	wr_loadArduinoIOLib( w );
	wr_loadArduinoSTDLib( w );
	wr_loadArduinoWireLib( w );
}
