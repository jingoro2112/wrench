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

//------------------------------------------------------------------------------
// key   : anything hashable
// clear : [OPTIONAL] default 0/'false' 
//
// returns : value if it was there
void wr_mboxRead( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		WRValue* args = stackTop - argn;
		int clear = (argn > 1) ? args[1].asInt() : 0;

		WRValue* msg = c->w->globalRegistry.exists( args[0].getHash(), true, clear );

		if ( msg )
		{
			*stackTop = *msg;
			return;
		}

	}

	stackTop->init();
}

//------------------------------------------------------------------------------
// key  : anything hashable
// value: stores the "hash"
void wr_mboxWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		WRValue* msg = c->w->globalRegistry.getAsRawValueHashTable( args[0].getHash() );
		msg->ui = args[1].getHash();
		msg->p2 = INIT_AS_INT;
	}
}

//------------------------------------------------------------------------------
// key  : anything hashable
void wr_mboxClear( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		c->w->globalRegistry.exists((stackTop - argn)->getHash(), true, true );
	}
}

//------------------------------------------------------------------------------
// key  : anything hashable
//
// return : true/false
void wr_mboxPeek( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0
		 && c->w->globalRegistry.exists((stackTop - argn)->getHash(), true, false) )
	{
		stackTop->i = 1;
	}
}

//------------------------------------------------------------------------------
void wr_loadMessageLib( WRState* w )
{
	wr_registerLibraryFunction( w, "msg::read", wr_mboxRead ); // read (true to clear, false to leave)
	wr_registerLibraryFunction( w, "msg::write", wr_mboxWrite ); // write message
	wr_registerLibraryFunction( w, "msg::clear", wr_mboxClear ); // remove if exists
	wr_registerLibraryFunction( w, "msg::peek", wr_mboxPeek ); // does the message exist?
}
