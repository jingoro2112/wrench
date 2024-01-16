/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

//------------------------------------------------------------------------------
// returns : 0 - function  does not exist
//         : 1 - function is native (callback)
//         : 2 - function is in wrench (was in source code)
void wr_function( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0 )
	{
		WRValue* arg = stackTop - argn;
		const char* name = (char *)(arg->array());
		if ( name )
		{
			uint32_t hash = wr_hashStr( name );
			if ( c->w->globalRegistry.exists(hash, true, false) )
			{
				stackTop->i = 1;
			}
			else if ( c->registry.exists(hash, true, false) )
			{
				stackTop->i = 2;
			}
		}
	}
}

//------------------------------------------------------------------------------
// takes a single argument: how many invokations to ignore
void wr_gcPause( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		WRValue* arg = stackTop - argn;
		c->gcPauseCount = arg->asInt();
	}
}

//------------------------------------------------------------------------------
void wr_loadSysLib( WRState* w )
{
	wr_registerLibraryFunction( w, "sys::function", wr_function );
	wr_registerLibraryFunction( w, "sys::gcPause", wr_gcPause );

//	wr_registerLibraryFunction( w, "sys::serializeValue", wr_serializeValue );
//	wr_registerLibraryFunction( w, "sys::deserializeValue", wr_deserializeValue );

}