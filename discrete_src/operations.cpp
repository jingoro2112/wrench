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
WRGCObject* wr_growValueArray( WRGCObject* va, int newSize )
{
	WRGCObject* newArray = (WRGCObject*)malloc( sizeof(WRGCObject) );
	newArray->init( newSize + 1, (WRGCObjectType)va->m_type );
	
	newArray->m_next = va->m_next;
	va->m_next = newArray;

	int size_of = (va->m_type == SV_CHAR) ? 1 : sizeof(WRValue);

	int size_el = va->m_size * size_of;
	memcpy(newArray->m_Cdata, va->m_Cdata, size_el);
	memset(newArray->m_Cdata + size_el, 0, (newArray->m_size * size_of) - size_el);

	return newArray;
}

//------------------------------------------------------------------------------
WRValue& WRValue::singleValue() const
{
	static WRValue temp;

	if ( (temp = deref()).type > WR_FLOAT )
	{
		temp.ui = temp.getHash();
		temp.p2 = INIT_AS_INT;
	}

	return temp;
}

//------------------------------------------------------------------------------
WRValue& WRValue::deref() const
{
	static WRValue temp;
	
	if ( type == WR_REF )
	{
		return r->deref();
	}

	if ( !IS_ARRAY_MEMBER(xtype) )
	{
		return const_cast<WRValue&>(*this);
	}

	temp.p2 = INIT_AS_INT;
	unsigned int s = DECODE_ARRAY_ELEMENT_FROM_P2(p2);

	if ( IS_RAW_ARRAY(r->xtype) )
	{
		temp.ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(r->p2))) ? (uint32_t)(unsigned char)(r->c[s]) : 0;
	}
	else if ( s < r->va->m_size )
	{
		if ( r->va->m_type == SV_VALUE )
		{
			return r->va->m_Vdata[s];
		}
		else
		{
			temp.ui = (uint32_t)(unsigned char)r->va->m_Cdata[s];
		}
	}

	return temp;
}

//------------------------------------------------------------------------------
uint32_t WRValue::getHashEx() const
{
	// QUICKLY return the easy answers, thats why this code looks a bit
	// convoluted
	if ( type == WR_REF )
	{
		return r->getHash();
	}

	if ( IS_ARRAY_MEMBER(xtype) )
	{
		return deref().getHash();
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		return wr_hash( va->m_Cdata, va->m_size );
	}

	return 0;
}


//------------------------------------------------------------------------------
void wr_valueToArray( const WRValue* array, WRValue* value )
{
	uint32_t s = DECODE_ARRAY_ELEMENT_FROM_P2(array->p2);

	if ( IS_RAW_ARRAY(array->r->xtype ) )
	{
		if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(array->r->p2)) )
		{
			array->r->c[s] = value->ui;
		}
	}
	else if ( array->r->va->m_type == SV_CHAR)
	{
		if ( s < array->r->va->m_size ) 
		{
			array->r->va->m_Cdata[s] = value->ui;
		}
	}
	else
	{
		if ( s >= array->r->va->m_size )
		{
			array->r->va = wr_growValueArray(array->r->va, s);
		}

		WRValue* V = array->r->va->m_Vdata + s;
		wr_assign[(V->type<<2)+value->type](V, value);
	}
}

//------------------------------------------------------------------------------
void wr_countOfArrayElement( WRValue* array, WRValue* target )
{
	array = &array->deref();
				
	if ( IS_EXARRAY_TYPE(array->xtype) )
	{
		target->i = array->va->m_size;
	}
	else
	{
		target->i = 0;
	}

	target->p2 = INIT_AS_INT;
}

//------------------------------------------------------------------------------
// put from into TO as a hash table, if to is not a hash table, make it
// one, leave the result in target
void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table )
{
	if ( value->type == WR_REF )
	{
		wr_assignToHashTable( c, index, value->r, table );
		return;
	}
	
	if ( table->type == WR_REF )
	{
		wr_assignToHashTable( c, index, value, table->r );
		return;
	}

	if ( index->type == WR_REF )
	{
		wr_assignToHashTable( c, index->r, value, table );
		return;
	}

	if ( table->xtype != WR_EX_HASH_TABLE )
	{
		table->p2 = INIT_AS_HASH_TABLE;
		table->va = c->getSVA( 0, SV_HASH_TABLE, false );
	}

	WRValue *entry = (WRValue *)table->va->get( index->getHash() );

	*entry++ = *value;
	*entry = *index;
}

//------------------------------------------------------------------------------
bool doLogicalNot_X( WRValue* value ) { return value->i == 0; } // float also uses 0x0000000 as 'zero'
bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
bool doLogicalNot_E( WRValue* value )
{
	WRValue& V = value->singleValue();
	return wr_LogicalNot[ V.type ]( &V );
}
WRReturnSingleFunc wr_LogicalNot[4] = 
{
	doLogicalNot_X,  doLogicalNot_X,  doLogicalNot_R,  doLogicalNot_E
};


//------------------------------------------------------------------------------
void doNegate_I( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = -value->i; }
void doNegate_F( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = -value->f; }
void doNegate_E( WRValue* value, WRValue* target )
{
	WRValue& V = value->singleValue();
	return wr_negate[ V.type ]( &V, target );
}
void doNegate_R( WRValue* value, WRValue* target )
{
	wr_negate[ value->r->type ]( value->r, target );
}

WRSingleTargetFunc wr_negate[4] = 
{
	doNegate_I,  doNegate_F,  doNegate_R,  doNegate_E
};

//------------------------------------------------------------------------------
uint32_t doBitwiseNot_I( WRValue* value ) { return ~value->ui; }
uint32_t doBitwiseNot_F( WRValue* value ) { return 0; }
uint32_t doBitwiseNot_R( WRValue* value ) { return wr_bitwiseNot[ value->r->type ]( value->r ); }
uint32_t doBitwiseNot_E( WRValue* value )
{
	WRValue& V = value->singleValue();
	return wr_bitwiseNot[ V.type ]( &V );
}
WRUint32Call wr_bitwiseNot[4] = 
{
	doBitwiseNot_I,  doBitwiseNot_F,  doBitwiseNot_R,  doBitwiseNot_E
};

//------------------------------------------------------------------------------
void pushIterator_X( WRValue* on, WRValue* to ) { on->init(); }
void pushIterator_R( WRValue* on, WRValue* to ) { wr_pushIterator[on->r->type]( on->r, to ); }
void pushIterator_E( WRValue* on, WRValue* to )
{
	if ( on->xtype == WR_EX_ARRAY || on->xtype == WR_EX_HASH_TABLE )
	{
		to->va = on->va;
		to->p2 = INIT_AS_ITERATOR;
	}
}

WRVoidFunc wr_pushIterator[4] =
{
	pushIterator_X, pushIterator_X, pushIterator_R, pushIterator_E
};

//------------------------------------------------------------------------------
void doAssign_X_E( WRValue* to, WRValue* from )
{
	if ( IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();
		wr_assign[(WR_EX<<2)+V.type](to, &V);
	}
	else
	{
		*to = *from;
	}
}

void doAssign_E_X( WRValue* to, WRValue* from )
{
	if ( IS_ARRAY_MEMBER(to->xtype) )
	{
		wr_valueToArray( to, from );
	}
	else
	{
		*to = *from;
	}
}

void doAssign_E_E( WRValue* to, WRValue* from )
{
	if ( IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();
		wr_assign[(WR_EX<<2)+V.type](to, &V);
		return;
	}
	else if ( IS_ARRAY_MEMBER(to->xtype)
			  && IS_EXARRAY_TYPE(to->r->xtype)
			  && (to->r->va->m_type == SV_VALUE) )
	{
		unsigned int index = DECODE_ARRAY_ELEMENT_FROM_P2(to->p2);

		if ( index > to->r->va->m_size )
		{
			if ( to->r->va->m_skipGC )
			{
				return;
			}

			to->r->va = wr_growValueArray( to->r->va, index );	
		}

		to->r->va->m_Vdata[index] = *from;
		return;
	}

	*to = *from;
}


//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================

#ifdef WRENCH_COMPACT


void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
void doAssign_R_X( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->type](to->r, from); }
void doAssign_X_R( WRValue* to, WRValue* from ) { wr_assign[(to->type<<2)|from->r->type](to, from->r); }
void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_R_X,  doAssign_R_X,  doAssign_R_R,  doAssign_R_X,
	doAssign_E_X,  doAssign_E_X,  doAssign_X_R,  doAssign_E_E,
};


extern bool CompareEQI( int a, int b );
extern bool CompareEQF( float a, float b );

//------------------------------------------------------------------------------
void unaryPost_E( WRValue* value, WRValue* stack, int add )
{
	WRValue& V = value->singleValue();
	WRValue temp;
	m_unaryPost[ V.type ]( &V, &temp, add );
	wr_valueToArray( value, &V );
	*stack = temp;
}
void unaryPost_I( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_INT; stack->i = value->i; value->i += add; }
void unaryPost_R( WRValue* value, WRValue* stack, int add ) { m_unaryPost[ value->r->type ]( value->r, stack, add ); }
void unaryPost_F( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f; value->f += add; }
WRVoidPlusFunc m_unaryPost[4] = 
{
	unaryPost_I,  unaryPost_F,  unaryPost_R, unaryPost_E
};

void FuncAssign_R_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	wr_FuncAssign[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
void FuncAssign_E_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue& V = to->singleValue();
	
	wr_FuncAssign[(V.type<<2)|from->type]( &V, from, intCall, floatCall );
	
	wr_valueToArray( to, &V );
	*from = V;
}
void FuncAssign_E_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	if ( IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();

		wr_FuncAssign[(WR_EX<<2)|V.type]( to, &V, intCall, floatCall );
		*from = to->deref();
	}
	else if ( intCall == wr_addI
			  && IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !to->va->m_skipGC
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		char* t = to->va->m_SCdata;
		to->va->m_SCdata = (char*)malloc( to->va->m_size + from->va->m_size + 1 );
		memcpy( to->va->m_SCdata, t, to->va->m_size );
		memcpy( to->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size + 1 );
		to->va->m_size = to->va->m_size + from->va->m_size;
		free( t );
	}
}
void FuncAssign_X_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue& V = from->singleValue();
	wr_FuncAssign[(to->type<<2)|V.type](to, &V, intCall, floatCall);
	*from = *to;
}
void FuncAssign_E_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}

void FuncAssign_R_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue temp = *from->r; wr_FuncAssign[(to->r->type<<2)|temp.type](to->r, &temp, intCall, floatCall); *from = *to->r;
}

void FuncAssign_R_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); *from = *to->r;
}

void FuncAssign_X_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); *from = *to;
}

void FuncAssign_F_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->f = floatCall( to->f, from->f );
}

void FuncAssign_I_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->i = intCall( to->i, from->i );
}

void FuncAssign_I_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->p2 = INIT_AS_FLOAT;
	to->f = floatCall( (float)to->i, from->f );
}
void FuncAssign_F_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->f = floatCall( to->f, (float)from->i );
}

WRFuncAssignFunc wr_FuncAssign[16] = 
{
	FuncAssign_I_I,  FuncAssign_I_F,  FuncAssign_X_R,  FuncAssign_X_E,
	FuncAssign_F_I,  FuncAssign_F_F,  FuncAssign_X_R,  FuncAssign_X_E,
	FuncAssign_R_X,  FuncAssign_R_X,  FuncAssign_R_R,  FuncAssign_R_E,
	FuncAssign_E_X,  FuncAssign_E_X,  FuncAssign_E_R,  FuncAssign_E_E,
};


//------------------------------------------------------------------------------
void FuncBinary_E_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	WRValue& V = to->singleValue();
	wr_funcBinary[(V.type<<2)|from->type](&V, from, target, intCall, floatCall);
}
void FuncBinary_E_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_ARRAY_MEMBER(to->xtype) && IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue V1 = to->deref();
		WRValue& V2 = from->deref();
		wr_funcBinary[(V1.type<<2)|V2.type](&V1, &V2, target, intCall, floatCall );
	}
	else if ( intCall == wr_addI
			  && IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !to->va->m_skipGC
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		target->p2 = INIT_AS_ARRAY;
		target->va = from->va->m_creatorContext->getSVA( from->va->m_size + to->va->m_size, SV_CHAR, false );
		memcpy( target->va->m_SCdata, from->va->m_SCdata, from->va->m_size );
		memcpy( target->va->m_SCdata + from->va->m_size, to->va->m_SCdata, to->va->m_size );
	}
}
void FuncBinary_X_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	WRValue& V = from->singleValue();
	wr_funcBinary[(to->type<<2)|V.type](to, &V, target, intCall, floatCall);
}
void FuncBinary_E_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(WR_EX<<2)|from->r->type](to, from->r, target, intCall, floatCall );
}

void FuncBinary_R_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(to->r->type<<2)|WR_EX](to->r, from, target, intCall, floatCall );
}

void FuncBinary_X_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->type<<2)+from->r->type](to, from->r, target, intCall, floatCall);
}

void FuncBinary_R_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->type](to->r, from, target, intCall, floatCall);
}

void FuncBinary_R_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target, intCall, floatCall);
}

void FuncBinary_I_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_INT; target->i = intCall( to->i, from->i );
}

void FuncBinary_I_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( (float)to->i, from->f );
}

void FuncBinary_F_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( to->f, (float)from->i );
}

void FuncBinary_F_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( to->f, from->f );
}

WRTargetCallbackFunc wr_funcBinary[16] = 
{
	FuncBinary_I_I,  FuncBinary_I_F,  FuncBinary_X_R,  FuncBinary_X_E,
	FuncBinary_F_I,  FuncBinary_F_F,  FuncBinary_X_R,  FuncBinary_X_E,
	FuncBinary_R_X,  FuncBinary_R_X,  FuncBinary_R_R,  FuncBinary_R_E,
	FuncBinary_E_X,  FuncBinary_E_X,  FuncBinary_E_R,  FuncBinary_E_E,
};



bool Compare_E_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue V1 = to->singleValue();
	WRValue& V2 = from->singleValue();
	return wr_Compare[(V1.type<<2)|V2.type](&V1, &V2, intCall, floatCall);
}
bool Compare_E_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue& V = to->singleValue();
	return wr_Compare[(V.type<<2)|from->type](&V, from, intCall, floatCall);
}
bool Compare_X_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue& V = from->singleValue();
	return wr_Compare[(to->type<<2)|V.type](to, &V, intCall, floatCall );
}
bool Compare_R_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall); }
bool Compare_E_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall); }
bool Compare_R_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->r->type](to->r, from->r, intCall, floatCall); }
bool Compare_R_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); }
bool Compare_X_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); }
bool Compare_I_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return intCall( to->i, from->i ); }
bool Compare_I_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( (float)to->i, from->f); }
bool Compare_F_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, (float)from->i); }
bool Compare_F_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, from->f); }
WRBoolCallbackReturnFunc wr_Compare[16] = 
{
	Compare_I_I, Compare_I_F, Compare_X_R, Compare_X_E,
	Compare_F_I, Compare_F_F, Compare_X_R, Compare_X_E,
	Compare_R_X, Compare_R_X, Compare_R_R, Compare_R_E,
	Compare_E_X, Compare_E_X, Compare_E_R, Compare_E_E,
};

//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================
#else 

// NOT COMPACT

void doAssign_R_E( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_EX](to->r, from); }
void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
void doAssign_E_R( WRValue* to, WRValue* from ) { wr_assign[(WR_EX<<2)|from->r->type](to, from->r); }
void doAssign_R_I( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_INT](to->r, from); }
void doAssign_R_F( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_FLOAT](to->r, from); }
void doAssign_I_R( WRValue* to, WRValue* from ) { wr_assign[(WR_INT<<2)|from->r->type](to, from->r); }
void doAssign_F_R( WRValue* to, WRValue* from ) { wr_assign[(WR_FLOAT<<2)|from->r->type](to, from->r); }
void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_I_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_F_R,  doAssign_X_E,
	doAssign_R_I,  doAssign_R_F,  doAssign_R_R,  doAssign_R_E,
	doAssign_E_X,  doAssign_E_X,  doAssign_E_R,  doAssign_E_E,
};


void doVoidFuncBlank( WRValue* to, WRValue* from ) {}

#define X_INT_ASSIGN( NAME, OPERATION ) \
void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	NAME##Assign[(WR_EX<<2)|from->r->type]( to, from->r );\
}\
void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	NAME##Assign[(to->r->type<<2)|WR_EX](to->r, from);\
}\
void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_INT]( &V, from );\
	\
	wr_valueToArray( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	WRValue& V = from->singleValue();\
	\
	NAME##Assign[(WR_EX<<2)+V.type]( to, &V );\
	*from = to->deref();\
}\
void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_INT<<2)+V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
void NAME##Assign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_INT<<2)+temp.type](to, &temp); *from = *to; }\
void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,   doVoidFuncBlank,   NAME##Assign_I_R,  NAME##Assign_I_E,\
	doVoidFuncBlank,    doVoidFuncBlank,    doVoidFuncBlank,   doVoidFuncBlank,\
	NAME##Assign_R_I,   doVoidFuncBlank,   NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,   doVoidFuncBlank,   NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_INT_ASSIGN( wr_Mod, % );
X_INT_ASSIGN( wr_OR, | );
X_INT_ASSIGN( wr_AND, & );
X_INT_ASSIGN( wr_XOR, ^ );
X_INT_ASSIGN( wr_RightShift, >> );
X_INT_ASSIGN( wr_LeftShift, << );


#define X_ASSIGN( NAME, OPERATION ) \
void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_INT]( &V, from );\
	\
	wr_valueToArray( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_F( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_FLOAT]( &V, from );\
	\
	wr_valueToArray( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	WRValue& V = from->singleValue();\
	\
	NAME##Assign[(WR_EX<<2)|V.type]( to, &V );\
	*from = to->deref();\
}\
void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_INT<<2)|V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_F_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_FLOAT<<2)|V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	NAME##Assign[(WR_EX<<2)|from->r->type]( to, from->r );\
}\
void NAME##Assign_R_E( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_EX](to->r, from); *from = *to->r; } \
void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
void NAME##Assign_R_F( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }\
void NAME##Assign_I_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }\
void NAME##Assign_F_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }\
void NAME##Assign_F_F( WRValue* to, WRValue* from ) { to->f OPERATION##= from->f; }\
void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
void NAME##Assign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i OPERATION from->f; }\
void NAME##Assign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f OPERATION##= (float)from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,  NAME##Assign_I_F,  NAME##Assign_I_R,  NAME##Assign_I_E,\
	NAME##Assign_F_I,  NAME##Assign_F_F,  NAME##Assign_F_R,  NAME##Assign_F_E,\
	NAME##Assign_R_I,  NAME##Assign_R_F,  NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,  NAME##Assign_E_F,  NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_ASSIGN( wr_Subtract, - );
//X_ASSIGN( wr_Add, + );
X_ASSIGN( wr_Multiply, * );
X_ASSIGN( wr_Divide, / );





void wr_AddAssign_E_I( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	
	wr_AddAssign[(V.type<<2)|WR_INT]( &V, from );
	
	wr_valueToArray( to, &V );
	*from = V;
}

void wr_AddAssign_E_F( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();

	wr_AddAssign[(V.type<<2)|WR_FLOAT]( &V, from );

	wr_valueToArray( to, &V );
	*from = V;
}
void wr_AddAssign_E_E( WRValue* to, WRValue* from ) 
{
	if ( IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();

		wr_AddAssign[(WR_EX<<2)|V.type]( to, &V );
		*from = to->deref();
	}
	else if ( IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !to->va->m_skipGC
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		char* t = to->va->m_SCdata;
		to->va->m_SCdata = (char*)malloc( to->va->m_size + from->va->m_size + 1 );
		memcpy( to->va->m_SCdata, t, to->va->m_size );
		memcpy( to->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size + 1 );
		to->va->m_size = to->va->m_size + from->va->m_size;
		free( t );
	}
}
void wr_AddAssign_I_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_AddAssign[(WR_INT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_AddAssign_F_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_AddAssign[(WR_FLOAT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_AddAssign_E_R( WRValue* to, WRValue* from )
{
	wr_AddAssign[(WR_EX<<2)|from->r->type]( to, from );
}
void wr_AddAssign_R_E( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_EX](to->r, from); *from = *to->r; }
void wr_AddAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }
void wr_AddAssign_R_I( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }
void wr_AddAssign_R_F( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }
void wr_AddAssign_I_R( WRValue* to, WRValue* from ) { wr_AddAssign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }
void wr_AddAssign_F_R( WRValue* to, WRValue* from ) { wr_AddAssign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }
void wr_AddAssign_F_F( WRValue* to, WRValue* from ) { to->f += from->f; }
void wr_AddAssign_I_I( WRValue* to, WRValue* from ) { to->i += from->i; }
void wr_AddAssign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i + from->f; }
void wr_AddAssign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f += (float)from->i; }
WRVoidFunc wr_AddAssign[16] = 
{
	wr_AddAssign_I_I,  wr_AddAssign_I_F,  wr_AddAssign_I_R,  wr_AddAssign_I_E,
	wr_AddAssign_F_I,  wr_AddAssign_F_F,  wr_AddAssign_F_R,  wr_AddAssign_F_E,
	wr_AddAssign_R_I,  wr_AddAssign_R_F,  wr_AddAssign_R_R,  wr_AddAssign_R_E,
	wr_AddAssign_E_I,  wr_AddAssign_E_F,  wr_AddAssign_E_R,  wr_AddAssign_E_E,
};


//------------------------------------------------------------------------------
#define X_BINARY( NAME, OPERATION ) \
void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_INT](&V, from, target);\
}\
void NAME##Binary_E_F( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_FLOAT](&V, from, target);\
}\
void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	NAME##Binary[(V1.type<<2)|V2.type](&V1, &V2, target);\
}\
void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_INT<<2)|V.type](to, &V, target);\
}\
void NAME##Binary_F_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_FLOAT<<2)|V.type](to, &V, target);\
}\
void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_EX]( to->r, from, target); }\
void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_EX<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_R_F( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }\
void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
void NAME##Binary_F_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
void NAME##Binary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i OPERATION from->f; }\
void NAME##Binary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION (float)from->i; }\
void NAME##Binary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION from->f; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I,  NAME##Binary_I_F,  NAME##Binary_I_R,  NAME##Binary_I_E,\
	NAME##Binary_F_I,  NAME##Binary_F_F,  NAME##Binary_F_R,  NAME##Binary_F_E,\
	NAME##Binary_R_I,  NAME##Binary_R_F,  NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I,  NAME##Binary_E_F,  NAME##Binary_E_R,  NAME##Binary_E_E,\
};\

//X_BINARY( wr_Addition, + );  -- broken out so strings work
X_BINARY( wr_Multiply, * );
X_BINARY( wr_Subtract, - );
X_BINARY( wr_Divide, / );



void wr_AdditionBinary_E_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_AdditionBinary[(V.type<<2)|WR_INT](&V, from, target);
}
void wr_AdditionBinary_E_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_AdditionBinary[(V.type<<2)|WR_FLOAT](&V, from, target);
}
void wr_AdditionBinary_E_E( WRValue* to, WRValue* from, WRValue* target )
{
	if ( IS_ARRAY_MEMBER(to->xtype) && IS_ARRAY_MEMBER(from->xtype) )
	{
		WRValue V1 = to->deref();
		WRValue& V2 = from->deref();
		wr_AdditionBinary[(V1.type<<2)|V2.type](&V1, &V2, target);
	}
	else if ( IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !to->va->m_skipGC
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		target->p2 = INIT_AS_ARRAY;
		target->va = from->va->m_creatorContext->getSVA( from->va->m_size + to->va->m_size, SV_CHAR, false );
		memcpy( target->va->m_SCdata, from->va->m_SCdata, from->va->m_size );
		memcpy( target->va->m_SCdata + from->va->m_size, to->va->m_SCdata, to->va->m_size );
	}
}
void wr_AdditionBinary_I_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_AdditionBinary[(WR_INT<<2)|V.type](to, &V, target);
}
void wr_AdditionBinary_F_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_AdditionBinary[(WR_FLOAT<<2)|V.type](to, &V, target);
}
void wr_AdditionBinary_R_E( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_EX]( to->r, from, target); }
void wr_AdditionBinary_E_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_EX<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_INT<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }
void wr_AdditionBinary_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }
void wr_AdditionBinary_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_INT](to->r, from, target); }
void wr_AdditionBinary_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i + from->i; }
void wr_AdditionBinary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i + from->f; }
void wr_AdditionBinary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f + (float)from->i; }
void wr_AdditionBinary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f + from->f; }
WRTargetFunc wr_AdditionBinary[16] = 
{
	wr_AdditionBinary_I_I,  wr_AdditionBinary_I_F,  wr_AdditionBinary_I_R,  wr_AdditionBinary_I_E,
	wr_AdditionBinary_F_I,  wr_AdditionBinary_F_F,  wr_AdditionBinary_F_R,  wr_AdditionBinary_F_E,
	wr_AdditionBinary_R_I,  wr_AdditionBinary_R_F,  wr_AdditionBinary_R_R,  wr_AdditionBinary_R_E,
	wr_AdditionBinary_E_I,  wr_AdditionBinary_E_F,  wr_AdditionBinary_E_R,  wr_AdditionBinary_E_E,
};




void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}

//------------------------------------------------------------------------------
#define X_INT_BINARY( NAME, OPERATION ) \
void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_INT](&V, from, target);\
}\
void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	NAME##Binary[(V1.type<<2)|V2.type](&V1, &V2, target);\
}\
void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_INT<<2)+V.type](to, &V, target);\
}\
void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_EX)|from->r->type](to, from->r, target); }\
void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_EX]( to->r, from, target); }\
void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I, doTargetFuncBlank,   NAME##Binary_I_R,  NAME##Binary_I_E,\
   doTargetFuncBlank, doTargetFuncBlank,  doTargetFuncBlank, doTargetFuncBlank,\
	NAME##Binary_R_I, doTargetFuncBlank,   NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I, doTargetFuncBlank,   NAME##Binary_E_R,  NAME##Binary_E_E,\
};\


X_INT_BINARY( wr_LeftShift, << );
X_INT_BINARY( wr_RightShift, >> );
X_INT_BINARY( wr_Mod, % );
X_INT_BINARY( wr_AND, & );
X_INT_BINARY( wr_OR, | );
X_INT_BINARY( wr_XOR, ^ );





#define X_COMPARE( NAME, OPERATION ) \
bool NAME##_E_E( WRValue* to, WRValue* from )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	return NAME[(V1.type<<2)|V2.type](&V1, &V2);\
}\
bool NAME##_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	return NAME[(V.type<<2)|WR_INT](&V, from);\
}\
bool NAME##_E_F( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	return NAME[(V.type<<2)|WR_FLOAT](&V, from);\
}\
bool NAME##_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	return NAME[(WR_INT<<2)|V.type](to, &V);\
}\
bool NAME##_F_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	return NAME[(WR_FLOAT<<2)|V.type](to, &V);\
}\
bool NAME##_R_E( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_EX](to->r, from); }\
bool NAME##_E_R( WRValue* to, WRValue* from ) { return NAME[(WR_EX<<2)|from->r->type](to, from->r); }\
bool NAME##_R_R( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|from->r->type](to->r, from->r); }\
bool NAME##_R_I( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_INT](to->r, from); }\
bool NAME##_R_F( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_FLOAT](to->r, from); }\
bool NAME##_I_R( WRValue* to, WRValue* from ) { return NAME[(WR_INT<<2)+from->r->type](to, from->r); }\
bool NAME##_F_R( WRValue* to, WRValue* from ) { return NAME[(WR_FLOAT<<2)+from->r->type](to, from->r); }\
bool NAME##_I_I( WRValue* to, WRValue* from ) { return to->i OPERATION from->i; }\
bool NAME##_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->f OPERATION from->f; }\
bool NAME##_F_I( WRValue* to, WRValue* from ) { return to->f OPERATION (float)from->i; }\
bool NAME##_F_F( WRValue* to, WRValue* from ) { return to->f OPERATION from->f; }\
WRReturnFunc NAME[16] = \
{\
	NAME##_I_I, NAME##_I_F, NAME##_I_R, NAME##_I_E,\
	NAME##_F_I, NAME##_F_F, NAME##_F_R, NAME##_F_E,\
	NAME##_R_I, NAME##_R_F, NAME##_R_R, NAME##_R_E,\
	NAME##_E_I, NAME##_E_F, NAME##_E_R, NAME##_E_E,\
};\

X_COMPARE( wr_CompareGT, > );
X_COMPARE( wr_CompareLT, < );
X_COMPARE( wr_LogicalAND, && );
X_COMPARE( wr_LogicalOR, || );
X_COMPARE( wr_CompareEQ, == );

//------------------------------------------------------------------------------
#define X_UNARY_PRE( NAME, OPERATION ) \
void NAME##_E( WRValue* value )\
{\
	WRValue& V = value->singleValue();\
	NAME [ V.type ]( &V );\
	wr_valueToArray( value, &V );\
}\
void NAME##_I( WRValue* value ) { OPERATION value->i; }\
void NAME##_F( WRValue* value ) { OPERATION value->f; }\
void NAME##_R( WRValue* value ) { NAME [ value->r->type ]( value->r ); *value = *value->r; }\
WRUnaryFunc NAME[4] = \
{\
	NAME##_I, NAME##_F, NAME##_R, NAME##_E\
};\

X_UNARY_PRE( wr_preinc, ++ );
X_UNARY_PRE( wr_predec, -- );

//------------------------------------------------------------------------------
#define X_UNARY_POST( NAME, OPERATION ) \
void NAME##_E( WRValue* value, WRValue* stack )\
{\
	WRValue& V = value->singleValue();\
	WRValue temp; \
	NAME [ V.type ]( &V, &temp );\
	wr_valueToArray( value, &V );\
	*stack = temp; \
}\
void NAME##_I( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_INT; stack->i = value->i OPERATION; }\
void NAME##_R( WRValue* value, WRValue* stack ) { NAME[ value->r->type ]( value->r, stack ); }\
void NAME##_F( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f OPERATION; }\
WRVoidFunc NAME[4] = \
{\
	NAME##_I,  NAME##_F,  NAME##_R, NAME##_E\
};\

X_UNARY_POST( wr_postinc, ++ );
X_UNARY_POST( wr_postdec, -- );

#endif
