/*******************************************************************************
Copyright (c) 2025 Curt Hartung -- curt.hartung@gmail.com

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
WRValue* wr_ifValueArray( WRValue* val )
{
	WRValue* ret = &(val->deref());
	return (IS_ARRAY(ret->xtype) && ret->va->m_type == SV_VALUE) ? ret : 0;
}

//------------------------------------------------------------------------------
WRValue* wr_ifHash( WRValue* val )
{
	WRValue* ret = &(val->deref());
	return IS_HASH_TABLE(ret->xtype) ? ret : 0;
}

//------------------------------------------------------------------------------
void wr_arrayCount( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	if( argn == 0 || !(A = wr_ifValueArray(stackTop - argn)) )
	{
		return;
	}

	stackTop->ui = A->va->m_size;
}

//------------------------------------------------------------------------------
void wr_arrayClear( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn >= 1 )
	{
		WRValue* args = stackTop - argn;

		WRValue* A = &(args->deref());

		A->p2 = INIT_AS_ARRAY;

		unsigned int count = 0;
		if ( argn > 1 )
		{
			count = args[1].asInt();
		}
		
		A->va = c->getSVA( count, SV_VALUE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !A->va )
		{
			A->p2 = INIT_AS_INT;
			return;
		}
#endif

		*stackTop = *A;
	}
}

//------------------------------------------------------------------------------
void wr_arrayPeek( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	if( argn == 0 || !(A = wr_ifValueArray(stackTop - argn)) )
	{
		return;
	}

	if ( A->va->m_size > 0 )
	{
		stackTop->p2 = INIT_AS_CONTAINER_MEMBER;// | ENCODE_ARRAY_ELEMENT_TO_P2( 0 );
		stackTop->r = A;
	}
}

//------------------------------------------------------------------------------
void wr_arrayPeekBack( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	if( argn == 0 || !(A = wr_ifValueArray(stackTop - argn)) )
	{
		return;
	}

	if ( A->va->m_size > 0 )
	{
		stackTop->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( A->va->m_size - 1 );
		stackTop->r = A;
	}
}

//------------------------------------------------------------------------------
void wr_arrayRemoveEx( WRValue* A, const unsigned int where, const int count, WRValue* stackTop, WRContext* c )
{
	if ( where < A->va->m_size )
	{
		unsigned int end = where + count;

		if ( end >= A->va->m_size )
		{
			A->va->m_size = where; // simple truncation
		}
		else
		{
			memmove( (char*)(A->va->m_Vdata + where), // to here
					 (char*)(A->va->m_Vdata + end),   // from here
					 (A->va->m_size - where) * sizeof(WRValue) ); // this many

			A->va->m_size -= count;
		}

		c->gc( stackTop + 1 );
	}
}

//------------------------------------------------------------------------------
void wr_arrayRemove( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 2) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	unsigned int where = (unsigned int)(args[1].asInt());

	int count = 1;
	if ( argn > 2 )
	{
		count = args[2].asInt();
	}

	wr_arrayRemoveEx( A, where, count, stackTop, c );

	*stackTop = *A;
}

//------------------------------------------------------------------------------
void wr_arrayTruncate( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 1) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	unsigned int size = (unsigned int)(args[1].asInt());

	if ( size < A->va->m_size )
	{
		A->va->m_size = size; // *snip*
		c->gc( stackTop + 1 );
	}

	*stackTop = *A;
}

//------------------------------------------------------------------------------
void wr_arrayInsertEx( WRValue* A, const unsigned int where, const unsigned int count, WRValue* stackTop, WRContext* c )
{
	unsigned int originalSize = A->va->m_size;
	// accomodate new size (passed value is expected to be the highest accessible index)
	wr_growValueArray( A->va, originalSize + (count - 1) );

	// move tail of array UP "count" entries
	unsigned int entries = originalSize - where;
	if ( entries )
	{
		unsigned int to = where + count;

		memmove( (char*)(A->va->m_Vdata + to),
				 (char*)(A->va->m_Vdata + where),
				 entries * sizeof(WRValue) );

		memset( (char*)(A->va->m_Vdata + where),
				0,
				count * sizeof(WRValue) );
	}

	c->gc( stackTop + 1 );
}

//------------------------------------------------------------------------------
void wr_arrayInsert( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 2) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	unsigned int where = (unsigned int)(args[1].asInt());

	int count = 1;
	if ( argn > 2 )
	{
		count = args[2].asInt();
	}

	wr_arrayInsertEx( A, where, count, stackTop, c );

	*stackTop = *A;
}

//------------------------------------------------------------------------------
void wr_arrayPop( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 1) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	if ( A->va->m_size )
	{
		*stackTop = *A->va->m_Vdata;
		wr_arrayRemoveEx( A, 0, 1, stackTop, c );
	}
}

//------------------------------------------------------------------------------
void wr_arrayPopBack( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 1) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	if ( A->va->m_size )
	{
		--A->va->m_size;
		*stackTop = A->va->m_Vdata[A->va->m_size];
		c->gc( stackTop + 1 );
	}
}

//------------------------------------------------------------------------------
void wr_arrayPushBack( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 2) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	const unsigned int where = A->va->m_size;

	wr_arrayInsertEx( A, where, 1, stackTop, c );
	A->va->m_Vdata[where] = args[1];
}

//------------------------------------------------------------------------------
void wr_arrayPush( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* A;
	WRValue* args = stackTop - argn;

	if( (argn < 2) || !(A = wr_ifValueArray(args)) )
	{
		return;
	}

	wr_arrayInsertEx( A, 0, 1, stackTop, c );
	A->va->m_Vdata[0] = args[1];
}

//------------------------------------------------------------------------------
void wr_hashClear( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn >= 1 )
	{
		WRValue* A = &((stackTop - argn)->deref());

		A->p2 = INIT_AS_HASH_TABLE;
		A->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !A->va )
		{
			A->p2 = INIT_AS_INT;
			return;
		}
#endif

		*stackTop = *A;
		
		c->gc( stackTop + 1 );
	}
}

//------------------------------------------------------------------------------
void wr_hashCount( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* H;
	if( argn == 0 || !(H = wr_ifHash(stackTop - argn)) )
	{
		return;
	}

	stackTop->ui = H->va->m_size;
}

//------------------------------------------------------------------------------
void wr_hashAdd( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* H;
	WRValue* args = stackTop - argn;
	if( argn < 3 || !(H = wr_ifHash(args)) )
	{
		return;
	}

	uint32_t hash = args[2].getHash(); // key
	int element;
	WRValue* entry = (WRValue*)( H->va->get(hash, &element) );

	*entry = args[1].deref();
	*(entry + 1) = args[2].deref();

	stackTop->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( element );
	stackTop->vb = (WRGCBase*)(H->va->m_Vdata) - 1;
}

//------------------------------------------------------------------------------
void wr_hashExistEx( WRValue* stackTop, const int argn, bool remove )
{
	WRValue* H;
	WRValue* args = stackTop - argn;
	if( (argn >= 2) && (H = wr_ifHash(args)) )
	{
		stackTop->p = H->va->exists( args[1].getHash(), remove );
	}
}

//------------------------------------------------------------------------------
void wr_hashRemove( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_hashExistEx( stackTop, argn, true );
}

//------------------------------------------------------------------------------
void wr_hashExists( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_hashExistEx( stackTop, argn, false );
}

//------------------------------------------------------------------------------
void wr_loadContainerLib( WRState* w )
{
	wr_registerLibraryFunction( w, "array::clear", wr_arrayClear );       // ( array )
	wr_registerLibraryFunction( w, "array::count", wr_arrayCount );       // ( array )
	wr_registerLibraryFunction( w, "array::remove", wr_arrayRemove );     // ( array, where, [count == 1] )
	wr_registerLibraryFunction( w, "array::insert", wr_arrayInsert );     // ( array, where, [count == 1] )
	wr_registerLibraryFunction( w, "array::truncate", wr_arrayTruncate ); // ( array, newSize )

	wr_registerLibraryFunction( w, "hash::clear", wr_hashClear );   // ( hash )
	wr_registerLibraryFunction( w, "hash::count", wr_hashCount );   // ( hash )
	wr_registerLibraryFunction( w, "hash::add", wr_hashAdd );       // ( hash, item, key )
	wr_registerLibraryFunction( w, "hash::remove", wr_hashRemove ); // ( hash, key )
	wr_registerLibraryFunction( w, "hash::exists", wr_hashExists ); // ( hash, key )

	wr_registerLibraryFunction( w, "list::clear", wr_arrayClear );        // ( list )
	wr_registerLibraryFunction( w, "list::count", wr_arrayCount );        // ( list )
	wr_registerLibraryFunction( w, "list::peek", wr_arrayPeek );          // ( list )
	wr_registerLibraryFunction( w, "list::pop", wr_arrayPop );            // ( list )
	wr_registerLibraryFunction( w, "list::pop_front", wr_arrayPop );      // ( list )
	wr_registerLibraryFunction( w, "list::pop_back", wr_arrayPopBack );   // ( list )
	wr_registerLibraryFunction( w, "list::push", wr_arrayPush );          // ( list, item )
	wr_registerLibraryFunction( w, "list::push_front", wr_arrayPush );    // ( list, item )
	wr_registerLibraryFunction( w, "list::push_back", wr_arrayPushBack ); // ( list )

	wr_registerLibraryFunction( w, "queue::clear", wr_arrayClear );   // ( q )
	wr_registerLibraryFunction( w, "queue::count", wr_arrayCount );   // ( q )
	wr_registerLibraryFunction( w, "queue::push", wr_arrayPush );     // ( q, item )
	wr_registerLibraryFunction( w, "queue::pop", wr_arrayPopBack );   // ( q )
	wr_registerLibraryFunction( w, "queue::peek", wr_arrayPeekBack ); // ( q )

	wr_registerLibraryFunction( w, "stack::clear", wr_arrayClear ); // ( stack )
	wr_registerLibraryFunction( w, "stack::count", wr_arrayCount ); // ( stack )
	wr_registerLibraryFunction( w, "stack::push", wr_arrayPush );   // ( stack, item )
	wr_registerLibraryFunction( w, "stack::pop", wr_arrayPop );     // ( stack )
	wr_registerLibraryFunction( w, "stack::peek", wr_arrayPeek );   // ( stack )
}

