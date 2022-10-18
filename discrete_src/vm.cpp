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
		WRGCObject* next = svAllocated->m_next;
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
void WRContext::mark( WRValue* s )
{
	if ( IS_REFARRAY(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		if ( !s->r->va->m_preAllocated )
		{
			mark( s->r );
		}
		return;
	}

	if ( s->va->m_preAllocated )
	{
		return;
	}
	
	WRGCObject* sva = s->va;
	
	if ( IS_SVA_VALUE_TYPE(sva) )
	{
		// this is an array of values, check them for array-ness too

		WRValue* top = sva->m_Vdata + sva->m_size;
		for( WRValue* V = sva->m_Vdata; V<top; ++V )
		{
			if ( IS_EXARRAY_TYPE(V->xtype) && !(V->va->m_preAllocated) && !(V->va->m_size & 0x40000000) )
			{
				mark( V );
			}
		}
	}

	sva->m_size |= 0x40000000;
}

//------------------------------------------------------------------------------
void WRContext::gc( WRValue* stackTop )
{
	if ( !svAllocated )
	{
		return;
	}
	
	// mark stack
	for( WRValue* s=w->stack; s<stackTop; ++s)
	{
		// an array in the chain?
		if ( IS_EXARRAY_TYPE(s->xtype) && !(s->va->m_preAllocated) )
		{
			mark( s );
		}
	}

	// mark context's global
	for( int i=0; i<globals; ++i )
	{
		if ( IS_EXARRAY_TYPE(globalSpace[i].xtype) && !(globalSpace[i].va->m_preAllocated) )
		{
			mark( globalSpace + i );
		}
	}

	// sweep
	WRGCObject* current = svAllocated;
	WRGCObject* prev = 0;
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

//------------------------------------------------------------------------------
WRGCObject* WRContext::getSVA( int size, WRGCObjectType type, bool init )
{
	WRGCObject* ret = new WRGCObject( size, type );
	if ( init )
	{
		memset( (char*)ret->m_Cdata, 0, sizeof(WRValue) * size);
	}

	ret->m_next = svAllocated;
	svAllocated = ret;

	return ret;
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
	return 0x3FFFFFF; // larger than any bytecode possible
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block )
{
	WRContext* C = new WRContext( w );
	C->next = w->contextList;
	C->bottom = block;
	
	w->contextList = C;

	if ( wr_callFunction(w, C, (int32_t)0) )
	{
		wr_destroyContext( w, C );
		return 0;
	}

	return C;
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
int WRValue::asInt() const
{
	// this should be faster than a switch by checking the most common cases first
	if ( type == WR_INT )
	{
		return i;
	}
	if ( type == WR_REF )
	{
		return r->asInt();
	}
	if ( type == WR_FLOAT )
	{
		return (int)f;
	}
	if ( type == WR_EX )
	{
		return IS_REFARRAY(xtype) ? arrayValueAsInt() : 0;
	}
	return i;
}

//------------------------------------------------------------------------------
float WRValue::asFloat() const
{
	if ( type == WR_FLOAT )
	{
		return f;
	}
	if ( type == WR_REF )
	{
		return r->asFloat();
	}
	if ( type == WR_INT )
	{
		return (float)i;
	}
	if ( type == WR_EX )
	{
		return IS_REFARRAY(xtype) ? arrayValueAsFloat() : 0;
	}
	return f;
}

//------------------------------------------------------------------------------
int wr_itoa( int i, char* string, size_t len )
{
	char buf[14];
	size_t pos = 0;
	int val;
	if ( i < 0 )
	{
		string[pos++] = '-';
		val = -i;
	}
	else
	{
		val = i;
	}

	int digit = 0;
	do
	{
		buf[digit++] = (val % 10) + '0';
	} while( val /= 10 );

	--digit;

	for( ; digit>=0 && pos < len; --digit )
	{
		string[pos++] = buf[digit];
	}
	string[pos] = 0;
	return pos;
}

//------------------------------------------------------------------------------
void wr_ftoa( float f, char* buf, int len )
{
	int pos = 0;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		buf[pos++] = '-';
	}

	if ( pos > len )
	{
		buf[0] = 0;
		return;
	}

	// round value according the precision
	f += 5.f / (float)10e6;

	// integer part...
	int intPart = (int)f;
	f -= intPart;

	if ( intPart )
	{
		pos += wr_itoa( intPart, buf + pos, len - pos );
	}
	else
	{
		buf[pos++] = '0';
	}

	// decimal part place decimal point
	buf[pos++] = '.';
	
	// convert
	for( int p=0; pos < len && p<5; ++p )
	{
		f *= 10.0;
		char c = (char)f;
		buf[pos++] = '0' + c;
		f -= c;
	}

	for( --pos; buf[pos] == '0' && buf[pos] != '.' ; --pos );

	if ( buf[pos] != '.' )
	{
		++pos;
	}
	
	buf[pos] = 0;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, size_t len ) const
{
	if ( xtype )
	{
		switch( xtype )
		{
			case WR_EX_USR:
			{
				return string;
			}
			
			case WR_EX_STRUCT:
			{
				strncpy( string, "struct", len );
				return string;
			}
			
			case WR_EX_ARRAY:
			{
				unsigned int s = 0;

				size_t size = va->m_size > len ? len : va->m_size;
				for( ; s<size; ++s )
				{
					switch( va->m_type)
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
				if ( IS_EXARRAY_TYPE(r->xtype) )
				{
					WRValue temp;
					wr_arrayToValue(this, &temp);
					return temp.asString(string, len);
				}
				else
				{
					return r->asString(string, len);
				}
			}

			case WR_EX_NONE:
			{
				strncpy( string, "None", len );
				break;
			}
		}
	}
	else
	{
		switch( type )
		{
			case WR_FLOAT:
			{
				wr_ftoa( f, string, len );
				break;
			}

			case WR_INT:
			{
				wr_itoa( i, string, len );
				break;
			}

			case WR_REF: { return r->asString( string, len ); }
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
WRValue* wr_returnValueFromLastCall( WRState* w )
{
	return w->stack + 1; // this is where it ends up
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
#define CONTINUE { PER_INSTRUCTION; goto *opcodeJumptable[*pc++];  }
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
		&&ReserveGlobalFrame,
		&&FunctionListSize,

		&&LiteralInt32,
		&&LiteralZero,
		&&LiteralFloat,
		&&LiteralString,

		&&CallFunctionByHash,
		&&CallFunctionByHashAndPop,
		&&CallFunctionByIndex,
		&&PushIndexFunctionReturnValue,

		&&CallLibFunction,
		&&CallLibFunctionAndPop,

		&&NewObjectTable,
		&&AssignToObjectTableByOffset,
		&&AssignToHashTableAndPop,

		&&PopOne,
		&&ReturnZero,
		&&Return,
		&&Stop,

		&&Index,
		&&IndexSkipLoad,
		&&CountOf,
		&&HashOf,

		&&StackIndexHash,
		&&GlobalIndexHash,
		&&LocalIndexHash,

		&&StackSwap,
		&&SwapTwoToTop,

		&&LoadFromLocal,
		&&LoadFromGlobal,

		&&LLValues,
		&&LGValues,
		&&GLValues,
		&&GGValues,

		&&BinaryRightShiftSkipLoad,
		&&BinaryLeftShiftSkipLoad,
		&&BinaryAndSkipLoad,
		&&BinaryOrSkipLoad,
		&&BinaryXORSkipLoad,
		&&BinaryModSkipLoad,

		&&BinaryMultiplication,
		&&BinarySubtraction,
		&&BinaryDivision,
		&&BinaryRightShift,
		&&BinaryLeftShift,
		&&BinaryMod,
		&&BinaryOr,
		&&BinaryXOR,
		&&BinaryAnd,
		&&BinaryAddition,

		&&BitwiseNOT,

		&&CoerceToInt,
		&&CoerceToFloat,

		&&RelativeJump,
		&&RelativeJump8,

		&&BZ,
		&&BZ8,

		&&LogicalAnd,
		&&LogicalOr,
		&&CompareLE,
		&&CompareGE,
		&&CompareGT,
		&&CompareLT,
		&&CompareEQ,
		&&CompareNE, 

		&&GGCompareEQ, 
		&&GGCompareNE, 
		&&GGCompareGT,
		&&GGCompareLT,
		
		&&LLCompareGT,
		&&LLCompareLT,
		&&LLCompareEQ, 
		&&LLCompareNE, 

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

		&&LLCompareLTBZ,
		&&LLCompareGTBZ,
		&&LLCompareEQBZ,
		&&LLCompareNEBZ,
		&&GGCompareLTBZ,
		&&GGCompareGTBZ,
		&&GGCompareEQBZ,
		&&GGCompareNEBZ,

		&&LLCompareLTBZ8,
		&&LLCompareGTBZ8,
		&&LLCompareEQBZ8,
		&&LLCompareNEBZ8,
		&&GGCompareLTBZ8,
		&&GGCompareGTBZ8,
		&&GGCompareEQBZ8,
		&&GGCompareNEBZ8,

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

		&&Assign,
		&&AssignAndPop,
		&&AssignToGlobalAndPop,
		&&AssignToLocalAndPop,
		&&AssignToArrayAndPop,

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

		&&LogicalNot,
		&&Negate,

		&&LiteralInt8,
		&&LiteralInt16,

		&&IndexLiteral8,
		&&IndexLiteral16,

		&&IndexLocalLiteral8,
		&&IndexGlobalLiteral8,
		&&IndexLocalLiteral16,
		&&IndexGlobalLiteral16,

		&&BinaryAdditionAndStoreGlobal,
		&&BinarySubtractionAndStoreGlobal,
		&&BinaryMultiplicationAndStoreGlobal,
		&&BinaryDivisionAndStoreGlobal,

		&&BinaryAdditionAndStoreLocal,
		&&BinarySubtractionAndStoreLocal,
		&&BinaryMultiplicationAndStoreLocal,
		&&BinaryDivisionAndStoreLocal,

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

	const unsigned char* pc;
	
	WRValue* register0 = 0;
	union
	{
		WRValue* register1;
		WR_LIB_CALLBACK lib;
	};
	WRValue* frameBase = 0;
	WRValue* stackTop = w->stack;
	WRValue* globalSpace = context->globalSpace;

	union
	{
		// never used at the same time..save RAM!
		WRValue* register2;
		unsigned char args;
		WRVoidFunc* voidFunc;
		WRReturnFunc* returnFunc;
		WRTargetFunc* targetFunc;
	};

	w->err = WR_ERR_None;

	if ( function )
	{
		stackTop->p = 0;
		(stackTop++)->p2 = INIT_AS_INT;

		for( args = 0; args < argn; ++args )
		{
			*stackTop++ = argv[args];
		}
		
		pc = context->bottom + context->stopLocation;
		goto callFunction;
	}

	pc = context->bottom;

#ifdef WRENCH_JUMPTABLE_INTERPRETER

	CONTINUE;
	
#else

	for(;;)
	{
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
				context->localFunctions[ index ].offset = (stackTop - 1)->i + context->bottom; // absolute
				context->localFunctions[ index ].frameBaseAdjustment = 1
																	   + context->localFunctions[ index ].frameSpaceNeeded
																	   + context->localFunctions[ index ].arguments;

				context->localFunctionRegistry.set( context->localFunctions[ index ].hash,
													context->localFunctions + index );
				stackTop -= 3;
				CONTINUE;
			}

			CASE(ReserveGlobalFrame):
			{
				delete[] context->globalSpace;
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

			CASE(FunctionListSize):
			{
				delete context->localFunctions;
				context->localFunctionRegistry.clear();
				context->localFunctions = new WRFunction[ *pc++ ];
				CONTINUE;
			}
		
			CASE(LiteralInt32):
			{
				register0 = stackTop++;
				register0->p2 = INIT_AS_INT;
				goto load32ToTemp;
			}

			CASE(LiteralZero):
			{
literalZero:
				(stackTop++)->init();
				CONTINUE;
			}

			CASE(LiteralFloat):
			{
				register0 = stackTop++;
				register0->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralString):
			{
				uint16_t len = (((uint16_t)*pc)<<8) | (uint16_t)*(pc + 1);
				pc += 2;
				
				context->gc( stackTop  );
				stackTop->p2 = INIT_AS_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR, false );
				memcpy( (unsigned char *)stackTop->va->m_data, pc, len );
				pc += len;
				
				++stackTop;
				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				args = *pc++;

				WRCFunctionCallback* cF;

				// initialize a return value of 'zero'
				register0 = stackTop;
				register0->p = 0;
				register0->p2 = INIT_AS_INT;

				if ( (cF = w->c_functionRegistry.get( (((int32_t)*pc) << 24)
													  | (((int32_t)*(pc+1)) << 16)
													  | (((int32_t)*(pc+2)) << 8)
													  | ((int32_t)*(pc+3)))) )
				{
					cF->function( w, stackTop - args, args, *stackTop, cF->usr );
				}

				// DO care about return value, which will be at the top
				// of the stack

				if ( args )
				{
					stackTop -= (args - 1);
					*stackTop = *register0;
				}
				else
				{
					++stackTop; // register0 IS the stack top no other work needed
				}
				pc += 4;
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				args = *pc++;

				WRCFunctionCallback* cF;
				if ( (cF = w->c_functionRegistry.get( (((int32_t)*pc) << 24)
													  | (((int32_t)*(pc+1)) << 16)
													  | (((int32_t)*(pc+2)) << 8)
													  | ((int32_t)*(pc+3)))) )
				{
					cF->function( w, stackTop - args, args, *stackTop, cF->usr );
				}

				stackTop -= args;
				pc += 4;
				CONTINUE;
			}

			CASE(CallFunctionByIndex):
			{
				args = *pc++;
				function = context->localFunctions + *pc++;
				pc += *pc;
callFunction:				
				// rectify arg count?
				if ( args != function->arguments ) // expect correctness, so optimize that code path
				{
					if ( args > function->arguments )
					{
						stackTop -= args - function->arguments; // extra arguments are *poofed*
					}
					else
					{
						// un-specified arguments are set to IntZero
						for( ; args < function->arguments; ++args )
						{
							stackTop->p = 0;
							(stackTop++)->p2 = INIT_AS_INT;
						}
					}
				}

				// locals are NOT initialized.. but we have to make
				// sure they have a non-collectable type or the gc can
				// get confused.. shame though this would be SO much
				// faster... TODO figure out how to use it!
				//				stackTop += function->frameSpaceNeeded; 
				for( int l=0; l<function->frameSpaceNeeded; ++l )
				{
					(stackTop++)->p2 = INIT_AS_INT;
				}

				// temp value contains return vector/frame base
				register0 = stackTop++; // return vector
				register0->frame = frameBase;

				register0->p = pc;
				pc = function->offset;

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				CONTINUE;
			}

			CASE(PushIndexFunctionReturnValue):
			{
				if ( (++register0)->type == WR_REF )
				{
					*(stackTop++) = *register0->r;
				}
				else
				{
					*(stackTop++) = *register0;
				}

				CONTINUE;
			}

			// it kills me that these are identical except for the "+1"
			// but I have yet to figure out a way around that, "andPop"
			// is just too good and common an optimization :(
			CASE(CallLibFunction):
			{
				args = *pc++; // which have already been pushed
				register0 = stackTop;
				register0->p2 = INIT_AS_INT;
				register0->p = 0;

				if ( (lib = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
															  | (((int32_t)*(pc+1)) << 16)
															  | (((int32_t)*(pc+2)) << 8)
															  | ((int32_t)*(pc+3)) )) )
				{
					lib( stackTop, args, context );
				}

				stackTop -= (args - 1);
				*(stackTop - 1) = *register0;

				pc += 4;
				CONTINUE;
			}

			CASE(CallLibFunctionAndPop):
			{
				args = *pc++; // which have already been pushed

				if ( (lib = w->c_libFunctionRegistry.getItem( (((int32_t)*pc) << 24)
															  | (((int32_t)*(pc+1)) << 16)
															  | (((int32_t)*(pc+2)) << 8)
															  | ((int32_t)*(pc+3)) )) )
				{
					lib( stackTop, args, context );
				}

				stackTop -= args;
				pc += 4;
				CONTINUE;
			}

			CASE(NewObjectTable):
			{
				const unsigned char* table = context->bottom + (int32_t)((((int16_t)*pc)<<8) + *(pc+1));
				pc += 2;

				if ( table > context->bottom )
				{
					// if unit was called with no arguments from global
					// level there are not "free" stack entries to
					// gnab, so create it here, but preserve the
					// first value

					// NOTE: we are guaranteed to have at least one
					// value if table > context->bottome

					unsigned char count = *table++;

					register1 = (stackTop + *table)->r;
					register2 = (stackTop + *table)->r2;

					stackTop->p2 = INIT_AS_STRUCT;

					// table : members in local space
					// table + 1 : arguments + 1 (+1 to save the calculation below)
					// table +2/3 : m_mod
					// table + 4: [static hash table ]

					stackTop->va = context->getSVA( count, SV_VALUE, false );

					stackTop->va->m_ROMHashTable = table + 3;
					stackTop->va->m_mod = (((int16_t)*(table+1)) << 8) + *(table+2);

					register0 = (WRValue*)(stackTop->va->m_data);
					register0->r = register1;
					(register0++)->r2 = register2;

					if ( --count > 0 )
					{
						memcpy( (char*)register0, stackTop + *table + 1, count*sizeof(WRValue) );
					}

					context->gc(++stackTop); // dop this here to take care of any memory the 'new' allocated
				}
				else
				{
					goto literalZero;
				}

				CONTINUE;
			}

			CASE(AssignToObjectTableByOffset):
			{
				register0 = --stackTop;
				register1 = (stackTop - 1);
				if ( !IS_EXARRAY_TYPE(register1->xtype) || *pc < register1->va->m_size )
				{
					register1 = register1->va->m_Vdata + *pc++;
					wr_assign[register1->type<<2|register0->type]( register1, register0 );
				}
				else
				{
					++pc;
				}

				CONTINUE;
			}

			CASE(AssignToHashTableAndPop):
			{
				register0 = --stackTop; // value
				register1 = --stackTop; // index
				
				wr_assignToHashTable( context, register1, register0, stackTop - 1 );
				CONTINUE;
			}

			CASE(PopOne):
			{
				--stackTop;
				CONTINUE;
			}

			CASE(ReturnZero):
			{
				(stackTop++)->init();
			}
			CASE(Return):
			{
				register0 = stackTop - 2;

				pc = (unsigned char*)register0->p; // grab return PC

				stackTop = frameBase;
				frameBase = register0->frame;
				CONTINUE;
			}

			CASE(Stop):
			{
				context->stopLocation = (int32_t)((pc - 1) - context->bottom);
				return WR_ERR_None;
			}

			CASE(Index):
			{
				register0 = --stackTop;
				register1 = --stackTop;
			}

			CASE(IndexSkipLoad):
			{
				wr_index[(register0->type<<2)|register1->type]( context, register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(CountOf):
			{
				register0 = stackTop - 1;
				while( register0->type == WR_REF )
				{
					register0 = register0->r;
				}
				wr_countOfArrayElement( register0, stackTop - 1 );
				CONTINUE;
			}

			CASE(HashOf):
			{
				register0 = stackTop - 1;
				register0->ui = register0->getHash();
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(StackIndexHash):
			{
				register0 = stackTop - 1;
				register1 = register0;
				goto indexHash;
			}

			CASE(GlobalIndexHash):
			{
				register0 = globalSpace + *pc++;
				goto indexHashPreload;
			}

			CASE(LocalIndexHash):
			{
				register0 = frameBase + *pc++;
indexHashPreload:
				register1 = stackTop++;
indexHash:
				wr_IndexHash[ register0->type ]( register0,
												 register1,
												 (((uint32_t)*pc) << 24) |
												 (((uint32_t)*(pc+1)) << 16) |
												 (((uint32_t)*(pc+2)) << 8) |
												 (uint32_t)*(pc+3) );
				pc += 4;
				CONTINUE;
			}
			
			CASE(StackSwap):
			{
				register0 = stackTop - 1;
				register1 = stackTop - *pc++;
				register2 = register0->r2;
				WRValue* r = register0->r;

				register0->r2 = register1->r2;
				register0->r = register1->r;

				register1->r = r;
				register1->r2 = register2;

				CONTINUE;
			} 

			CASE(SwapTwoToTop): // accomplish two (or three when optimized) swaps into one instruction
			{
				register0 = stackTop - *pc++;

				uint32_t t = (stackTop - 1)->p2;
				const void* p = (stackTop - 1)->p;

				(stackTop - 1)->p2 = register0->p2;
				(stackTop - 1)->p = register0->p;

				register0->p = p;
				register0->p2 = t;

				register0 = stackTop - *pc++;

				t = (stackTop - 2)->p2;
				p = (stackTop - 2)->p;

				(stackTop - 2)->p2 = register0->p2;
				(stackTop - 2)->p = register0->p;

				register0->p = p;
				register0->p2 = t;

				CONTINUE;
			}

			CASE(LoadFromLocal):
			{
				stackTop->p = frameBase + *pc++;
				(stackTop++)->p2 = INIT_AS_REF;
				CONTINUE;
			}

			CASE(LoadFromGlobal):
			{
				stackTop->p = globalSpace + *pc++;
				(stackTop++)->p2 = INIT_AS_REF;
				CONTINUE;
			}

			CASE(LLValues): { register0 = frameBase + *pc++; register1 = frameBase + *pc++; CONTINUE; }
			CASE(LGValues): { register0 = frameBase + *pc++; register1 = globalSpace + *pc++; CONTINUE; }
			CASE(GLValues): { register0 = globalSpace + *pc++; register1 = frameBase + *pc++; CONTINUE; }
			CASE(GGValues):	{ register0 = globalSpace + *pc++; register1 = globalSpace + *pc++; CONTINUE; }

			CASE(BinaryRightShiftSkipLoad): { targetFunc = wr_RightShiftBinary; goto targetFuncOpSkipLoad; }
			CASE(BinaryLeftShiftSkipLoad): { targetFunc = wr_LeftShiftBinary; goto targetFuncOpSkipLoad; }
			CASE(BinaryAndSkipLoad): { targetFunc = wr_ANDBinary; goto targetFuncOpSkipLoad; }
			CASE(BinaryOrSkipLoad): { targetFunc = wr_ORBinary; goto targetFuncOpSkipLoad; }
			CASE(BinaryXORSkipLoad): { targetFunc = wr_XORBinary; goto targetFuncOpSkipLoad; }
			CASE(BinaryModSkipLoad):
			{
				targetFunc = wr_ModBinary;
targetFuncOpSkipLoad:
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, stackTop++ );
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
				register0 = --stackTop;
				register1 = stackTop - 1;
				targetFunc[(register0->type<<2)|register1->type]( register0, register1, register1 );
				CONTINUE;
			}

			CASE(BitwiseNOT):
			{
				register0 = stackTop - 1;
				wr_bitwiseNot[ register0->type ]( register0 );
				CONTINUE;
			}

			CASE(CoerceToInt):
			{
				register0 = stackTop - 1;
				wr_toInt[ register0->type ]( register0 );
				CONTINUE;
			}

			CASE(CoerceToFloat):
			{
				register0 = stackTop - 1;
				wr_toFloat[ register0->type ]( register0 );
				CONTINUE;
			}

			CASE(RelativeJump):
			{
				pc += (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(RelativeJump8):
			{
				pc += (int8_t)*pc;
				CONTINUE;
			}

			CASE(BZ):
			{
				register0 = --stackTop;
				pc += wr_LogicalNot[register0->type](register0) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}

			CASE(BZ8):
			{
				register0 = --stackTop;
				pc += wr_LogicalNot[register0->type](register0) ? (int8_t)*pc : 2;
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
				register0 = --stackTop;
returnFuncPostLoad:
				register1 = stackTop - 1;
				register1->i = (int)returnFunc[(register0->type<<2)|register1->type]( register0, register1 );
				register1->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(CompareNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncInverted:
				register0 = --stackTop;
returnFuncInvertedPostLoad:
				register1 = stackTop - 1;
				register1->i = (int)!returnFunc[(register0->type<<2)|register1->type]( register0, register1 );
				register1->p2 = INIT_AS_INT;
				CONTINUE;
			}
			
			CASE(GGCompareEQ):
			{
				returnFunc = wr_CompareEQ;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareEQPost;
			}
			CASE(GGCompareNE):
			{
				returnFunc = wr_CompareEQ;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareNEPost;
			}
			CASE(GGCompareGT):
			{
				returnFunc = wr_CompareGT;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareEQPost;
			}
			CASE(GGCompareLT):
			{
				returnFunc = wr_CompareLT;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareEQPost;
			}

			CASE(LLCompareGT): { returnFunc = wr_CompareGT; goto returnCompareEQ; }
			CASE(LLCompareLT): { returnFunc = wr_CompareLT; goto returnCompareEQ; }
			CASE(LLCompareEQ):
			{
				returnFunc = wr_CompareEQ;
returnCompareEQ:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
returnCompareEQPost:
				stackTop->i = (int)returnFunc[(register0->type<<2)|register1->type]( register0, register1 );
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}
			CASE(LLCompareNE):
			{
				returnFunc = wr_CompareEQ;
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
returnCompareNEPost:
				stackTop->i = (int)!returnFunc[(register0->type<<2)|register1->type]( register0, register1 );
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}
			
			CASE(GSCompareEQ):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareEQ):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareNE):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareNE):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareEQ;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(GSCompareGT):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareGT):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareLT):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncPostLoad;
			}
			
			CASE(LSCompareLT):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncPostLoad;
			}
			
			CASE(GSCompareGE):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareGE):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareLT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(GSCompareLE):
			{
				register0 = globalSpace + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncInvertedPostLoad;
			}
			
			CASE(LSCompareLE):
			{
				register0 = frameBase + *pc++;
				returnFunc = wr_CompareGT;
				goto returnFuncInvertedPostLoad;
			}


			CASE(GSCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareGInverted; }
			CASE(GSCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareGInverted; }
			CASE(GSCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareGInverted:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}
			
			CASE(GSCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareGNormal; }
			CASE(GSCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareGNormal; }
			CASE(GSCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareGNormal:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}
			
			CASE(LSCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareLInverted; }
			CASE(LSCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareLInverted; }
			CASE(LSCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareLInverted:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}
			
			CASE(LSCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareLNormal; }
			CASE(LSCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareLNormal; }
			CASE(LSCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareLNormal:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}
			
			CASE(GSCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareG8Inverted; }
			CASE(GSCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareG8Inverted; }
			CASE(GSCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareG8Inverted:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(GSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareG8Normal; }
			CASE(GSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareG8Normal; }
			CASE(GSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareG8Normal:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(LSCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareL8Inverted; }
			CASE(LSCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareL8Inverted; }
			CASE(LSCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareL8Inverted:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(LSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareL8Normal; }
			CASE(LSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareL8Normal; }
			CASE(LSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareL8Normal:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}
			
			CASE(LLCompareEQBZ):
			{
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				pc += wr_CompareEQ[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}
			CASE(LLCompareNEBZ): { returnFunc = wr_CompareEQ; goto CompareLL; }
			CASE(LLCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareLL; }
			CASE(LLCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareLL:
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}

			CASE(LLCompareEQBZ8):
			{
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				pc += wr_CompareEQ[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}
			CASE(LLCompareNEBZ8): { returnFunc = wr_CompareEQ; goto CompareLL8; }
			CASE(LLCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareLL8; }
			CASE(LLCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareLL8:
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(GGCompareEQBZ):
			{
				register0 = globalSpace + *pc++;
				register1 = globalSpace + *pc++;
				pc += wr_CompareEQ[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(GGCompareNEBZ): { returnFunc = wr_CompareEQ; goto CompareGG; }
			CASE(GGCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareGG; }
			CASE(GGCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareGG:
				register0 = globalSpace + *pc++;
				register1 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
				CONTINUE;
			}
			
			CASE(GGCompareEQBZ8):
			{
				register0 = globalSpace + *pc++;
				register1 = globalSpace + *pc++;
				pc += wr_CompareEQ[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}
			CASE(GGCompareNEBZ8): { returnFunc = wr_CompareEQ; goto CompareGG8; }
			CASE(GGCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareGG8; }
			CASE(GGCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareGG8:
				register0 = globalSpace + *pc++;
				register1 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ?  (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(PostIncrement): { register0 = stackTop - 1; wr_postinc[ register0->type ]( register0, register0 ); CONTINUE; }
			CASE(PostDecrement): { register0 = stackTop - 1; wr_postdec[ register0->type ]( register0, register0 ); CONTINUE; }
			CASE(PreIncrement): { register0 = stackTop - 1; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrement): { register0 = stackTop - 1; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreIncrementAndPop): { register0 = --stackTop; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrementAndPop): { register0 = --stackTop; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncGlobal): { register0 = globalSpace + *pc++; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecGlobal): { register0 = globalSpace + *pc++; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncLocal): { register0 = frameBase + *pc++; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecLocal): { register0 = frameBase + *pc++; wr_predec[ register0->type ]( register0 ); CONTINUE; }

			CASE(LogicalNot):
			{
				register0 = stackTop - 1;
				register0->i = wr_LogicalNot[ register0->type ]( register0 );
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(Negate):
			{
				register0 = stackTop - 1;
				wr_negate[ register0->type ]( register0 );
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
				register0 = --stackTop;
				register1 = stackTop - 1;
				voidFunc[(register0->type<<2)|register1->type]( register0, register1 );
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
				register0 = --stackTop;
				register1 = --stackTop;
				voidFunc[(register0->type<<2)|register1->type]( register0, register1 );
				
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreGlobal) : { targetFunc = wr_AdditionBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinarySubtractionAndStoreGlobal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryMultiplicationAndStoreGlobal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryDivisionAndStoreGlobal):
			{
				targetFunc = wr_DivideBinary;

targetFuncStoreGlobalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				targetFunc[(register0->type<<2)|register1->type]( register0, register1, globalSpace + *pc++ );
				CONTINUE;
			}

			CASE(BinaryAdditionAndStoreLocal): { targetFunc = wr_AdditionBinary; goto targetFuncStoreLocalOp; }
			CASE(BinarySubtractionAndStoreLocal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryMultiplicationAndStoreLocal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryDivisionAndStoreLocal):
			{
				targetFunc = wr_DivideBinary;

targetFuncStoreLocalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				targetFunc[(register0->type<<2)|register1->type]( register0, register1, frameBase + *pc++ );
				CONTINUE;
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
				register0 = stackTop - 1;
				wr_index[(WR_INT<<4)|register0->type]( context, stackTop, register0, register0 );
				CONTINUE;
			}

			
			CASE(IndexLocalLiteral16):
			{
				register0 = frameBase + *pc++;
				(++stackTop)->i = (int32_t)(int16_t)((((int16_t)*(pc)) << 8) | ((int16_t)*(pc+1)));
				pc += 2;
				goto indexTempLiteralPostLoad;
			}
			
			CASE(IndexLocalLiteral8):
			{
				register0 = frameBase + *pc++;
indexTempLiteral:
				(++stackTop)->i = *pc++;
				stackTop->p2 = INIT_AS_INT;
indexTempLiteralPostLoad:
				wr_index[(WR_INT<<4)|register0->type]( context, stackTop, register0, stackTop - 1 );
				CONTINUE;
			}
			
			CASE(IndexGlobalLiteral16):
			{
				register0 = globalSpace + *pc++;
				(++stackTop)->i = (int32_t)(int16_t)((((int16_t)*(pc)) << 8) | ((int16_t)*(pc+1)));
				pc += 2;
				goto indexTempLiteralPostLoad;
			}

			CASE(IndexGlobalLiteral8):
			{
				register0 = globalSpace + *pc++;
				goto indexTempLiteral;
			}
			
			CASE(AssignToGlobalAndPop):
			{
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				wr_assign[(register0->type<<2)|register1->type]( register0, register1 );
				CONTINUE;
			}

			CASE(AssignToLocalAndPop):
			{
				register0 = frameBase + *pc++;
				register1 = --stackTop;

doAssignToLocalAndPop:
				wr_assign[(register0->type<<2)|register1->type]( register0, register1 );
				CONTINUE;
			}

			CASE(AssignToArrayAndPop):
			{
				stackTop->p2 = INIT_AS_INT; // index
				stackTop->i = (int32_t)(uint16_t)((((uint16_t)*(pc)) << 8) | ((uint16_t)*(pc+1)));
				pc += 2;
				register1 = stackTop - 1; // value
				register0 = stackTop - 2; // array
				if ( register0->xtype == WR_EX_REFARRAY )
				{
					register0 = register0->r;
				}

				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, stackTop + 1 );
				register0 = stackTop-- + 1;

				goto doAssignToLocalAndPop;
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
			
			CASE(BLA):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalAND[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(BLA8):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalAND[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(BLO):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(BLO8):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(GGBinaryMultiplication):
			{
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryMultiplication):
			{
				register1 = globalSpace + *pc++;
				register0 = frameBase + *pc++;
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryMultiplication):
			{
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinaryAddition):
			{
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryAddition):
			{
				register1 = globalSpace + *pc++;
				register0 = frameBase + *pc++;
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryAddition):
			{
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinarySubtraction):
			{
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinarySubtraction):
			{
				register1 = globalSpace + *pc++;
				register0 = frameBase + *pc++;
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinarySubtraction):
			{
				register1 = frameBase + *pc++;
				register0 = globalSpace + *pc++;
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinarySubtraction):
			{
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GGBinaryDivision):
			{
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryDivision):
			{
				register1 = globalSpace + *pc++;
				register0 = frameBase + *pc++;
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinaryDivision):
			{
				register1 = frameBase + *pc++;
				register0 = globalSpace + *pc++;
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryDivision):
			{
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
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
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1));
				CONTINUE;
			}

			CASE(CompareBNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int32_t)(int16_t)((((int16_t)*pc)<<8) + *(pc+1)) : 2;
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
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : *pc;
				CONTINUE;
			}

			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? *pc : 2;
				CONTINUE;
			}
			
			CASE(LiteralInt8ToGlobal):
			{
				register0 = globalSpace + *pc++;
				register0->i = (int32_t)(int8_t)*pc++;
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToGlobal):
			{
				register0 = globalSpace + *pc++;
				register0->i = (int32_t)(int16_t)((((int16_t) * (pc)) << 8) | ((int16_t) * (pc + 1)));
				register0->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralInt32ToLocal):
			{
				register0 = frameBase + *pc++;
				register0->p2 = INIT_AS_INT;
				goto load32ToTemp;
			}
			
			CASE(LiteralInt8ToLocal):
			{
				register0 = frameBase + *pc++;
				register0->i = (int32_t)(int8_t)*pc++;
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToLocal):
			{
				register0 = frameBase + *pc++;
				register0->i = (int32_t)(int16_t)((((int16_t) * (pc)) << 8) | ((int16_t) * (pc + 1)));
				register0->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralFloatToGlobal):
			{
				register0 = globalSpace + *pc++;
				register0->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}
			
			CASE(LiteralFloatToLocal):
			{
				register0 = frameBase + *pc++;
				register0->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralInt32ToGlobal):
			{
				register0 = globalSpace + *pc++;
				register0->p2 = INIT_AS_INT;
load32ToTemp:
				register0->i = (((int32_t)*pc) << 24)
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
WRContainerData::~WRContainerData()
{
	while( m_head )
	{
		UDNode* next = m_head->next;

		if ( IS_EXARRAY_TYPE(m_head->val->xtype) )
		{
			delete m_head->val->va;
		}

		delete m_head->val;
		delete m_head;
		m_head = next;
	}

	while (m_nodeOnlyHead)
	{
		UDNode* next = m_nodeOnlyHead->next;
		delete m_nodeOnlyHead;
		m_nodeOnlyHead = next;
	}
}

//------------------------------------------------------------------------------
void wr_makeUserData( WRValue* val, int sizeHint )
{
	val->p2 = INIT_AS_USR;
	val->u = new WRContainerData( sizeHint );
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
	val->va = new WRGCObject( len, SV_CHAR, data );
}

//------------------------------------------------------------------------------
void wr_addUserIntArray( WRValue* userData, const char* name, const int* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->p2 = INIT_AS_ARRAY;
	val->va = new WRGCObject( len, SV_INT, data );
}

//------------------------------------------------------------------------------
void wr_addUserFloatArray( WRValue* userData, const char* name, const float* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->p2 = INIT_AS_ARRAY;
	val->va = new WRGCObject( len, SV_FLOAT, data );
}

//------------------------------------------------------------------------------
void WRValue::free()
{
	if ( xtype == WR_EX_USR )
	{
		delete u;
	}
}

