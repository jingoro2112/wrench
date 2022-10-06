/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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
WRValue* WRValue::asValueArray( int* len )
{
	if ( type == WR_REF )
	{
		return r->asValueArray(len);
	}

	if ( (xtype != WR_EX_ARRAY) || ((va->m_type) != SV_VALUE) )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return va->m_Vdata;
}

//------------------------------------------------------------------------------
unsigned char* WRValue::asCharArray( int* len )
{
	if ( type == WR_REF )
	{
		return r->asCharArray(len);
	}

	if ( (xtype != WR_EX_ARRAY) || ((va->m_type) != SV_CHAR) )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (unsigned char*)va->m_data;
}

//------------------------------------------------------------------------------
int* WRValue::asIntArray( int* len )
{
	if ( type == WR_REF )
	{
		return r->asIntArray(len);
	}

	if ( (xtype != WR_EX_ARRAY) || ((va->m_type) != SV_INT) )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (int*)va->m_data;
}

//------------------------------------------------------------------------------
float* WRValue::asFloatArray( int* len )
{
	if ( type == WR_REF )
	{
		return r->asFloatArray(len);
	}

	if ( (xtype != WR_EX_ARRAY) || ((va->m_type) != SV_FLOAT) )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (float*)va->m_data;
}

//------------------------------------------------------------------------------
void growValueArray( WRValue* v, int newSize )
{
	WRGCArray* newArray = new WRGCArray( newSize + 1, SV_VALUE );
	
	newArray->m_next = v->va->m_next;
	v->va->m_next = newArray;
	memcpy( newArray->m_Cdata, v->va->m_Cdata, sizeof(WRValue) * v->va->m_size );
	memset( (char*)(newArray->m_Vdata + v->va->m_size), 0, (newArray->m_size - v->va->m_size) * sizeof(WRValue) );
	v->va = newArray;
}

//------------------------------------------------------------------------------
int WRValue::arrayValueAsInt() const
{
	unsigned int s = ARRAY_ELEMENT_FROM_P2(p2);
	switch( r->va->m_type )
	{
		case SV_VALUE:
		{
			if ( s >= r->va->m_size )
			{
				growValueArray( r, s + 1 );
			}

			return r->va->m_Vdata[s].asInt();
		}
		
		case SV_CHAR: { return r->va->m_Cdata[(s >= r->va->m_size) ? 0 : s]; }
		case SV_INT: { return r->va->m_Idata[(s >= r->va->m_size) ? 0 : s]; }
		case SV_FLOAT: { return (int)r->va->m_Fdata[(s >= r->va->m_size) ? 0 : s]; }
		default: return 0;
	}
}

//------------------------------------------------------------------------------
float WRValue::arrayValueAsFloat() const
{
	unsigned int s = ARRAY_ELEMENT_FROM_P2(p2);
	switch( r->va->m_type )
	{
		case SV_VALUE:
		{
			if ( s >= r->va->m_size )
			{
				growValueArray( r, s + 1 );
			}

			return r->va->m_Vdata[s].asFloat();
		}

		case SV_CHAR: { return r->va->m_Cdata[(s >= r->va->m_size) ? 0 : s]; }
		case SV_INT: { return (float)r->va->m_Idata[(s >= r->va->m_size) ? 0 : s]; }
		case SV_FLOAT: { return r->va->m_Fdata[(s >= r->va->m_size) ? 0 : s]; }
		default: return 0;
	}
}

//------------------------------------------------------------------------------
WRState* wr_newState( int stackSize )
{
	return new WRState( stackSize );
}

static void doVoidFuncBlank( WRValue* to, WRValue* from ) {}

//------------------------------------------------------------------------------
void wr_arrayToValue( const WRValue* array, WRValue* value )
{
	if ( !(array->r->xtype & 0x4) )
	{
		value->init();
		return;
	}

	unsigned int s = ARRAY_ELEMENT_FROM_P2(array->p2);
	
	switch( array->r->va->m_type )
	{
		case SV_VALUE:
		{
			if ( s >= array->r->va->m_size )
			{
				growValueArray( array->r, s + 1 );
			}
			*value = array->r->va->m_Vdata[s];
			break;
		}

		case SV_CHAR:
		{
			value->i = (s >= array->r->va->m_size) ? 0 : array->r->va->m_Cdata[s];
			value->p2 = INIT_AS_INT;
			return;
		}

		case SV_INT:
		{
			value->i = (s >= array->r->va->m_size) ? 0 : array->r->va->m_Idata[s];
			value->p2 = INIT_AS_INT;
			return;
		}

		case SV_FLOAT:
		{
			value->f = (s >= array->r->va->m_size) ? 0 : array->r->va->m_Fdata[s];
			value->p2 = INIT_AS_FLOAT;
			return;
		}
	}
}

//------------------------------------------------------------------------------
void wr_intValueToArray( const WRValue* array, int32_t I )
{
	if ( !(array->r->xtype & 0x4) )
	{
		return;
	}
	
	unsigned int s = ARRAY_ELEMENT_FROM_P2(array->p2);

	switch( array->r->va->m_type )
	{
		case SV_VALUE:
		{
			if ( s >= array->r->va->m_size )
			{
				growValueArray( array->r, s + 1 );
			}
			WRValue* val = array->r->va->m_Vdata + s;
			val->i = I;
			val->p2 = INIT_AS_INT;
			break;
		}

		case SV_CHAR:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Cdata[s] = I;
			}
			
			break;
		}
		case SV_INT:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Idata[s] = I;
			}
			break;
		}
		case SV_FLOAT:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Fdata[s] = (float)I;
			}
			break;
		}
	}
}

//------------------------------------------------------------------------------
void wr_floatValueToArray( const WRValue* array, float F )
{
	if ( !(array->r->xtype & 0x4) )
	{
		return;
	}
				
	unsigned int s = ARRAY_ELEMENT_FROM_P2(array->p2);

	switch( array->r->va->m_type )
	{
		case SV_VALUE:
		{
			if ( s >= array->r->va->m_size )
			{
				growValueArray( array->r, s + 1 );
			}
			WRValue* val = array->r->va->m_Vdata + s;
			val->f = F;
			val->p2 = INIT_AS_FLOAT;
			break;
		}

		case SV_CHAR:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Cdata[s] = (unsigned char)F;
			}

			break;
		}
		case SV_INT:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Idata[s] = (int)F;
			}
			break;
		}
		case SV_FLOAT:
		{
			if ( s < array->r->va->m_size )
			{
				array->r->va->m_Fdata[s] = F;
			}
			break;
		}
	}
}

//------------------------------------------------------------------------------
void wr_countOfArrayElement( WRValue* array, WRValue* target )
{
	if ( array->xtype & 0x4 )
	{
		if ( array->xtype == WR_EX_REFARRAY )
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
static void doAssign_X_E( WRValue* to, WRValue* from )
{
	if ( from->xtype == WR_EX_REFARRAY )
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
static void doAssign_E_F( WRValue* to, WRValue* from )
{
	if ( to->xtype == WR_EX_REFARRAY )
	{
		wr_floatValueToArray( to, from->f );
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_I( WRValue* to, WRValue* from )
{
	if ( to->xtype == WR_EX_REFARRAY )
	{
		wr_intValueToArray( to, from->i );
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_E( WRValue* to, WRValue* from )
{
	if ( from->xtype == WR_EX_REFARRAY )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_assign[(WR_EX<<2)+element.type](to, &element);
	}
	else if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))
	{
		if ( to->r->va->m_type == SV_VALUE )
		{
			unsigned int index = ARRAY_ELEMENT_FROM_P2(to->p2);
			
			if ( index > to->r->va->m_size )
			{
				growValueArray( to->r, index + 1 );
			}

			to->r->va->m_Vdata[index] = *from;
		}
		else
		{
			*to = *from;
		}
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_R_E( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_EX](to->r, from); }
static void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
static void doAssign_E_R( WRValue* to, WRValue* from ) { wr_assign[(WR_EX<<2)|from->r->type](to, from->r); }
static void doAssign_R_X( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->type](to->r, from); }
static void doAssign_X_R( WRValue* to, WRValue* from ) { wr_assign[(to->type<<2)|from->r->type](to, from->r); }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_R_X,  doAssign_R_X,  doAssign_R_R,  doAssign_R_E,
	doAssign_E_I,  doAssign_E_F,  doAssign_E_R,  doAssign_E_E,
};
//==============================================================================


#define X_ASSIGN( NAME, OPERATION ) \
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
\
		wr_intValueToArray( to, element.i );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_F( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
\
		NAME##Assign[(element.type<<2)|WR_FLOAT]( &element, from );\
\
		wr_floatValueToArray( to, element.f );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
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
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_INT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_F_E( WRValue* to, WRValue* from )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_FLOAT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)|temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
static void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
static void NAME##Assign_R_F( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }\
static void NAME##Assign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_INT<<2)+temp.type](to, &temp); *from = *to; }\
static void NAME##Assign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }\
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


#define X_INT_ASSIGN( NAME, OPERATION ) \
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)+temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
\
		wr_intValueToArray( to, element.i );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4) )\
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
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
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


//------------------------------------------------------------------------------
#define X_BINARY( NAME, OPERATION ) \
static void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->type](&element, from, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_F( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_FLOAT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( to->xtype == WR_EX_REFARRAY && from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4) && (to->r->xtype&0x4))\
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
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_INT<<2)|element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_F_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
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
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->type](&element, from, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( to->xtype == WR_EX_REFARRAY && from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4) && (to->r->xtype&0x4) )\
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
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
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
	if ( to->xtype == WR_EX_REFARRAY && from->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4) && (from->r->xtype&0x4))\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		return NAME[(element1.type<<2)|element2.type](&element1, &element2);\
	}\
return false;\
}\
static bool NAME##_R_E( WRValue* to, WRValue* from )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(to->type<<2)|element.type](to, &element);\
	}\
return false;\
}\
static bool NAME##_E_R( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|from->type](&element, from);\
	}\
return false;\
}\
static bool NAME##_E_I( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_INT](&element, from);\
	}\
return false;\
}\
static bool NAME##_E_F( WRValue* to, WRValue* from )\
{\
	if ( to->xtype == WR_EX_REFARRAY && (to->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_FLOAT](&element, from);\
	}\
return false;\
}\
static bool NAME##_I_E( WRValue* to, WRValue* from )\
{\
	if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(WR_INT<<2)|element.type](to, &element);\
	}\
	return false;\
}\
static bool NAME##_F_E( WRValue* to, WRValue* from )\
{\
   if ( from->xtype == WR_EX_REFARRAY && (from->r->xtype&0x4))\
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

X_COMPARE( wr_CompareEQ, == );
X_COMPARE( wr_CompareGT, > );
X_COMPARE( wr_CompareLT, < );
X_COMPARE( wr_LogicalAND, && );
X_COMPARE( wr_LogicalOR, || );

static void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	c->gc( target );

	// all we know is the value is not an array, so make it one
	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target, index->i );

	value->p2 = INIT_AS_ARRAY;
	value->va = c->getSVA( index->i+1, SV_VALUE, true );
}
static void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	if ( value->xtype != WR_EX_ARRAY )
	{
		c->gc( target );

		// nope, make it one of this size and return a ref
		value->p2 = INIT_AS_ARRAY;
		value->va = c->getSVA( index->i+1, SV_VALUE, true );
	}

	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target, index->i );
}
static void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_INT<<2)|value->r->type](c, index, value->r, target); }
static void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_INT](c, index->r, value, target); }
static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }
static void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_FLOAT](c, index->r, value, target); }
static void doIndex_R_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { if (value->xtype == WR_EX_ARRAY) { wr_index[(index->r->type<<2)|WR_EX](c, index->r, value, target); } }

static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

WRStateFunc wr_index[16] = 
{
	    doIndex_I_X,     doIndex_I_X,     doIndex_I_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
        doIndex_R_I,     doIndex_R_F,     doIndex_R_R,     doIndex_R_E, 
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
};

//------------------------------------------------------------------------------
#define X_UNARY_PRE( NAME, OPERATION ) \
static void NAME##_E( WRValue* value )\
{\
	if ( value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))\
	{\
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);\
		switch( value->r->va->m_type )\
		{\
			case SV_VALUE:\
			{\
				if ( s >= value->r->va->m_size )\
				{\
					growValueArray( value->r, s + 1 );\
				}\
\
				WRValue* val = (WRValue *)value->r->va->m_data + s;\
				NAME[ val->type ]( val );\
				*value = *val;\
				return;\
			}\
\
			case SV_CHAR: {	value->i = (s >= value->r->va->m_size) ? 0 : OPERATION value->r->va->m_Cdata[s]; value->p2 = INIT_AS_INT; return; }\
			case SV_INT: { value->i = (s >= value->r->va->m_size) ? 0 : OPERATION value->r->va->m_Idata[s]; value->p2 = INIT_AS_INT; return; }\
			case SV_FLOAT: { value->f = (s >= value->r->va->m_size) ? 0 : OPERATION value->r->va->m_Fdata[s]; value->p2 = INIT_AS_FLOAT; return; }\
		}\
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
static void doSingleVoidBlank( WRValue* value ) {}

//------------------------------------------------------------------------------
static void doToInt_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void doToInt_F( WRValue* value ) { value->p2 = WR_INT; value->i = (int32_t)value->f; }
WRUnaryFunc wr_toInt[4] = 
{
	doSingleVoidBlank, doToInt_F, doToInt_R, doSingleVoidBlank
};

static void doToFloat_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void doToFloat_I( WRValue* value ) { value->p2 = INIT_AS_FLOAT; value->f = (float)value->i; }
WRUnaryFunc wr_toFloat[4] = 
{
	doToFloat_I, doSingleVoidBlank, doToFloat_R, doSingleVoidBlank
};

static void bitwiseNOT_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void bitwiseNOT_I( WRValue* value ) { value->i = ~value->i; }
WRUnaryFunc wr_bitwiseNOT[4] = 
{
	bitwiseNOT_I, doSingleVoidBlank, bitwiseNOT_R, doSingleVoidBlank
};

//------------------------------------------------------------------------------
#define X_UNARY_POST( NAME, OPERATION ) \
static void NAME##_E( WRValue* value, WRValue* stack )\
{\
	if ( value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))\
	{\
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);\
		switch( value->r->va->m_type )\
		{\
			case SV_VALUE:\
			{\
				if ( s >= value->r->va->m_size )\
				{\
					growValueArray( value->r, s + 1 );\
				}\
				WRValue* val = value->r->va->m_Vdata + s;\
				NAME[ val->type ]( val, stack );\
				break;\
			}\
\
			case SV_CHAR:\
			{\
				stack->p2 = INIT_AS_INT;\
				stack->i = (s >= value->r->va->m_size) ? 0 : value->r->va->m_Cdata[s] OPERATION;\
				break;\
			}\
\
			case SV_INT:\
			{\
				stack->p2 = INIT_AS_INT;\
				stack->i = (s >= value->r->va->m_size) ? 0 : value->r->va->m_Idata[s] OPERATION;\
				break;\
			}\
\
			case SV_FLOAT:\
			{\
				stack->p2 = INIT_AS_FLOAT;\
				stack->f = (s >= value->r->va->m_size) ? 0 : value->r->va->m_Fdata[s] OPERATION;\
				break;\
			}\
		}\
	}\
}\
static void NAME##_I( WRValue* value, WRValue* stack ) { *stack = *value; OPERATION  value->i; }\
static void NAME##_R( WRValue* value, WRValue* stack ) { NAME[ value->r->type ]( value->r, stack ); }\
static void NAME##_F( WRValue* value, WRValue* stack ) { *stack = *value; OPERATION value->f; }\
WRVoidFunc NAME[4] = \
{\
	NAME##_I,  NAME##_F,  NAME##_R, NAME##_E\
};\

X_UNARY_POST( wr_postinc, ++ );
X_UNARY_POST( wr_postdec, -- );


//------------------------------------------------------------------------------
static void doIndexHash_X( WRValue* value, WRValue* target, uint32_t hash ) { }
static void doIndexHash_R( WRValue* value, WRValue* target, uint32_t hash ) { wr_IndexHash[ value->r->type ]( value->r, target, hash ); }
static void doIndexHash_E( WRValue* value, WRValue* target, uint32_t hash )
{
	if (value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))
	{

		if ( value->r->va->m_type == SV_VALUE )
		{
			unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);
			if ( s >= value->r->va->m_size )
			{
				growValueArray( value->r, s + 1 );
			}

			WRValue* val = value->r->va->m_Vdata + s;
			wr_IndexHash[ val->type ]( val, target, hash );
		}
	}
	else if ( value->xtype == WR_EX_USR )
	{
		target->p2 = INIT_AS_REF;
		if ( !(target->r = value->u->get(hash)) )
		{
			target->p = 0;
			target->p2 = 0;
		}
	}
	else if ( value->xtype == WR_EX_STRUCT )
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( ((((uint32_t)*table) << 24)
			  | (((uint32_t)*(table + 1)) << 16)
			  | (((uint32_t)*(table + 2)) << 8)
			  | ((uint32_t)*(table + 3))) == hash )
		{
			target->p2 = INIT_AS_REF;
			target->p = ((WRValue*)(value->va->m_data)) + *(table + 4);
		}
		else
		{
			target->init();
		}
	}
	else if ( value->xtype == WR_EX_HASH )
	{
		
	}
}
WRIndexHashFunc wr_IndexHash[4] = 
{
	doIndexHash_X,  doIndexHash_X,  doIndexHash_R,  doIndexHash_E
};

//------------------------------------------------------------------------------
static bool doLogicalNot_I( WRValue* value ) { return value->i == 0; }
static bool doLogicalNot_F( WRValue* value ) { return value->f == 0; }
static bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
static bool doLogicalNot_E( WRValue* value )
{
	if ( value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		return wr_LogicalNot[ element.type ]( &element );
	}
	return false;
}
WRReturnSingleFunc wr_LogicalNot[4] = 
{
	doLogicalNot_I,  doLogicalNot_F,  doLogicalNot_R,  doLogicalNot_E
};


//------------------------------------------------------------------------------
static void doNegate_I( WRValue* value ) { value->i = -value->i; }
static void doNegate_E( WRValue* value )
{
	if ( value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);

		switch( value->r->va->m_type )
		{
			case SV_VALUE:
			{
				if ( s >= value->r->va->m_size )
				{
					growValueArray( value->r, s + 1 );
				}

				WRValue* val = value->r->va->m_Vdata + s;
				if ( val->type == WR_INT )
				{
					val->i = -val->i;
					*value = *val;
				}
				else if ( val->type == WR_FLOAT )
				{
					val->f = -val->f;
					*value = *val;
				}
				break;
			}

			case SV_CHAR:
			{
				value->p2 = INIT_AS_INT;
				value->i = (s >= value->r->va->m_size) ? 0 : (value->r->va->m_Cdata[s] = -value->r->va->m_Cdata[s]);
				break;
			}

			case SV_INT:
			{
				value->p2 = INIT_AS_INT;
				value->i = (s >= value->r->va->m_size) ? 0 : (value->r->va->m_Idata[s] = -value->r->va->m_Idata[s]);
				break;
			}

			case SV_FLOAT:
			{
				value->p2 = INIT_AS_FLOAT;
				value->f = (s >= value->r->va->m_size) ? 0 : (value->r->va->m_Fdata[s] = -value->r->va->m_Fdata[s]);
				break;
			}
		}
	}
}
static void doNegate_R( WRValue* value )
{
	wr_negate[ value->r->type ]( value->r );
	*value = *value->r;
}

static void doNegate_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_negate[4] = 
{
	doNegate_I,  doNegate_F,  doNegate_R,  doNegate_E
};

//------------------------------------------------------------------------------
static void doBitwiseNot_I( WRValue* value ) { value->i = ~value->i; }
static void doBitwiseNot_R( WRValue* value ) { wr_bitwiseNot[ value->r->type ]( value->r ); *value = *value->r; }
static void doBitwiseNot_E( WRValue* value )
{
	if ( value->xtype == WR_EX_REFARRAY && (value->r->xtype&0x4))
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);

		switch( value->r->va->m_type )
		{
			case SV_VALUE:
			{
				if ( s >= value->r->va->m_size )
				{
					growValueArray( value->r, s + 1 );
				}

				WRValue* val = (WRValue *)value->r->va->m_data + s;
				if ( val->type == WR_INT )
				{
					val->i = ~val->i;
					*value = *val;
				}
				break;
			}

			case SV_CHAR:
			{
				value->p2 = INIT_AS_INT;
				value->i = (s >= value->r->va->m_size) ? 0 : (value->r->va->m_Cdata[s] = ~value->r->va->m_Cdata[s]);
				break;
			}

			case SV_INT:
			{
				value->p2 = INIT_AS_INT;
				value->i = (s >= value->r->va->m_size) ? 0 : (value->r->va->m_Idata[s] = ~value->r->va->m_Idata[s]);
				break;
			}

			case SV_FLOAT:
			{
				break;
			}
		}
	}
}

WRUnaryFunc wr_bitwiseNot[4] = 
{
	doBitwiseNot_I,  doSingleVoidBlank, doBitwiseNot_R, doBitwiseNot_E
};
