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
#include <assert.h>

/*
{ I_I, I_R, I_F, I_U, I_A }
{ R_I, R_R, R_F, R_U, R_A }
{ F_I, F_R, F_F, F_U, F_A }
{ U_I, U_R, U_F, U_U, U_A }
{ A_I, A_R, A_F, A_U, A_A }
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

//------------------------------------------------------------------------------
void doAssign_R_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_assign[to->type][element.type](to, &element);
	}
	else
	{
		wr_assign[to->type][from->r->type](to, from->r);
	}
}

void doAssign_X_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		*to = element;
	}
	else
	{
		to->p = from->r->p;
		to->type = from->r->type;
	}
}

void doAssign_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		floatValueToArray( to, from->f );
	}
	else
	{
		to->r->p = from->p;
		to->r->type = from->type;
	}
}

void doAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		intValueToArray( to, from->i );
	}
	else
	{
		to->r->p = from->p;
		to->r->type = from->type;
	}
}

void doAssign_X_X( WRValue* to, WRValue* from )
{
	to->p = from->p;
	to->type = from->type;
}

WRVoidFunc wr_assign[5][5] = 
{
	{     doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank },
	{     doAssign_R_I,    doAssign_R_R,     doAssign_R_F,  doVoidFuncBlank,  doVoidFuncBlank },
	{     doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank },
	{  doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank },
	{  doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank },
};


//------------------------------------------------------------------------------
void doSubtractAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_SubtractAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_SubtractAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doSubtractAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Subtracting it off
		arrayToValue( to, &element );

		wr_SubtractAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_SubtractAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doSubtractAssign_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Subtracting it off
		arrayToValue( to, &element );

		wr_SubtractAssign[element.type][WR_FLOAT]( &element, from );

		floatValueToArray( to, element.f );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_SubtractAssign[to->r->type][WR_FLOAT](to->r, from);
	}
}
void doSubtractAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_SubtractAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_SubtractAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doSubtractAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i -= from->i);
}
void doSubtractAssign_I_F( WRValue* to, WRValue* from )
{
	to->type = WR_FLOAT;
	to->f = (float)to->i - from->f;
	from->f = to->f;
}
void doSubtractAssign_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_SubtractAssign[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		wr_SubtractAssign[WR_FLOAT][from->r->type](to, from->r);
	}
}
void doSubtractAssign_F_I( WRValue* to, WRValue* from )
{
	from->type = WR_FLOAT;
	from->f = (to->f -= (float)from->i);
}
void doSubtractAssign_F_F( WRValue* to, WRValue* from )
{
	from->f = (to->f -= from->f);
}
WRVoidFunc wr_SubtractAssign[5][5] = 
{
	{  doSubtractAssign_I_I,  doSubtractAssign_I_R,  doSubtractAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doSubtractAssign_R_I,  doSubtractAssign_R_R,  doSubtractAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doSubtractAssign_F_I,  doSubtractAssign_F_R,  doSubtractAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doAddAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_AddAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_AddAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doAddAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Adding it off
		arrayToValue( to, &element );

		wr_AddAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_AddAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doAddAssign_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Adding it off
		arrayToValue( to, &element );

		wr_AddAssign[element.type][WR_FLOAT]( &element, from );

		floatValueToArray( to, element.f );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_AddAssign[to->r->type][WR_FLOAT](to->r, from);
	}
}
void doAddAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_AddAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_AddAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doAddAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i += from->i);
}
void doAddAssign_I_F( WRValue* to, WRValue* from )
{
	to->type = WR_FLOAT;
	to->f = (float)to->i + from->f;
	from->f = to->f;
}
void doAddAssign_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_AddAssign[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		wr_AddAssign[WR_FLOAT][from->r->type](to, from->r);
	}
}
void doAddAssign_F_I( WRValue* to, WRValue* from )
{
	from->type = WR_FLOAT;
	from->f = (to->f += (float)from->i);
}
void doAddAssign_F_F( WRValue* to, WRValue* from )
{
	from->f = (to->f += from->f);
}
WRVoidFunc wr_AddAssign[5][5] = 
{
	{  doAddAssign_I_I,  doAddAssign_I_R,  doAddAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doAddAssign_R_I,  doAddAssign_R_R,  doAddAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doAddAssign_F_I,  doAddAssign_F_R,  doAddAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};


//------------------------------------------------------------------------------
void doMultiplyAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_MultiplyAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_MultiplyAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doMultiplyAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Multiplying it off
		arrayToValue( to, &element );

		wr_MultiplyAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_MultiplyAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doMultiplyAssign_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Multiplying it off
		arrayToValue( to, &element );

		wr_MultiplyAssign[element.type][WR_FLOAT]( &element, from );

		floatValueToArray( to, element.f );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_MultiplyAssign[to->r->type][WR_FLOAT](to->r, from);
	}
}
void doMultiplyAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_MultiplyAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_MultiplyAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doMultiplyAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i *= from->i);
}
void doMultiplyAssign_I_F( WRValue* to, WRValue* from )
{
	to->type = WR_FLOAT;
	to->f = (float)to->i * from->f;
	from->f = to->f;
}
void doMultiplyAssign_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_MultiplyAssign[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		wr_MultiplyAssign[WR_FLOAT][from->r->type](to, from->r);
	}
}
void doMultiplyAssign_F_I( WRValue* to, WRValue* from )
{
	from->type = WR_FLOAT;
	from->f = (to->f *= (float)from->i);
}
void doMultiplyAssign_F_F( WRValue* to, WRValue* from )
{
	from->f = (to->f *= from->f);
}
WRVoidFunc wr_MultiplyAssign[5][5] = 
{
	{  doMultiplyAssign_I_I,  doMultiplyAssign_I_R,  doMultiplyAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doMultiplyAssign_R_I,  doMultiplyAssign_R_R,  doMultiplyAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doMultiplyAssign_F_I,  doMultiplyAssign_F_R,  doMultiplyAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};


//------------------------------------------------------------------------------
void doDivideAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_DivideAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_DivideAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doDivideAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Divideing it off
		arrayToValue( to, &element );

		wr_DivideAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_DivideAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doDivideAssign_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Divideing it off
		arrayToValue( to, &element );

		wr_DivideAssign[element.type][WR_FLOAT]( &element, from );

		floatValueToArray( to, element.f );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_DivideAssign[to->r->type][WR_FLOAT](to->r, from);
	}
}
void doDivideAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_DivideAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_DivideAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doDivideAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i /= from->i);
}
void doDivideAssign_I_F( WRValue* to, WRValue* from )
{
	to->type = WR_FLOAT;
	to->f = (float)to->i / from->f;
	from->f = to->f;
}
void doDivideAssign_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_DivideAssign[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		wr_DivideAssign[WR_FLOAT][from->r->type](to, from->r);
	}
}
void doDivideAssign_F_I( WRValue* to, WRValue* from )
{
	from->type = WR_FLOAT;
	from->f = (to->f /= (float)from->i);
}
void doDivideAssign_F_F( WRValue* to, WRValue* from )
{
	from->f = (to->f /= from->f);
}
WRVoidFunc wr_DivideAssign[5][5] = 
{
	{  doDivideAssign_I_I,  doDivideAssign_I_R,  doDivideAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doDivideAssign_R_I,  doDivideAssign_R_R,  doDivideAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank },
	{  doDivideAssign_F_I,  doDivideAssign_F_R,  doDivideAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{       doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};


//------------------------------------------------------------------------------
void doModAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_ModAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_ModAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doModAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after Moding it off
		arrayToValue( to, &element );

		wr_ModAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_ModAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doModAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_ModAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_ModAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doModAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i %= from->i);
}
WRVoidFunc wr_ModAssign[5][5] = 
{
	{  doModAssign_I_I,  doModAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doModAssign_R_I,  doModAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doORAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_ORAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_ORAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doORAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after ORing it off
		arrayToValue( to, &element );

		wr_ORAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_ORAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doORAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_ORAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_ORAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doORAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i |= from->i);
}
WRVoidFunc wr_ORAssign[5][5] = 
{
	{  doORAssign_I_I,  doORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doORAssign_R_I,  doORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doANDAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_ANDAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_ANDAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doANDAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after ANDing it off
		arrayToValue( to, &element );

		wr_ANDAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_ANDAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doANDAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_ANDAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_ANDAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doANDAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i &= from->i);
}
WRVoidFunc wr_ANDAssign[5][5] = 
{
	{  doANDAssign_I_I,  doANDAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doANDAssign_R_I,  doANDAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doXORAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_XORAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_XORAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doXORAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after XORing it off
		arrayToValue( to, &element );

		wr_XORAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_XORAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doXORAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_XORAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_XORAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doXORAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i ^= from->i);
}
WRVoidFunc wr_XORAssign[5][5] = 
{
	{  doXORAssign_I_I,  doXORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doXORAssign_R_I,  doXORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doRightShiftAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_RightShiftAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_RightShiftAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doRightShiftAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after RightShifting it off
		arrayToValue( to, &element );

		wr_RightShiftAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_RightShiftAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doRightShiftAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_RightShiftAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_RightShiftAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doRightShiftAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i >>= from->i);
}
WRVoidFunc wr_RightShiftAssign[5][5] = 
{
	{  doRightShiftAssign_I_I,  doRightShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doRightShiftAssign_R_I,  doRightShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doLeftShiftAssign_R_R( WRValue* to, WRValue* from ) 
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		// 'to' might ALSO be an array, so kick it off
		wr_LeftShiftAssign[to->type][element.type](to, &element);

		*from = element;
	}
	else
	{
		WRValue temp;
		temp.type = from->r->type;
		temp.p = from->r->p;

		wr_LeftShiftAssign[WR_REF][from->r->type](to, &temp); 

		from->type = temp.type;
		from->p = temp.p;
	}
}
void doLeftShiftAssign_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element; // proxy for 'to' come back here after LeftShifting it off
		arrayToValue( to, &element );

		wr_LeftShiftAssign[element.type][WR_INT]( &element, from );

		intValueToArray( to, element.i );
		*to = element;
		*from = *to;
	}
	else
	{
		wr_LeftShiftAssign[to->r->type][WR_INT](to->r, from);
	}
}
void doLeftShiftAssign_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_LeftShiftAssign[WR_INT][element.type](to, &element);
	}
	else
	{
		wr_LeftShiftAssign[WR_INT][from->r->type](to, from->r);
	}
}
void doLeftShiftAssign_I_I( WRValue* to, WRValue* from )
{
	from->i = (to->i <<= from->i);
}
WRVoidFunc wr_LeftShiftAssign[5][5] = 
{
	{  doLeftShiftAssign_I_I,  doLeftShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doLeftShiftAssign_R_I,  doLeftShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
	{  doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryAddition_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryAddition[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryAddition[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryAddition[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryAddition_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryAddition[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryAddition[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryAddition_R_F( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryAddition[element.type][WR_FLOAT](&element, from, target);
	}
	else
	{
		wr_binaryAddition[to->r->type][WR_FLOAT](to->r, from, target);
	}
}
void doBinaryAddition_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryAddition[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryAddition[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryAddition_I_I( WRValue* to, WRValue* from, WRValue* target ) 
{
	target->type = WR_INT; 
	target->i = from->i + to->i; 
}
void doBinaryAddition_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = (int)from->f + to->i; }
void doBinaryAddition_F_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryAddition[WR_FLOAT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryAddition[WR_FLOAT][from->r->type](to, from->r, target);
	}
}
void doBinaryAddition_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)from->i + to->f; }
void doBinaryAddition_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = from->f + to->f; }
WRTargetFunc wr_binaryAddition[5][5] = 
{
	{  doBinaryAddition_I_I,  doBinaryAddition_I_R,  doBinaryAddition_I_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryAddition_R_I,  doBinaryAddition_R_R,  doBinaryAddition_R_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryAddition_F_I,  doBinaryAddition_F_R,  doBinaryAddition_F_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryMultiply_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryMultiply[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryMultiply[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryMultiply[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryMultiply_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryMultiply[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryMultiply[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryMultiply_R_F( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryMultiply[element.type][WR_FLOAT](&element, from, target);
	}
	else
	{
		wr_binaryMultiply[to->r->type][WR_FLOAT](to->r, from, target);
	}
}
void doBinaryMultiply_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryMultiply[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryMultiply[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryMultiply_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = from->i * to->i; }
void doBinaryMultiply_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = (int)from->f * to->i; }
void doBinaryMultiply_F_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryMultiply[WR_FLOAT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryMultiply[WR_FLOAT][from->r->type](to, from->r, target);
	}
}
void doBinaryMultiply_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)from->i * to->f; }
void doBinaryMultiply_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = from->f * to->f; }
WRTargetFunc wr_binaryMultiply[5][5] = 
{
	{  doBinaryMultiply_I_I,  doBinaryMultiply_I_R,  doBinaryMultiply_I_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryMultiply_R_I,  doBinaryMultiply_R_R,  doBinaryMultiply_R_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryMultiply_F_I,  doBinaryMultiply_F_R,  doBinaryMultiply_F_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinarySubtract_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binarySubtract[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binarySubtract[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binarySubtract[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinarySubtract_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binarySubtract[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binarySubtract[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinarySubtract_R_F( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binarySubtract[element.type][WR_FLOAT](&element, from, target);
	}
	else
	{
		wr_binarySubtract[to->r->type][WR_FLOAT](to->r, from, target);
	}
}
void doBinarySubtract_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binarySubtract[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binarySubtract[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinarySubtract_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i - from->i; }
void doBinarySubtract_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i - (int)from->f; }
void doBinarySubtract_F_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binarySubtract[WR_FLOAT][element.type](to, &element, target);
	}
	else
	{
		wr_binarySubtract[WR_FLOAT][from->r->type](to, from->r, target);
	}
}
void doBinarySubtract_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - (float)from->i; }
void doBinarySubtract_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - from->f; }
WRTargetFunc wr_binarySubtract[5][5] = 
{
	{  doBinarySubtract_I_I,  doBinarySubtract_I_R,  doBinarySubtract_I_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinarySubtract_R_I,  doBinarySubtract_R_R,  doBinarySubtract_R_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinarySubtract_F_I,  doBinarySubtract_F_R,  doBinarySubtract_F_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryDivide_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryDivide[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryDivide[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryDivide[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryDivide_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryDivide[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryDivide[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryDivide_R_F( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryDivide[element.type][WR_FLOAT](&element, from, target);
	}
	else
	{
		wr_binaryDivide[to->r->type][WR_FLOAT](to->r, from, target);
	}
}
void doBinaryDivide_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryDivide[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryDivide[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryDivide_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i / from->i; }
void doBinaryDivide_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i / (int)from->f; }
void doBinaryDivide_F_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryDivide[WR_FLOAT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryDivide[WR_FLOAT][from->r->type](to, from->r, target);
	}
}
void doBinaryDivide_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / (float)from->i; }
void doBinaryDivide_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / from->f; }
WRTargetFunc wr_binaryDivide[5][5] = 
{
	{  doBinaryDivide_I_I,  doBinaryDivide_I_R,  doBinaryDivide_I_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryDivide_R_I,  doBinaryDivide_R_R,  doBinaryDivide_R_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doBinaryDivide_F_I,  doBinaryDivide_F_R,  doBinaryDivide_F_F,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{  doTargetFuncBlank,        doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryMod_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryMod[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryMod[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryMod[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryMod_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryMod[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryMod[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryMod_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryMod[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryMod[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryMod_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i % from->i; }
WRTargetFunc wr_binaryMod[5][5] = 
{
	{     doBinaryMod_I_I,     doBinaryMod_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{     doBinaryMod_R_I,     doBinaryMod_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryOr_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryOr[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryOr[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryOr[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryOr_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryOr[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryOr[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryOr_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryOr[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryOr[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryOr_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i | from->i; }
WRTargetFunc wr_binaryOr[5][5] = 
{
	{     doBinaryOr_I_I,     doBinaryOr_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{     doBinaryOr_R_I,     doBinaryOr_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryAnd_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryAnd[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryAnd[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryAnd[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryAnd_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryAnd[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryAnd[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryAnd_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryAnd[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryAnd[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryAnd_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i & from->i; }
WRTargetFunc wr_binaryAnd[5][5] = 
{
	{     doBinaryAnd_I_I,     doBinaryAnd_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{     doBinaryAnd_R_I,     doBinaryAnd_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryXOR_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryXOR[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryXOR[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryXOR[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryXOR_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryXOR[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryXOR[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryXOR_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryXOR[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryXOR[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryXOR_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i ^ from->i; }
WRTargetFunc wr_binaryXOR[5][5] = 
{
	{     doBinaryXOR_I_I,     doBinaryXOR_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{     doBinaryXOR_R_I,     doBinaryXOR_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
	{   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank },
};

//------------------------------------------------------------------------------
bool doCompareEQ_R_R( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareEQ[element.type][from->type](&element, from);
	}
	else  if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareEQ[to->type][element.type](to, &element);
	}
	else
	{
		return wr_CompareEQ[to->r->type][from->r->type](to->r, from->r);
	}
}
bool doCompareEQ_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareEQ[element.type][WR_INT](&element, from);
	}
	else
	{
		return wr_CompareEQ[to->r->type][WR_INT](to->r, from);
	}
}
bool doCompareEQ_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareEQ[element.type][WR_FLOAT](&element, from);
	}
	else
	{
		return wr_CompareEQ[to->r->type][WR_FLOAT](to->r, from);
	}
}
bool doCompareEQ_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareEQ[WR_INT][element.type](to, &element);
	}
	else
	{
		return wr_CompareEQ[WR_INT][from->r->type](to, from->r);
	}
}
bool doCompareEQ_I_I( WRValue* to, WRValue* from ) { return to->i == from->i; }
bool doCompareEQ_I_F( WRValue* to, WRValue* from ) { return (float)to->i == from->f; }
bool doCompareEQ_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareEQ[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		return wr_CompareEQ[WR_FLOAT][from->r->type](to, from->r);
	}
}
bool doCompareEQ_F_I( WRValue* to, WRValue* from ) { return to->f == (float)from->i; }
bool doCompareEQ_F_F( WRValue* to, WRValue* from ) { return to->f == from->f; }
WRReturnFunc wr_CompareEQ[5][5] = 
{
	{  doCompareEQ_I_I,  doCompareEQ_I_R,  doCompareEQ_I_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareEQ_R_I,  doCompareEQ_R_R,  doCompareEQ_R_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareEQ_F_I,  doCompareEQ_F_R,  doCompareEQ_F_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
};


//------------------------------------------------------------------------------
bool doCompareGT_R_R( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareGT[element.type][from->type](&element, from);
	}
	else  if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareGT[to->type][element.type](to, &element);
	}
	else
	{
		return wr_CompareGT[to->r->type][from->r->type](to->r, from->r);
	}
}
bool doCompareGT_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareGT[element.type][WR_INT](&element, from);
	}
	else
	{
		return wr_CompareGT[to->r->type][WR_INT](to->r, from);
	}
}
bool doCompareGT_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareGT[element.type][WR_FLOAT](&element, from);
	}
	else
	{
		return wr_CompareGT[to->r->type][WR_FLOAT](to->r, from);
	}
}
bool doCompareGT_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareGT[WR_INT][element.type](to, &element);
	}
	else
	{
		return wr_CompareGT[WR_INT][from->r->type](to, from->r);
	}
}
bool doCompareGT_I_I( WRValue* to, WRValue* from ) { return to->i > from->i; }
bool doCompareGT_I_F( WRValue* to, WRValue* from ) { return (float)to->i > from->f; }
bool doCompareGT_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareGT[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		return wr_CompareGT[WR_FLOAT][from->r->type](to, from->r);
	}
}
bool doCompareGT_F_I( WRValue* to, WRValue* from ) { return to->f > (float)from->i; }
bool doCompareGT_F_F( WRValue* to, WRValue* from ) { return to->f > from->f; }
WRReturnFunc wr_CompareGT[5][5] = 
{
	{  doCompareGT_I_I,  doCompareGT_I_R,  doCompareGT_I_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareGT_R_I,  doCompareGT_R_R,  doCompareGT_R_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareGT_F_I,  doCompareGT_F_R,  doCompareGT_F_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
};



//------------------------------------------------------------------------------
bool doCompareLT_R_R( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareLT[element.type][from->type](&element, from);
	}
	else  if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareLT[to->type][element.type](to, &element);
	}
	else
	{
		return wr_CompareLT[to->r->type][from->r->type](to->r, from->r);
	}
}
bool doCompareLT_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareLT[element.type][WR_INT](&element, from);
	}
	else
	{
		return wr_CompareLT[to->r->type][WR_INT](to->r, from);
	}
}
bool doCompareLT_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_CompareLT[element.type][WR_FLOAT](&element, from);
	}
	else
	{
		return wr_CompareLT[to->r->type][WR_FLOAT](to->r, from);
	}
}
bool doCompareLT_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareLT[WR_INT][element.type](to, &element);
	}
	else
	{
		return wr_CompareLT[WR_INT][from->r->type](to, from->r);
	}
}
bool doCompareLT_I_I( WRValue* to, WRValue* from ) { return to->i < from->i; }
bool doCompareLT_I_F( WRValue* to, WRValue* from ) { return (float)to->i < from->f; }
bool doCompareLT_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_CompareLT[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		return wr_CompareLT[WR_FLOAT][from->r->type](to, from->r);
	}
}
bool doCompareLT_F_I( WRValue* to, WRValue* from ) { return to->f < (float)from->i; }
bool doCompareLT_F_F( WRValue* to, WRValue* from ) { return to->f < from->f; }
WRReturnFunc wr_CompareLT[5][5] = 
{
	{  doCompareLT_I_I,  doCompareLT_I_R,  doCompareLT_I_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareLT_R_I,  doCompareLT_R_R,  doCompareLT_R_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doCompareLT_F_I,  doCompareLT_F_R,  doCompareLT_F_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
};

//------------------------------------------------------------------------------
bool doLogicalAnd_R_R( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalAnd[element.type][from->type](&element, from);
	}
	else  if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalAnd[to->type][element.type](to, &element);
	}
	else
	{
		return wr_LogicalAnd[to->r->type][from->r->type](to->r, from->r);
	}
}
bool doLogicalAnd_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalAnd[element.type][WR_INT](&element, from);
	}
	else
	{
		return wr_LogicalAnd[to->r->type][WR_INT](to->r, from);
	}
}
bool doLogicalAnd_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalAnd[element.type][WR_FLOAT](&element, from);
	}
	else
	{
		return wr_LogicalAnd[to->r->type][WR_FLOAT](to->r, from);
	}
}
bool doLogicalAnd_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalAnd[WR_INT][element.type](to, &element);
	}
	else
	{
		return wr_LogicalAnd[WR_INT][from->r->type](to, from->r);
	}
}
bool doLogicalAnd_I_I( WRValue* to, WRValue* from ) { return to->i && from->i; }
bool doLogicalAnd_I_F( WRValue* to, WRValue* from ) { return (float)to->i && from->f; }
bool doLogicalAnd_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalAnd[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		return wr_LogicalAnd[WR_FLOAT][from->r->type](to, from->r);
	}
}
bool doLogicalAnd_F_I( WRValue* to, WRValue* from ) { return to->f && (float)from->i; }
bool doLogicalAnd_F_F( WRValue* to, WRValue* from ) { return to->f && from->f; }
WRReturnFunc wr_LogicalAnd[5][5] = 
{
	{  doLogicalAnd_I_I,  doLogicalAnd_I_R,  doLogicalAnd_I_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doLogicalAnd_R_I,  doLogicalAnd_R_R,  doLogicalAnd_R_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doLogicalAnd_F_I,  doLogicalAnd_F_R,  doLogicalAnd_F_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
};


//------------------------------------------------------------------------------
bool doLogicalOr_R_R( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalOr[element.type][from->type](&element, from);
	}
	else  if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalOr[to->type][element.type](to, &element);
	}
	else
	{
		return wr_LogicalOr[to->r->type][from->r->type](to->r, from->r);
	}
}
bool doLogicalOr_R_I( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalOr[element.type][WR_INT](&element, from);
	}
	else
	{
		return wr_LogicalOr[to->r->type][WR_INT](to->r, from);
	}
}
bool doLogicalOr_R_F( WRValue* to, WRValue* from )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		return wr_LogicalOr[element.type][WR_FLOAT](&element, from);
	}
	else
	{
		return wr_LogicalOr[to->r->type][WR_FLOAT](to->r, from);
	}
}
bool doLogicalOr_I_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalOr[WR_INT][element.type](to, &element);
	}
	else
	{
		return wr_LogicalOr[WR_INT][from->r->type](to, from->r);
	}
}
bool doLogicalOr_I_I( WRValue* to, WRValue* from ) { return to->i || from->i; }
bool doLogicalOr_I_F( WRValue* to, WRValue* from ) { return (float)to->i || from->f; }
bool doLogicalOr_F_R( WRValue* to, WRValue* from )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		return wr_LogicalOr[WR_FLOAT][element.type](to, &element);
	}
	else
	{
		return wr_LogicalOr[WR_FLOAT][from->r->type](to, from->r);
	}
}
bool doLogicalOr_F_I( WRValue* to, WRValue* from ) { return to->f || (float)from->i; }
bool doLogicalOr_F_F( WRValue* to, WRValue* from ) { return to->f || from->f; }
WRReturnFunc wr_LogicalOr[5][5] = 
{
	{  doLogicalOr_I_I,  doLogicalOr_I_R,  doLogicalOr_I_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doLogicalOr_R_I,  doLogicalOr_R_R,  doLogicalOr_R_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doLogicalOr_F_I,  doLogicalOr_F_R,  doLogicalOr_F_F,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
	{  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank,  doReturnFuncBlank, doReturnFuncBlank },
};


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
			value->type = WR_REF;
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
	else
	{
		assert(0); // usr array is a container, can't clobber it by making it a ref
	}
}

void doIndex_R_I( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type][WR_INT](c, index->r, value);
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
		
		wr_index[index->r->type][WR_REF](c, index->r, value);
	}
}
void doIndex_R_F( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type][WR_FLOAT](c, index->r, value);
}
void doIndex_R_A( WRRunContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type][WR_ARRAY](c, index->r, value);
}
WRStateFunc wr_index[5][5] = 
{
	{      doIndex_I_X,     doIndex_I_R,     doIndex_I_X, doVoidIndexFunc,     doIndex_I_A },
	{      doIndex_R_I,     doIndex_R_R,     doIndex_R_F, doVoidIndexFunc,     doIndex_R_A },
	{  doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc },
	{  doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc },
	{  doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc },
};

//------------------------------------------------------------------------------
void doPreInc_I( WRValue* value ) { ++value->i; }
void doPreInc_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_preinc[ value->r->type ]( value->r );
		*value = *value->r;
	}
}
void doPreInc_F( WRValue* value )
{
	++value->f;
}
WRUnaryFunc wr_preinc[5] = 
{
	doPreInc_I,  doPreInc_R,  doPreInc_F,  doSingleVoidBlank,  doSingleVoidBlank
};

//------------------------------------------------------------------------------
void doPreDec_I( WRValue* value ) { --value->i; }
void doPreDec_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_predec[ value->r->type ]( value->r );
		*value = *value->r;
	}
}

void doPreDec_F( WRValue* value ) { --value->f; }
WRUnaryFunc wr_predec[5] = 
{
	doPreDec_I,  doPreDec_R,  doPreDec_F,  doSingleVoidBlank,  doSingleVoidBlank
};

//------------------------------------------------------------------------------
void doPostInc_I( WRValue* value, WRValue* stack ) { *stack = *value; ++value->i; }
void doPostInc_R( WRValue* value, WRValue* stack )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_postinc[ value->r->type ]( value->r, stack );
	}
}
void doPostInc_F( WRValue* value, WRValue* stack ) { *stack = *value; ++value->f; }
WRVoidFunc wr_postinc[5] = 
{
	doPostInc_I,  doPostInc_R,  doPostInc_F,  doVoidFuncBlank,  doVoidFuncBlank
};

//------------------------------------------------------------------------------
void doPostDec_I( WRValue* value, WRValue* stack ) { *stack = *value; --value->i; }
void doPostDec_R( WRValue* value, WRValue* stack )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_postdec[ value->r->type ]( value->r, stack );
	}
}

void doPostDec_F( WRValue* value, WRValue* stack ) { *stack = *value; --value->f; }
WRVoidFunc wr_postdec[5] = 
{
	doPostDec_I,  doPostDec_R,  doPostDec_F,  doVoidFuncBlank, doVoidFuncBlank
};

////------------------------------------------------------------------------------
void doUserHash_X( WRValue* value, WRValue* target, int32_t hash ) { }
void doUserHash_R( WRValue* value, WRValue* target, int32_t hash )
{
	wr_UserHash[ value->r->type ]( value->r, target, hash );
}
void doUserHash_U( WRValue* value, WRValue* target, int32_t hash )
{
	if ( !(target->r = value->u->index.getItem(hash)) )
	{
		target->clear();
	}
	else
	{
		target->type = WR_REF;
	}
}
WRUserHashFunc wr_UserHash[5] = 
{
	doUserHash_X,  doUserHash_R,  doUserHash_X,  doUserHash_U,  doUserHash_X
};

//------------------------------------------------------------------------------
bool doZeroCheck_I( WRValue* value ) { return value->i == 0; }
bool doZeroCheck_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( value, &element );
		return wr_ZeroCheck[ element.type ]( &element );
	}
	else
	{
		return wr_ZeroCheck[ value->r->type ]( value->r );
	}
}
bool doZeroCheck_F( WRValue* value ) { return value->f == 0; }
WRValueCheckFunc wr_ZeroCheck[5] = 
{
	doZeroCheck_I,  doZeroCheck_R,  doZeroCheck_F,  doSingleBlank, doSingleBlank
};


//------------------------------------------------------------------------------
bool doLogicalNot_I( WRValue* value ) { return value->i == 0; }
bool doLogicalNot_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( value, &element );
		return wr_LogicalNot[ element.type ]( &element );
	}
	else
	{
		return wr_LogicalNot[ value->r->type ]( value->r );
	}
}
bool doLogicalNot_F( WRValue* value ) { return value->f == 0; }
WRReturnSingleFunc wr_LogicalNot[5] = 
{
	doLogicalNot_I,  doLogicalNot_R,  doLogicalNot_F,  doSingleBlank, doSingleBlank
};

//------------------------------------------------------------------------------
void doNegate_I( WRValue* value ) { value->i = -value->i; }
void doNegate_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_negate[ value->r->type ]( value->r );
		*value = *value->r;
	}
}
void doNegate_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_negate[5] = 
{
	doNegate_I,  doNegate_R,  doNegate_F,  doSingleVoidBlank,  doSingleVoidBlank
};

//------------------------------------------------------------------------------
void doBitwiseNot_I( WRValue* value ) { value->i = -value->i; }
void doBitwiseNot_R( WRValue* value )
{
	if ( value->r->type == WR_ARRAY )
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
	else
	{
		wr_BitwiseNot[ value->r->type ]( value->r );
		*value = *value->r;
	}
}
void doBitwiseNot_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_BitwiseNot[5] = 
{
	doBitwiseNot_I,  doBitwiseNot_R,  doBitwiseNot_F,  doSingleVoidBlank,  doSingleVoidBlank
};
