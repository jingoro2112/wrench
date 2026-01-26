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

#ifdef ARDUINO

#include "wrench.h"

#include <Arduino.h>
#include <FastLED.h>

static CRGB* wr_leds = 0;
static int wr_numLeds = 0;

//------------------------------------------------------------------------------
void wr_fastled_setup2812( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		if ( wr_leds )
		{
			g_free( wr_leds );
		}
		
		wr_numLeds = (stackTop - argn)->asInt();

		FastLED.addLeds<WS2812B, WR_FASTLED_DATA_PIN, GRB>(wr_leds, wr_numLeds);
	}
}

//------------------------------------------------------------------------------
void wr_fastled_brightness( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		FastLED.setBrightness( (stackTop - argn)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_fastled_set( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 3 )
	{
		WRValue* args = stackTop - argn;

		int led = args[0].asInt();
		if ( led >= 0 && led < wr_numLeds )
		{
			wr_leds[led].r = args[1].asInt();
			wr_leds[led].g = args[2].asInt();
			wr_leds[led].b = args[3].asInt();
		}

		FastLED.addLeds<WS2812B, WR_FASTLED_DATA_PIN, GRB>(wr_leds, wr_numLeds);
	}
}

//------------------------------------------------------------------------------
void wr_fastled_show( WRValue* stackTop, const int argn, WRContext* c )
{
	FastLED.show();
}

//------------------------------------------------------------------------------
void wr_loadFastLEDLib( WRState* w )
{
	wr_registerLibraryFunction( w, "fastled::setup2812", wr_fastled_setup2812 ); // ( data_pin, num leds )
	wr_registerLibraryFunction( w, "fastled::brightness", wr_fastled_show ); // ( brightness )
	wr_registerLibraryFunction( w, "fastled::set", wr_fastled_show ); // ( led, r, g, b )
	wr_registerLibraryFunction( w, "fastled::show", wr_fastled_show ); // ()
}

#endif
