/*******************************************************************************
Copyright (c) 2026 Curt Hartung -- curt.hartung@gmail.com

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

#include <math.h>

//------------------------------------------------------------------------------
void wr_math_sin( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = sin( (stackTop - 1)->asDouble() );
#else
		stackTop->f = sinf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_cos( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = cos( (stackTop - 1)->asDouble() );
#else
		stackTop->f = cosf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_tan( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = tan( (stackTop - 1)->asDouble() );
#else
		stackTop->f = tanf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_sinh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = sinh( (stackTop - 1)->asDouble() );
#else
		stackTop->f = sinhf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_cosh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = cosh( (stackTop - 1)->asDouble() );
#else
		stackTop->f = coshf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_tanh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = tanh( (stackTop - 1)->asDouble() );
#else
		stackTop->f = tanhf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_asin( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = asin( (stackTop - 1)->asDouble() );
#else
		stackTop->f = asinf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_acos( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = acos( (stackTop - 1)->asDouble() );
#else
		stackTop->f = acosf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_atan( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = atan( (stackTop - 1)->asDouble() );
#else
		stackTop->f = atanf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_atan2( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = atan2( (stackTop - 2)->asDouble(), (stackTop - 1)->asDouble() );
#else
		stackTop->f = atan2f( (stackTop - 2)->asFloat(), (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_log( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = log( (stackTop - 1)->asDouble() );
#else
		stackTop->f = logf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_log10( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = log10( (stackTop - 1)->asDouble() );
#else
		stackTop->f = log10f( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_exp( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = exp( (stackTop - 1)->asDouble() );
#else
		stackTop->f = expf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_sqrt( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = sqrt( (stackTop - 1)->asDouble() );
#else
		stackTop->f = sqrtf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_ceil( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = ceil( (stackTop - 1)->asDouble() );
#else
		stackTop->f = ceilf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_floor( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = floor( (stackTop - 1)->asDouble() );
#else
		stackTop->f = floorf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_abs( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = fabs( (stackTop - 1)->asDouble() );
#else
		stackTop->f = (float)fabs( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_pow( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = pow( (stackTop - 2)->asDouble(), (stackTop - 1)->asDouble() );
#else
		stackTop->f = powf( (stackTop - 2)->asFloat(), (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_fmod( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = fmod( (stackTop - 2)->asDouble(), (stackTop - 1)->asDouble() );
#else
		stackTop->f = fmodf( (stackTop - 2)->asFloat(), (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_trunc( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = trunc( (stackTop - 1)->asDouble() );
#else
		stackTop->f = truncf( (stackTop - 1)->asFloat() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_ldexp( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = ldexp( (stackTop - 2)->asDouble(), (stackTop - 1)->asInt() );
#else
		stackTop->f = ldexpf( (stackTop - 2)->asFloat(), (stackTop - 1)->asInt() );
#endif
	}
}

//------------------------------------------------------------------------------
#ifdef WRENCH_DOUBLE_PRECISION
const double wr_PI = 3.14159265358979323846;
const double wr_toDegrees = (180.0 / wr_PI);
const double wr_toRadians = (1.0 / wr_toDegrees);
#else
const float wr_PI = 3.14159265358979323846f;
const float wr_toDegrees = (180.f / wr_PI);
const float wr_toRadians = (1.f / wr_toDegrees);
#endif

//------------------------------------------------------------------------------
void wr_math_rad2deg( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = wr_toDegrees * (stackTop - 1)->asDouble();
#else
		stackTop->f = wr_toDegrees * (stackTop - 1)->asFloat();
#endif
	}
}

//------------------------------------------------------------------------------
void wr_math_deg2rad( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
#ifdef WRENCH_DOUBLE_PRECISION
		stackTop->d = wr_toRadians * (stackTop - 1)->asDouble();
#else
		stackTop->f = wr_toRadians * (stackTop - 1)->asFloat();
#endif
	}
}

//------------------------------------------------------------------------------
void wr_intScale(WRValue* stackTop, const int argn, WRContext* c)
{
	int32_t res = 0;

	if (argn > 2)
	{
		WRValue* args = stackTop - argn;
		int32_t dvsr = args[2].asInt();

		if (dvsr != 0)
		{
			int32_t fac1 = args[0].asInt();
			int32_t fac2 = args[1].asInt();

			res = (int32_t) (((int64_t)fac1 * fac2) / dvsr);
		}
	}

	stackTop->i = res;
}

//------------------------------------------------------------------------------
void wr_sizeofFloat(WRValue* stackTop, const int argn, WRContext* c)
{
#ifdef WRENCH_DOUBLE_PRECISION
	stackTop->i = 8;
#else
	stackTop->i = 4;
#endif
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

	wr_registerLibraryFunction( w, "math::iscale", wr_intScale);

	wr_registerLibraryFunction( w, "math::sizeofFloat", wr_sizeofFloat);
}
