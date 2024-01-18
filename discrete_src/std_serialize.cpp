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
void wr_stdSerialize( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn )
	{
		char* buf;
		int len;
		if ( wr_serialize(&buf, &len, *(stackTop - argn) ) )
		{
			stackTop->p2 = INIT_AS_ARRAY;
			stackTop->va = c->getSVA( 0, SV_CHAR, false );
			free( stackTop->va->m_Cdata );
			stackTop->va->m_data = buf;
			stackTop->va->m_size = len;
		}
	}
}

//------------------------------------------------------------------------------
void wr_stdDeserialize( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn )
	{
		WRValue& V = (stackTop - argn)->deref();
		if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_CHAR )
		{
			if ( !wr_deserialize( c, *stackTop, V.va->m_SCdata, V.va->m_size ) )
			{
				stackTop->init();
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_loadSerializeLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::serialize", wr_stdSerialize );
	wr_registerLibraryFunction( w, "std::deserialize", wr_stdDeserialize );
}
