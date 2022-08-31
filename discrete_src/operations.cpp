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

/*
{
  I_I, I_R, I_F, I_U, I_A, I_Y,
  R_I, R_R, R_F, R_U, R_A, R_Y,
  F_I, F_R, F_F, F_U, F_A, F_Y,
  U_I, U_R, U_F, U_U, U_A, U_Y,
  A_I, A_R, A_F, A_U, A_A, A_Y,
  Y_I, Y_R, Y_F, Y_U, Y_A, Y_Y,
}
*/


void doVoidFuncBlank( WRValue* to, WRValue* from ) {}
bool doReturnFuncBlank( WRValue* to, WRValue* from ) { return false; }
void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}
void doVoidIndexFunc( WRRunContext* c, WRValue* index, WRValue* value ) {}
bool doSingleBlank( WRValue* value ) { return false; }
void doSingleVoidBlank( WRValue* value ) {}


//------------------------------------------------------------------------------
void arrayToValue( const WRValue* array, WRValue* value )
{
	unsigned int index = array->arrayElement >> 8;
	int s = index < array->r->va->m_size ? index : array->r->va->m_size - 1;

	switch( array->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			*value = ((WRValue *)array->r->va->m_data)[s];
			break;
		}

		case SV_CHAR:
		{
			value->i = ((unsigned char *)array->r->va->m_data)[s];
			value->type = WR_INT;
			return;
		}

		case SV_INT:
		{
			value->i = ((int *)array->r->va->m_data)[s];
			value->type = WR_INT;
			return;
		}

		case SV_FLOAT:
		{
			value->f = ((float *)array->r->va->m_data)[s];
			value->type = WR_FLOAT;
			return;
		}
	}
}

//------------------------------------------------------------------------------
void intValueToArray( const WRValue* array, int32_t I )
{
	unsigned int index = array->arrayElement >> 8;
	int s = index < array->r->va->m_size ? index : array->r->va->m_size - 1;

	switch( array->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)array->r->va->m_data + s;
			val->i = I;
			val->type = WR_INT;
			break;
		}

		case SV_CHAR: {	((unsigned char *)array->r->va->m_data)[s] = I; break; }
		case SV_INT: { ((int *)array->r->va->m_data)[s] = I; break; }
		case SV_FLOAT: { ((float *)array->r->va->m_data)[s] = (float)I; break; }
	}
}

//------------------------------------------------------------------------------
void floatValueToArray( const WRValue* array, float F )
{
	unsigned int index = array->arrayElement >> 8;
	int s = index < array->r->va->m_size ? index : array->r->va->m_size - 1;

	switch( array->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)array->r->va->m_data + s;
			val->f = F;
			val->type = WR_FLOAT;
			break;
		}

		case SV_CHAR: {	((unsigned char *)array->r->va->m_data)[s] = (unsigned char)F; break; }
		case SV_INT: { ((int *)array->r->va->m_data)[s] = (int)F; break; }
		case SV_FLOAT: { ((float *)array->r->va->m_data)[s] = F; break; }
	}
}

//==============================================================================
void doAssign_R_R( WRValue* to, WRValue* from )
{
	wr_assign[to->type*6+from->r->type](to, from->r);
}

void doAssign_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_assign[to->type*6+element.type](to, &element);
}

void doAssign_X_R( WRValue* to, WRValue* from )
{
	*to = *from->r;
}

void doAssign_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_assign[element.type*6+from->r->type](&element, from->r);
}

void doAssign_X_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	*to = element;
}

void doAssign_R_X( WRValue* to, WRValue* from )
{
	*to->r = *from;
}

void doAssign_Y_F( WRValue* to, WRValue* from )
{
	floatValueToArray( to, from->f );
}

void doAssign_Y_I( WRValue* to, WRValue* from )
{
	intValueToArray( to, from->i );
}

void doAssign_X_X( WRValue* to, WRValue* from )
{
	*to = *from;
}

WRVoidFunc wr_assign[36] = 
{
	doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doAssign_R_X,    doAssign_R_R,     doAssign_R_X,  doVoidFuncBlank,  doVoidFuncBlank,     doAssign_R_Y,
	doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doAssign_Y_I,    doAssign_Y_R,     doAssign_Y_F,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
};
//==============================================================================




//------------------------------------------------------------------------------
void doSubtractAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_SubtractAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doSubtractAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_SubtractAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doSubtractAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_SubtractAssign[element.type*6+WR_FLOAT]( &element, from );

	floatValueToArray( to, element.f );
	*from = element;
}

void doSubtractAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_SubtractAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doSubtractAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_SubtractAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doSubtractAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_SubtractAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

void doSubtractAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doSubtractAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doSubtractAssign_R_I( WRValue* to, WRValue* from ) { wr_SubtractAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doSubtractAssign_R_F( WRValue* to, WRValue* from ) { wr_SubtractAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
void doSubtractAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
void doSubtractAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

void doSubtractAssign_F_F( WRValue* to, WRValue* from ) { to->f -= from->f; }
void doSubtractAssign_I_I( WRValue* to, WRValue* from ) { to->i -= from->i; }
void doSubtractAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i - from->f; }
void doSubtractAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f -= (float)from->i; }

WRVoidFunc wr_SubtractAssign[36] = 
{
	doSubtractAssign_I_I,  doSubtractAssign_I_R,  doSubtractAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_I_Y,
	doSubtractAssign_R_I,  doSubtractAssign_R_R,  doSubtractAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_R_Y,
	doSubtractAssign_F_I,  doSubtractAssign_F_R,  doSubtractAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doSubtractAssign_Y_I,  doSubtractAssign_Y_R,  doSubtractAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_Y_Y,
};
//==============================================================================





//------------------------------------------------------------------------------
void doAddAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_AddAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doAddAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_AddAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doAddAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_AddAssign[element.type*6+WR_FLOAT]( &element, from );

	floatValueToArray( to, element.f );
	*from = element;
}

void doAddAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_AddAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doAddAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_AddAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doAddAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_AddAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

void doAddAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doAddAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doAddAssign_R_I( WRValue* to, WRValue* from ) { wr_AddAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doAddAssign_R_F( WRValue* to, WRValue* from ) { wr_AddAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
void doAddAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
void doAddAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

void doAddAssign_F_F( WRValue* to, WRValue* from ) { to->f += from->f; }
void doAddAssign_I_I( WRValue* to, WRValue* from ) { to->i += from->i; }
void doAddAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i + from->f; }
void doAddAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f += (float)from->i; }

WRVoidFunc wr_AddAssign[36] = 
{
	doAddAssign_I_I,  doAddAssign_I_R,  doAddAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_I_Y,
	doAddAssign_R_I,  doAddAssign_R_R,  doAddAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_R_Y,
	doAddAssign_F_I,  doAddAssign_F_R,  doAddAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doAddAssign_Y_I,  doAddAssign_Y_R,  doAddAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_Y_Y,
};
//==============================================================================








//------------------------------------------------------------------------------
void doMultiplyAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_MultiplyAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doMultiplyAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_MultiplyAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doMultiplyAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_MultiplyAssign[element.type*6+WR_FLOAT]( &element, from );

	floatValueToArray( to, element.f );
	*from = element;
}

void doMultiplyAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_MultiplyAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doMultiplyAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_MultiplyAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doMultiplyAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_MultiplyAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

void doMultiplyAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doMultiplyAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doMultiplyAssign_R_I( WRValue* to, WRValue* from ) { wr_MultiplyAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doMultiplyAssign_R_F( WRValue* to, WRValue* from ) { wr_MultiplyAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
void doMultiplyAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
void doMultiplyAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

void doMultiplyAssign_F_F( WRValue* to, WRValue* from ) { to->f *= from->f; }
void doMultiplyAssign_I_I( WRValue* to, WRValue* from ) { to->i *= from->i; }
void doMultiplyAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i * from->f; }
void doMultiplyAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f *= (float)from->i; }

WRVoidFunc wr_MultiplyAssign[36] = 
{
	doMultiplyAssign_I_I,  doMultiplyAssign_I_R,  doMultiplyAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_I_Y,
	doMultiplyAssign_R_I,  doMultiplyAssign_R_R,  doMultiplyAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_R_Y,
	doMultiplyAssign_F_I,  doMultiplyAssign_F_R,  doMultiplyAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doMultiplyAssign_Y_I,  doMultiplyAssign_Y_R,  doMultiplyAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_Y_Y,
};
//==============================================================================







//------------------------------------------------------------------------------
void doDivideAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_DivideAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doDivideAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_DivideAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doDivideAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_DivideAssign[element.type*6+WR_FLOAT]( &element, from );

	floatValueToArray( to, element.f );
	*from = element;
}

void doDivideAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_DivideAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doDivideAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_DivideAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doDivideAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_DivideAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

void doDivideAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doDivideAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doDivideAssign_R_I( WRValue* to, WRValue* from ) { wr_DivideAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doDivideAssign_R_F( WRValue* to, WRValue* from ) { wr_DivideAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
void doDivideAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
void doDivideAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

void doDivideAssign_F_F( WRValue* to, WRValue* from ) { to->f /= from->f; }
void doDivideAssign_I_I( WRValue* to, WRValue* from ) { to->i /= from->i; }
void doDivideAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i / from->f; }
void doDivideAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f /= (float)from->i; }

WRVoidFunc wr_DivideAssign[36] = 
{
	doDivideAssign_I_I,  doDivideAssign_I_R,  doDivideAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_I_Y,
	doDivideAssign_R_I,  doDivideAssign_R_R,  doDivideAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_R_Y,
	doDivideAssign_F_I,  doDivideAssign_F_R,  doDivideAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doDivideAssign_Y_I,  doDivideAssign_Y_R,  doDivideAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_Y_Y,
};
//==============================================================================















//------------------------------------------------------------------------------
void doModAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ModAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doModAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_ModAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doModAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_ModAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doModAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ModAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doModAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doModAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doModAssign_R_I( WRValue* to, WRValue* from ) { wr_ModAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doModAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doModAssign_I_I( WRValue* to, WRValue* from ) { to->i %= from->i; }

WRVoidFunc wr_ModAssign[36] = 
{
	doModAssign_I_I,  doModAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_I_Y,
	doModAssign_R_I,  doModAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doModAssign_Y_I,  doModAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_Y_Y,
};
//==============================================================================



//------------------------------------------------------------------------------
void doORAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ORAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doORAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_ORAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doORAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_ORAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doORAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ORAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doORAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doORAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doORAssign_R_I( WRValue* to, WRValue* from ) { wr_ORAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doORAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doORAssign_I_I( WRValue* to, WRValue* from ) { to->i |= from->i; }

WRVoidFunc wr_ORAssign[36] = 
{
	doORAssign_I_I,  doORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_I_Y,
	doORAssign_R_I,  doORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doORAssign_Y_I,  doORAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_Y_Y,
};
//==============================================================================






//------------------------------------------------------------------------------
void doANDAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ANDAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doANDAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_ANDAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doANDAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_ANDAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doANDAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_ANDAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doANDAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doANDAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doANDAssign_R_I( WRValue* to, WRValue* from ) { wr_ANDAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doANDAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doANDAssign_I_I( WRValue* to, WRValue* from ) { to->i &= from->i; }

WRVoidFunc wr_ANDAssign[36] = 
{
	doANDAssign_I_I,  doANDAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_I_Y,
	doANDAssign_R_I,  doANDAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doANDAssign_Y_I,  doANDAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_Y_Y,
};
//==============================================================================






//------------------------------------------------------------------------------
void doXORAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_XORAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doXORAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_XORAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doXORAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_XORAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doXORAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_XORAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doXORAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doXORAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doXORAssign_R_I( WRValue* to, WRValue* from ) { wr_XORAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doXORAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doXORAssign_I_I( WRValue* to, WRValue* from ) { to->i ^= from->i; }

WRVoidFunc wr_XORAssign[36] = 
{
	doXORAssign_I_I,  doXORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_I_Y,
	doXORAssign_R_I,  doXORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doXORAssign_Y_I,  doXORAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_Y_Y,
};
//==============================================================================




//------------------------------------------------------------------------------
void doRightShiftAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_RightShiftAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doRightShiftAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_RightShiftAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doRightShiftAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_RightShiftAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doRightShiftAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_RightShiftAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doRightShiftAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doRightShiftAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doRightShiftAssign_R_I( WRValue* to, WRValue* from ) { wr_RightShiftAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doRightShiftAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doRightShiftAssign_I_I( WRValue* to, WRValue* from ) { to->i >>= from->i; }

WRVoidFunc wr_RightShiftAssign[36] = 
{
	doRightShiftAssign_I_I,  doRightShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_I_Y,
	doRightShiftAssign_R_I,  doRightShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doRightShiftAssign_Y_I,  doRightShiftAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_Y_Y,
};
//==============================================================================





//------------------------------------------------------------------------------
void doLeftShiftAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );
	wr_LeftShiftAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

void doLeftShiftAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );

	wr_LeftShiftAssign[element.type*6+WR_INT]( &element, from );

	intValueToArray( to, element.i );
	*from = element;
}

void doLeftShiftAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	arrayToValue( from, &element );

	wr_LeftShiftAssign[WR_REFARRAY*6+element.type]( to, &element );
	arrayToValue( to, from );
}

void doLeftShiftAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_LeftShiftAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

void doLeftShiftAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
void doLeftShiftAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[WR_REFARRAY*6+temp.type]( to, &temp ); arrayToValue( to, from ); }
void doLeftShiftAssign_R_I( WRValue* to, WRValue* from ) { wr_LeftShiftAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
void doLeftShiftAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

void doLeftShiftAssign_I_I( WRValue* to, WRValue* from ) { to->i <<= from->i; }

WRVoidFunc wr_LeftShiftAssign[36] = 
{
	doLeftShiftAssign_I_I,  doLeftShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_I_Y,
	doLeftShiftAssign_R_I,  doLeftShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doLeftShiftAssign_Y_I,  doLeftShiftAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_Y_Y,
};
//==============================================================================







//------------------------------------------------------------------------------
void doBinaryAddition_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+from->type](&element, from, target);
}

void doBinaryAddition_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryAddition[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryAddition_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+WR_INT](&element, from, target);
}

void doBinaryAddition_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+WR_FLOAT](&element, from, target);
}

void doBinaryAddition_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryAddition[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryAddition_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryAddition[WR_INT*6+element.type](to, &element, target);
}

void doBinaryAddition_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryAddition[WR_FLOAT*6+element.type](to, &element, target);
}

void doBinaryAddition_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryAddition_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+WR_FLOAT](to->r, from, target); }
void doBinaryAddition_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryAddition_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+WR_INT](to->r, from, target); }
void doBinaryAddition_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[WR_FLOAT*6+from->r->type](to, from->r, target); }

void doBinaryAddition_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i + from->i; }
void doBinaryAddition_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i + (int)from->f; }
void doBinaryAddition_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f + (float)from->i; }
void doBinaryAddition_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f + from->f; }

WRTargetFunc wr_binaryAddition[36] = 
{
	doBinaryAddition_I_I,  doBinaryAddition_I_R,  doBinaryAddition_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_I_Y,
	doBinaryAddition_R_I,  doBinaryAddition_R_R,  doBinaryAddition_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_R_Y,
	doBinaryAddition_F_I,  doBinaryAddition_F_R,  doBinaryAddition_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryAddition_Y_I,  doBinaryAddition_Y_R,  doBinaryAddition_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_Y_Y,
};

WRTargetFunc wr_binaryAddition2[36] = 
{
	doBinaryAddition_I_I,  doBinaryAddition_I_R,  doBinaryAddition_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_I_Y,
	doBinaryAddition_R_I,  doBinaryAddition_R_R,  doBinaryAddition_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_R_Y,
	doBinaryAddition_F_I,  doBinaryAddition_F_R,  doBinaryAddition_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryAddition_Y_I,  doBinaryAddition_Y_R,  doBinaryAddition_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_Y_Y,
};
//==============================================================================





//------------------------------------------------------------------------------
void doBinaryMultiply_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+from->type](&element, from, target);
}

void doBinaryMultiply_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryMultiply[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryMultiply_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+WR_INT](&element, from, target);
}

void doBinaryMultiply_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+WR_FLOAT](&element, from, target);
}

void doBinaryMultiply_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryMultiply[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryMultiply_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryMultiply[WR_INT*6+element.type](to, &element, target);
}

void doBinaryMultiply_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryMultiply[WR_FLOAT*6+element.type](to, &element, target);
}

void doBinaryMultiply_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryMultiply_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+WR_FLOAT](to->r, from, target); }
void doBinaryMultiply_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryMultiply_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+WR_INT](to->r, from, target); }
void doBinaryMultiply_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[WR_FLOAT*6+from->r->type](to, from->r, target); }

void doBinaryMultiply_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i * from->i; }
void doBinaryMultiply_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i * (int)from->f; }
void doBinaryMultiply_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f * (float)from->i; }
void doBinaryMultiply_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f * from->f; }

WRTargetFunc wr_binaryMultiply[36] = 
{
	doBinaryMultiply_I_I,  doBinaryMultiply_I_R,  doBinaryMultiply_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_I_Y,
	doBinaryMultiply_R_I,  doBinaryMultiply_R_R,  doBinaryMultiply_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_R_Y,
	doBinaryMultiply_F_I,  doBinaryMultiply_F_R,  doBinaryMultiply_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryMultiply_Y_I,  doBinaryMultiply_Y_R,  doBinaryMultiply_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_Y_Y,
};

//==============================================================================






//------------------------------------------------------------------------------
void doBinarySubtract_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+from->type](&element, from, target);
}

void doBinarySubtract_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binarySubtract[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinarySubtract_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+WR_INT](&element, from, target);
}

void doBinarySubtract_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+WR_FLOAT](&element, from, target);
}

void doBinarySubtract_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binarySubtract[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinarySubtract_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binarySubtract[WR_INT*6+element.type](to, &element, target);
}

void doBinarySubtract_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binarySubtract[WR_FLOAT*6+element.type](to, &element, target);
}

void doBinarySubtract_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[WR_INT*6+from->r->type](to, from->r, target); }
void doBinarySubtract_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+WR_FLOAT](to->r, from, target); }
void doBinarySubtract_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinarySubtract_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+WR_INT](to->r, from, target); }
void doBinarySubtract_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[WR_FLOAT*6+from->r->type](to, from->r, target); }

void doBinarySubtract_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i - from->i; }
void doBinarySubtract_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i - (int)from->f; }
void doBinarySubtract_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - (float)from->i; }
void doBinarySubtract_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - from->f; }

WRTargetFunc wr_binarySubtract[36] = 
{
	doBinarySubtract_I_I,  doBinarySubtract_I_R,  doBinarySubtract_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_I_Y,
	doBinarySubtract_R_I,  doBinarySubtract_R_R,  doBinarySubtract_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_R_Y,
	doBinarySubtract_F_I,  doBinarySubtract_F_R,  doBinarySubtract_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinarySubtract_Y_I,  doBinarySubtract_Y_R,  doBinarySubtract_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_Y_Y,
};

//==============================================================================






//------------------------------------------------------------------------------
void doBinaryDivide_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+from->type](&element, from, target);
}

void doBinaryDivide_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryDivide[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryDivide_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+WR_INT](&element, from, target);
}

void doBinaryDivide_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+WR_FLOAT](&element, from, target);
}

void doBinaryDivide_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryDivide[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryDivide_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryDivide[WR_INT*6+element.type](to, &element, target);
}

void doBinaryDivide_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryDivide[WR_FLOAT*6+element.type](to, &element, target);
}

void doBinaryDivide_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryDivide_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+WR_FLOAT](to->r, from, target); }
void doBinaryDivide_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryDivide_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+WR_INT](to->r, from, target); }
void doBinaryDivide_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[WR_FLOAT*6+from->r->type](to, from->r, target); }

void doBinaryDivide_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i / from->i; }
void doBinaryDivide_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i / (int)from->f; }
void doBinaryDivide_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / (float)from->i; }
void doBinaryDivide_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / from->f; }

WRTargetFunc wr_binaryDivide[36] = 
{
	doBinaryDivide_I_I,  doBinaryDivide_I_R,  doBinaryDivide_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_I_Y,
	doBinaryDivide_R_I,  doBinaryDivide_R_R,  doBinaryDivide_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_R_Y,
	doBinaryDivide_F_I,  doBinaryDivide_F_R,  doBinaryDivide_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryDivide_Y_I,  doBinaryDivide_Y_R,  doBinaryDivide_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_Y_Y,
};

//==============================================================================







//------------------------------------------------------------------------------
void doBinaryMod_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryMod[element.type*6+from->type](&element, from, target);
}

void doBinaryMod_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryMod[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryMod_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryMod[element.type*6+WR_INT](&element, from, target);
}

void doBinaryMod_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryMod[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryMod_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryMod[WR_INT*6+element.type](to, &element, target);
}

void doBinaryMod_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryMod_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryMod_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryMod_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i % from->i; }

WRTargetFunc wr_binaryMod[36] = 
{
	doBinaryMod_I_I,  doBinaryMod_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_I_Y,
	doBinaryMod_R_I,  doBinaryMod_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryMod_Y_I,  doBinaryMod_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_Y_Y,
};

//==============================================================================











//------------------------------------------------------------------------------
void doBinaryLeftShift_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryLeftShift[element.type*6+from->type](&element, from, target);
}

void doBinaryLeftShift_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryLeftShift[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryLeftShift_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryLeftShift[element.type*6+WR_INT](&element, from, target);
}

void doBinaryLeftShift_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryLeftShift[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryLeftShift_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryLeftShift[WR_INT*6+element.type](to, &element, target);
}

void doBinaryLeftShift_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryLeftShift_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryLeftShift_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryLeftShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i << from->i; }

WRTargetFunc wr_binaryLeftShift[36] = 
{
	doBinaryLeftShift_I_I,  doBinaryLeftShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_I_Y,
	doBinaryLeftShift_R_I,  doBinaryLeftShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryLeftShift_Y_I,  doBinaryLeftShift_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_Y_Y,
};

//==============================================================================




//------------------------------------------------------------------------------
void doBinaryRightShift_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryRightShift[element.type*6+from->type](&element, from, target);
}

void doBinaryRightShift_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryRightShift[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryRightShift_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryRightShift[element.type*6+WR_INT](&element, from, target);
}

void doBinaryRightShift_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryRightShift[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryRightShift_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryRightShift[WR_INT*6+element.type](to, &element, target);
}

void doBinaryRightShift_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryRightShift_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryRightShift_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryRightShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i >> from->i; }

WRTargetFunc wr_binaryRightShift[36] = 
{
	doBinaryRightShift_I_I,  doBinaryRightShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_I_Y,
	doBinaryRightShift_R_I,  doBinaryRightShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryRightShift_Y_I,  doBinaryRightShift_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_Y_Y,
};

//==============================================================================



//------------------------------------------------------------------------------
void doBinaryOR_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryOR[element.type*6+from->type](&element, from, target);
}

void doBinaryOR_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryOR[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryOR_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryOR[element.type*6+WR_INT](&element, from, target);
}

void doBinaryOR_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryOR[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryOR_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryOR[WR_INT*6+element.type](to, &element, target);
}

void doBinaryOR_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryOR_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryOR_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryOR_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i | from->i; }

WRTargetFunc wr_binaryOR[36] = 
{
	doBinaryOR_I_I,  doBinaryOR_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_I_Y,
	doBinaryOR_R_I,  doBinaryOR_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryOR_Y_I,  doBinaryOR_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_Y_Y,
};

//==============================================================================




//------------------------------------------------------------------------------
void doBinaryAND_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryAND[element.type*6+from->type](&element, from, target);
}

void doBinaryAND_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryAND[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryAND_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryAND[element.type*6+WR_INT](&element, from, target);
}

void doBinaryAND_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryAND[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryAND_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryAND[WR_INT*6+element.type](to, &element, target);
}

void doBinaryAND_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryAND_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryAND_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryAND_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i & from->i; }

WRTargetFunc wr_binaryAND[36] = 
{
	doBinaryAND_I_I,  doBinaryAND_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_I_Y,
	doBinaryAND_R_I,  doBinaryAND_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryAND_Y_I,  doBinaryAND_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_Y_Y,
};

//==============================================================================





//------------------------------------------------------------------------------
void doBinaryXOR_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryXOR[element.type*6+from->type](&element, from, target);
}

void doBinaryXOR_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryXOR[to->r->type*6+element.type]( to->r, &element, target);
}

void doBinaryXOR_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( to, &element );
	wr_binaryXOR[element.type*6+WR_INT](&element, from, target);
}

void doBinaryXOR_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	wr_binaryXOR[element1.type*6+element2.type](&element1, &element2, target);
}

void doBinaryXOR_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	arrayToValue( from, &element );
	wr_binaryXOR[WR_INT*6+element.type](to, &element, target);
}

void doBinaryXOR_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[WR_INT*6+from->r->type](to, from->r, target); }
void doBinaryXOR_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[to->r->type*6+from->r->type](to->r, from->r, target); }
void doBinaryXOR_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[to->r->type*6+WR_INT](to->r, from, target); }

void doBinaryXOR_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i ^ from->i; }

WRTargetFunc wr_binaryXOR[36] = 
{
	doBinaryXOR_I_I,  doBinaryXOR_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_I_Y,
	doBinaryXOR_R_I,  doBinaryXOR_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryXOR_Y_I,  doBinaryXOR_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_Y_Y,
};

//==============================================================================




//------------------------------------------------------------------------------
bool doCompareEQ_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	return wr_CompareEQ[element1.type*6+element2.type](&element1, &element2);
}

bool doCompareEQ_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareEQ[to->type*6+element.type](to, &element);
}

bool doCompareEQ_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+from->type](&element, from);
}

bool doCompareEQ_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+WR_INT](&element, from);
}

bool doCompareEQ_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+WR_FLOAT](&element, from);
}

bool doCompareEQ_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareEQ[WR_INT*6+element.type](to, &element);
}
bool doCompareEQ_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareEQ[WR_FLOAT*6+element.type](to, &element);
}

bool doCompareEQ_R_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+from->r->type](to->r, from->r); }
bool doCompareEQ_R_I( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+WR_INT](to->r, from); }
bool doCompareEQ_R_F( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+WR_FLOAT](to->r, from); }
bool doCompareEQ_I_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[WR_INT*6+from->r->type](to, from->r); }
bool doCompareEQ_F_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[WR_FLOAT*6+from->r->type](to, from->r); }

bool doCompareEQ_I_I( WRValue* to, WRValue* from ) { return to->i == from->i; }
bool doCompareEQ_I_F( WRValue* to, WRValue* from ) { return (float)to->i == from->f; }
bool doCompareEQ_F_I( WRValue* to, WRValue* from ) { return to->f == (float)from->i; }
bool doCompareEQ_F_F( WRValue* to, WRValue* from ) { return to->f == from->f; }

WRReturnFunc wr_CompareEQ[36] = 
{
	doCompareEQ_I_I,   doCompareEQ_I_R,   doCompareEQ_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_I_Y,
	doCompareEQ_R_I,   doCompareEQ_R_R,   doCompareEQ_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_R_Y,
	doCompareEQ_F_I,   doCompareEQ_F_R,   doCompareEQ_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareEQ_Y_I,   doCompareEQ_Y_R,   doCompareEQ_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_Y_Y,
};
//==============================================================================





//------------------------------------------------------------------------------
bool doCompareGT_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	return wr_CompareGT[element1.type*6+element2.type](&element1, &element2);
}

bool doCompareGT_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareGT[to->type*6+element.type](to, &element);
}

bool doCompareGT_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+from->type](&element, from);
}

bool doCompareGT_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+WR_INT](&element, from);
}

bool doCompareGT_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+WR_FLOAT](&element, from);
}

bool doCompareGT_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareGT[WR_INT*6+element.type](to, &element);
}
bool doCompareGT_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareGT[WR_FLOAT*6+element.type](to, &element);
}

bool doCompareGT_R_R( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+from->r->type](to->r, from->r); }
bool doCompareGT_R_I( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+WR_INT](to->r, from); }
bool doCompareGT_R_F( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+WR_FLOAT](to->r, from); }
bool doCompareGT_I_R( WRValue* to, WRValue* from ) { return wr_CompareGT[WR_INT*6+from->r->type](to, from->r); }
bool doCompareGT_F_R( WRValue* to, WRValue* from ) { return wr_CompareGT[WR_FLOAT*6+from->r->type](to, from->r); }

bool doCompareGT_I_I( WRValue* to, WRValue* from ) { return to->i > from->i; }
bool doCompareGT_I_F( WRValue* to, WRValue* from ) { return (float)to->i > from->f; }
bool doCompareGT_F_I( WRValue* to, WRValue* from ) { return to->f > (float)from->i; }
bool doCompareGT_F_F( WRValue* to, WRValue* from ) { return to->f > from->f; }

WRReturnFunc wr_CompareGT[36] = 
{
	doCompareGT_I_I,   doCompareGT_I_R,   doCompareGT_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_I_Y,
	doCompareGT_R_I,   doCompareGT_R_R,   doCompareGT_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_R_Y,
	doCompareGT_F_I,   doCompareGT_F_R,   doCompareGT_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareGT_Y_I,   doCompareGT_Y_R,   doCompareGT_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_Y_Y,
};
//==============================================================================



//------------------------------------------------------------------------------
bool doCompareLT_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	return wr_CompareLT[element1.type*6+element2.type](&element1, &element2);
}

bool doCompareLT_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareLT[to->type*6+element.type](to, &element);
}

bool doCompareLT_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+from->type](&element, from);
}

bool doCompareLT_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+WR_INT](&element, from);
}

bool doCompareLT_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+WR_FLOAT](&element, from);
}

bool doCompareLT_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareLT[WR_INT*6+element.type](to, &element);
}
bool doCompareLT_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_CompareLT[WR_FLOAT*6+element.type](to, &element);
}

bool doCompareLT_R_R( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+from->r->type](to->r, from->r); }
bool doCompareLT_R_I( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+WR_INT](to->r, from); }
bool doCompareLT_R_F( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+WR_FLOAT](to->r, from); }
bool doCompareLT_I_R( WRValue* to, WRValue* from ) { return wr_CompareLT[WR_INT*6+from->r->type](to, from->r); }
bool doCompareLT_F_R( WRValue* to, WRValue* from ) { return wr_CompareLT[WR_FLOAT*6+from->r->type](to, from->r); }

bool doCompareLT_I_I( WRValue* to, WRValue* from ) { return to->i < from->i; }
bool doCompareLT_I_F( WRValue* to, WRValue* from ) { return (float)to->i < from->f; }
bool doCompareLT_F_I( WRValue* to, WRValue* from ) { return to->f < (float)from->i; }
bool doCompareLT_F_F( WRValue* to, WRValue* from ) { return to->f < from->f; }

WRReturnFunc wr_CompareLT[36] = 
{
	doCompareLT_I_I,   doCompareLT_I_R,   doCompareLT_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_I_Y,
	doCompareLT_R_I,   doCompareLT_R_R,   doCompareLT_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_R_Y,
	doCompareLT_F_I,   doCompareLT_F_R,   doCompareLT_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareLT_Y_I,   doCompareLT_Y_R,   doCompareLT_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_Y_Y,
};
//==============================================================================



//------------------------------------------------------------------------------
bool doLogicalAND_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	return wr_LogicalAND[element1.type*6+element2.type](&element1, &element2);
}

bool doLogicalAND_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalAND[to->type*6+element.type](to, &element);
}

bool doLogicalAND_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+from->type](&element, from);
}

bool doLogicalAND_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+WR_INT](&element, from);
}

bool doLogicalAND_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+WR_FLOAT](&element, from);
}

bool doLogicalAND_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalAND[WR_INT*6+element.type](to, &element);
}
bool doLogicalAND_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalAND[WR_FLOAT*6+element.type](to, &element);
}

bool doLogicalAND_R_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+from->r->type](to->r, from->r); }
bool doLogicalAND_R_I( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+WR_INT](to->r, from); }
bool doLogicalAND_R_F( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+WR_FLOAT](to->r, from); }
bool doLogicalAND_I_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[WR_INT*6+from->r->type](to, from->r); }
bool doLogicalAND_F_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[WR_FLOAT*6+from->r->type](to, from->r); }

bool doLogicalAND_I_I( WRValue* to, WRValue* from ) { return to->i && from->i; }
bool doLogicalAND_I_F( WRValue* to, WRValue* from ) { return (float)to->i && from->f; }
bool doLogicalAND_F_I( WRValue* to, WRValue* from ) { return to->f && (float)from->i; }
bool doLogicalAND_F_F( WRValue* to, WRValue* from ) { return to->f && from->f; }

WRReturnFunc wr_LogicalAND[36] = 
{
	doLogicalAND_I_I,   doLogicalAND_I_R,   doLogicalAND_I_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_I_Y,
	doLogicalAND_R_I,   doLogicalAND_R_R,   doLogicalAND_R_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_R_Y,
	doLogicalAND_F_I,   doLogicalAND_F_R,   doLogicalAND_F_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doLogicalAND_Y_I,   doLogicalAND_Y_R,   doLogicalAND_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_Y_Y,
};
//==============================================================================



//------------------------------------------------------------------------------
bool doLogicalOR_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	arrayToValue( to, &element1 );
	WRValue element2;
	arrayToValue( from, &element2 );
	return wr_LogicalOR[element1.type*6+element2.type](&element1, &element2);
}

bool doLogicalOR_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalOR[to->type*6+element.type](to, &element);
}

bool doLogicalOR_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+from->type](&element, from);
}

bool doLogicalOR_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+WR_INT](&element, from);
}

bool doLogicalOR_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+WR_FLOAT](&element, from);
}

bool doLogicalOR_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalOR[WR_INT*6+element.type](to, &element);
}
bool doLogicalOR_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	arrayToValue( from, &element );
	return wr_LogicalOR[WR_FLOAT*6+element.type](to, &element);
}

bool doLogicalOR_R_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+from->r->type](to->r, from->r); }
bool doLogicalOR_R_I( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+WR_INT](to->r, from); }
bool doLogicalOR_R_F( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+WR_FLOAT](to->r, from); }
bool doLogicalOR_I_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[WR_INT*6+from->r->type](to, from->r); }
bool doLogicalOR_F_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[WR_FLOAT*6+from->r->type](to, from->r); }

bool doLogicalOR_I_I( WRValue* to, WRValue* from ) { return to->i || from->i; }
bool doLogicalOR_I_F( WRValue* to, WRValue* from ) { return (float)to->i || from->f; }
bool doLogicalOR_F_I( WRValue* to, WRValue* from ) { return to->f || (float)from->i; }
bool doLogicalOR_F_F( WRValue* to, WRValue* from ) { return to->f || from->f; }

WRReturnFunc wr_LogicalOR[36] = 
{
	doLogicalOR_I_I,   doLogicalOR_I_R,   doLogicalOR_I_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_I_Y,
	doLogicalOR_R_I,   doLogicalOR_R_R,   doLogicalOR_R_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_R_Y,
	doLogicalOR_F_I,   doLogicalOR_F_R,   doLogicalOR_F_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doLogicalOR_Y_I,   doLogicalOR_Y_R,   doLogicalOR_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_Y_Y,
};
//==============================================================================



//------------------------------------------------------------------------------
void doIndex_I_X( WRRunContext* c, WRValue* index, WRValue* value )
{
	// indexing with an int, but what we are indexing is NOT an array,
	// make it one and return a ref
	value->type = WR_ARRAY;
	value->va = c->getSVA( index->i+1 );

	value->r = value->asValueArray() + index->i;
	value->type = WR_REF;

}
void doIndex_I_R( WRRunContext* c, WRValue* index, WRValue* value ) 
{
	// indexing with an int into a ref, is it an array?
	if ( value->r->type != WR_ARRAY )
	{
		// nope, make it one and return a ref
		value->r->type = WR_ARRAY;
		value->r->va = c->getSVA( index->i+1 );

		value->r = value->r->asValueArray() + index->i;
		value->type = WR_REF;
	}
	else
	{
		// yes it is, index it
		if ( (value->r->va->m_type&0x3) == SV_VALUE )
		{
			// value is easy, return a ref to the value
			value->r = value->r->asValueArray() + index->i;
			value->type = WR_REF;
		}
		else
		{
			// this is a native array, value remains a reference to an array, but set the
			// element to point to the indexed value
			value->arrayElement = index->i << 8;
			value->type = WR_REFARRAY;
		}
	}
}

void doIndex_I_A( WRRunContext* c, WRValue* index, WRValue* value )
{
	if ( (value->va->m_type&0x3) == SV_VALUE )
	{
		value->r = value->asValueArray() + index->i;
		value->type = WR_REF;
	}
}

void doIndex_R_I( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_INT](c, index->r, value);
}

void doIndex_R_R( WRRunContext* c, WRValue* index, WRValue* value )
{
	if ( value->r->type != WR_USR )
	{
		if ( value->r->type != WR_ARRAY )
		{
			// indexing something that isn't a USR or ARRAY, make it an
			// array
			value->r->type = WR_ARRAY;
			value->r->va = c->getSVA( index->i+1 );
		}

		wr_index[index->r->type*6+WR_REF](c, index->r, value);
	}
}
void doIndex_R_F( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_FLOAT](c, index->r, value);
}
void doIndex_R_A( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_ARRAY](c, index->r, value);
}
WRStateFunc wr_index[36] = 
{
	doIndex_I_X,     doIndex_I_R,     doIndex_I_X, doVoidIndexFunc,     doIndex_I_A,     doIndex_I_R,
	doIndex_R_I,     doIndex_R_R,     doIndex_R_F, doVoidIndexFunc,     doIndex_R_A, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
};

//------------------------------------------------------------------------------
void doPreInc_I( WRValue* value ) { ++value->i; }
void doPreInc_Y( WRValue* value )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
			wr_preinc[ val->type ]( val );
			*value = *val;
			return;
		}

		case SV_CHAR: {	value->i = ++((char *)value->r->va->m_data)[s]; value->type = WR_INT; return; }
		case SV_INT: { value->i = ++((int *)value->r->va->m_data)[s]; value->type = WR_INT; return; }
		case SV_FLOAT: { value->f = ++((float *)value->r->va->m_data)[s]; value->type = WR_FLOAT; return; }
	}
}

void doPreInc_R( WRValue* value )
{
	wr_preinc[ value->r->type ]( value->r );
	*value = *value->r;
}
void doPreInc_F( WRValue* value )
{
	++value->f;
}

WRUnaryFunc wr_preinc[6] = 
{
	doPreInc_I,  doPreInc_R,  doPreInc_F,  doSingleVoidBlank,  doSingleVoidBlank,  doPreInc_Y
};

//------------------------------------------------------------------------------
void doPreDec_I( WRValue* value ) { --value->i; }
void doPreDec_Y( WRValue* value )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
			wr_predec[ val->type ]( val );
			*value = *val;
			return;
		}
		
		case SV_CHAR: {	value->i = --((char *)value->r->va->m_data)[s];	value->type = WR_INT; return; }
		case SV_INT: { value->i = --((int *)value->r->va->m_data)[s]; value->type = WR_INT; return; }
		case SV_FLOAT: { value->f = --((float *)value->r->va->m_data)[s]; value->type = WR_FLOAT; return; }
	}
}
void doPreDec_R( WRValue* value )
{
	wr_predec[ value->r->type ]( value->r );
	*value = *value->r;
}

void doPreDec_F( WRValue* value ) { --value->f; }
WRUnaryFunc wr_predec[6] = 
{
	doPreDec_I,  doPreDec_R,  doPreDec_F,  doSingleVoidBlank,  doSingleVoidBlank,  doPreDec_Y
};

//------------------------------------------------------------------------------
void doPostInc_I( WRValue* value, WRValue* stack ) { *stack = *value; ++value->i; }
void doPostInc_Y( WRValue* value, WRValue* stack )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
			wr_postinc[ val->type ]( val, stack );
			break;
		}

		case SV_CHAR:
		{
			stack->type = WR_INT;
			char c = (((char*)value->r->va->m_data)[s])++;
			stack->i = c;
			break;
		}

		case SV_INT:
		{
			stack->type = WR_INT;
			int i = (((int*)value->r->va->m_data)[s])++;
			stack->i = i;
			break;
		}

		case SV_FLOAT:
		{
			stack->type = WR_FLOAT;
			float f = (((float*)value->r->va->m_data)[s])++;
			stack->f = f;
			break;
		}
	}
}

void doPostInc_R( WRValue* value, WRValue* stack )
{
	wr_postinc[ value->r->type ]( value->r, stack );
}

void doPostInc_F( WRValue* value, WRValue* stack ) { *stack = *value; ++value->f; }
WRVoidFunc wr_postinc[6] = 
{
	doPostInc_I,  doPostInc_R,  doPostInc_F,  doVoidFuncBlank,  doVoidFuncBlank, doPostInc_Y
};

//------------------------------------------------------------------------------
void doPostDec_I( WRValue* value, WRValue* stack ) { *stack = *value; --value->i; }
void doPostDec_Y( WRValue* value, WRValue* stack )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
			wr_postdec[ val->type ]( val, stack );
			break;
		}

		case SV_CHAR:
		{
			stack->type = WR_INT;
			char c = (((char*)value->r->va->m_data)[s])--;
			stack->i = c;
			break;
		}

		case SV_INT:
		{
			stack->type = WR_INT;
			int i = (((char*)value->r->va->m_data)[s])--;
			stack->i = i;
			break;
		}

		case SV_FLOAT:
		{
			stack->type = WR_FLOAT;
			float f = (((char*)value->r->va->m_data)[s])--;
			stack->f = f;
			break;
		}
	}
}

void doPostDec_R( WRValue* value, WRValue* stack )
{
	wr_postdec[ value->r->type ]( value->r, stack );
}

void doPostDec_F( WRValue* value, WRValue* stack ) { *stack = *value; --value->f; }
WRVoidFunc wr_postdec[6] = 
{
	doPostDec_I, doPostDec_R, doPostDec_F, doVoidFuncBlank, doVoidFuncBlank, doPostDec_Y
};

//------------------------------------------------------------------------------
void doUserHash_X( WRValue* value, WRValue* target, int32_t hash ) { }
void doUserHash_R( WRValue* value, WRValue* target, int32_t hash )
{
	wr_UserHash[ value->r->type ]( value->r, target, hash );
}
void doUserHash_U( WRValue* value, WRValue* target, int32_t hash )
{
	if ( !(target->r = value->u->get(hash)) )
	{
		target->init();
	}
	else// if ( target->r->type == WR_ARRAY )
	{
		target->type = WR_REF;
	}
}
WRUserHashFunc wr_UserHash[6] = 
{
	doUserHash_X,  doUserHash_R,  doUserHash_X,  doUserHash_U,  doUserHash_X, doUserHash_X
};

//------------------------------------------------------------------------------
bool doZeroCheck_I( WRValue* value ) { return value->i == 0; }
bool doZeroCheck_R( WRValue* value )
{
	return wr_ZeroCheck[ value->r->type ]( value->r );
}
bool doZeroCheck_Y( WRValue* value )
{
	WRValue element;
	arrayToValue( value, &element );
	return wr_ZeroCheck[ element.type ]( &element );
}

bool doZeroCheck_F( WRValue* value ) { return value->f == 0; }
WRValueCheckFunc wr_ZeroCheck[6] = 
{
	doZeroCheck_I,  doZeroCheck_R,  doZeroCheck_F,  doSingleBlank, doSingleBlank, doZeroCheck_Y
};


//------------------------------------------------------------------------------
bool doLogicalNot_I( WRValue* value ) { return value->i == 0; }
bool doLogicalNot_R( WRValue* value )
{
	return wr_LogicalNot[ value->r->type ]( value->r );
}
bool doLogicalNot_Y( WRValue* value )
{
	WRValue element;
	arrayToValue( value, &element );
	return wr_LogicalNot[ element.type ]( &element );
}
bool doLogicalNot_F( WRValue* value ) { return value->f == 0; }
WRReturnSingleFunc wr_LogicalNot[6] = 
{
	doLogicalNot_I,  doLogicalNot_R,  doLogicalNot_F,  doSingleBlank, doSingleBlank, doLogicalNot_Y
};

//------------------------------------------------------------------------------
void doNegate_I( WRValue* value ) { value->i = -value->i; }
void doNegate_Y( WRValue* value )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
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
			value->type = WR_INT;
			char c = ((((char*)value->r->va->m_data)[s]) = -(((char*)value->r->va->m_data)[s]));
			value->i = c;
			break;
		}

		case SV_INT:
		{
			value->type = WR_INT;
			int i = ((((int*)value->r->va->m_data)[s]) = -(((int*)value->r->va->m_data)[s]));
			value->i = i;
			break;
		}

		case SV_FLOAT:
		{
			value->type = WR_FLOAT;
			float f = ((((float*)value->r->va->m_data)[s]) = -(((float*)value->r->va->m_data)[s]));
			value->f = f;
			break;
		}
	}
}
void doNegate_R( WRValue* value )
{
	wr_negate[ value->r->type ]( value->r );
	*value = *value->r;
}

void doNegate_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_negate[6] = 
{
	doNegate_I,  doNegate_R,  doNegate_F,  doSingleVoidBlank,  doSingleVoidBlank, doNegate_Y
};

//------------------------------------------------------------------------------
void doBitwiseNot_I( WRValue* value ) { value->i = -value->i; }
void doBitwiseNot_R( WRValue* value )
{
	wr_BitwiseNot[ value->r->type ]( value->r );
	*value = *value->r;
}
void doBitwiseNot_Y( WRValue* value )
{
	unsigned int index = value->arrayElement >> 8;
	int s = index < value->r->va->m_size ? index : value->r->va->m_size - 1;

	switch( value->r->va->m_type&0x3 )
	{
		case SV_VALUE:
		{
			WRValue* val = (WRValue *)value->r->va->m_data + s;
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
			value->type = WR_INT;
			char c = ((((char*)value->r->va->m_data)[s]) = ~(((char*)value->r->va->m_data)[s]));
			value->i = c;
			break;
		}

		case SV_INT:
		{
			value->type = WR_INT;
			int i = ((((int*)value->r->va->m_data)[s]) = ~(((int*)value->r->va->m_data)[s]));
			value->i = i;
			break;
		}

		case SV_FLOAT:
		{
			break;
		}
	}
}

void doBitwiseNot_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_BitwiseNot[6] = 
{
	doBitwiseNot_I,  doBitwiseNot_R,  doBitwiseNot_F,  doSingleVoidBlank,  doSingleVoidBlank,doBitwiseNot_Y
};
