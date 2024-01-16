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
void elementToTarget( const uint32_t index, WRValue* target, WRValue* value )
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
		target->p2 = INIT_AS_ARRAY_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( index );
	}
}

//------------------------------------------------------------------------------
void doIndexHash( WRValue* value, WRValue* target, uint32_t hash )
{
	if ( value->xtype == WR_EX_HASH_TABLE ) 
	{
		if ( IS_EX_SINGLE_CHAR_RAW_P2( (target->r = (WRValue*)value->va->get(hash))->p2) )
		{
			target->p2 = INIT_AS_ARRAY_MEMBER;
		}
		else
		{
			target->p2 = INIT_AS_REF;
		}
	}
	else // naming an element of a struct "S.element"
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( (uint32_t)READ_32_FROM_PC(table) == hash )
		{
			target->p2 = INIT_AS_REF;
			target->p = ((WRValue*)(value->va->m_data)) + READ_8_FROM_PC(table + 4);
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
	value->p2 = INIT_AS_ARRAY;
	value->va = c->getSVA( index->ui + 1, SV_VALUE, true );
	
	elementToTarget( index->ui, target, value );
}

//------------------------------------------------------------------------------
void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	value = &value->deref();
	
	if (EXPECTS_HASH_INDEX(value->xtype))
	{
		doIndexHash( value, target, index->ui );
		return;
	}
	else if ( IS_RAW_ARRAY(value->xtype) )
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
			value->p2 = INIT_AS_ARRAY;
			value->va = c->getSVA( index->ui+1, SV_VALUE, true );
		}
		else
		{
			value->p2 = INIT_AS_HASH_TABLE;
			value->va = c->getSVA( 0, SV_HASH_TABLE, false );
			doIndexHash( value, target, I->getHash() );
			return;
		}
	}
	else if ( index->ui >= value->va->m_size )
	{
		if ( value->va->m_skipGC )
		{
boundsFailed:
			target->init();
			return;
		}

		value->va = wr_growValueArray( value->va, index->ui );
	}

	elementToTarget( index->ui, target, value );
}

//------------------------------------------------------------------------------
void doIndex_E_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	WRValue* V = &value->deref();

	// nope, make it one of this size and return a ref
	WRValue* I = &index->deref();
	if ( I->type == WR_INT )
	{
		wr_index[WR_INT<<2 | V->type](c, I, V, target);
	}
	else
	{
		if ( !IS_HASH_TABLE(V->xtype) )
		{
			V->p2 = INIT_AS_HASH_TABLE;
			V->va = c->getSVA( 0, SV_HASH_TABLE, false );
		}
		doIndexHash( V, target, I->getHash() );
	}
}

//------------------------------------------------------------------------------
void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target);
}

void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}


#ifdef WRENCH_REALLY_COMPACT

void doIndex_X_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->type<<2)|value->r->type](c, index, value->r, target); }
void doIndex_R_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->type](c, index->r, value, target); }

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_X_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_X,     doIndex_R_X,     doIndex_R_R,     doIndex_R_X, 
	doVoidIndexFunc, doVoidIndexFunc, doIndex_X_R,     doIndex_E_E,
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
	doVoidIndexFunc, doVoidIndexFunc, doIndex_E_R,     doIndex_E_E,
};

#endif
