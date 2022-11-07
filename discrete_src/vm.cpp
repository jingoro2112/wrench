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
WRState::WRState( int EntriesInStack ) : c_functionRegistry( 0, SV_VOID_HASH_TABLE )
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
		wr_destroyContext( this, contextList );
	}

	delete[] stack;
}

//------------------------------------------------------------------------------
void WRContext::mark( WRValue* s )
{
	//printf("marking %p->%p: [%d:%p] 0x%08X\n", s, s->r, s->i, s->p, s->type);

	if ( IS_REFARRAY(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		if ( !s->r->va->m_skipGC )
		{
			mark( s->r );
		}
		return;
	}

	if ( s->va->m_skipGC )
	{
		return;
	}

	assert( s->xtype != WR_EX_RAW_ARRAY );

	WRGCObject* sva = s->va;

	
	if ( IS_SVA_VALUE_TYPE(sva) )
	{
		// this is an array of values, check them for array-ness too

		WRValue* top = sva->m_Vdata + sva->m_size;
		for( WRValue* V = sva->m_Vdata; V<top; ++V )
		{
			if ( IS_EXARRAY_TYPE(V->xtype) && !(V->va->m_skipGC) && !(V->va->m_size & 0x40000000) )
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

	if ( gcPauseCount )
	{
		--gcPauseCount;
		return;
	}

	// mark stack
	for( WRValue* s=w->stack; s<stackTop; ++s)
	{
		// an array in the chain?
		if ( IS_EXARRAY_TYPE(s->xtype) && !(s->va->m_skipGC) )
		{
			mark( s );
		}
	}

	WRValue* globalSpace = (WRValue *)(this + 1);
	
	// mark context's global
	for( int i=0; i<globals; ++i, ++globalSpace )
	{
		if ( IS_EXARRAY_TYPE(globalSpace->xtype) && !(globalSpace->va->m_skipGC) )
		{
			mark( globalSpace );
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
WRError wr_getLastError( WRState* w )
{
	return (WRError)w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block )
{
	int needed = block[1] * sizeof(WRValue) + sizeof(WRContext);
	
	WRContext* C = (WRContext *)malloc( needed );
	
	memset((char*)C, 0, needed);

	C->globals = block[1];
	C->w = w;

	if ( block[0] )
	{
		C->localFunctions = new WRFunction[ *block ];
	}
	
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


			delete[] context->localFunctions;

			while ( context->svAllocated )
			{
				WRGCObject* next = context->svAllocated->m_next;
				delete context->svAllocated;
				context->svAllocated = next;
			}

			free( context );
			
			break;
		}
		prev = c;
	}
}

//------------------------------------------------------------------------------
int wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	uint32_t hash = wr_hashStr( name );

	WRValue* V = w->c_functionRegistry.getAsRawValueHashTable(hash);
	V->usr = usr;
	V->ccb = function;
	
	return 0;
}

//------------------------------------------------------------------------------
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function )
{
	w->c_functionRegistry.getAsRawValueHashTable(wr_hashStr(signature) )->lcb = function;
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

	return IS_REFARRAY(xtype) ? arrayValueAsInt() : 0;
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
	
	return IS_REFARRAY(xtype) ? (float)arrayValueAsInt() : 0;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, size_t len ) const
{
	if ( xtype )
	{
		switch( xtype )
		{
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

			case WR_EX_RAW_ARRAY:
			case WR_EX_STRUCT:
			case WR_EX_NONE:
			{
				string[0] = 0;
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

			case WR_REF:
			{
				return r->asString( string, len );
			}
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
	WRValue* cF = 0;
	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_run_must_be_called_by_itself_first;
		}

		cF = w->c_functionRegistry.getAsRawValueHashTable( hash ^ context->hashOffset );
		if ( !cF->wrf )
		{
			return w->err = WR_ERR_wrench_function_not_found;
		}
	}

	return wr_callFunction( w, context, cF ? cF->wrf : 0, argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_returnValueFromLastCall( WRState* w )
{
	return w->stack; // this is where it ends up
}

//------------------------------------------------------------------------------
WRFunction* wr_getFunction( WRState* w, WRContext* context, const char* functionName )
{
	return w->c_functionRegistry.getAsRawValueHashTable( wr_hashStr(functionName) ^ context->hashOffset )->wrf;
}

//------------------------------------------------------------------------------
inline bool wr_getNextValue( WRValue* iterator, WRValue* value, WRValue* key )
{
	if ( !IS_ITERATOR(iterator->xtype) )
	{
		return false;
	}

	uint32_t element = ARRAY_ELEMENT_FROM_P2( iterator->p2 );

	if ( iterator->r->va->m_type == SV_HASH_TABLE )
	{
		for( ; element<iterator->r->va->m_mod && !iterator->r->va->m_hashTable[element]; ++element );

		if ( element >= iterator->r->va->m_mod )
		{
			return false;
		}
		
		ARRAY_ELEMENT_TO_P2( iterator->p2, (element + 1) );
		
		element <<= 1;
		
		value->p2 = INIT_AS_REF;
		value->r = iterator->r->va->m_Vdata + element;
		if ( key )
		{
			key->p2 = INIT_AS_REF;
			key->r = iterator->r->va->m_Vdata + element + 1;
		}
	}
	else
	{
		if ( element >= iterator->r->va->m_size )
		{
			return false;
		}
		
		if ( key )
		{
			key->p2 = INIT_AS_INT;
			key->i = element;
		}
		
		value->p2 = INIT_AS_REF;
		value->r = iterator->r->va->m_Vdata + element;

		ARRAY_ELEMENT_TO_P2( iterator->p2, ++element );
	}

	return true;
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

#ifdef WRENCH_COMPACT

static float divisionF( float a, float b ) { return a / b; }
static float addF( float a, float b ) { return a + b; }
static float subtractionF( float a, float b ) { return a - b; }
static float multiplicationF( float a, float b ) { return a * b; }

static int divisionI( int a, int b ) { return a / b; }
static int addI( int a, int b ) { return a + b; }
static int subtractionI( int a, int b ) { return a - b; }
static int multiplicationI( int a, int b ) { return a * b; }

static int rightShiftI( int a, int b ) { return a >> b; }
static int leftShiftI( int a, int b ) { return a << b; }
static int modI( int a, int b ) { return a % b; }
static int orI( int a, int b ) { return a | b; }
static int xorI( int a, int b ) { return a ^ b; }
static int andI( int a, int b ) { return a & b; }

static bool CompareGTI( int a, int b ) { return a > b; }
static bool CompareLTI( int a, int b ) { return a < b; }
static bool CompareANDI( int a, int b ) { return a && b; }
static bool CompareORI( int a, int b ) { return a || b; }

static bool CompareLTF( float a, float b ) { return a < b; }
static bool CompareGTF( float a, float b ) { return a > b; }

bool CompareEQI( int a, int b ) { return a == b; }
bool CompareEQF( float a, float b ) { return a == b; }

static bool CompareBlankF( float a, float b ) { return false; }
static float blankF( float a, float b ) { return 0; }

int32_t READ_32_FROM_PC( const unsigned char* P )
{
	return ( (((int32_t)*(P)) << 24) | (((int32_t)*((P)+1)) << 16) | (((int32_t)*((P)+2)) << 8) | ((int32_t)*((P)+3)) );
}

int16_t READ_16_FROM_PC( const unsigned char* P )
{
	return ( ((int16_t)*(P)) << 8) | ((int16_t)*(P+1) );
}

#endif

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, WRFunction* function, const WRValue* argv, const int argn )
{
#ifdef WRENCH_JUMPTABLE_INTERPRETER
	const void* opcodeJumptable[] =
	{
		&&RegisterFunction,

		&&LiteralInt32,
		&&LiteralZero,
		&&LiteralFloat,
		&&LiteralString,

		&&CallFunctionByHash,
		&&CallFunctionByHashAndPop,
		&&CallFunctionByIndex,
		&&PushIndexFunctionReturnValue,

		&&CallLibFunction,

		&&NewObjectTable,
		&&AssignToObjectTableByOffset,

		&&AssignToHashTableAndPop,
		&&RemoveFromHashTable,
		&&HashEntryExists,

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

		&&GGCompareGT,
		&&GGCompareGE,
		&&GGCompareLT,
		&&GGCompareLE,
		&&GGCompareEQ, 
		&&GGCompareNE, 

		&&LLCompareGT,
		&&LLCompareGE,
		&&LLCompareLT,
		&&LLCompareLE,
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
		&&LLCompareLEBZ,
		&&LLCompareGTBZ,
		&&LLCompareGEBZ,
		&&LLCompareEQBZ,
		&&LLCompareNEBZ,

		&&GGCompareLTBZ,
		&&GGCompareLEBZ,
		&&GGCompareGTBZ,
		&&GGCompareGEBZ,
		&&GGCompareEQBZ,
		&&GGCompareNEBZ,

		&&LLCompareLTBZ8,
		&&LLCompareLEBZ8,
		&&LLCompareGTBZ8,
		&&LLCompareGEBZ8,
		&&LLCompareEQBZ8,
		&&LLCompareNEBZ8,

		&&GGCompareLTBZ8,
		&&GGCompareLEBZ8,
		&&GGCompareGTBZ8,
		&&GGCompareGEBZ8,
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

		&&LogicalNot, //X
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

		&&GC_Command,

		&&GPushIterator,
		&&LPushIterator,
		&&GGNextKeyValueOrJump,
		&&GLNextKeyValueOrJump,
		&&LGNextKeyValueOrJump,
		&&LLNextKeyValueOrJump,
		&&GNextValueOrJump,
		&&LNextValueOrJump,

		&&Switch,
		&&SwitchLinear,

		&&GlobalStop,
	};
#endif

	const unsigned char* pc;

	union
	{
		unsigned char findex;
		WRValue* register0 = 0;
		const unsigned char *hashLoc;
		uint32_t hashLocInt;
	};
	
	union
	{
		WRValue* register1;
		uint16_t switchMod;
	};
	WRValue* frameBase = 0;
	WRValue* stackTop = w->stack;
	WRValue* globalSpace = (WRValue *)(context + 1);//->globalSpace;

	union
	{
		// never used at the same time..save RAM!
		WRValue* register2;
		int args;
		uint32_t hash;
		WRVoidFunc* voidFunc;
		WRReturnFunc* returnFunc;
		WRTargetFunc* targetFunc;
	};

#ifdef WRENCH_COMPACT
	union
	{
		WRFuncIntCall intCall;
		WRCompareFuncIntCall boolIntCall;
	};
	union
	{
		WRFuncFloatCall floatCall;
		WRCompareFuncFloatCall boolFloatCall;
	};
#endif

	w->err = WR_ERR_None;

	if ( function )
	{
		stackTop->p = 0;
		(stackTop++)->p2 = INIT_AS_INT;

		for( args = 0; args < argn; ++args )
		{
			*stackTop++ = argv[args];
		}
		
		pc = context->stopLocation;
		goto callFunction;
	}

	pc = context->bottom + 2;

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
				hash = (stackTop -= 3)->i;
				findex = hash;

				context->localFunctions[findex].arguments = (unsigned char)(hash>>8);
				context->localFunctions[findex].frameSpaceNeeded = (unsigned char)(hash>>16);
				context->localFunctions[findex].hash = (stackTop + 1)->i;
				context->localFunctions[findex].offset = context->bottom + (stackTop + 2)->i;

				context->localFunctions[findex].frameBaseAdjustment = 1
																	  + context->localFunctions[ findex ].frameSpaceNeeded
																	  + context->localFunctions[ findex ].arguments;
				w->c_functionRegistry.getAsRawValueHashTable(context->localFunctions[findex].hash ^ context->hashOffset)->wrf = context->localFunctions + findex;

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
				stackTop->p = 0;
				(stackTop++)->p2 = INIT_AS_INT;
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
				uint16_t len = (uint16_t)READ_16_FROM_PC(pc);
				pc += 2;
				
				context->gc( stackTop  );
				stackTop->p2 = INIT_AS_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR, false );
				memcpy( (unsigned char *)(stackTop++)->va->m_data, pc, len );
				pc += len;
				
				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				args = *pc++;

				// initialize a return value of 'zero'
				register0 = stackTop;
				register0->p = 0;
				register0->p2 = INIT_AS_INT;

				if ( (register1 = w->c_functionRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb )
				{
					register1->ccb( w, stackTop - args, args, *stackTop, register1->usr );
				}

				// DO care about return value, which will be at the top
				// of the stack

				if ( args )
				{
					stackTop -= (args - 1);
					*(stackTop - 1) = *register0;
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

				if ( (register1 = w->c_functionRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb )
				{
					register1->ccb( w, stackTop - args, args, *stackTop, register1->usr );
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
				stackTop->p2 = INIT_AS_INT;
				stackTop->p = 0;

				args = *pc++; // which have already been pushed

				if ( (register1 = w->c_functionRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb )
				{
					register1->lcb( stackTop, args, context );
				}
				pc += 4;

				stackTop -= --args;
				*(stackTop - 1) = *(stackTop + args);

				CONTINUE;
			}

			CASE(NewObjectTable):
			{
				const unsigned char* table = context->bottom + READ_16_FROM_PC(pc);
				pc += 2;

				if ( table > context->bottom )
				{
					// if unit was called with no arguments from global
					// level there are not "free" stack entries to
					// gnab, so create it here, but preserve the
					// first value

					// NOTE: we are guaranteed to have at least one
					// value if table > context->bottom

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
					stackTop->va->m_mod = READ_16_FROM_PC(table+1);

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
					register1 = register1->va->m_Vdata + *pc;
					wr_assign[register1->type<<2|register0->type]( register1, register0 );
				}

				++pc;

				CONTINUE;
			}

			CASE(AssignToHashTableAndPop):
			{
				register0 = --stackTop; // value
				register1 = --stackTop; // index
				
				wr_assignToHashTable( context, register1, register0, stackTop - 1 );
				CONTINUE;
			}
			
			CASE(RemoveFromHashTable):
			{
				hash = (--stackTop)->getHash();
				if ( (register0 = stackTop - 1)->type == WR_REF )
				{
					register0 = register0->r;
				}

				if ( register0->xtype == WR_EX_HASH_TABLE )
				{
					hash %= register0->va->m_mod;
					register0->va->m_hashTable[ hash ] = 0;
					hash <<= 1;
					memset( (char*)(register0->va->m_Vdata + hash), 0, sizeof(WRValue)*2 );
				}
				
				CONTINUE;	
			}

			CASE(HashEntryExists):
			{
				register0 = --stackTop;
				register1 = (stackTop - 1);
				register2 = (register1->type == WR_REF) ? register1->r : register1;
				register1->i = ((register2->xtype == WR_EX_HASH_TABLE
								 && register2->va->m_hashTable[register0->getHash() % register2->va->m_mod])) ? 1 : 0;
				register1->p2 = INIT_AS_INT;
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

			CASE(GlobalStop):
			{
				register0 = w->stack - 1;
				++pc;
			}
			CASE(Stop):
			{
				*w->stack = *(register0 + 1);
				context->stopLocation = pc - 1;
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
#ifdef WRENCH_COMPACT
				wr_countOfArrayElement( register0, register0 );
#else
				while( register0->type == WR_REF )
				{
					register0 = register0->r;
				}
				wr_countOfArrayElement( register0, stackTop - 1 );
#endif
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
												 READ_32_FROM_PC(pc) );
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

			CASE(BitwiseNOT):
			{
				register0 = stackTop - 1;
				register0->ui = wr_bitwiseNot[ register0->type ]( register0 );
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(RelativeJump):
			{
				pc += READ_16_FROM_PC(pc);
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
				pc += wr_LogicalNot[register0->type](register0) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}

			CASE(BZ8):
			{
				register0 = --stackTop;
				pc += wr_LogicalNot[register0->type](register0) ? (int8_t)*pc : 2;
				CONTINUE;
			}

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
			
			CASE(IndexLiteral16):
			{
				stackTop->i = READ_16_FROM_PC(pc);
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
				(++stackTop)->i = READ_16_FROM_PC(pc);
				pc += 2;
				goto indexTempLiteralPostLoad;
			}
			
			CASE(IndexLocalLiteral8):
			{
				register0 = frameBase + *pc++;
indexTempLiteral:
				(++stackTop)->i = *pc++;
indexTempLiteralPostLoad:
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT<<4)|register0->type]( context, stackTop, register0, stackTop - 1 );
				CONTINUE;
			}
			
			CASE(IndexGlobalLiteral16):
			{
				register0 = globalSpace + *pc++;
				(++stackTop)->i = READ_16_FROM_PC(pc);
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
#ifdef WRENCH_COMPACT
				goto doAssignToLocalAndPopPreLoad;
#else
				register1 = --stackTop;
				wr_assign[(register0->type<<2)|register1->type]( register0, register1 );
				CONTINUE;
#endif
			}

			CASE(AssignToLocalAndPop):
			{
				register0 = frameBase + *pc++;

#ifdef WRENCH_COMPACT
doAssignToLocalAndPopPreLoad:
#endif
				register1 = --stackTop;

doAssignToLocalAndPop:
				wr_assign[(register0->type<<2)|register1->type]( register0, register1 );
				CONTINUE;
			}

			CASE(AssignToArrayAndPop):
			{
				stackTop->p2 = INIT_AS_INT; // index
				stackTop->i = READ_16_FROM_PC(pc);
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
				stackTop->i = READ_16_FROM_PC(pc);
				pc += 2;
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(PostIncrement):
			{
				register0 = stackTop - 1;
				wr_postinc[ register0->type ]( register0, register0 );
				CONTINUE;
			}

			CASE(PostDecrement):
			{
				register0 = stackTop - 1;
				wr_postdec[ register0->type ]( register0, register0 );
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
				register0->i = READ_16_FROM_PC(pc);
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
				register0->i = READ_16_FROM_PC(pc);
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
				register0->i = READ_32_FROM_PC(pc);
				pc += 4;
				CONTINUE;
			}

			CASE(GC_Command):
			{
				context->gcPauseCount = READ_16_FROM_PC(pc);
				pc+=2;
				CONTINUE;
			}
			
			CASE(GPushIterator):
			{
				register0 = globalSpace + *pc++;
				wr_pushIterator[register0->type]( register0, globalSpace + *pc++ );
				CONTINUE;
			}

			CASE(LPushIterator):
			{
				register0 = frameBase + *pc++;
				wr_pushIterator[register0->type]( register0, globalSpace + *pc++ );
				CONTINUE;
			}

			CASE(GGNextKeyValueOrJump): { register1 = globalSpace + *pc++; register0 = globalSpace + *pc++; goto NextIterator; }
			CASE(GLNextKeyValueOrJump):	{ register1 = globalSpace + *pc++; register0 = frameBase + *pc++; goto NextIterator; }
			CASE(LGNextKeyValueOrJump): { register1 = frameBase + *pc++; register0 = globalSpace + *pc++; goto NextIterator; }
			CASE(LLNextKeyValueOrJump): { register1 = frameBase + *pc++; register0 = frameBase + *pc++; goto NextIterator; }
			CASE(GNextValueOrJump): { register0 = globalSpace + *pc++; register1 = 0; goto NextIterator; }
			CASE(LNextValueOrJump):
			{
				register0 = frameBase + *pc++;
				register1 = 0;
NextIterator:
				register2 = globalSpace + *pc++;
				pc += wr_getNextValue( register2, register0, register1) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}
			
			CASE(Switch):
			{
				hash = (--stackTop)->getHash(); // hash has been loaded
				hashLoc = pc + 4 + 6*(hash % (uint16_t)READ_16_FROM_PC(pc)); // jump into the table

				if ( (uint32_t)READ_32_FROM_PC(hashLoc) == hash ) // hash match exactly?
				{
					hashLoc += 4; // yup, point hashLoc to jump vector
				}
				else
				{
					hashLoc = pc + 2; // nope, point it at default vector
				}
				
				pc += (uint16_t)READ_16_FROM_PC(hashLoc);
				CONTINUE;
			}

			CASE(SwitchLinear):
			{
				hashLocInt = (--stackTop)->getHash(); // the "hashes" were all 0<=h<256
				if ( hashLocInt < *pc++ ) // doesn't mean you switched() on one though ;) check against default top
				{
					hashLoc = pc + (hashLocInt<<1) + 2; // jump to vector
					pc += READ_16_FROM_PC(hashLoc); // and read it
				}
				else
				{
					pc += READ_16_FROM_PC(pc); // already at default vector
				}
				CONTINUE;
			}

#ifdef WRENCH_COMPACT

			CASE(BinaryRightShiftSkipLoad): { intCall = rightShiftI; goto targetFuncOpSkipLoad; }
			CASE(BinaryLeftShiftSkipLoad): { intCall = leftShiftI; goto targetFuncOpSkipLoad; }
			CASE(BinaryAndSkipLoad): { intCall = andI; goto targetFuncOpSkipLoad; }
			CASE(BinaryOrSkipLoad): { intCall = orI; goto targetFuncOpSkipLoad; }
			CASE(BinaryXORSkipLoad): { intCall = xorI; goto targetFuncOpSkipLoad; }
			CASE(BinaryModSkipLoad):
			{
				intCall = modI;
targetFuncOpSkipLoad:
				register2 = stackTop++;
				floatCall = blankF;
targetFuncOpSkipLoadAndReg2:
				wr_funcBinary[(register1->type<<2)|register0->type]( register1,
																	 register0,
																	 register2,
																	 intCall,
																	 floatCall );
				CONTINUE;
			}
			
			CASE(BinaryMultiplication): { floatCall = multiplicationF; intCall = multiplicationI; goto targetFuncOp; }
			CASE(BinarySubtraction): { floatCall = subtractionF; intCall = subtractionI; goto targetFuncOp; } 
			CASE(BinaryDivision): { floatCall = divisionF; intCall = divisionI; goto targetFuncOp; }
			CASE(BinaryRightShift): { floatCall = blankF; intCall = rightShiftI; goto targetFuncOp; }
			CASE(BinaryLeftShift): { floatCall = blankF; intCall = leftShiftI; goto targetFuncOp; }
			CASE(BinaryMod): { floatCall = blankF; intCall = modI; goto targetFuncOp; }
			CASE(BinaryOr): { floatCall = blankF; intCall = orI; goto targetFuncOp; }
			CASE(BinaryXOR): { floatCall = blankF; intCall = xorI; goto targetFuncOp; }
			CASE(BinaryAnd): { floatCall = blankF; intCall = andI; goto targetFuncOp; }
			CASE(BinaryAddition):
			{
				floatCall = addF;
				intCall = addI;
targetFuncOp:
				register1 = --stackTop;
				register0 = stackTop - 1;
				register2 = register0;
				goto targetFuncOpSkipLoadAndReg2;
			}
			
			
			CASE(SubtractAssign): { floatCall = subtractionF; intCall = subtractionI; goto binaryTableOp; }
			CASE(AddAssign): { floatCall = addF; intCall = addI; goto binaryTableOp; }
			CASE(MultiplyAssign): { floatCall = multiplicationF; intCall = multiplicationI; goto binaryTableOp; }
			CASE(DivideAssign): { floatCall = divisionF; intCall = divisionI; goto binaryTableOp; }
			CASE(ModAssign): { intCall = modI; goto binaryTableOpBlankF; }
			CASE(ORAssign): { intCall = orI; goto binaryTableOpBlankF; }
			CASE(ANDAssign): { intCall = andI; goto binaryTableOpBlankF; }
			CASE(XORAssign): { intCall = xorI; goto binaryTableOpBlankF; }
			CASE(RightShiftAssign): { intCall = rightShiftI; goto binaryTableOpBlankF; }
			CASE(LeftShiftAssign):
			{
				intCall = leftShiftI;
binaryTableOpBlankF:	
				floatCall = blankF; 
binaryTableOp:	
				register0 = --stackTop;
				register1 = stackTop - 1;
				goto binaryTableOpAndPopCall;
			}
			
			
			CASE(Assign):
			{
				register0 = --stackTop;
				register1 = stackTop - 1;
				goto assignAndPopEx;
			}
			
			CASE(SubtractAssignAndPop): { floatCall = subtractionF; intCall = subtractionI; goto binaryTableOpAndPop; }
			CASE(AddAssignAndPop): { floatCall = addF; intCall = addI; goto binaryTableOpAndPop; }
			CASE(MultiplyAssignAndPop): { floatCall = multiplicationF; intCall = multiplicationI; goto binaryTableOpAndPop; }
			CASE(DivideAssignAndPop): { floatCall = divisionF; intCall = divisionI; goto binaryTableOpAndPop; }
			CASE(ModAssignAndPop): { intCall = modI; goto binaryTableOpAndPopBlankF; }
			CASE(ORAssignAndPop): { intCall = orI; goto binaryTableOpAndPopBlankF; }
			CASE(ANDAssignAndPop): { intCall = andI; goto binaryTableOpAndPopBlankF; }
			CASE(XORAssignAndPop): { intCall = xorI; goto binaryTableOpAndPopBlankF; }
			CASE(RightShiftAssignAndPop): { intCall = rightShiftI; goto binaryTableOpAndPopBlankF; }
			CASE(LeftShiftAssignAndPop): 
			{
				intCall = leftShiftI;
				
binaryTableOpAndPopBlankF:
				floatCall = blankF;
binaryTableOpAndPop:
				register0 = --stackTop;
				register1 = --stackTop;
binaryTableOpAndPopCall:
				wr_FuncAssign[(register0->type<<2)|register1->type]( register0, register1, intCall, floatCall );
				CONTINUE;
			}
			
			CASE(AssignAndPop):
			{
				register0 = --stackTop;
				register1 = --stackTop;
assignAndPopEx:
				wr_assign[(register0->type<<2)|register1->type]( register0, register1 );
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreGlobal) : { floatCall = addF; intCall = addI; goto targetFuncStoreGlobalOp; }
			CASE(BinarySubtractionAndStoreGlobal): { floatCall = subtractionF; intCall = subtractionI; goto targetFuncStoreGlobalOp; }
			CASE(BinaryMultiplicationAndStoreGlobal): { floatCall = multiplicationF; intCall = multiplicationI; goto targetFuncStoreGlobalOp; }
			CASE(BinaryDivisionAndStoreGlobal):
			{
				floatCall = divisionF;
				intCall = divisionI;
				
targetFuncStoreGlobalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				wr_funcBinary[(register0->type<<2)|register1->type]( register0, register1, globalSpace + *pc++, intCall, floatCall );
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreLocal): { floatCall = addF; intCall = addI; goto targetFuncStoreLocalOp; }
			CASE(BinarySubtractionAndStoreLocal): { floatCall = subtractionF; intCall = subtractionI; goto targetFuncStoreLocalOp; }
			CASE(BinaryMultiplicationAndStoreLocal): { floatCall = multiplicationF; intCall = multiplicationI; goto targetFuncStoreLocalOp; }
			CASE(BinaryDivisionAndStoreLocal):
			{
				floatCall = divisionF;
				intCall = divisionI;
				
targetFuncStoreLocalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				wr_funcBinary[(register0->type<<2)|register1->type]( register0, register1, frameBase + *pc++, intCall, floatCall );
				CONTINUE;
			}
			
			CASE(PreIncrement):
			{
				register0 = stackTop - 1;
compactPreIncrement:
				intCall = addI;
				floatCall = addF;

compactIncrementWork:
				stackTop->i = 1;
				stackTop->p2 = INIT_AS_INT;
				register1 = stackTop;
				goto binaryTableOpAndPopCall;
			}

			CASE(PreDecrement):
			{
				register0 = stackTop - 1;
compactPreDecrement:
				intCall = subtractionI;
				floatCall = subtractionF;
				goto compactIncrementWork;
			}
			CASE(PreIncrementAndPop):
			{
				register0 = --stackTop;
				goto compactPreIncrement;
			}
			
			CASE(PreDecrementAndPop):
			{
				register0 = --stackTop;
				goto compactPreDecrement;
			}

			CASE(IncGlobal):
			{
				register0 = globalSpace + *pc++;
				goto compactPreIncrement;
			}
			
			CASE(DecGlobal):
			{
				register0 = globalSpace + *pc++;
				goto compactPreDecrement;
			}
			CASE(IncLocal):
			{
				register0 = frameBase + *pc++;
				goto compactPreIncrement;
			}

			CASE(DecLocal):
			{
				register0 = frameBase + *pc++;
				goto compactPreDecrement;
			}
													
			CASE(BLA):
			{
				boolIntCall = CompareANDI;
				boolFloatCall = CompareBlankF;
compactBLAPreLoad:
				register0 = --stackTop;
				register1 = --stackTop;
compactBLA:
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}

			CASE(BLA8):
			{
				boolIntCall = CompareANDI;
				boolFloatCall = CompareBlankF;
compactBLA8PreLoad:
				register0 = --stackTop;
compactBLA8PreReg1:
				register1 = --stackTop;
compactBLA8:
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			CASE(BLO):
			{
				boolIntCall = CompareORI;
				boolFloatCall = CompareBlankF;
				register0 = --stackTop;
				register1 = --stackTop;
				goto compactBLA;
			}

			CASE(BLO8):
			{
				boolIntCall = CompareORI;
				boolFloatCall = CompareBlankF;
				register0 = --stackTop;
				register1 = --stackTop;
				goto compactBLA8;
			}

			CASE(GGBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactGGFunc:
				register0 = globalSpace + *pc++;
				register1 = globalSpace + *pc++;
				goto targetFuncOpSkipLoad;
			}

			CASE(GLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactGLFunc:
				register0 = globalSpace + *pc++;
				register1 = frameBase + *pc++;
				goto targetFuncOpSkipLoad;
			}

			CASE(LLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactFFFunc:
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				goto targetFuncOpSkipLoad;
			}

			CASE(GGBinaryAddition):
			{
				floatCall = addF;
				intCall = addI;
				goto CompactGGFunc;
			}

			CASE(GLBinaryAddition):
			{
				floatCall = addF;
				intCall = addI;
				goto CompactGLFunc;
			}

			CASE(LLBinaryAddition):
			{
				floatCall = addF;
				intCall = addI;
				goto CompactFFFunc;
			}

			CASE(GGBinarySubtraction):
			{
				floatCall = subtractionF;
				intCall = subtractionI;
				goto CompactGGFunc;
			}

			CASE(GLBinarySubtraction):
			{
				floatCall = subtractionF;
				intCall = subtractionI;
				goto CompactGLFunc;
			}

			CASE(LGBinarySubtraction):
			{
				floatCall = subtractionF;
				intCall = subtractionI;
CompactFGFunc:
				register0 = frameBase + *pc++;
				register1 = globalSpace + *pc++;
				goto targetFuncOpSkipLoad;
			}

			CASE(LLBinarySubtraction):
			{
				floatCall = subtractionF;
				intCall = subtractionI;
				goto CompactFFFunc;
			}

			CASE(GGBinaryDivision):
			{
				floatCall = divisionF;
				intCall = divisionI;
				goto CompactGGFunc;
			}

			CASE(GLBinaryDivision):
			{
				floatCall = divisionF;
				intCall = divisionI;
				goto CompactGLFunc;
			}

			CASE(LGBinaryDivision):
			{
				floatCall = divisionF;
				intCall = divisionI;
				goto CompactFGFunc;
			}

			CASE(LLBinaryDivision):
			{
				floatCall = divisionF;
				intCall = divisionI;
				goto CompactFFFunc;
			}

			CASE(LogicalAnd): { boolIntCall = CompareANDI; goto compactReturnFuncNormal; }
			CASE(LogicalOr): { boolIntCall = CompareORI; goto compactReturnFuncNormal; }
			CASE(CompareLE): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncInverted; }
			CASE(CompareGE): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncInverted; }
			CASE(CompareGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncNormal; }
			CASE(CompareEQ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncNormal; }
			CASE(CompareLT):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
				
compactReturnFuncNormal:
				register0 = --stackTop;
compactReturnFuncPostLoad:
				register1 = stackTop - 1;
				register1->i = (int)wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall );
				register1->p2 = INIT_AS_INT;
				CONTINUE;
			}
											  
			CASE(CompareNE):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;

compactReturnFuncInverted:
				register0 = --stackTop;
compactReturnFuncInvertedPostLoad:
				register1 = stackTop - 1;
				register1->i = (int)!wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall );
				register1->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(GSCompareEQ): { register0 = globalSpace + *pc++; boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareNE): { register0 = globalSpace + *pc++; boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncInvertedPostLoad; }
			CASE(GSCompareGT): { register0 = globalSpace + *pc++; boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareLT): { register0 = globalSpace + *pc++; boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareGE): { register0 = globalSpace + *pc++; boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncInvertedPostLoad; }
			CASE(GSCompareLE): { register0 = globalSpace + *pc++; boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncInvertedPostLoad; }
							   
			CASE(LSCompareEQ): { register0 = frameBase + *pc++; boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareNE): { register0 = frameBase + *pc++; boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncInvertedPostLoad; }
			CASE(LSCompareGT): { register0 = frameBase + *pc++; boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareLT): { register0 = frameBase + *pc++; boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareGE): { register0 = frameBase + *pc++; boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncInvertedPostLoad; }
			CASE(LSCompareLE): { register0 = frameBase + *pc++; boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncInvertedPostLoad; }

			CASE(CompareBLE): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncBInverted; }
			CASE(CompareBGE): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncBInverted; }
			CASE(CompareBGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactBLAPreLoad; }
			CASE(CompareBLT): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactBLAPreLoad; }
			CASE(CompareBEQ): 
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF; 
				goto compactBLAPreLoad;
			}

			CASE(CompareBNE):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnFuncBInverted:
				register0 = --stackTop;
compactReturnFuncBInvertedPreReg1:
				register1 = --stackTop;
compactReturnFuncBInvertedPostReg1:
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}
			
			CASE(CompareBLE8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncBInverted8; }
			CASE(CompareBGE8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncBInverted8; }
			CASE(CompareBGT8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactBLA8PreLoad; }
			CASE(CompareBLT8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactBLA8PreLoad; }
			CASE(CompareBEQ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
				goto compactBLA8PreLoad;
			}
			
			CASE(CompareBNE8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnFuncBInverted8:
				register0 = --stackTop;
compactReturnFuncBInverted8PreReg1:
				register1 = --stackTop;
compactReturnFuncBInverted8Post:
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? (int8_t)*pc : 2;
				CONTINUE;
			}

			CASE(GGCompareLE): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnCompareGGNEPost; }
			CASE(GGCompareGE): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnCompareGGNEPost; }
			CASE(GGCompareNE):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnCompareGGNEPost:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactReturnCompareNEPost;
			}

			CASE(GGCompareGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactEQCompareGGReg; }
			CASE(GGCompareEQ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactEQCompareGGReg; }
			CASE(GGCompareLT):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactEQCompareGGReg:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactReturnCompareEQPost;
			}

			CASE(LLCompareGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnCompareEQ; }
			CASE(LLCompareLT): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnCompareEQ; }
			CASE(LLCompareEQ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnCompareEQ:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
compactReturnCompareEQPost:
				stackTop->i = (int)wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall );
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LLCompareGE): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnCompareNE; }
			CASE(LLCompareLE): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnCompareNE; }
			CASE(LLCompareNE):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnCompareNE:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
compactReturnCompareNEPost:
				stackTop->i = (int)!wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall );
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}

			
			CASE(GSCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareGInverted; }
			CASE(GSCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGInverted; }
			CASE(GSCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareGInverted:
				register0 = globalSpace + *pc++;
				goto compactReturnFuncBInvertedPreReg1;
			}
			
			CASE(GSCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGNormal; }
			CASE(GSCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGNormal; }
			CASE(GSCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGNormal:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				goto compactBLA;
			}
			
			CASE(LSCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareLInverted; }
			CASE(LSCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLInverted; }
			CASE(LSCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareLInverted:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				goto compactReturnFuncBInvertedPostReg1;
			}
			
			CASE(LSCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareLNormal; }
			CASE(LSCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLNormal; }
			CASE(LSCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareLNormal:
				register0 = frameBase + *pc++;
				register1 = --stackTop;
				goto compactBLA;
			}
			
			CASE(GSCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareG8Inverted; }
			CASE(GSCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareG8Inverted; }
			CASE(GSCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareG8Inverted:
				register0 = globalSpace + *pc++;
				goto compactReturnFuncBInverted8PreReg1;
			}
			
			CASE(GSCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareG8Normal; }
			CASE(GSCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareG8Normal; }
			CASE(GSCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareG8Normal:
				register0 = globalSpace + *pc++;
				goto compactBLA8PreReg1;
			}
			
			CASE(LSCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareL8Inverted; }
			CASE(LSCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareL8Inverted; }
			CASE(LSCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareL8Inverted:
				register0 = frameBase + *pc++;
				goto compactReturnFuncBInverted8PreReg1;
			}
			
			CASE(LSCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareL8Normal; }
			CASE(LSCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareL8Normal; }
			CASE(LSCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareL8Normal:
				register0 = frameBase + *pc++;
				goto compactBLA8PreReg1;
			}

			CASE(LLCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLLInv; }
			CASE(LLCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareLLInv; }
			CASE(LLCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareLLInv:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				goto compactReturnFuncBInvertedPostReg1;
			}
			CASE(LLCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareLL; }
			CASE(LLCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLL; }
			CASE(LLCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareLL:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				goto compactBLA;
			}
			
			CASE(LLCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLLInv8; }
			CASE(LLCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareLLInv8; }
			CASE(LLCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareLLInv8:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				goto compactReturnFuncBInverted8Post;
			}
			CASE(LLCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareLL8; }
			CASE(LLCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLL8; }
			CASE(LLCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareLL8:	
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				goto compactBLA8;
			}

			CASE(GGCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareGGInv; }
			CASE(GGCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGGInv; }
			CASE(GGCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareGGInv:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactReturnFuncBInvertedPostReg1;
			}
			
			CASE(GGCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGG; }
			CASE(GGCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGG; }
			CASE(GGCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGG:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactBLA;
			}
			
			CASE(GGCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareGGInv8; }
			CASE(GGCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGGInv8; }
			CASE(GGCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareGGInv8:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactReturnFuncBInverted8Post;
			}
			CASE(GGCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGG8; }
			CASE(GGCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGG8; }
			CASE(GGCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGG8:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto compactBLA8;
			}
	
#else // ---------------------------- Non-Compact:

			CASE(PreIncrement): { register0 = stackTop - 1; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrement): { register0 = stackTop - 1; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreIncrementAndPop): { register0 = --stackTop; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrementAndPop): { register0 = --stackTop; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncGlobal): { register0 = globalSpace + *pc++; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecGlobal): { register0 = globalSpace + *pc++; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncLocal): { register0 = frameBase + *pc++; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecLocal): { register0 = frameBase + *pc++; wr_predec[ register0->type ]( register0 ); CONTINUE; }

			CASE(BLA):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalAND[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
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
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
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

			CASE(GSCompareEQ): { register0 = globalSpace + *pc++; returnFunc = wr_CompareEQ; goto returnFuncPostLoad; }
			CASE(GSCompareNE): { register0 = globalSpace + *pc++; returnFunc = wr_CompareEQ; goto returnFuncInvertedPostLoad; }
			CASE(GSCompareGT): { register0 = globalSpace + *pc++; returnFunc = wr_CompareGT; goto returnFuncPostLoad; }
			CASE(GSCompareLT): { register0 = globalSpace + *pc++; returnFunc = wr_CompareLT; goto returnFuncPostLoad; }
			CASE(GSCompareGE): { register0 = globalSpace + *pc++; returnFunc = wr_CompareLT; goto returnFuncInvertedPostLoad; }
			CASE(GSCompareLE): { register0 = globalSpace + *pc++; returnFunc = wr_CompareGT; goto returnFuncInvertedPostLoad; }
							   
			CASE(LSCompareEQ): { register0 = frameBase + *pc++; returnFunc = wr_CompareEQ; goto returnFuncPostLoad; }
			CASE(LSCompareNE): { register0 = frameBase + *pc++; returnFunc = wr_CompareEQ; goto returnFuncInvertedPostLoad; }
			CASE(LSCompareGT): { register0 = frameBase + *pc++; returnFunc = wr_CompareGT; goto returnFuncPostLoad; }
			CASE(LSCompareLT): { register0 = frameBase + *pc++; returnFunc = wr_CompareLT; goto returnFuncPostLoad; }
			CASE(LSCompareGE): { register0 = frameBase + *pc++; returnFunc = wr_CompareLT; goto returnFuncInvertedPostLoad; }
			CASE(LSCompareLE): { register0 = frameBase + *pc++; returnFunc = wr_CompareGT; goto returnFuncInvertedPostLoad; }

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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}
			
			CASE(CompareBNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}
			
			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
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
			CASE(GGCompareGE):
			{
				returnFunc = wr_CompareLT;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareNEPost;
			}
			CASE(GGCompareLT):
			{
				returnFunc = wr_CompareLT;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareEQPost;
			}
			CASE(GGCompareLE):
			{
				returnFunc = wr_CompareGT;
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				goto returnCompareNEPost;
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
			
			CASE(LLCompareGE): { returnFunc = wr_CompareLT; goto returnCompareNE; }
			CASE(LLCompareLE): { returnFunc = wr_CompareGT; goto returnCompareNE; }
			CASE(LLCompareNE):
			{
				returnFunc = wr_CompareEQ;
returnCompareNE:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
returnCompareNEPost:
				stackTop->i = (int)!returnFunc[(register0->type<<2)|register1->type]( register0, register1 );
				(stackTop++)->p2 = INIT_AS_INT;
				CONTINUE;
			}


			
			CASE(GSCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareGInverted; }
			CASE(GSCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareGInverted; }
			CASE(GSCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareGInverted:
				register0 = globalSpace + *pc++;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
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

			CASE(LLCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareLLInv; }
			CASE(LLCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareLLInv; }
			CASE(LLCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareLLInv:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}
			CASE(LLCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareLL; }
			CASE(LLCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareLL; }
			CASE(LLCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareLL:	
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(LLCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareLL8Inv; }
			CASE(LLCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareLL8Inv; }
			CASE(LLCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareLL8Inv:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}
			CASE(LLCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareLL8; }
			CASE(LLCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareLL8; }
			CASE(LLCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareLL8:
				register1 = frameBase + *pc++;
				register0 = frameBase + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			
			CASE(GGCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareGGInv; }
 		    CASE(GGCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareGGInv; }
			CASE(GGCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareGGInv:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}

			CASE(GGCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareGG; }
			CASE(GGCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareGG; }
			CASE(GGCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareGG:	
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(GGCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareGG8Inv; }
			CASE(GGCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareGG8Inv; }
			CASE(GGCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareGG8Inv:
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)*pc : 2;
				CONTINUE;
			}
			CASE(GGCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareGG8; }
			CASE(GGCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareGG8; }
			CASE(GGCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareGG8:	
				register1 = globalSpace + *pc++;
				register0 = globalSpace + *pc++;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)*pc;
				CONTINUE;
			}

			
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
				register1 = --stackTop;
				register0 = stackTop - 1;
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, register0 );
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
#endif		
		
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
void wr_makeContainer( WRValue* val )
{
	val->p2 = INIT_AS_HASH_TABLE;
	val->va = new WRGCObject( 0, SV_HASH_TABLE );
	val->va->m_skipGC = 1;
}


//------------------------------------------------------------------------------
WRValue* wr_getContainerEntryEx( WRValue* container, const char* name )
{
	if ( container->xtype != WR_EX_HASH_TABLE )
	{
		return 0;
	}

	uint32_t hash = wr_hashStr( name );

	WRValue *entry = (WRValue *)container->va->get( hash );
	(entry + 1)->p2 = INIT_AS_INT;
	(entry + 1)->ui = hash;
	return entry;
}


//------------------------------------------------------------------------------
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value )
{
	WRValue* entry = wr_getContainerEntryEx( container, name );
	if ( entry )
	{
		entry->r = value;
		entry->p2 = INIT_AS_REF;
	}
}

//------------------------------------------------------------------------------
void wr_addArrayToContainer( WRValue* container, const char* name, char* array )
{
	WRValue* entry = wr_getContainerEntryEx( container, name );
	if ( entry )
	{
		entry->c = array;
		entry->p2 = INIT_AS_RAW_ARRAY;
	}
}

//------------------------------------------------------------------------------
void wr_destroyContainer( WRValue* val )
{
	if ( val->xtype != WR_EX_HASH_TABLE )
	{
		return;
	}

	delete val->va;
	val->init();
}

