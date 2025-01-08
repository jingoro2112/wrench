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

#ifdef WRENCH_COMPACT

extern bool CompareEQI( int a, int b );
extern bool CompareEQF( float a, float b );

//------------------------------------------------------------------------------
void unaryPost_E( WRValue* value, WRValue* stack, int add )
{
	WRValue& V = value->singleValue();
	WRValue temp;
	m_unaryPost[ V.type ]( &V, &temp, add );
	wr_valueToEx( value, &V );
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
void FuncAssign_E_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	if ( IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();

		wr_FuncAssign[(WR_EX<<2)|V.type]( to, &V, intCall, floatCall );
		*from = to->deref();
	}
	else if ( wr_concatStringCheck(to, from, to) )
	{
		*from = *to;
	}
}
void FuncAssign_E_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( wr_concatStringCheck(to, from, to) )
	{
		*from = *to;
	}
	else
	{
		WRValue& V = to->singleValue();
		wr_FuncAssign[(V.type<<2)|from->type]( &V, from, intCall, floatCall );
		wr_valueToEx( to, &V );
		*from = V;
	}
}
void FuncAssign_X_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( !wr_concatStringCheck(to, from, to) )
	{
		WRValue& V = from->singleValue();
		wr_FuncAssign[(to->type<<2)|V.type](to, &V, intCall, floatCall);
	}

	*from = *to;
}
void FuncAssign_E_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}

void FuncAssign_R_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue temp = *from->r; 
	wr_FuncAssign[(to->r->type<<2)|temp.type](to->r, &temp, intCall, floatCall); 
	*from = *to->r;
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
	if ( intCall != wr_addI || !wr_concatStringCheck(to, from, target) )
	{
		WRValue& V = to->singleValue();
		wr_funcBinary[(V.type<<2)|from->type](&V, from, target, intCall, floatCall);
	}
}
void FuncBinary_E_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_CONTAINER_MEMBER(to->xtype) || IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue V1 = to->deref();
		WRValue& V2 = from->deref();
		wr_funcBinary[(V1.type<<2)|V2.type](&V1, &V2, target, intCall, floatCall );
	}
	else if ( intCall == wr_addI )
	{
		wr_concatStringCheck( to, from, target );
	}
}
void FuncBinary_X_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( intCall != wr_addI || !wr_concatStringCheck(to, from, target) )
	{
		WRValue& V = from->singleValue();
		wr_funcBinary[(to->type<<2)|V.type](to, &V, target, intCall, floatCall);
	}
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


#endif
