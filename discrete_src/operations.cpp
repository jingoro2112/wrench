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
void* WRValue::array( unsigned int* len, char* arrayType ) const
{
	if ( type == WR_REF )
	{
		return r->array( len, arrayType );
	}

	if ( xtype != WR_EX_ARRAY )
	{
		return 0;
	}

	if ( arrayType )
	{
		*arrayType = va->m_type;
	}

	if ( len )
	{
		*len = va->m_size;
	}

	return va->m_data;
}

//------------------------------------------------------------------------------
const char* WRValue::c_str( unsigned int* len ) const
{
	char arrayType = 0;
	void* ret = array( len, &arrayType );
	return (arrayType == SV_CHAR) ? (char*)ret : 0;
}

//------------------------------------------------------------------------------
WRGCObject* growValueArray( WRGCObject* va, int newSize )
{
	WRGCObject* newArray = (WRGCObject*)malloc( sizeof(WRGCObject) );
	newArray->init( newSize + 1, (WRGCObjectType)va->m_type );
	
	newArray->m_next = va->m_next;
	va->m_next = newArray;

	int size_of = sizeof(WRValue);
	if ( va->m_type == SV_CHAR )
	{
		size_of = 1;
	}
	
	memcpy( newArray->m_Cdata, va->m_Cdata, va->m_size * size_of );
	memset( newArray->m_Cdata + (va->m_size*size_of), 0, (newArray->m_size - va->m_size) * size_of );
	return newArray;
}

//------------------------------------------------------------------------------
void WRValue::arrayValue( WRValue* val ) const
{
	unsigned int s = ARRAY_ELEMENT_FROM_P2(p2);

	if ( IS_RAW_ARRAY(r->xtype) )
	{
		val->p2 = INIT_AS_INT;
		val->ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(r->p2))) ? (uint32_t)(unsigned char)(r->c[s]) : 0;
	}
	else if ( r->va->m_type == SV_VALUE )
	{
		if ( s < r->va->m_size )
		{
			*val = r->va->m_Vdata[s];
		}
	}
	else if ( r->va->m_type == SV_CHAR )
	{
		val->p2 = INIT_AS_INT;
		val->ui = (s < r->va->m_size) ? (uint32_t)(unsigned char)r->va->m_Cdata[s] : 0;
	}
	else
	{
		val->init();
	}
}

//------------------------------------------------------------------------------
void wr_arrayToValue( const WRValue* array, WRValue* value, int index )
{
	uint32_t s = index == -1 ? ARRAY_ELEMENT_FROM_P2(array->p2) : index;

	if ( IS_RAW_ARRAY(array->r->xtype) )
	{
		value->p2 = INIT_AS_INT;
		value->ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(array->r->p2))) ? (uint32_t)(unsigned char)(array->r->c[s]) : 0;
	}
	else if( array->r->va->m_type == SV_VALUE )
	{
		if ( s >= array->r->va->m_size )
		{
			if ( array->r->va->m_skipGC )
			{
				value->init();
				return;
			}

			array->r->va = growValueArray( array->r->va, s );
		}
		*value = array->r->va->m_Vdata[s];
	}
	else if ( array->r->va->m_type == SV_CHAR )
	{
		value->p2 = INIT_AS_INT;
		value->i = (s < array->r->va->m_size) ? (uint32_t)(unsigned char)(array->r->va->m_Cdata[s]) : 0;
	}
	else
	{
		value->init();
	}
}

//------------------------------------------------------------------------------
uint32_t WRValue::getHashEx() const
{
	if ( type <= (int)WR_FLOAT )
	{
		return ui;
	}
	else if ( type == WR_REF )
	{
		return r->getHashEx();
	}

	if ( IS_REFARRAY(xtype) )
	{
		WRValue element;
		wr_arrayToValue( this, &element );
		return element.getHash();
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
	uint32_t s = ARRAY_ELEMENT_FROM_P2(array->p2);

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
			array->r->va = growValueArray(array->r->va, s);
		}
		WRValue* V = array->r->va->m_Vdata + s;
		wr_assign[(V->type<<2)+value->type](V, value);
	}
}

//------------------------------------------------------------------------------
void wr_countOfArrayElement( WRValue* array, WRValue* target )
{
#ifdef WRENCH_COMPACT
	if ( array->type == WR_REF )
	{
		return wr_countOfArrayElement( array->r, target );
	}
	else
#endif
	if ( IS_EXARRAY_TYPE(array->xtype) )
	{
		if ( IS_REFARRAY(array->xtype) )
		{
			wr_arrayToValue( array, target );
			wr_countOfArrayElement( target, target );
		}
		else
		{
			target->i = array->va->m_size;
			target->p2 = INIT_AS_INT;
		}
	}
}

//------------------------------------------------------------------------------
// put from into TO as a hash table, if to is not a hash table, make it
// one, leave the result in target
void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table )
{
	if ( value->type == WR_REF )
	{
		value = value->r;
	}
	if ( table->type == WR_REF )
	{
		table = table->r;
	}

	if ( table->xtype != WR_EX_HASH_TABLE )
	{
		c->gc( value + 1 );
		
		table->p2 = INIT_AS_HASH_TABLE;
		table->va = c->getSVA( 0, SV_HASH_TABLE, false );
	}

	WRValue *entry = (WRValue *)table->va->get( index->getHash() );

	*entry++ = *value;
	*entry = index->type == WR_REF ? *index->r : *index;
}

//------------------------------------------------------------------------------
static void doIndexHash_X( WRValue* value, WRValue* target, uint32_t hash ) { target->init(); }
static void doIndexHash_R( WRValue* value, WRValue* target, uint32_t hash ) { wr_IndexHash[ value->r->type ]( value->r, target, hash ); }
static void doIndexHash_E( WRValue* value, WRValue* target, uint32_t hash )
{
	if (IS_REFARRAY(value->xtype) )
	{
		if ( !IS_RAW_ARRAY(value->r->xtype)
			 && (value->r->va->m_type == SV_VALUE) )
		{
			unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);
			if ( s >= value->r->va->m_size )
			{
				value->r->va = growValueArray( value->r->va, s );
			}

			WRValue* val = value->r->va->m_Vdata + s;
			wr_IndexHash[ val->type ]( val, target, hash );
		}
	}
	else if ( value->xtype == WR_EX_STRUCT )
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( (uint32_t)READ_32_FROM_PC(table) == hash )
		{
			target->p2 = INIT_AS_REF;
			target->p = ((WRValue*)(value->va->m_data)) + *(table + 4);
		}
		else
		{
			target->init();
		}
	}
	else if ( value->xtype == WR_EX_HASH_TABLE )
	{
		if ( IS_EX_SINGLE_CHAR_RAW_P2( (target->r = (WRValue*)value->va->get(hash))->p2) )
		{
			target->p2 = INIT_AS_REFARRAY;
			//ARRAY_ELEMENT_TO_P2( target->p2, 1 );
		}
		else
		{
			target->p2 = INIT_AS_REF;
		}
	}
}
WRIndexHashFunc wr_IndexHash[4] = 
{
	doIndexHash_X,  doIndexHash_X,  doIndexHash_R,  doIndexHash_E
};

//------------------------------------------------------------------------------
static bool doLogicalNot_X( WRValue* value ) { return value->i == 0; } // float also uses 0x0000000 as 'zero'
static bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
static bool doLogicalNot_E( WRValue* value )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		return wr_LogicalNot[ element.type ]( &element );
	}
	return false;
}
WRReturnSingleFunc wr_LogicalNot[4] = 
{
	doLogicalNot_X,  doLogicalNot_X,  doLogicalNot_R,  doLogicalNot_E
};


//------------------------------------------------------------------------------
static void doNegate_I( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = -value->i; }
static void doNegate_F( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = -value->f; }
static void doNegate_E( WRValue* value, WRValue* target )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);

		if ( IS_RAW_ARRAY(value->r->xtype) )
		{
			target->p2 = INIT_AS_INT;
			target->i = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(value->r->p2))) ? -value->r->c[s] : 0;
		}
		else if (value->r->va->m_type == SV_VALUE)
		{
			if ( s < value->r->va->m_size )
			{
				wr_negate[ (value->r->va->m_Vdata + s)->type ]( value->r->va->m_Vdata + s, target );
			}
			else
			{
				target->init();
			}
		}
		else if ( value->r->va->m_type == SV_CHAR )
		{
			target->p2 = INIT_AS_INT;
			target->i = (s < value->r->va->m_size) ? -value->r->va->m_Cdata[s] : 0;
		}
	}
}
static void doNegate_R( WRValue* value, WRValue* target )
{
	wr_negate[ value->r->type ]( value->r, target );
}

WRSingleTargetFunc wr_negate[4] = 
{
	doNegate_I,  doNegate_F,  doNegate_R,  doNegate_E
};

//------------------------------------------------------------------------------
static void doGetValue_E( WRValue** value )
{
	if ( IS_REFARRAY( (*value)->xtype) )
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2((*value)->p2);

		if ( IS_RAW_ARRAY((*value)->r->xtype) )
		{
			if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2((*value)->r->p2)) )
			{
				(*value)->p2 = INIT_AS_INT;
				(*value)->i = (*value)->r->c[s];
			}
		}
		else if ((*value)->r->va->m_type == SV_VALUE)
		{
			if ( s < (*value)->r->va->m_size )
			{
				*value = (*value)->r->va->m_Vdata + s;
				wr_getValue[ (*value)->type ]( value );
			}
		}
		else if ((*value)->r->va->m_type == SV_CHAR )
		{
			(*value)->p2 = INIT_AS_INT;
			(*value)->i = (s >= (*value)->r->va->m_size) ? 0 : (*value)->r->va->m_Cdata[s];
		}
	}
}
static void doGetValue_R( WRValue** value )
{
	wr_getValue[ (*value = (*value)->r)->type ]( value );
}
static void doGetValue_X( WRValue** value ) { }

WRGetValueFunc wr_getValue[4] = 
{
	doGetValue_X,  doGetValue_X,  doGetValue_R,  doGetValue_E
};

//------------------------------------------------------------------------------
static uint32_t doBitwiseNot_I( WRValue* value ) { return ~value->ui; }
static uint32_t doBitwiseNot_F( WRValue* value ) { return 0; }
static uint32_t doBitwiseNot_R( WRValue* value ) { return wr_bitwiseNot[ value->r->type ]( value->r ); }
static uint32_t doBitwiseNot_E( WRValue* value )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		return wr_bitwiseNot[ element.type ]( &element );
	}
	return 0;
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
static void doAssign_X_E( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_assign[(WR_EX<<2)+element.type](to, &element);
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_X( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		wr_valueToArray( to, from );
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_E( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_assign[(WR_EX<<2)+element.type](to, &element);
		return;
	}
	else if ( IS_REFARRAY(to->xtype)
			  && IS_EXARRAY_TYPE(to->r->xtype)
			  && (to->r->va->m_type == SV_VALUE) )
	{
		unsigned int index = ARRAY_ELEMENT_FROM_P2(to->p2);

		if ( index > to->r->va->m_size )
		{
			if ( to->r->va->m_skipGC )
			{
				return;
			}

			to->r->va = growValueArray( to->r->va, index );	
		}

		to->r->va->m_Vdata[index] = *from;
		return;
	}

	*to = *from;
}



static void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	c->gc( target );

	// all we know is the value is not an array, so make it one
	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target->p2, index->i );

	value->p2 = INIT_AS_ARRAY;
	value->va = c->getSVA( index->i+1, SV_VALUE, true );
}

static void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	if ( value->xtype == WR_EX_HASH_TABLE )
	{
		target->r = (WRValue*)value->va->get( index->ui );
		target->p2 = INIT_AS_REF;
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
		c->gc( target );

		// nope, make it one of this size and return a ref
		value->p2 = INIT_AS_ARRAY;
		value->va = c->getSVA( index->ui+1, SV_VALUE, true );
	}
	else if ( index->ui >= value->va->m_size )
	{
		if ( value->va->m_skipGC )
		{
boundsFailed:
			target->init();
			return;
		}

		value->va = growValueArray( value->va, index->ui );
	}

	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target->p2, index->ui );
}
static void doIndex_E_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	// the specific case of a string indexing a hash table
	if ( value->xtype == WR_EX_HASH_TABLE )
	{
		target->p2 = INIT_AS_REF;
		target->r = (WRValue*)value->va->get( index->getHash() );
	}
}

//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================

#ifdef WRENCH_COMPACT


static void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
static void doAssign_R_X( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->type](to->r, from); }
static void doAssign_X_R( WRValue* to, WRValue* from ) { wr_assign[(to->type<<2)|from->r->type](to, from->r); }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_R_X,  doAssign_R_X,  doAssign_R_R,  doAssign_R_X,
	doAssign_E_X,  doAssign_E_X,  doAssign_X_R,  doAssign_E_E,
};

static void doIndex_X_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->type<<2)|value->r->type](c, index, value->r, target); }
static void doIndex_R_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->type](c, index->r, value, target); }
static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }
static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_X_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_X,     doIndex_R_X,     doIndex_R_R,     doIndex_R_X, 
	doVoidIndexFunc, doVoidIndexFunc, doIndex_X_R,     doIndex_E_E,
};

extern bool CompareEQI( int a, int b );
extern bool CompareEQF( float a, float b );

//------------------------------------------------------------------------------
static void unaryPost_E( WRValue* value, WRValue* stack, int add )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		WRValue temp;
		m_unaryPost[ element.type ]( &element, &temp, add );
		wr_valueToArray( value, &element );
		*stack = temp;
	}
}
static void unaryPost_I( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_INT; stack->i = value->i; value->i += add; }
static void unaryPost_R( WRValue* value, WRValue* stack, int add ) { m_unaryPost[ value->r->type ]( value->r, stack, add ); }
static void unaryPost_F( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f; value->f += add; }
WRVoidPlusFunc m_unaryPost[4] = 
{
	unaryPost_I,  unaryPost_F,  unaryPost_R, unaryPost_E
};

static void FuncAssign_R_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	wr_FuncAssign[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
static void FuncAssign_E_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );

		wr_FuncAssign[(element.type<<2)|from->type]( &element, from, intCall, floatCall );

		wr_valueToArray( to, &element );
		*from = element;
	}
}
static void FuncAssign_E_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );

		wr_FuncAssign[(WR_EX<<2)|element.type]( to, &element, intCall, floatCall );
		wr_arrayToValue( to, from );
	}
}
static void FuncAssign_X_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_FuncAssign[(to->type<<2)|element.type](to, &element, intCall, floatCall);
		*from = *to;
	}
}
static void FuncAssign_E_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}

static void FuncAssign_R_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue temp = *from->r; wr_FuncAssign[(to->r->type<<2)|temp.type](to->r, &temp, intCall, floatCall); *from = *to->r;
}

static void FuncAssign_R_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); *from = *to->r;
}

static void FuncAssign_X_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); *from = *to;
}

static void FuncAssign_F_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->f = floatCall( to->f, from->f );
}

static void FuncAssign_I_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->i = intCall( to->i, from->i );
}

static void FuncAssign_I_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->p2 = INIT_AS_FLOAT;
	to->f = floatCall( (float)to->i, from->f );
}
static void FuncAssign_F_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
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
static void FuncBinary_E_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );
		wr_funcBinary[(element.type<<2)|from->type](&element, from, target, intCall, floatCall);
	}
}
static void FuncBinary_E_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )
	{
		WRValue element1;
		wr_arrayToValue( to, &element1 );
		WRValue element2;
		wr_arrayToValue( from, &element2 );
		wr_funcBinary[(element1.type<<2)|element2.type](&element1, &element2, target, intCall, floatCall );
	}
}
static void FuncBinary_X_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_funcBinary[(to->type<<2)|element.type](to, &element, target, intCall, floatCall);
	}
}
static void FuncBinary_E_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(WR_EX<<2)|from->r->type](to, from->r, target, intCall, floatCall );
}

static void FuncBinary_R_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(to->r->type<<2)|WR_EX](to->r, from, target, intCall, floatCall );
}

static void FuncBinary_X_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->type<<2)+from->r->type](to, from->r, target, intCall, floatCall);
}

static void FuncBinary_R_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->type](to->r, from, target, intCall, floatCall);
}

static void FuncBinary_R_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target, intCall, floatCall);
}

static void FuncBinary_I_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_INT; target->i = intCall( to->i, from->i );
}

static void FuncBinary_I_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( (float)to->i, from->f );
}

static void FuncBinary_F_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( to->f, (float)from->i );
}

static void FuncBinary_F_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
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

static bool Compare_E_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )
	{
		WRValue element1;
		wr_arrayToValue( to, &element1 );
		WRValue element2;
		wr_arrayToValue( from, &element2 );
		return wr_Compare[(element1.type<<2)|element2.type](&element1, &element2, intCall, floatCall);
	}
	return intCall(0,0) && to->getHash() == from->getHash();
}
static bool Compare_E_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );
		return wr_Compare[(element.type<<2)|from->type](&element, from, intCall, floatCall);
	}
	return false;
}
static bool Compare_X_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		return wr_Compare[(to->type<<2)|element.type](to, &element, intCall, floatCall );
	}
	return false;
}
static bool Compare_R_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	return wr_Compare[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
static bool Compare_E_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	return wr_Compare[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}
static bool Compare_R_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->r->type](to->r, from->r, intCall, floatCall); }
static bool Compare_R_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); }
static bool Compare_X_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); }
static bool Compare_I_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return intCall( to->i, from->i ); }
static bool Compare_I_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( (float)to->i, from->f); }
static bool Compare_F_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, (float)from->i); }
static bool Compare_F_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, from->f); }
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

static void doAssign_R_E( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_EX](to->r, from); }
static void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
static void doAssign_E_R( WRValue* to, WRValue* from ) { wr_assign[(WR_EX<<2)|from->r->type](to, from->r); }
static void doAssign_R_I( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_INT](to->r, from); }
static void doAssign_R_F( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_FLOAT](to->r, from); }
static void doAssign_I_R( WRValue* to, WRValue* from ) { wr_assign[(WR_INT<<2)|from->r->type](to, from->r); }
static void doAssign_F_R( WRValue* to, WRValue* from ) { wr_assign[(WR_FLOAT<<2)|from->r->type](to, from->r); }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_I_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_F_R,  doAssign_X_E,
	doAssign_R_I,  doAssign_R_F,  doAssign_R_R,  doAssign_R_E,
	doAssign_E_X,  doAssign_E_X,  doAssign_E_R,  doAssign_E_E,
};

static void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_INT<<2)|value->r->type](c, index, value->r, target); }
static void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_INT](c, index->r, value, target); }
static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }
static void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_FLOAT](c, index->r, value, target); }
static void doIndex_R_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_EX](c, index->r, value, target); }
static void doIndex_E_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_EX<<2)|value->r->type](c, index, value->r, target); }

static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_I_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_I,     doIndex_R_F,     doIndex_R_R,     doIndex_R_E, 
	doVoidIndexFunc, doVoidIndexFunc, doIndex_E_R,     doIndex_E_E,
};

static void doVoidFuncBlank( WRValue* to, WRValue* from ) {}

#define X_INT_ASSIGN( NAME, OPERATION ) \
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)+temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		\
		NAME##Assign[(WR_EX<<2)+element.type]( to, &element );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_INT<<2)+element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
static void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
static void NAME##Assign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_INT<<2)+temp.type](to, &temp); *from = *to; }\
static void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
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
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_F( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_FLOAT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		\
		NAME##Assign[(WR_EX<<2)|element.type]( to, &element );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_INT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_F_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_FLOAT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)|temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
static void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
static void NAME##Assign_R_F( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }\
static void NAME##Assign_I_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }\
static void NAME##Assign_F_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }\
static void NAME##Assign_F_F( WRValue* to, WRValue* from ) { to->f OPERATION##= from->f; }\
static void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
static void NAME##Assign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i OPERATION from->f; }\
static void NAME##Assign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f OPERATION##= (float)from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,  NAME##Assign_I_F,  NAME##Assign_I_R,  NAME##Assign_I_E,\
	NAME##Assign_F_I,  NAME##Assign_F_F,  NAME##Assign_F_R,  NAME##Assign_F_E,\
	NAME##Assign_R_I,  NAME##Assign_R_F,  NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,  NAME##Assign_E_F,  NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_ASSIGN( wr_Subtract, - );
X_ASSIGN( wr_Add, + );
X_ASSIGN( wr_Multiply, * );
X_ASSIGN( wr_Divide, / );

//------------------------------------------------------------------------------
#define X_BINARY( NAME, OPERATION ) \
static void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->r->type](&element, from->r, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_F( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_FLOAT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		NAME##Binary[(element1.type<<2)|element2.type](&element1, &element2, target);\
	}\
}\
static void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_INT<<2)|element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_F_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_FLOAT<<2)|element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_R_F( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }\
static void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
static void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
static void NAME##Binary_F_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
static void NAME##Binary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i OPERATION from->f; }\
static void NAME##Binary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION (float)from->i; }\
static void NAME##Binary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION from->f; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I,  NAME##Binary_I_F,  NAME##Binary_I_R,  NAME##Binary_I_E,\
	NAME##Binary_F_I,  NAME##Binary_F_F,  NAME##Binary_F_R,  NAME##Binary_F_E,\
	NAME##Binary_R_I,  NAME##Binary_R_F,  NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I,  NAME##Binary_E_F,  NAME##Binary_E_R,  NAME##Binary_E_E,\
};\

X_BINARY( wr_Addition, + );
X_BINARY( wr_Multiply, * );
X_BINARY( wr_Subtract, - );
X_BINARY( wr_Divide, / );


static void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}

//------------------------------------------------------------------------------
#define X_INT_BINARY( NAME, OPERATION ) \
static void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->r->type](&element, from->r, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		NAME##Binary[(element1.type<<2)|element2.type](&element1, &element2, target);\
	}\
}\
static void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_INT<<2)+element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
static void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
static void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
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
static bool NAME##_E_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		return NAME[(element1.type<<2)|element2.type](&element1, &element2);\
	}\
	return (0 OPERATION 0) && to->getHash() == from->getHash(); \
}\
static bool NAME##_R_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(to->r->type<<2)|element.type](to->r, &element);\
	}\
	return (0 OPERATION 0) && to->r->getHash() == from->getHash(); \
}\
static bool NAME##_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|from->r->type](&element, from->r);\
	}\
	return (0 OPERATION 0) && to->getHash() == from->r->getHash(); \
}\
static bool NAME##_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_INT](&element, from);\
	}\
	return false;\
}\
static bool NAME##_E_F( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_FLOAT](&element, from);\
	}\
	return false;\
}\
static bool NAME##_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(WR_INT<<2)|element.type](to, &element);\
	}\
	return false;\
}\
static bool NAME##_F_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(WR_FLOAT<<2)|element.type](to, &element);\
	}\
	return false;\
}\
static bool NAME##_R_R( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|from->r->type](to->r, from->r); }\
static bool NAME##_R_I( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_INT](to->r, from); }\
static bool NAME##_R_F( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_FLOAT](to->r, from); }\
static bool NAME##_I_R( WRValue* to, WRValue* from ) { return NAME[(WR_INT<<2)+from->r->type](to, from->r); }\
static bool NAME##_F_R( WRValue* to, WRValue* from ) { return NAME[(WR_FLOAT<<2)+from->r->type](to, from->r); }\
static bool NAME##_I_I( WRValue* to, WRValue* from ) { return to->i OPERATION from->i; }\
static bool NAME##_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->f OPERATION from->f; }\
static bool NAME##_F_I( WRValue* to, WRValue* from ) { return to->f OPERATION (float)from->i; }\
static bool NAME##_F_F( WRValue* to, WRValue* from ) { return to->f OPERATION from->f; }\
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
static void NAME##_E( WRValue* value )\
{\
	if ( IS_REFARRAY(value->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( value, &element );\
		NAME [ element.type ]( &element );\
		wr_valueToArray( value, &element );\
	}\
}\
static void NAME##_I( WRValue* value ) { OPERATION value->i; }\
static void NAME##_F( WRValue* value ) { OPERATION value->f; }\
static void NAME##_R( WRValue* value ) { NAME [ value->r->type ]( value->r ); *value = *value->r; }\
WRUnaryFunc NAME[4] = \
{\
	NAME##_I, NAME##_F, NAME##_R, NAME##_E\
};\

X_UNARY_PRE( wr_preinc, ++ );
X_UNARY_PRE( wr_predec, -- );

//------------------------------------------------------------------------------
#define X_UNARY_POST( NAME, OPERATION ) \
static void NAME##_E( WRValue* value, WRValue* stack )\
{\
	if ( IS_REFARRAY(value->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( value, &element );\
		WRValue temp; \
		NAME [ element.type ]( &element, &temp );\
		wr_valueToArray( value, &element );\
		*stack = temp; \
	}\
}\
static void NAME##_I( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_INT; stack->i = value->i OPERATION; }\
static void NAME##_R( WRValue* value, WRValue* stack ) { NAME[ value->r->type ]( value->r, stack ); }\
static void NAME##_F( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f OPERATION; }\
WRVoidFunc NAME[4] = \
{\
	NAME##_I,  NAME##_F,  NAME##_R, NAME##_E\
};\

X_UNARY_POST( wr_postinc, ++ );
X_UNARY_POST( wr_postdec, -- );

#endif
