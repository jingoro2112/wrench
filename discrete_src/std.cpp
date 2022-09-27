/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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

// standard functions that sort of come up a lot

int32_t wr_Seed;

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len )
{
	// in-place implementation of murmer
	uint32_t hash = 0x811C9DC5;
	const unsigned char* data = (const unsigned char *)dat;

	for( int i=0; i<len; ++i )
	{
		hash ^= (uint32_t)data[i];
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hashStr( const char* dat )
{
	uint32_t hash = 0x811C9DC5;
	const char* data = dat;
	while ( *data )
	{
		hash ^= (uint32_t)(*data++);
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
void wr_std_rand( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_INT;

	int32_t	k = wr_Seed / 127773;
	wr_Seed = 16807 * ( wr_Seed - k * 127773 ) - 2836 * k;
	ret->i = (uint32_t)wr_Seed % (uint32_t)stackTop->asInt();
}

//------------------------------------------------------------------------------
void wr_std_srand( WRValue* stackTop, const int argn )
{
	wr_Seed = stackTop->asInt();
}

#include <math.h>

//------------------------------------------------------------------------------
void wr_math_sin( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = sinf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_cos( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = cosf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_tan( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = tanf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_sinh( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = sinhf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_cosh( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = coshf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_tanh( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = tanhf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_asin( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = asinf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_acos( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = acosf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_atan( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = atanf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_atan2( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
		ret->p2 = INIT_AS_FLOAT;
		ret->f = atan2f( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_log( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = logf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_log10( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = log10f( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_exp( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = expf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_sqrt( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = sqrtf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_ceil( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = ceilf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_floor( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = floorf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_abs( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = (float)fabs( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_pow( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
		ret->p2 = INIT_AS_FLOAT;
		ret->f = powf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_fmod( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
		ret->p2 = INIT_AS_FLOAT;
		ret->f = fmodf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_trunc( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = truncf( stackTop->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_ldexp( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
		ret->p2 = INIT_AS_FLOAT;
		ret->f = ldexpf( (stackTop - 1)->asFloat(), (stackTop - 2)->asInt() );
	}
}

//------------------------------------------------------------------------------
const float wr_PI = 3.14159265358979323846f;
const float wr_toDegrees = (180.f / wr_PI);
const float wr_toRadians = (1.f / wr_toDegrees);

//------------------------------------------------------------------------------
void wr_math_rad2deg( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = wr_toDegrees * stackTop->asFloat();
}

//------------------------------------------------------------------------------
void wr_math_deg2rad( WRValue* stackTop, const int argn )
{
	WRValue* ret = GET_LIB_RETURN_VALUE_LOCATION(stackTop, argn);
	ret->p2 = INIT_AS_FLOAT;
	ret->f = wr_toRadians * stackTop->asFloat();
}

//------------------------------------------------------------------------------
void wr_loadAllLibs( WRState* w )
{
	wr_loadMathLib( w );
	wr_loadStdLib( w );
}

//------------------------------------------------------------------------------
void wr_loadMathLib( WRState* w )
{
	wr_registerLibraryFunction( w, "math::sin", wr_math_sin );
	wr_registerLibraryFunction( w, "math::cos", wr_math_cos );
	wr_registerLibraryFunction( w, "math::tan", wr_math_tan );
	wr_registerLibraryFunction( w, "math::sinh", wr_math_sinh );
	wr_registerLibraryFunction( w, "math::cosh", wr_math_cosh );
	wr_registerLibraryFunction( w, "math::tanh", wr_math_tanh );
	wr_registerLibraryFunction( w, "math::asin", wr_math_asin );
	wr_registerLibraryFunction( w, "math::acos", wr_math_acos );
	wr_registerLibraryFunction( w, "math::atan", wr_math_atan );
	wr_registerLibraryFunction( w, "math::atan2", wr_math_atan2 );
	wr_registerLibraryFunction( w, "math::log", wr_math_log );
	wr_registerLibraryFunction( w, "math::ln", wr_math_log );
	wr_registerLibraryFunction( w, "math::log10", wr_math_log10 );
	wr_registerLibraryFunction( w, "math::exp", wr_math_exp );
	wr_registerLibraryFunction( w, "math::pow", wr_math_pow );
	wr_registerLibraryFunction( w, "math::fmod", wr_math_fmod );
	wr_registerLibraryFunction( w, "math::trunc", wr_math_trunc );
	wr_registerLibraryFunction( w, "math::sqrt", wr_math_sqrt );
	wr_registerLibraryFunction( w, "math::ceil", wr_math_ceil );
	wr_registerLibraryFunction( w, "math::floor", wr_math_floor );
	wr_registerLibraryFunction( w, "math::abs", wr_math_abs );
	wr_registerLibraryFunction( w, "math::ldexp", wr_math_ldexp );

	wr_registerLibraryFunction( w, "math::deg2rad", wr_math_deg2rad );
	wr_registerLibraryFunction( w, "math::rad2deg", wr_math_rad2deg );

}

//------------------------------------------------------------------------------
void wr_loadStdLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::rand", wr_std_rand );
	wr_registerLibraryFunction( w, "std::srand", wr_std_srand );
}
