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
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_VALUE )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (WRValue*)va->m_data;
}

//------------------------------------------------------------------------------
unsigned char* WRValue::asCharArray( int* len )
{
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_CHAR )
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
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_INT )
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
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_FLOAT )
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
WRState* wr_newState( int stackSize )
{
	return new WRState( stackSize );
}

//------------------------------------------------------------------------------
WRContext::WRContext( WRState* state ) : w(state)
{
	localFunctions = 0;
	globalSpace = 0;
	globals = 0;
	svAllocated = 0;
	stopLocation = 0;
	bottom = 0;
}

//------------------------------------------------------------------------------
WRContext::~WRContext()
{
	delete[] globalSpace;
	delete[] localFunctions;

	while( svAllocated )
	{
		WRStaticValueArray* next = svAllocated->m_next;
		delete svAllocated;
		svAllocated = next;
	}
}

//------------------------------------------------------------------------------
WRState::WRState( int EntriesInStack )
{
	err = WR_ERR_None;

	stackSize = EntriesInStack;
	stack = new WRValue[ stackSize ];
	for( int i=0; i<stackSize; ++i )
	{
		stack[i].init();
	}
	stackTop = stack;

	loader = 0;
	usr = 0;
	returnValue = 0;
	contextList = 0;
}

//------------------------------------------------------------------------------
WRState::~WRState()
{
	while( contextList )
	{
		WRContext* next = contextList->next;
		delete contextList;
		contextList = next;
	}

	for( int i=0; i<stackSize; ++i )
	{
		stack[i].init();
	}
	delete[] stack;
}

//------------------------------------------------------------------------------
WRStaticValueArray* WRContext::getSVA( int size, WRStaticValueArrayType type )
{
	// gc before every alloc may seem a bit much but we want to be miserly
	if ( svAllocated )
	{
		// mark stack
		for( WRValue* s=w->stack; s<w->stackTop; ++s)
		{
			// an array in the chain?
			if ( s->type == WR_ARRAY && !(s->va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( s->va );
			}
		}
		
		// mark context's global
		for( int i=0; i<globals; ++i )
		{
			// an array in the chain?
			if ( globalSpace[i].type == WR_ARRAY && !(globalSpace[i].va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( globalSpace[i].va );
			}
		}

		// sweep
		WRStaticValueArray* current = svAllocated;
		WRStaticValueArray* prev = 0;
		while( current )
		{
			// if set, clear it
			if ( current->m_size & 0x40000000 )
			{
				current->m_size &= ~0x40000000;
				prev = current;
				current = current->m_next;
			}
			// otherwise nuke it as unreferenced
			else if ( prev == 0 )
			{
				svAllocated = current->m_next;
				delete current;
				current = svAllocated;
			}
			else
			{
				prev->m_next = current->m_next;
				delete current;
				current = prev->m_next;
			}
		}
	}

	
	WRStaticValueArray* ret = new WRStaticValueArray( size, type );
	if ( type == SV_VALUE )
	{
		WRValue *array = (WRValue *)ret->m_data;
		for( int i=0; i<size; ++i )
		{
			array[i].p = 0;
			array[i].p2 = 0;
		}
	}

	ret->m_next = svAllocated;
	svAllocated = ret;

	return ret;
}

//------------------------------------------------------------------------------
void WRContext::gcArray( WRStaticValueArray* sva )
{
	sva->m_size |= 0x40000000;

	if ( (sva->m_type&0x3) == SV_VALUE )
	{
		// this is an array of values, check them for array-ness too
		
		WRValue* top = (WRValue*)sva->m_data + (sva->m_size & ~0x40000000);
		for( WRValue* i = (WRValue*)sva->m_data; i<top; ++i )
		{
			if ( i->type == WR_ARRAY && !(i->va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( i->va );
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	delete w;
}

//------------------------------------------------------------------------------
unsigned int wr_loadSingleBlock( int offset, const unsigned char** block, void* usr )
{
	*block = (unsigned char *)usr;
	return 0xFFFFFFF; // larger than any bytecode possible
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_runEx( WRState* w )
{
	WRContext* C = new WRContext( w );
	C->next = w->contextList;
	w->contextList = C;
	
	if ( wr_callFunction(w, C, (int32_t)0) )
	{
		wr_destroyContext( w, C );
		return 0;
	}

	return C;
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int size )
{
	w->loader = wr_loadSingleBlock;
	w->usr = (void*)block;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, WR_LOAD_BLOCK_FUNC loader, void* usr )
{
	w->loader = loader;
	w->usr = usr;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
void wr_destroyContext( WRState* w, WRContext* context )
{
	WRContext* prev = 0;

	// unlink it
	for( WRContext* c = w->contextList; c; c = c->next )
	{
		if ( c == context )
		{
			if ( prev )
			{
				prev->next = c->next;
			}
			else
			{
				w->contextList = w->contextList->next;
			}

			break;
		}
		prev = c;
	}

	delete context;
}

//------------------------------------------------------------------------------
int wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRCFunctionCallback callback;
	callback.usr = usr;
	callback.function = function;

	uint32_t hash = wr_hashStr( name );
	if ( !w->c_functionRegistry.set(hash, callback) )
	{
		return w->err = WR_ERR_hash_table_size_exceeded;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function )
{
	w->c_libFunctionRegistry.set( wr_hashStr(signature), function );
}

//------------------------------------------------------------------------------
char* wr_valueToString( WRValue const& value, char* string )
{
	return value.asString( string );
}


#ifdef SPRINTF_OPERATIONS
#include <stdio.h>
#endif
//------------------------------------------------------------------------------
char* WRValue::asString( char* string ) const
{
	switch( type )
	{
#ifdef SPRINTF_OPERATIONS
		case WR_INT: { sprintf( string, "%d", i ); break; }
		case WR_FLOAT: { sprintf( string, "%g", f ); break; }
#else
		case WR_INT: 
		case WR_FLOAT:
		{
			string[0] = 0;
			break;
		}
#endif
		case WR_REF: { return r->asString( string ); }
		case WR_USR:
		{
			return string;
		}

		case WR_ARRAY:
		{
			unsigned int s = 0;

			for( ; s<va->m_size; ++s )
			{
				switch( va->m_type & 0x3)
				{
					case SV_VALUE: string[s] = ((WRValue *)va->m_data)[s].i; break;
					case SV_CHAR: string[s] = ((char *)va->m_data)[s]; break;
					default: break;
				}
			}
			string[s] = 0;
			break;
		}

		case WR_REFARRAY:
		{
			WRValue temp;
			wr_arrayToValue(this, &temp);
			return temp.asString(string);
		}
	}
	
	return string;
}


#ifdef D_OPCODE
#define PER_INSTRUCTION printf( "S[%d] %d:%s\n", (int)(stackTop - w->stack), (int)*pc, c_opcodeName[*pc]);
#else
#define PER_INSTRUCTION
#endif

#ifdef JUMPTABLE_INTERPRETER
#define CONTINUE { goto *opcodeJumptable[*pc++]; PER_INSTRUCTION; }
#define CASE(LABEL) LABEL
#else
#define CONTINUE continue
#define CASE(LABEL) case O_##LABEL
#endif


//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( w, context, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, const int32_t hash, const WRValue* argv, const int argn )
{
	WRFunction* function = 0;
	if ( hash )
	{
		// if a function name is provided it is meant to be called
		// directly. Set that up with a return vector of "stop"
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_run_must_be_called_by_itself_first;
		}

		function = context->localFunctionRegistry.getItem( hash );

		if ( !function )
		{
			return w->err = WR_ERR_wrench_function_not_found;
		}
	}

	return wr_callFunction( w, context, function, argv, argn );
}


//------------------------------------------------------------------------------
WRFunction* wr_functionToIndex( WRContext* context, const int32_t hash )
{
	return context->localFunctionRegistry.getItem( hash );
}

//------------------------------------------------------------------------------
WRFunction* wr_functionToIndex( WRContext* context, const char* functionName )
{
	return wr_functionToIndex( context, wr_hashStr(functionName) );
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, WRFunction* function, const WRValue* argv, const int argn )
//int wr_callFunction( WRState* w, WRContext* context, const int32_t hash, const WRValue* argv, const int argn )
{
#ifdef JUMPTABLE_INTERPRETER
	const void* opcodeJumptable[] =
	{
		&&RegisterFunction,
		&&FunctionListSize,
		&&LiteralZero,
		&&LiteralInt8,
		&&LiteralInt32,
		&&LiteralFloat,
		&&LiteralString,
		&&CallFunctionByHash,
		&&CallFunctionByHashAndPop,
		&&CallFunctionByIndex,
		&&CallLibFunction,
		&&CallLibFunctionAndPop,
		&&Index,
		&&IndexLiteral8,
		&&IndexLiteral32,
		&&StackIndexHash,
		&&GlobalIndexHash,
		&&LocalIndexHash,
		&&Assign,
		&&AssignAndPop,
		&&AssignToGlobalAndPop,
		&&AssignToLocalAndPop,
		&&StackSwap,
		&&ReserveFrame,
		&&ReserveGlobalFrame,
		&&LoadFromLocal,
		&&LoadFromGlobal,
		&&PopOne,
		&&Return,
		&&Stop,
		&&BinaryAddition,
		&&BinarySubtraction,
		&&BinaryMultiplication,
		&&BinaryDivision,
		&&BinaryRightShift,
		&&BinaryLeftShift,
		&&BinaryMod,
		&&BinaryAnd,
		&&BinaryOr,
		&&BinaryXOR,
		&&BitwiseNOT,
		&&CoerceToInt,
		&&CoerceToFloat,
		&&RelativeJump,
		&&RelativeJump8,
		&&BZ,
		&&BZ8,
		&&BNZ,
		&&BNZ8,
		&&CompareEQ,
		&&CompareNE,
		&&CompareGE,
		&&CompareLE,
		&&CompareGT,
		&&CompareLT,
		&&CompareBEQ,
		&&CompareBNE,
		&&CompareBGE,
		&&CompareBLE,
		&&CompareBGT,
		&&CompareBLT,
		&&CompareBEQ8,
		&&CompareBNE8,
		&&CompareBGE8,
		&&CompareBLE8,
		&&CompareBGT8,
		&&CompareBLT8,
		&&PostIncrement,
		&&PostDecrement,
		&&PreIncrement,
		&&PreDecrement,
		&&Negate,
		&&SubtractAssign,
		&&AddAssign,
		&&ModAssign,
		&&MultiplyAssign,
		&&DivideAssign,
		&&ORAssign,
		&&ANDAssign,
		&&XORAssign,
		&&RightShiftAssign,
		&&LeftShiftAssign,
		&&SubtractAssignAndPop,
		&&AddAssignAndPop,
		&&ModAssignAndPop,
		&&MultiplyAssignAndPop,
		&&DivideAssignAndPop,
		&&ORAssignAndPop,
		&&ANDAssignAndPop,
		&&XORAssignAndPop,
		&&RightShiftAssignAndPop,
		&&LeftShiftAssignAndPop,
		&&LogicalAnd,
		&&LogicalOr,
		&&LogicalNot,
		&&LiteralInt8ToGlobal,
		&&LiteralInt32ToLocal,
		&&LiteralInt8ToLocal,
		&&LiteralFloatToGlobal,
		&&LiteralFloatToLocal,
		&&LiteralInt32ToGlobal,
	};
#endif

	register const unsigned char* pc;
	register WRValue* tempValue;
	register WRValue* tempValue2;
	WRValue* frameBase = 0;
	WRValue* stackTop = w->stack;

	w->err = WR_ERR_None;
	
	union
	{
		// these are never used at the same time so don't waste ram
		unsigned char args;
		WRVoidFunc* voidFunc;
		WRReturnFunc* returnFunc;
		WRTargetFunc* targetFunc;
	};

#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
	if ( !(pc = context->bottom) )
	{
		w->loader( 0, &pc, w->usr );
		context->bottom = pc;
	}
#else
	// cache these values they are used a lot
	WR_LOAD_BLOCK_FUNC loader = w->loader;
	void* usr = w->usr;
	const unsigned char* top = 0;
	pc = 0;
	int absoluteBottom = 0; // pointer to where in the codebase out bottom actually points to
#endif

	if ( function )
	{
		args = 0;
		if ( argv && argn )
		{
			for( ; args < argn; ++args )
			{
				*stackTop++ = argv[args];
			}
		}
		
#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
		pc = context->bottom + context->stopLocation;
#else
		unsigned int size = loader( absoluteBottom = context->stopLocation, &pc, usr );
		top = pc + (size - 6);
		context->bottom = pc;
#endif
		goto callFunction;
	}

#ifdef JUMPTABLE_INTERPRETER

	CONTINUE;
	
#else

	for(;;)
	{
        #ifdef WRENCH_PARTIAL_BYTECODE_LOADS
		if ( pc >= top )
		{
			unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
			top = pc + (size - 6);
			context->bottom = pc;
		}
        #endif

		switch( *pc++)
		{
#endif
			CASE(RegisterFunction):
			{
				int index = (stackTop - 5)->i;
				context->localFunctions[ index ].arguments = (stackTop - 4)->i;
				context->localFunctions[ index ].frameSpaceNeeded = (stackTop - 3)->i;
				context->localFunctions[ index ].hash = (stackTop - 2)->i;
				
#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				context->localFunctions[index].offset = (stackTop - 1)->i + context->bottom; // absolute
#else
				context->localFunctions[index].offsetI = (stackTop - 1)->i; // relative
#endif
				
				context->localFunctions[ index ].frameBaseAdjustment = 2
																	   + context->localFunctions[ index ].frameSpaceNeeded
																	   + context->localFunctions[ index ].arguments;

				context->localFunctionRegistry.set( context->localFunctions[ index ].hash,
													context->localFunctions + index );
				stackTop -= 5;
				CONTINUE;
			}

			CASE(FunctionListSize):
			{
				delete context->localFunctions;
				context->localFunctionRegistry.clear();
				context->localFunctions = new WRFunction[ *pc++ ];
				CONTINUE;
			}

			CASE(LiteralZero):
			{
				stackTop->p = 0;
				(stackTop++)->p2 = 0;
				CONTINUE;
			}
			
			CASE(LiteralInt8):
			{
				stackTop->type = WR_INT;
				(stackTop++)->i = (int32_t)(int8_t)*pc++;
				CONTINUE;
			}
			
			CASE(LiteralFloat):
			{
				stackTop->type = WR_FLOAT;
				goto load32ToStackTop;
			}
			
			CASE(LiteralInt32):
			{
				stackTop->type = WR_INT;
load32ToStackTop:
				(stackTop++)->i = (((int32_t)*pc) << 24)
								  | (((int32_t)*(pc+1)) << 16)
								  | (((int32_t)*(pc+2)) << 8)
								  | ((int32_t)*(pc+3));
				pc += 4;
				CONTINUE;
			}

			CASE(LiteralString):
			{
				int16_t len = (((int16_t)*pc)<<8) | (int16_t)*(pc + 1);
				pc += 2;
				stackTop->type = WR_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR );

#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				memcpy( (unsigned char *)stackTop->va->m_data, pc, len );
				pc += len;
#else
				for( int c=0; c<len; ++c )
				{
					if ( pc >= top )
					{
						unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
						top = pc + (size - 6);
						context->bottom = pc;
					}
				}
#endif
				++stackTop;
				CONTINUE;
			}
			
			CASE(ReserveFrame):
			{
				frameBase = stackTop;
				for( unsigned char i=0; i<*pc; ++i )
				{
					(stackTop)->p = 0;
					(++stackTop)->p2 = 0;

				}
				++pc;
				CONTINUE;
			}

			CASE(ReserveGlobalFrame):
			{
				delete[] context->globalSpace;
				context->globals = *pc++;
				context->globalSpace = new WRValue[ context->globals ];
				for( unsigned char i=0; i<context->globals; ++i )
				{
					context->globalSpace[i].p = 0;
					context->globalSpace[i].p2 = 0;
				}
				CONTINUE;
			}

			CASE(LoadFromLocal):
			{
				stackTop->type = WR_REF;
				(stackTop++)->p = frameBase + *pc++;
				CONTINUE;
			}

			CASE(LoadFromGlobal):
			{
				stackTop->type = WR_REF;
				(stackTop++)->p = context->globalSpace + *pc++;
				CONTINUE;
			}

			CASE(AssignToGlobalAndPop):
			{
				tempValue = context->globalSpace + *pc++;
				tempValue2 = --stackTop;
				wr_assign[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(AssignToLocalAndPop):
			{
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				wr_assign[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(Index):
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_index[tempValue->type*6+tempValue2->type]( context, tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(IndexLiteral32):
			{
				stackTop->i = (((int32_t)*pc) << 24) |
					(((int32_t) * (pc + 1)) << 16) |
					(((int32_t) * (pc + 2)) << 8) |
					(int32_t) * (pc + 3);
				pc += 4;
				goto indexLiteral;
			}

			CASE(IndexLiteral8):
			{
				stackTop->i = *pc++;
indexLiteral:
				tempValue = stackTop - 1;
				wr_index[WR_INT*6+tempValue->type]( context, stackTop, tempValue );
				CONTINUE;
			}

			CASE(StackIndexHash):
			{
				tempValue = stackTop - 1;
				wr_UserHash[ tempValue->type ]( tempValue,
												tempValue,
												(((int32_t)*pc) << 24) |
												(((int32_t)*(pc+1)) << 16) |
												(((int32_t)*(pc+2)) << 8) |
												(int32_t)*(pc+3) );
				pc += 4;
				CONTINUE;
			}

			CASE(GlobalIndexHash):
			{
				tempValue = context->globalSpace + *pc++;
				goto indexHash;
			}

			CASE(LocalIndexHash):
			{
				tempValue = frameBase + *pc++;
indexHash:
				wr_UserHash[ tempValue->type ]( tempValue,
												stackTop++,
												(((int32_t)*pc) << 24) |
												(((int32_t)*(pc+1)) << 16) |
												(((int32_t)*(pc+2)) << 8) |
												(int32_t)*(pc+3) );
				pc += 4;
				CONTINUE;
			}
			
			CASE(BinaryMultiplication): { targetFunc = wr_binaryMultiply; goto targetFuncOp; }
			CASE(BinarySubtraction): { targetFunc = wr_binarySubtract; goto targetFuncOp; }
			CASE(BinaryDivision): { targetFunc = wr_binaryDivide; goto targetFuncOp; }
			CASE(BinaryRightShift): { targetFunc = wr_binaryRightShift; goto targetFuncOp; }
			CASE(BinaryLeftShift): { targetFunc = wr_binaryLeftShift; goto targetFuncOp; }
			CASE(BinaryMod): { targetFunc = wr_binaryMod; goto targetFuncOp; }
			CASE(BinaryOr): { targetFunc = wr_binaryOR; goto targetFuncOp; }
			CASE(BinaryXOR): { targetFunc = wr_binaryXOR; goto targetFuncOp; }
			CASE(BinaryAnd): { targetFunc = wr_binaryAND; goto targetFuncOp; }
			CASE(BinaryAddition):
			{
				targetFunc = wr_binaryAddition;
targetFuncOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				targetFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2, tempValue2 );
				CONTINUE;
			}

			CASE(BitwiseNOT):
			{
				tempValue = stackTop - 1;
				wr_bitwiseNot[ tempValue->type ]( tempValue );
				CONTINUE;
			}


			CASE(SubtractAssign): { voidFunc = wr_SubtractAssign; goto binaryTableOp; }
			CASE(AddAssign): { voidFunc = wr_AddAssign; goto binaryTableOp; }
			CASE(ModAssign): { voidFunc = wr_ModAssign; goto binaryTableOp; }
			CASE(MultiplyAssign): { voidFunc = wr_MultiplyAssign; goto binaryTableOp; }
			CASE(DivideAssign): { voidFunc = wr_DivideAssign; goto binaryTableOp; }
			CASE(ORAssign): { voidFunc = wr_ORAssign; goto binaryTableOp; }
			CASE(ANDAssign): { voidFunc = wr_ANDAssign; goto binaryTableOp; }
			CASE(XORAssign): { voidFunc = wr_XORAssign; goto binaryTableOp; }
			CASE(RightShiftAssign): { voidFunc = wr_RightShiftAssign; goto binaryTableOp; }
			CASE(LeftShiftAssign): { voidFunc = wr_LeftShiftAssign; goto binaryTableOp; }
			CASE(Assign):
			{
				voidFunc = wr_assign;
binaryTableOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				voidFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(SubtractAssignAndPop): { voidFunc = wr_SubtractAssign; goto binaryTableOpAndPop; }
			CASE(AddAssignAndPop): { voidFunc = wr_AddAssign; goto binaryTableOpAndPop; }
			CASE(ModAssignAndPop): { voidFunc = wr_ModAssign; goto binaryTableOpAndPop; }
			CASE(MultiplyAssignAndPop): { voidFunc = wr_MultiplyAssign; goto binaryTableOpAndPop; }
			CASE(DivideAssignAndPop): { voidFunc = wr_DivideAssign; goto binaryTableOpAndPop; }
			CASE(ORAssignAndPop): { voidFunc = wr_ORAssign; goto binaryTableOpAndPop; }
			CASE(ANDAssignAndPop): { voidFunc = wr_ANDAssign; goto binaryTableOpAndPop; }
			CASE(XORAssignAndPop): { voidFunc = wr_XORAssign; goto binaryTableOpAndPop; }
			CASE(RightShiftAssignAndPop): { voidFunc = wr_RightShiftAssign; goto binaryTableOpAndPop; }
			CASE(LeftShiftAssignAndPop): { voidFunc = wr_LeftShiftAssign; goto binaryTableOpAndPop; }
			CASE(AssignAndPop):
			{
				voidFunc = wr_assign;

binaryTableOpAndPop:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				voidFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				
				CONTINUE;
			}

			CASE(StackSwap):
			{
				tempValue = stackTop - 1;
				tempValue2 = stackTop - *pc++;
				uint32_t t = tempValue->p2;
				const void* p = tempValue->p;
				
				tempValue->p2 = tempValue2->p2;
				tempValue->p = tempValue2->p;

				tempValue2->p = p;
				tempValue2->p2 = t;

				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				int32_t hash = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				args = *(pc+4); // which have already been pushed
				pc += 5;

				WRCFunctionCallback* cF = w->c_functionRegistry.get( hash );
				if ( cF )
				{
					if ( !args )
					{
						cF->function( w, 0, 0, *stackTop++, cF->usr );
					}
					else
					{
						stackTop->p = 0;
						stackTop->p2 = 0;
						cF->function( w, stackTop - args, args, *stackTop, cF->usr );
						*(stackTop - args) = *stackTop;
						stackTop -= args - 1;
					}
				}
				else
				{
					w->err = WR_WARN_c_function_not_found;
					stackTop -= args;
					(stackTop)->p = 0;
					(stackTop++)->p2 = 0; // push a fake return value
				}
				
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				// don't care about return value
				int32_t hash = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				args = *(pc+4); // which have already been pushed
				pc += 6; // skip past the pop operation that is next in line (linking can't remove the instruction... yet)

				WRCFunctionCallback* cF = w->c_functionRegistry.get( hash );
				if ( cF )
				{
					if ( !args )
					{
						cF->function( w, 0, 0, *stackTop, cF->usr );
					}
					else
					{
						cF->function( w, stackTop - args, args, *stackTop, cF->usr );
						stackTop -= args;
					}
				}
				else
				{
					w->err = WR_WARN_c_function_not_found;
					stackTop -= args;
				}
				
				CONTINUE;
			}

			CASE(CallFunctionByIndex):
			{
				function = context->localFunctions + *pc;
				pc += 4;
				args = *pc++;
callFunction:				
				// rectify arg count
				if ( args != function->arguments )
				{
					if ( args > function->arguments )
					{
						stackTop -= args - function->arguments; // poof
					}
					else
					{
						for( char a=args; a < function->arguments; ++a )
						{
							stackTop->p = 0;
							(stackTop++)->p2 = 0;
						}
					}
				}

				// the arguments are at framepace 0, add more to make
				// up for the locals in the function
				for( int i=0; i<function->frameSpaceNeeded; ++i )
				{
					stackTop->p = 0;
					(stackTop++)->p2 = 0;
				}

				// temp value contains return vector
				tempValue = stackTop++; // return vector
				tempValue->type = WR_INT;
				
#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				
				tempValue->p = pc; // simple for big-blob case
				pc = function->offset;
#else
				// relative position in the code to return to
				tempValue->i = absoluteBottom + (pc - context->bottom);

				if ( function->offsetI >= absoluteBottom )
				{
					// easy, function is within this loaded block
					pc = context->bottom + (function->offsetI - absoluteBottom);
				}
				else
				{
					// less easy, have to load the new block
					unsigned int size = loader( absoluteBottom = function->offsetI, &pc, usr );
					top = pc + (size - 6);
					context->bottom = pc;
				}
				
#endif
				
				stackTop->type = WR_INT;
				(stackTop++)->p = frameBase; // very top is the old framebase

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				CONTINUE;
			}

			CASE(CallLibFunction):
			{
				WR_LIB_CALLBACK callback = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
																			  | (((int32_t)*(pc+1)) << 16)
																			  | (((int32_t)*(pc+2)) << 8)
																			  | ((int32_t)*(pc+3)) );
				args = *(pc+4); // which have already been pushed
				pc += 5;

				if ( callback )
				{
					callback( stackTop, args );
					if ( args )
					{
						*(stackTop - args) = *stackTop;
						stackTop -= args - 1;
					}
					else
					{
						++stackTop;
					}
				}
				else
				{
					w->err = WR_WARN_lib_function_not_found;
					stackTop -= args;
					stackTop->p = 0;
					(stackTop++)->p2 = 0; // fake return val
				}
				
				CONTINUE;
			}
			
			CASE(CallLibFunctionAndPop):
			{
				WR_LIB_CALLBACK callback = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
																			  | (((int32_t)*(pc+1)) << 16)
																			  | (((int32_t)*(pc+2)) << 8)
																			  | ((int32_t)*(pc+3)) );
				args = *(pc+4); // which have already been pushed
				pc += 5;

				if ( callback )
				{
					callback( stackTop, args );
				}
				else
				{
					w->err = WR_WARN_lib_function_not_found;
				}
				stackTop -= args;
				CONTINUE;
			}

			CASE(Stop):
			{
				// stack will be zero at this point, stop execution
				w->returnValue = stackTop--;
				context->stopLocation = (int32_t)((pc - 1) - context->bottom);
				return WR_ERR_None;
			}

			CASE(Return):
			{
#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				// copy the return value
				pc = (unsigned char*)((stackTop - 3)->p); // grab return PC
#else
				int returnOffset = (stackTop - 3)->i;
				if ( returnOffset >= absoluteBottom )
				{
					// easy, function is within this loaded block
					pc = context->bottom + (returnOffset - absoluteBottom);
				}
				else
				{
					// less easy, have to load the new block
					unsigned int size = loader( absoluteBottom = returnOffset, &pc, usr );
					top = pc + (size - 6);
					context->bottom = pc;
				}

#endif
				if ( (--stackTop)->type == WR_REF )
				{
					*frameBase = *stackTop->r;
				}
				else
				{
					*frameBase = *stackTop; // copy return value down
				}
				
				tempValue = (WRValue*)(stackTop - 1)->p;
				stackTop = frameBase + 1;
				frameBase = tempValue;
				CONTINUE;
			}
			
			CASE(PopOne):
			{
				--stackTop;
				CONTINUE;
			}

			CASE(CoerceToInt):
			{
				tempValue = stackTop - 1;
				wr_toInt[ tempValue->type ]( tempValue );
				CONTINUE;
			}

			CASE(CoerceToFloat):
			{
				tempValue = stackTop - 1;
				wr_toFloat[ tempValue->type ]( tempValue );
				CONTINUE;
			}

			CASE(RelativeJump):
			{
				pc += (int16_t)(((int16_t)*pc)<<8) + *(pc+1);
				CONTINUE;
			}

			CASE(RelativeJump8):
			{
				pc += (int8_t)*pc;
				CONTINUE;
			}

			CASE(BZ):
			{
				tempValue = --stackTop;
				pc += wr_ZeroCheck[tempValue->type](tempValue) ? (((int16_t)*pc)<< 8) + *(pc+1) : 2;
				CONTINUE;
			}
			
			CASE(BZ8):
			{
				tempValue = --stackTop;
				pc += wr_ZeroCheck[tempValue->type](tempValue) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(BNZ):
			{
				tempValue = --stackTop;
				pc += wr_ZeroCheck[tempValue->type](tempValue) ? 2 : (((int16_t)*pc)<< 8) + *(pc+1);
				CONTINUE;
			}
			
			CASE(BNZ8):
			{
				tempValue = --stackTop;
				pc += wr_ZeroCheck[tempValue->type](tempValue) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(LogicalAnd): { returnFunc = wr_LogicalAND; goto returnFuncNormal; }
			CASE(LogicalOr): { returnFunc = wr_LogicalOR; goto returnFuncNormal; }
			CASE(CompareLE): { returnFunc = wr_CompareGT; goto returnFuncInverted; }
			CASE(CompareGE): { returnFunc = wr_CompareLT; goto returnFuncInverted; }
			CASE(CompareGT): { returnFunc = wr_CompareGT; goto returnFuncNormal; }
			CASE(CompareLT): { returnFunc = wr_CompareLT; goto returnFuncNormal; }
			CASE(CompareEQ):
			{
				returnFunc = wr_CompareEQ;
returnFuncNormal:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				CONTINUE;
			}
			
			CASE(CompareNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncInverted:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)!returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				CONTINUE;
			}

			CASE(CompareBLE): { returnFunc = wr_CompareGT; goto returnFuncBInverted; }
			CASE(CompareBGE): { returnFunc = wr_CompareLT; goto returnFuncBInverted; }
			CASE(CompareBGT): { returnFunc = wr_CompareGT; goto returnFuncBNormal; }
			CASE(CompareBLT): { returnFunc = wr_CompareLT; goto returnFuncBNormal; }
			CASE(CompareBEQ):
			{
				returnFunc = wr_CompareEQ;
returnFuncBNormal:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 ) ? 2 : (((int16_t)*pc)<< 8) + *(pc+1);
				CONTINUE;
			}

			CASE(CompareBNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 ) ? (((int16_t)*pc)<< 8) + *(pc+1) : 2;
				CONTINUE;
			}

			CASE(CompareBLE8): { returnFunc = wr_CompareGT; goto returnFuncBInverted8; }
			CASE(CompareBGE8): { returnFunc = wr_CompareLT; goto returnFuncBInverted8; }
			CASE(CompareBGT8): { returnFunc = wr_CompareGT; goto returnFuncBNormal8; }
			CASE(CompareBLT8): { returnFunc = wr_CompareLT; goto returnFuncBNormal8; }
			CASE(CompareBEQ8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBNormal8:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 ) ? 2 : *pc;
				CONTINUE;
			}

			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[tempValue->type*6+tempValue2->type]( tempValue, tempValue2 ) ? *pc : 2;
				CONTINUE;
			}

			CASE(PostIncrement):
			{
				tempValue = stackTop - 1;
				wr_postinc[ tempValue->type ]( tempValue, tempValue );
				CONTINUE;
			}

			CASE(PostDecrement):
			{
				tempValue = stackTop - 1;
				wr_postdec[ tempValue->type ]( tempValue, tempValue );
				CONTINUE;
			}
			
			CASE(PreIncrement):
			{
				tempValue = stackTop - 1;
				wr_preinc[ tempValue->type ]( tempValue );
				CONTINUE;
			}
			
			CASE(PreDecrement):
			{
				tempValue = stackTop - 1;
				wr_predec[ tempValue->type ]( tempValue );
				CONTINUE;
			}

			CASE(LogicalNot):
			{
				tempValue = stackTop - 1;
				tempValue->i = wr_LogicalNot[ tempValue->type ]( tempValue );
				tempValue->type = WR_INT;
				CONTINUE;
			}

			CASE(Negate):
			{
				tempValue = stackTop - 1;
				wr_negate[ tempValue->type ]( tempValue );
				CONTINUE;
			}
			
			CASE(LiteralInt8ToGlobal):
			{
				tempValue = context->globalSpace + *pc++;
				tempValue->type = WR_INT;
				tempValue->i = (int32_t)(int8_t)*pc++;
				CONTINUE;
			}
			
			CASE(LiteralInt32ToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->type = WR_INT;
				goto load32ToTemp;
			}
			
			CASE(LiteralInt8ToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->type = WR_INT;
				tempValue->i = (int32_t)(int8_t)*pc++;
				CONTINUE;
			}
			
			CASE(LiteralFloatToGlobal):
			{
				tempValue = context->globalSpace + *pc++;
				tempValue->type = WR_FLOAT;
				goto load32ToTemp;
			}
			
			CASE(LiteralFloatToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->type = WR_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralInt32ToGlobal):
			{
				tempValue = context->globalSpace + *pc++;
				tempValue->type = WR_INT;
load32ToTemp:
				tempValue->i = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				pc += 4;
				CONTINUE;
			}
			
#ifndef JUMPTABLE_INTERPRETER
		}
	}
#endif
}

//------------------------------------------------------------------------------
void wr_makeInt( WRValue* val, int i )
{
	val->type = WR_INT;
	val->i = i;
}

//------------------------------------------------------------------------------
void wr_makeFloat( WRValue* val, float f )
{
	val->type = WR_FLOAT;
	val->f = f;
}

//------------------------------------------------------------------------------
void wr_makeUserData( WRValue* val, int sizeHint )
{
	val->type = WR_USR;
	val->u = new WRUserData( sizeHint );
}

//------------------------------------------------------------------------------
void wr_addUserValue( WRValue* userData, const char* key, WRValue* value )
{
	userData->u->registerValue( key, value );
}

//------------------------------------------------------------------------------
void wr_addUserCharArray( WRValue* userData, const char* name, const unsigned char* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_CHAR, data );
}

//------------------------------------------------------------------------------
void wr_addUserIntArray( WRValue* userData, const char* name, const int* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_INT, data );
}

//------------------------------------------------------------------------------
void wr_addUserFloatArray( WRValue* userData, const char* name, const float* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_FLOAT, data );
}

//------------------------------------------------------------------------------
void WRValue::free()
{
	if ( type == WR_USR )
	{
		delete u;
	}
	else if ( type == WR_ARRAY && (va->m_type & SV_PRE_ALLOCATED) )
	{
		delete va;
	}
}

