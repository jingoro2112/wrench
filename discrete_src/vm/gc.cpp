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

#include "wrench.h"

//------------------------------------------------------------------------------
void WRContext::mark( WRValue* s )
{
	if ( IS_CONTAINER_MEMBER(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		// we don't mark this type, but we might mark it's target
		mark( s->r );
		return;
	}

	if ( !IS_EXARRAY_TYPE(s->xtype) || (s->va->m_flags & GCFlag_Marked) )
	{
		return;
	}

	assert( !IS_RAW_ARRAY(s->xtype) );

	WRGCBase* svb = s->vb;

	if ( svb->m_type == SV_VALUE )
	{
		WRValue* top = ((WRGCObject*)svb)->m_Vdata + ((WRGCObject*)svb)->m_size;

		for( WRValue* V = ((WRGCObject*)svb)->m_Vdata; V<top; ++V )
		{
			mark( V );
		}
	}
	else if ( svb->m_type == SV_HASH_TABLE )
	{
		// hash table points one WRGCBase size PAST the actual pointer,
		// recover it and mark it
		((WRGCBase*)(((WRGCObject*)svb)->m_Vdata) - 1)->m_flags |= GCFlag_Marked;

		for( uint32_t i=0; i<((WRGCObject*)svb)->m_mod; ++i )
		{
			if ( ((WRGCObject*)svb)->m_hashTable[i] != WRENCH_NULL_HASH )
			{
				uint32_t item = i<<1;
				mark( ((WRGCObject*)svb)->m_Vdata + item++ );
				mark( ((WRGCObject*)svb)->m_Vdata + item );
			}
		}
	}
	else if ( svb->m_type == SV_HASH_ENTRY )
	{
		// mark the referenced table so it is not collected
		((WRGCHashReference*)svb)->m_referencedTable->m_flags |= GCFlag_Marked;
	}

	svb->m_flags |= GCFlag_Marked;
}

//------------------------------------------------------------------------------
void WRContext::gc( WRValue* stackTop )
{
	if ( allocatedMemoryHint < allocatedMemoryLimit )
	{
		return;
	}

	allocatedMemoryHint = 0;

	// mark stack
	for( WRValue* s=stack; s<stackTop; ++s)
	{
		// an array in the chain?
		mark( s );
	}

	// mark context's globals
	WRValue* globalSpace = (WRValue *)(this + 1); // globals are allocated directly after this context

	for( unsigned int i=0; i<globals; ++i, ++globalSpace )
	{
		mark( globalSpace );
	}

	// sweep
	WRGCBase* current = svAllocated;
	WRGCBase* prev = 0;
	while( current )
	{
		// if set, clear it
		if ( current->m_flags & GCFlag_Marked )
		{
			current->m_flags &= ~GCFlag_Marked;
			prev = current;
			current = current->m_nextGC;
		}
		// otherwise free it as unreferenced
		else
		{
			current->clear();

			if ( prev == 0 )
			{
				svAllocated = current->m_nextGC;
				g_free( current );
				current = svAllocated;
			}
			else
			{
				prev->m_nextGC = current->m_nextGC;
				g_free( current );
				current = prev->m_nextGC;
			}
		}
	}
}

/* .. meh..
//------------------------------------------------------------------------------
void WRContext::scavenge( WRGCObject* va )
{
	// free the sub-array from this object, if the object itself
	// needs to be cleaned that has to happen in the gc but this
	// can be done anywhere
	if ( va->m_type == SV_VOID_HASH_TABLE )
	{
		return;
	}

	g_free( va->m_data );
	va->m_data = 0;

	if ( va->m_type != SV_CHAR )
	{
		if ( va->m_type == SV_HASH_TABLE )
		{
			g_free( va->m_hashTable );
			allocatedMemoryHint -= (va->m_mod * 4);
		}

		allocatedMemoryHint -= va->m_size * 3;
		va->m_type = SV_CHAR;
	}

	allocatedMemoryHint -= va->m_size;
	va->m_size = 0;
}
*/

//------------------------------------------------------------------------------
WRGCObject* WRContext::getSVA( int size, WRGCObjectType type, bool init )
{
	WRGCObject* ret = (WRGCObject*)g_malloc( sizeof(WRGCObject) );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !ret )
	{
		g_mallocFailed = true;
		return 0;
	}
#endif

	memset( (unsigned char*)ret, 0, sizeof(WRGCObject) );
	ret->m_nextGC = svAllocated;
	svAllocated = ret;

	allocatedMemoryHint += ret->init( size, type, init );

	if ( (int)type >= SV_VALUE )
	{
		ret->m_creatorContext = this;
	}

	return ret;
}

