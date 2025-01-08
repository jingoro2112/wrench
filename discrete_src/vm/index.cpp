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
void arrayElementToTarget( const uint32_t index, WRValue* target, WRValue* value )
{
	if ( target == value )
	{
		// this happens when a return value is used directly instead of
		// assigned to something. So it should be safe to just
		// dereference the value and use it directly rather than
		// preserve the whole array

		if ( value->va->m_type == SV_VALUE )
		{
			*target = value->va->m_Vdata[ index ];
		}
		else if ( value->va->m_type == SV_CHAR )
		{
			target->p2 = INIT_AS_INT;
			target->ui = value->va->m_Cdata[ index ];
		}
		else // SV_HASH_TABLE, right?
		{
			*target = *(WRValue *)value->va->get( index );
		}
	}
	else
	{
		target->r = value;
		target->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( index );
	}
}

//------------------------------------------------------------------------------
void doIndexHash( WRValue* index, WRValue* value, WRValue* target )
{
	uint32_t hash = index->getHash();

	if ( value->xtype == WR_EX_HASH_TABLE ) 
	{
		int element;
		WRValue* entry = (WRValue*)(value->va->get(hash, &element));

		// need to create a hash ref
		*(entry + 1) = *index; // might be the first time it was registered

		target->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( element );
		target->vb = (WRGCBase*)(value->va->m_Vdata) - 1;
	}
	else // naming an element of a struct "S.element"
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( (uint32_t)READ_32_FROM_PC(table) == hash )
		{
			int o = READ_8_FROM_PC(table + 4);

			target->p2 = INIT_AS_REF;
			target->r = ((WRValue*)(value->va->m_data)) + o;

		}
		else
		{
			target->init();
		}
	}
}

//------------------------------------------------------------------------------
void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	// all we know is the value is not an array, so make it one
	value->va = c->getSVA( index->ui + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !value->va )
	{
		value->p2 = INIT_AS_INT;
		return;
	}
#endif
	value->p2 = INIT_AS_ARRAY;
	
	arrayElementToTarget( index->ui, target, value );
}

//------------------------------------------------------------------------------
void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	if ( IS_CONTAINER_MEMBER(value->xtype) && IS_RAW_ARRAY(value->r->xtype) )
	{
		unsigned int s = DECODE_ARRAY_ELEMENT_FROM_P2(value->r->p2);

		target->p2 = INIT_AS_INT;
		target->ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(value->r->p2))) ? (uint32_t)(unsigned char)(value->r->c[s]) : 0;
	}
	else
	{
		value = &value->deref();

		if (EXPECTS_HASH_INDEX(value->xtype))
		{
			doIndexHash( index, value, target );
			return;
		}

		// at this point we assume the value is an array, if it is not it
		// will be converted into one

		if ( IS_RAW_ARRAY(value->xtype) )
		{
			if ( index->ui >= (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(value->p2)) )
			{
				goto boundsFailed;
			}
		}
		else if ( !(value->xtype == WR_EX_ARRAY) )
		{
			// nope, make it one of this size and return a ref
			WRValue* I = &index->deref();

			if ( I->type == WR_INT )
			{
				value->va = c->getSVA( index->ui+1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
				if ( !value->va )
				{
					value->p2 = INIT_AS_INT;
					return;
				}
#endif
				value->p2 = INIT_AS_ARRAY;

			}
			else
			{
				value->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
				if ( !value->va )
				{
					value->p2 = INIT_AS_INT;
					return;
				}
#endif
				value->p2 = INIT_AS_HASH_TABLE;

				doIndexHash( I, value, target );
				return;
			}
		}
		else if ( index->ui >= value->va->m_size )
		{
			if ( value->va->m_flags & GCFlag_NoContext )
			{
boundsFailed:
				target->init();
				return;
			}

			wr_growValueArray( value->va, index->ui );
		}

		arrayElementToTarget( index->ui, target, value );
	}
}

//------------------------------------------------------------------------------
void doIndex_E_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	// a float or int is being indexed, make it a hash table and "index" it
	WRValue* V = &(value->deref());

	V->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !V->va )
	{
		V->p2 = INIT_AS_INT;
		return;
	}
#endif
	V->p2 = INIT_AS_HASH_TABLE;
	
	WRValue* I = &(index->singleValue());
	doIndex_I_E( c, I, V, target );
}

//------------------------------------------------------------------------------
void doIndex_E_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	WRValue* I = &(index->deref());

	if ( I->type > WR_FLOAT )
	{
		WRValue* V = &(value->deref());
		
		if ( !EXPECTS_HASH_INDEX(V->xtype) )
		{
			target->init();
		}
		else
		{
			doIndexHash( I, V, target );
		}
	}
	else
	{
		doIndex_I_E( c, I, value, target );
	}
}


void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }

#ifdef WRENCH_REALLY_COMPACT

void doIndex_X_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->type<<2)|value->r->type](c, index, value->r, target); }
void doIndex_R_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->type](c, index->r, value, target); }

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_X_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_X,     doIndex_R_X,     doIndex_R_R,     doIndex_R_X, 
	doIndex_E_I,     doIndex_E_I,     doIndex_X_R,     doIndex_E_E,
};


#else

void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_INT<<2)|value->r->type](c, index, value->r, target); }
void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_INT](c, index->r, value, target); }
void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_FLOAT](c, index->r, value, target); }
void doIndex_R_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_EX](c, index->r, value, target); }
void doIndex_E_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_EX<<2)|value->r->type](c, index, value->r, target); }

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_I_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_I,     doIndex_R_F,     doIndex_R_R,     doIndex_R_E, 
	doIndex_E_I,     doIndex_E_I,     doIndex_E_R,     doIndex_E_E,
};

#endif
