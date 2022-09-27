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

#include <memory.h>

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

	loader = 0;
	usr = 0;

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
WRStaticValueArray* WRContext::getSVA( int size, WRStaticValueArrayType type, WRValue* stackTop )
{
	// gc before every alloc may seem a bit much but we want to be miserly
	if ( svAllocated )
	{
		// mark stack
		for( WRValue* s=w->stack; s<stackTop; ++s)
		{
			// an array in the chain?
			if ( (s->xtype&0x3) == WR_EX_ARRAY && !(s->va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( s->va );
			}
		}
		
		// mark context's global
		for( int i=0; i<globals; ++i )
		{
			if ( globalSpace[i].xtype == WR_EX_ARRAY && !(globalSpace[i].va->m_type & SV_PRE_ALLOCATED) )
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

//	if ( !size )
//	{
//		return 0;
//	}

	WRStaticValueArray* ret = new WRStaticValueArray( size, type );
	if ( type == SV_VALUE )
	{
		WRValue *array = (WRValue *)ret->m_data;
		memset( (void*)array, 0, sizeof(WRValue) * size);
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
			if ( i->xtype == WR_EX_ARRAY && !(i->va->m_type & SV_PRE_ALLOCATED) )
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
	*block = (unsigned char *)usr + offset;
	return 0xFFFFFFF; // larger than any bytecode possible
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_runEx( WRState* w, WRContext* C )
{
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
WRContext* wr_run( WRState* w, const unsigned char* block )
{
	WRContext* C = new WRContext( w );
	C->loader = wr_loadSingleBlock;
	C->usr = (void*)block;
	return wr_runEx( w, C );
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, WR_LOAD_BLOCK_FUNC loader, void* usr )
{
	WRContext* C = new WRContext( w );
	C->loader = loader;
	C->usr = usr;
	return wr_runEx( w, C );
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

#ifdef WRENCH_SPRINTF_OPERATIONS
#include <stdio.h>
#endif
//------------------------------------------------------------------------------
char* WRValue::asString( char* string ) const
{
	if ( xtype )
	{
		switch( xtype )
		{
			case WR_EX_USR:
			{
				return string;
			}
			
			case WR_EX_ARRAY:
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
			
			case WR_EX_REFARRAY:
			{
				WRValue temp;
				wr_arrayToValue(this, &temp);
				return temp.asString(string);
			}
		}
	}
	else
	{
		switch( type )
		{
#ifdef WRENCH_SPRINTF_OPERATIONS
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
		}
	}
	
	return string;
}

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
WRFunction* wr_getFunction( WRContext* context, const char* functionName )
{
	return context->localFunctionRegistry.getItem( wr_hashStr(functionName) );
}

#ifdef D_OPCODE
#define PER_INSTRUCTION printf( "S[%d] %d:%s\n", (int)(stackTop - w->stack), (int)*pc, c_opcodeName[*pc]);
#else
#define PER_INSTRUCTION
#endif

#ifdef WRENCH_JUMPTABLE_INTERPRETER
#define CONTINUE { goto *opcodeJumptable[*pc++]; PER_INSTRUCTION; }
#define CASE(LABEL) LABEL
#else
#define CONTINUE { PER_INSTRUCTION; continue; }
#define CASE(LABEL) case O_##LABEL
#endif

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, WRFunction* function, const WRValue* argv, const int argn )
{
#ifdef WRENCH_JUMPTABLE_INTERPRETER
	const void* opcodeJumptable[] =
	{
		&&RegisterFunction,
		&&FunctionListSize,
		&&LiteralInt32,
		&&LiteralZero,
		&&LiteralFloat,
		&&LiteralString,
		&&CallFunctionByHash,
		&&CallFunctionByIndex,
		&&CallLibFunction,
		&&Index,
		&&StackIndexHash,
		&&GlobalIndexHash,
		&&LocalIndexHash,
		&&Assign,
		&&StackSwap,
		&&SwapTwoToTop,
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
		&&BZ,
		&&CompareEQ, 
		&&CompareNE, 
		&&CompareGE,
		&&CompareLE,
		&&CompareGT,
		&&CompareLT,
		&&GSCompareEQ, 
		&&LSCompareEQ, 
		&&GSCompareNE, 
		&&LSCompareNE, 
		&&GSCompareGE,
		&&LSCompareGE,
		&&GSCompareLE,
		&&LSCompareLE,
		&&GSCompareGT,
		&&LSCompareGT,
		&&GSCompareLT,
		&&LSCompareLT,
		&&GSCompareEQBZ, 
		&&LSCompareEQBZ, 
		&&GSCompareNEBZ, 
		&&LSCompareNEBZ, 
		&&GSCompareGEBZ,
		&&LSCompareGEBZ,
		&&GSCompareLEBZ,
		&&LSCompareLEBZ,
		&&GSCompareGTBZ,
		&&LSCompareGTBZ,
		&&GSCompareLTBZ,
		&&LSCompareLTBZ,
		&&GSCompareEQBZ8,
		&&LSCompareEQBZ8,
		&&GSCompareNEBZ8,
		&&LSCompareNEBZ8,
		&&GSCompareGEBZ8,
		&&LSCompareGEBZ8,
		&&GSCompareLEBZ8,
		&&LSCompareLEBZ8,
		&&GSCompareGTBZ8,
		&&LSCompareGTBZ8,
		&&GSCompareLTBZ8,
		&&LSCompareLTBZ8,
		&&PostIncrement,
		&&PostDecrement,
		&&PreIncrement,
		&&PreDecrement,
		&&PreIncrementAndPop,
		&&PreDecrementAndPop,
		&&IncGlobal,
		&&DecGlobal,
		&&IncLocal,
		&&DecLocal,
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
		&&LogicalAnd,
		&&LogicalOr,
		&&LogicalNot,
		&&RelativeJump8,
		&&LiteralInt8,
		&&LiteralInt16,
		&&CallFunctionByHashAndPop,
		&&CallLibFunctionAndPop,
		&&IndexLiteral8,
		&&IndexLiteral16,
		&&IndexLiteral32,
		&&AssignAndPop,
		&&AssignToGlobalAndPop,
		&&AssignToLocalAndPop,
		&&BinaryAdditionAndStoreGlobal,
		&&BinarySubtractionAndStoreGlobal,
		&&BinaryMultiplicationAndStoreGlobal,
		&&BinaryDivisionAndStoreGlobal,
		&&BinaryAdditionAndStoreLocal,
		&&BinarySubtractionAndStoreLocal,
		&&BinaryMultiplicationAndStoreLocal,
		&&BinaryDivisionAndStoreLocal,
		&&BZ8,
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
		&&BLA,
		&&BLA8,
		&&BLO,
		&&BLO8,
		&&LiteralInt8ToGlobal,
		&&LiteralInt16ToGlobal,
		&&LiteralInt32ToLocal,
		&&LiteralInt8ToLocal,
		&&LiteralInt16ToLocal,
		&&LiteralFloatToGlobal,
		&&LiteralFloatToLocal,
		&&LiteralInt32ToGlobal,
		&&GGBinaryMultiplication,
		&&GLBinaryMultiplication,
		&&LLBinaryMultiplication,
		&&GGBinaryAddition,
		&&GLBinaryAddition,
		&&LLBinaryAddition,
		&&GGBinarySubtraction,
		&&GLBinarySubtraction,
		&&LGBinarySubtraction,
		&&LLBinarySubtraction,
		&&GGBinaryDivision,
		&&GLBinaryDivision,
		&&LGBinaryDivision,
		&&LLBinaryDivision,
	};
#endif

	register const unsigned char* pc;
	register WRValue* tempValue;
	register WRValue* tempValue2;
	WRValue* frameBase = 0;
	WRValue* stackTop = w->stack;
	WRValue* globalSpace = context->globalSpace;

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
		context->loader( 0, &pc, context->usr );
		context->bottom = pc;
	}
#else
	// cache these values they are used a lot
	WR_LOAD_BLOCK_FUNC loader = context->loader;
	void* usr = context->usr;
	const unsigned char* top = 0;
	pc = 0;
	int absoluteBottom = 0; // pointer to where in the codebase out bottom actually points to
#endif

	// make room on the bottom for the return value of this script

	if ( function )
	{
		stackTop->p = 0;
		(stackTop++)->p2 = INIT_AS_INT;

		for( args = 0; args < argn; ++args )
		{
			*stackTop++ = argv[args];
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

#ifdef WRENCH_JUMPTABLE_INTERPRETER

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
				unsigned int i = (stackTop - 3)->i;
				unsigned char index = (unsigned char)i;

				context->localFunctions[ index ].arguments = (unsigned char)(i>>8);
				context->localFunctions[ index ].frameSpaceNeeded = (unsigned char)(i>>16);
				context->localFunctions[ index ].hash = (stackTop - 2)->i;

#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				context->localFunctions[ index ].offset = (stackTop - 1)->i + context->bottom; // absolute
#else
				context->localFunctions[ index ].offset = 0; // make sure the offset is clear
				context->localFunctions[ index ].offsetI = (stackTop - 1)->i; // relative
#endif

				context->localFunctions[ index ].frameBaseAdjustment = 1
																	   + context->localFunctions[ index ].frameSpaceNeeded
																	   + context->localFunctions[ index ].arguments;

				context->localFunctionRegistry.set( context->localFunctions[ index ].hash,
													context->localFunctions + index );
				stackTop -= 3;
				CONTINUE;
			}

			CASE(FunctionListSize):
			{
				delete context->localFunctions;
				context->localFunctionRegistry.clear();
				context->localFunctions = new WRFunction[ *pc++ ];
				CONTINUE;
			}

			CASE(LiteralFloat):
			{
				tempValue = stackTop++;
				tempValue->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}
			
			CASE(LiteralInt32):
			{
				tempValue = stackTop++;
				tempValue->p2 = INIT_AS_INT;
				goto load32ToTemp;
			}

			CASE(LiteralZero):
			{
				stackTop->p = 0;
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralString):
			{
				int16_t len = (((int16_t)*pc)<<8) | (int16_t)*(pc + 1);
				pc += 2;
				stackTop->p2 = INIT_AS_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR, stackTop );

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

					((unsigned char *)stackTop->va->m_data)[c] = *pc++;
				}
#endif
				++stackTop;
				CONTINUE;
			}
			
			CASE(ReserveFrame):
			{
				frameBase = stackTop;
				for( int i=0; i<*pc; ++i )
				{
					(stackTop)->p = 0;
					(++stackTop)->p2 = INIT_AS_INT;
				}
				++pc;
				CONTINUE;
			}

			CASE(ReserveGlobalFrame):
			{
				if ( context->globalSpace )
				{
					delete[] context->globalSpace;
				}
				context->globals = *pc++;
				context->globalSpace = new WRValue[ context->globals ];
				for( int i=0; i<context->globals; ++i )
				{
					context->globalSpace[i].p = 0;
					context->globalSpace[i].p2 = INIT_AS_INT;
				}
				globalSpace = context->globalSpace;
				CONTINUE;
			}

			CASE(LoadFromLocal):
			{
				stackTop->p2 = INIT_AS_REF;
				(stackTop++)->p = frameBase + *pc++;
				CONTINUE;
			}

			CASE(LoadFromGlobal):
			{
				stackTop->p2 = INIT_AS_REF;
				(stackTop++)->p = globalSpace + *pc++;
				CONTINUE;
			}

			CASE(Index):
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_index[(tempValue->type<<2)|tempValue2->type]( context, tempValue, tempValue2 );
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
				tempValue = globalSpace + *pc++;
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
returnFuncPostLoad:
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
				tempValue2->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(CompareNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncInverted:
				tempValue = --stackTop;
returnFuncInvertedPostLoad:
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)!returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
				tempValue2->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(GSCompareEQ):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareEQ):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareNE):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareNE):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(GSCompareGT):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareGT):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareLT):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareLT):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareGE):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareGE):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(GSCompareLE):
			{
				tempValue = globalSpace + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareLE):
			{
				tempValue = frameBase + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncInvertedPostLoad;
			}

			CASE(GSCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareGInverted; }
			CASE(GSCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareGInverted; }
			CASE(GSCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareGInverted:
				tempValue = globalSpace + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}
			
			CASE(GSCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareGNormal; }
			CASE(GSCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareGNormal; }
			CASE(GSCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareGNormal:
				tempValue = globalSpace + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}
			
			CASE(LSCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareLInverted; }
			CASE(LSCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareLInverted; }
			CASE(LSCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareLInverted:
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}
			
			CASE(LSCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareLNormal; }
			CASE(LSCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareLNormal; }
			CASE(LSCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareLNormal:
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}
			
			CASE(GSCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareG8Inverted; }
			CASE(GSCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareG8Inverted; }
			CASE(GSCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareG8Inverted:
				tempValue = globalSpace + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(GSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareG8Normal; }
			CASE(GSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareG8Normal; }
			CASE(GSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareG8Normal:
				tempValue = globalSpace + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(LSCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareL8Inverted; }
			CASE(LSCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareL8Inverted; }
			CASE(LSCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareL8Inverted:
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(LSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareL8Normal; }
			CASE(LSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareL8Normal; }
			CASE(LSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareL8Normal:
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}
			
			CASE(PostIncrement): { tempValue = stackTop - 1; wr_postinc[ tempValue->type ]( tempValue, tempValue ); CONTINUE; }
			CASE(PostDecrement): { tempValue = stackTop - 1; wr_postdec[ tempValue->type ]( tempValue, tempValue ); CONTINUE; }
			CASE(PreIncrement): { tempValue = stackTop - 1; wr_preinc[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(PreDecrement): { tempValue = stackTop - 1; wr_predec[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(PreIncrementAndPop): { tempValue = --stackTop; wr_preinc[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(PreDecrementAndPop): { tempValue = --stackTop; wr_predec[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(IncGlobal): { tempValue = globalSpace + *pc++; wr_preinc[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(DecGlobal): { tempValue = globalSpace + *pc++; wr_predec[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(IncLocal): { tempValue = frameBase + *pc++; wr_preinc[ tempValue->type ]( tempValue ); CONTINUE; }
			CASE(DecLocal): { tempValue = frameBase + *pc++; wr_predec[ tempValue->type ]( tempValue ); CONTINUE; }

			CASE(LogicalNot):
			{
				tempValue = stackTop - 1;
				tempValue->i = wr_LogicalNot[ tempValue->type ]( tempValue );
				tempValue->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(Negate):
			{
				tempValue = stackTop - 1;
				wr_negate[ tempValue->type ]( tempValue );
				CONTINUE;
			}

			CASE(BinaryMultiplication): { targetFunc = wr_MultiplyBinary; goto targetFuncOp; }
			CASE(BinarySubtraction): { targetFunc = wr_SubtractBinary; goto targetFuncOp; }
			CASE(BinaryDivision): { targetFunc = wr_DivideBinary; goto targetFuncOp; }
			CASE(BinaryRightShift): { targetFunc = wr_RightShiftBinary; goto targetFuncOp; }
			CASE(BinaryLeftShift): { targetFunc = wr_LeftShiftBinary; goto targetFuncOp; }
			CASE(BinaryMod): { targetFunc = wr_ModBinary; goto targetFuncOp; }
			CASE(BinaryOr): { targetFunc = wr_ORBinary; goto targetFuncOp; }
			CASE(BinaryXOR): { targetFunc = wr_XORBinary; goto targetFuncOp; }
			CASE(BinaryAnd): { targetFunc = wr_ANDBinary; goto targetFuncOp; }
			CASE(BinaryAddition):
			{
				targetFunc = wr_AdditionBinary;
targetFuncOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				targetFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, tempValue2 );
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
				voidFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
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

			CASE(SwapTwoToTop): // accomplish two (or three when optimized) swaps into one instruction
			{
				tempValue = stackTop - *pc++;

				uint32_t t = (stackTop - 1)->p2;
				const void* p = (stackTop - 1)->p;

				(stackTop - 1)->p2 = tempValue->p2;
				(stackTop - 1)->p = tempValue->p;

				tempValue->p = p;
				tempValue->p2 = t;

				tempValue = stackTop - *pc++;

				t = (stackTop - 2)->p2;
				p = (stackTop - 2)->p;

				(stackTop - 2)->p2 = tempValue->p2;
				(stackTop - 2)->p = tempValue->p;

				tempValue->p = p;
				tempValue->p2 = t;

				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				WRCFunctionCallback* cF;

				args = *(pc+4); // which have already been pushed
				if ( (cF = w->c_functionRegistry.get( (((int32_t)*pc) << 24)
													  | (((int32_t)*(pc+1)) << 16)
													  | (((int32_t)*(pc+2)) << 8)
													  | ((int32_t)*(pc+3)))) )
				{
					cF->function( w, stackTop - args, args, *(stackTop - (args+1)), cF->usr );
				}

				stackTop -= args;
				pc += 5;
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				// don't care about return value
				WRCFunctionCallback* cF;

				args = *(pc+4); // which have already been pushed
				if ( (cF = w->c_functionRegistry.get( (((int32_t)*pc) << 24)
													  | (((int32_t)*(pc+1)) << 16)
													  | (((int32_t)*(pc+2)) << 8)
													  | ((int32_t)*(pc+3)))) )
				{
					cF->function( w, stackTop - args, args, *(stackTop - (args+1)), cF->usr );
				}

				stackTop -= args + 1;
				pc += 6; // skip past the pop operation that is next in line (linking can't remove the instruction... yet)
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
						stackTop -= args - function->arguments; // unexpected arguments are poofed
					}
					else
					{
						// un-specified arguments are set to IntZero
						for( int a=args; a < function->arguments; ++a )
						{
							stackTop->p = 0;
							(stackTop++)->p2 = INIT_AS_INT;
						}
					}
				}

				// the arguments are at framepace +0, add more to make
				// up for the locals in the function

				// really wish I could do this.. alas.. if array chaff
				// is left on the stack, released memory might be
				// written to :(
				//stackTop += function->frameSpaceNeeded; // locals are NOT initialized

				for( int l=0; l<function->frameSpaceNeeded; ++l ) // locals are NOT initialized
				{
					(stackTop++)->p2 = INIT_AS_INT;
				}


				// temp value contains return vector/frame base
				tempValue = stackTop++; // return vector
				tempValue->frame = frameBase;
				
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

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				CONTINUE;
			}

			// it kills me that these are identical except for the "+1"
			// but I have yet to figure out a way around that, "andPop"
			// is just too good and common an optimization :(
			CASE(CallLibFunction):
			{
				WR_LIB_CALLBACK lib;
				stackTop -= (args = *(pc+4)); // which have already been pushed

				if ( (lib = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
															 | (((int32_t)*(pc+1)) << 16)
															 | (((int32_t)*(pc+2)) << 8)
															 | ((int32_t)*(pc+3)) )) )
				{
					lib( stackTop, args );
				}

				pc += 5;
				CONTINUE;
			}

			CASE(CallLibFunctionAndPop):
			{
				WR_LIB_CALLBACK lib;
				stackTop -= (args = *(pc+4)); // which have already been pushed

				if ( (lib = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
															 | (((int32_t)*(pc+1)) << 16)
															 | (((int32_t)*(pc+2)) << 8)
															 | ((int32_t)*(pc+3)) )) )
				{
					lib( stackTop, args );
				}
				
				--stackTop;
				pc += 5;
				CONTINUE;
			}
			
			CASE(Stop):
			{
				context->stopLocation = (int32_t)((pc - 1) - context->bottom);
				return WR_ERR_None;
			}

			CASE(Return):
			{
				tempValue = stackTop - 2;
				
#ifndef WRENCH_PARTIAL_BYTECODE_LOADS
				
				pc = (unsigned char*)tempValue->p; // grab return PC
				
#else
				int returnOffset = (stackTop - 2)->i;
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
					*(frameBase - 1) = *stackTop->r;
				}
				else
				{
					*(frameBase - 1) = *stackTop;
				}
				
				stackTop = frameBase;
				frameBase = tempValue->frame;
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
				pc += (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(BZ):
			{
				tempValue = --stackTop;
				pc += wr_LogicalNot[tempValue->type](tempValue) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
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
				voidFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
				
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreGlobal) : { targetFunc = wr_AdditionBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinarySubtractionAndStoreGlobal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryMultiplicationAndStoreGlobal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryDivisionAndStoreGlobal):
			{
				targetFunc = wr_DivideBinary;

targetFuncStoreGlobalOp:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				targetFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, globalSpace + *pc++ );
				CONTINUE;
			}

			CASE(BinaryAdditionAndStoreLocal): { targetFunc = wr_AdditionBinary; goto targetFuncStoreLocalOp; }
			CASE(BinarySubtractionAndStoreLocal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryMultiplicationAndStoreLocal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryDivisionAndStoreLocal):
			{
				targetFunc = wr_DivideBinary;

targetFuncStoreLocalOp:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				targetFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, frameBase + *pc++ );
				CONTINUE;
			}

			CASE(IndexLiteral32):
			{
				stackTop->i = (((int32_t)*pc) << 24) |
							  (((int32_t)*(pc + 1)) << 16) |
							  (((int32_t)*(pc + 2)) << 8) |
							  (int32_t)*(pc + 3);
				pc += 4;
				goto indexLiteral;
			}

			CASE(IndexLiteral16):
			{
				stackTop->i = (int32_t)(int16_t)((((int16_t)*(pc)) << 8) | ((int16_t)*(pc+1)));
				pc += 2;
				goto indexLiteral;
			}

			CASE(IndexLiteral8):
			{
				stackTop->i = *pc++;
indexLiteral:
				tempValue = stackTop - 1;
				wr_index[(WR_INT*4)|tempValue->type]( context, stackTop, tempValue );
				CONTINUE;
			}
			
			CASE(AssignToGlobalAndPop):
			{
				tempValue = globalSpace + *pc++;
				tempValue2 = --stackTop;
				wr_assign[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(AssignToLocalAndPop):
			{
				tempValue = frameBase + *pc++;
				tempValue2 = --stackTop;
				wr_assign[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 );
				CONTINUE;
			}

			CASE(LiteralInt8):
			{
				stackTop->i = (int32_t)(int8_t)*pc++;
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16):
			{
				stackTop->i = (int32_t)(int16_t)((((int16_t)*(pc)) << 8) | ((int16_t)*(pc+1)));
				pc += 2;
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}
			
			CASE(RelativeJump8):
			{
				pc += (int8_t)*pc;
				CONTINUE;
			}

			CASE(BZ8):
			{
				tempValue = --stackTop;
				pc += wr_LogicalNot[tempValue->type](tempValue) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(BLA):
			{
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += wr_LogicalAND[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(BLA8):
			{
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += wr_LogicalAND[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(BLO):
			{
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += wr_LogicalOR[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(BLO8):
			{
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += wr_LogicalOR[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(GGBinaryMultiplication):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = globalSpace + *pc++;
				wr_MultiplyBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryMultiplication):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = frameBase + *pc++;
				wr_MultiplyBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryMultiplication):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = frameBase + *pc++;
				wr_MultiplyBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinaryAddition):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = globalSpace + *pc++;
				wr_AdditionBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryAddition):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = frameBase + *pc++;
				wr_AdditionBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryAddition):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = frameBase + *pc++;
				wr_AdditionBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinarySubtraction):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = globalSpace + *pc++;
				wr_SubtractBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinarySubtraction):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = frameBase + *pc++;
				wr_SubtractBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinarySubtraction):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = globalSpace + *pc++;
				wr_SubtractBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinarySubtraction):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = frameBase + *pc++;
				wr_SubtractBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinaryDivision):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = globalSpace + *pc++;
				wr_DivideBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryDivision):
			{
				tempValue2 = globalSpace + *pc++;
				tempValue = frameBase + *pc++;
				wr_DivideBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinaryDivision):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = globalSpace + *pc++;
				wr_DivideBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryDivision):
			{
				tempValue2 = frameBase + *pc++;
				tempValue = frameBase + *pc++;
				wr_DivideBinary[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2, stackTop++ );
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
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(CompareBNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
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
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? 2 : *pc;
				CONTINUE;
			}

			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				pc += returnFunc[(tempValue->type<<2)|tempValue2->type]( tempValue, tempValue2 ) ? *pc : 2;
				CONTINUE;
			}
			
			CASE(LiteralInt8ToGlobal):
			{
				tempValue = globalSpace + *pc++;
				tempValue->i = (int32_t)(int8_t)*pc++;
				tempValue->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToGlobal):
			{
				tempValue = globalSpace + *pc++;
				tempValue->i = (int32_t)(int16_t)((((int16_t) * (pc)) << 8) | ((int16_t) * (pc + 1)));
				tempValue->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralInt32ToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->p2 = INIT_AS_INT;
				goto load32ToTemp;
			}
			
			CASE(LiteralInt8ToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->i = (int32_t)(int8_t)*pc++;
				tempValue->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->i = (int32_t)(int16_t)((((int16_t) * (pc)) << 8) | ((int16_t) * (pc + 1)));
				tempValue->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralFloatToGlobal):
			{
				tempValue = globalSpace + *pc++;
				tempValue->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}
			
			CASE(LiteralFloatToLocal):
			{
				tempValue = frameBase + *pc++;
				tempValue->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralInt32ToGlobal):
			{
				tempValue = globalSpace + *pc++;
				tempValue->p2 = INIT_AS_INT;
load32ToTemp:
				tempValue->i = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				pc += 4;
				CONTINUE;
			}
			
#ifndef WRENCH_JUMPTABLE_INTERPRETER
		}
	}
#endif
}

//------------------------------------------------------------------------------
void wr_makeInt( WRValue* val, int i )
{
	val->p2 = INIT_AS_INT;
	val->i = i;
}

//------------------------------------------------------------------------------
void wr_makeFloat( WRValue* val, float f )
{
	val->p2 = INIT_AS_FLOAT;
	val->f = f;
}

//------------------------------------------------------------------------------
void wr_makeUserData( WRValue* val, int sizeHint )
{
	val->p2 = INIT_AS_USR;
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
	val->p2 = INIT_AS_ARRAY;
	val->va = new WRStaticValueArray( len, SV_CHAR, data );
}

//------------------------------------------------------------------------------
void wr_addUserIntArray( WRValue* userData, const char* name, const int* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->p2 = INIT_AS_ARRAY;
	val->va = new WRStaticValueArray( len, SV_INT, data );
}

//------------------------------------------------------------------------------
void wr_addUserFloatArray( WRValue* userData, const char* name, const float* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->p2 = INIT_AS_ARRAY;
	val->va = new WRStaticValueArray( len, SV_FLOAT, data );
}

//------------------------------------------------------------------------------
void WRValue::free()
{
	if ( xtype == WR_EX_USR )
	{
		delete u;
	}
	else if ( xtype == WR_EX_ARRAY && (va->m_type & SV_PRE_ALLOCATED) )
	{
		delete va;
	}
}

