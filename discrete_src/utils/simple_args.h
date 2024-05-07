/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

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
#ifndef SIMPLE_ARGS_H
#define SIMPLE_ARGS_H
//------------------------------------------------------------------------------

#include <string.h>

//------------------------------------------------------------------------------
namespace SimpleArgs
{

//   by index: 0,1,2... first, second, third  -1, -2, -3... last, second to last..
// returns pointer to argv if found
inline const char* get( int argn, char *argv[], int index, char* param =0, const int maxParamLen=0 );

//   by option
// returns true if found, opt is filled in with whatever follows it,
// so:
// returns pointer to argv found.
// IF param is specified, returns a pointer to param found
// if not found returns null
inline const char* get( int argn, char *argv[], const char* opt, char* param =0, const int maxParamLen=0 );

}

//------------------------------------------------------------------------------
inline const char* SimpleArgs::get( int argn, char *argv[], int index, char* param, const int maxParamLen )
{
	int a = index;
	if ( a < 0 )
	{
		a = argn + a;
	}

	if ( a >= argn )
	{
		return 0;
	}

	if ( param )
	{
		strncpy( param, argv[a], maxParamLen ? maxParamLen : SIZE_MAX );
	}

	return argv[a];
}

//------------------------------------------------------------------------------
inline const char* SimpleArgs::get( int argn, char *argv[], const char* opt, char* param, const int maxParamLen )
{
	for( int a=0; a<argn; ++a )
	{
		if ( strncmp(argv[a], opt, strlen(opt))
			 || (strlen(argv[a]) != strlen(opt)) )
		{
			continue;
		}

		if ( param )
		{
			param[0] = 0;
			if ( ++a < argn )
			{
				strncpy( param, argv[a], maxParamLen ? maxParamLen : SIZE_MAX );
			}
			else
			{
				return 0; // no param and we wanted it, so nope
			}
		}

		return argv[a];
	}

	return 0;
}


#endif
