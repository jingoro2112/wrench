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
//#define DEBUG_OPCODE_NAMES
#ifdef DEBUG_OPCODE_NAMES
const char* c_opcodeName[] = 
{
	"RegisterFunction",

	"LiteralInt32",
	"LiteralZero",
	"LiteralFloat",
	"LiteralString",

	"CallFunctionByHash",
	"CallFunctionByHashAndPop",
	"CallFunctionByIndex",
	"PushIndexFunctionReturnValue",

	"CallLibFunction",
	"CallLibFunctionAndPop",

	"NewObjectTable",
	"AssignToObjectTableByOffset",

	"AssignToHashTableAndPop",
	"Remove",
	"HashEntryExists",

	"PopOne",
	"ReturnZero",
	"Return",
	"Stop",

	"Dereference",
	"Index",
	"IndexSkipLoad",
	"CountOf",
	"HashOf",

	"StackIndexHash",
	"GlobalIndexHash",
	"LocalIndexHash",

	"StackSwap",
	"SwapTwoToTop",

	"LoadFromLocal",
	"LoadFromGlobal",

	"LLValues",
	"LGValues",
	"GLValues",
	"GGValues",

	"BinaryRightShiftSkipLoad",
	"BinaryLeftShiftSkipLoad",
	"BinaryAndSkipLoad",
	"BinaryOrSkipLoad",
	"BinaryXORSkipLoad",
	"BinaryModSkipLoad",

	"BinaryMultiplication",
	"BinarySubtraction",
	"BinaryDivision",
	"BinaryRightShift",
	"BinaryLeftShift",
	"BinaryMod",
	"BinaryOr",
	"BinaryXOR",
	"BinaryAnd",
	"BinaryAddition",

	"BitwiseNOT",

	"RelativeJump",
	"RelativeJump8",

	"BZ",
	"BZ8",

	"LogicalAnd",
	"LogicalOr",
	"CompareLE",
	"CompareGE",
	"CompareGT",
	"CompareLT",
	"CompareEQ",
	"CompareNE",

	"GGCompareGT",
	"GGCompareGE",
	"GGCompareLT",
	"GGCompareLE",
	"GGCompareEQ", 
	"GGCompareNE", 

	"LLCompareGT",
	"LLCompareGE",
	"LLCompareLT",
	"LLCompareLE",
	"LLCompareEQ", 
	"LLCompareNE", 

	"GSCompareEQ",
	"LSCompareEQ", 
	"GSCompareNE", 
	"LSCompareNE", 
	"GSCompareGE",
	"LSCompareGE",
	"GSCompareLE",
	"LSCompareLE",
	"GSCompareGT",
	"LSCompareGT",
	"GSCompareLT",
	"LSCompareLT",

	"GSCompareEQBZ", 
	"LSCompareEQBZ", 
	"GSCompareNEBZ", 
	"LSCompareNEBZ", 
	"GSCompareGEBZ",
	"LSCompareGEBZ",
	"GSCompareLEBZ",
	"LSCompareLEBZ",
	"GSCompareGTBZ",
	"LSCompareGTBZ",
	"GSCompareLTBZ",
	"LSCompareLTBZ",

	"GSCompareEQBZ8",
	"LSCompareEQBZ8",
	"GSCompareNEBZ8",
	"LSCompareNEBZ8",
	"GSCompareGEBZ8",
	"LSCompareGEBZ8",
	"GSCompareLEBZ8",
	"LSCompareLEBZ8",
	"GSCompareGTBZ8",
	"LSCompareGTBZ8",
	"GSCompareLTBZ8",
	"LSCompareLTBZ8",

	"LLCompareLTBZ",
	"LLCompareLEBZ",
	"LLCompareGTBZ",
	"LLCompareGEBZ",
	"LLCompareEQBZ",
	"LLCompareNEBZ",

	"GGCompareLTBZ",
	"GGCompareLEBZ",
	"GGCompareGTBZ",
	"GGCompareGEBZ",
	"GGCompareEQBZ",
	"GGCompareNEBZ",

	"LLCompareLTBZ8",
	"LLCompareLEBZ8",
	"LLCompareGTBZ8",
	"LLCompareGEBZ8",
	"LLCompareEQBZ8",
	"LLCompareNEBZ8",

	"GGCompareLTBZ8",
	"GGCompareLEBZ8",
	"GGCompareGTBZ8",
	"GGCompareGEBZ8",
	"GGCompareEQBZ8",
	"GGCompareNEBZ8",

	"PostIncrement",
	"PostDecrement",
	"PreIncrement",
	"PreDecrement",

	"PreIncrementAndPop",
	"PreDecrementAndPop",

	"IncGlobal",
	"DecGlobal",
	"IncLocal",
	"DecLocal",

	"Assign",
	"AssignAndPop",
	"AssignToGlobalAndPop",
	"AssignToLocalAndPop",
	"AssignToArrayAndPop",

	"SubtractAssign",
	"AddAssign",
	"ModAssign",
	"MultiplyAssign",
	"DivideAssign",
	"ORAssign",
	"ANDAssign",
	"XORAssign",
	"RightShiftAssign",
	"LeftShiftAssign",

	"SubtractAssignAndPop",
	"AddAssignAndPop",
	"ModAssignAndPop",
	"MultiplyAssignAndPop",
	"DivideAssignAndPop",
	"ORAssignAndPop",
	"ANDAssignAndPop",
	"XORAssignAndPop",
	"RightShiftAssignAndPop",
	"LeftShiftAssignAndPop",

	"LogicalNot", //X
	"Negate",

	"LiteralInt8",
	"LiteralInt16",

	"IndexLiteral8",
	"IndexLiteral16",

	"IndexLocalLiteral8",
	"IndexGlobalLiteral8",
	"IndexLocalLiteral16",
	"IndexGlobalLiteral16",

	"BinaryAdditionAndStoreGlobal",
	"BinarySubtractionAndStoreGlobal",
	"BinaryMultiplicationAndStoreGlobal",
	"BinaryDivisionAndStoreGlobal",

	"BinaryAdditionAndStoreLocal",
	"BinarySubtractionAndStoreLocal",
	"BinaryMultiplicationAndStoreLocal",
	"BinaryDivisionAndStoreLocal",

	"CompareBEQ",
	"CompareBNE",
	"CompareBGE",
	"CompareBLE",
	"CompareBGT",
	"CompareBLT",

	"CompareBEQ8",
	"CompareBNE8",
	"CompareBGE8",
	"CompareBLE8",
	"CompareBGT8",
	"CompareBLT8",

	"BLA",
	"BLA8",
	"BLO",
	"BLO8",

	"LiteralInt8ToGlobal",
	"LiteralInt16ToGlobal",
	"LiteralInt32ToLocal",
	"LiteralInt8ToLocal",
	"LiteralInt16ToLocal",
	"LiteralFloatToGlobal",
	"LiteralFloatToLocal",
	"LiteralInt32ToGlobal",

	"GGBinaryMultiplication",
	"GLBinaryMultiplication",
	"LLBinaryMultiplication",

	"GGBinaryAddition",
	"GLBinaryAddition",
	"LLBinaryAddition",

	"GGBinarySubtraction",
	"GLBinarySubtraction",
	"LGBinarySubtraction",
	"LLBinarySubtraction",

	"GGBinaryDivision",
	"GLBinaryDivision",
	"LGBinaryDivision",
	"LLBinaryDivision",

	"GC_Command",

	"GPushIterator",
	"LPushIterator",
	"GGNextKeyValueOrJump",
	"GLNextKeyValueOrJump",
	"LGNextKeyValueOrJump",
	"LLNextKeyValueOrJump",
	"GNextValueOrJump",
	"LNextValueOrJump",

	"Switch",
	"SwitchLinear",

	"GlobalStop",

	"ToInt",
	"ToFloat",

	"LoadLibConstant",
	"InitArray",

	"DebugInfo",
};
#endif

//------------------------------------------------------------------------------
void WRContext::mark( WRValue* s )
{
	if ( IS_ARRAY_MEMBER(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		// we don't mark this type, but we might mark it's target
		mark( s->r );
		return;
	}

	if ( !IS_EXARRAY_TYPE(s->xtype) || s->va->m_skipGC || (s->va->m_size & 0x40000000) )
	{
		return;
	}

	assert( !IS_RAW_ARRAY(s->xtype) );

	WRGCObject* sva = s->va;

	if ( sva->m_type == SV_VALUE )
	{
		WRValue* top = sva->m_Vdata + sva->m_size;

		for( WRValue* V = sva->m_Vdata; V<top; ++V )
		{
			mark( V );
		}
	}
	else if ( sva->m_type == SV_HASH_TABLE )
	{
		for( uint32_t i=0; i<sva->m_mod; ++i )
		{
			if ( sva->m_hashTable[i] )
			{
				uint32_t item = i<<1;
				mark( sva->m_Vdata + item++ );
				mark( sva->m_Vdata + item );
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
		mark( s );
	}

	// mark context's globals
	WRValue* globalSpace = (WRValue *)(this + 1); // globals are allocated directly after this context

	for( int i=0; i<globals; ++i, ++globalSpace )
	{
		mark( globalSpace );
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
		// otherwise free it as unreferenced
		else
		{
			current->clear();

			if ( prev == 0 )
			{
				svAllocated = current->m_next;
				free( current );
				current = svAllocated;
			}
			else
			{
				prev->m_next = current->m_next;
				free( current );
				current = prev->m_next;
			}
		}
	}
}

//------------------------------------------------------------------------------
WRGCObject* WRContext::getSVA( int size, WRGCObjectType type, bool init )
{
	WRGCObject* ret = (WRGCObject*)malloc( sizeof(WRGCObject) );
	ret->init( size, type );
	if ( init )
	{
		memset( (char*)ret->m_Vdata, 0, sizeof(WRValue) * size);
	}

	if ( type == SV_CHAR )
	{
		ret->m_creatorContext = this;
	}
	
	ret->m_next = svAllocated;
	svAllocated = ret;

	return ret;
}

//------------------------------------------------------------------------------
inline bool wr_getNextValue( WRValue* iterator, WRValue* value, WRValue* key )
{
	if ( !IS_ITERATOR(iterator->xtype) )
	{
		return false;
	}

	uint32_t element = DECODE_ARRAY_ELEMENT_FROM_P2( iterator->p2 );

	if ( iterator->va->m_type == SV_HASH_TABLE )
	{
		for( ; element<iterator->va->m_mod; ++element )
		{
			if ( iterator->va->m_hashTable[element] )
			{
				iterator->p2 = INIT_AS_ITERATOR | ENCODE_ARRAY_ELEMENT_TO_P2(element+1);
				
				element <<= 1;

				value->p2 = INIT_AS_REF;
				value->r = iterator->va->m_Vdata + element;
				if ( key )
				{
					key->p2 = INIT_AS_REF;
					key->r = iterator->va->m_Vdata + element + 1;
				}
				
				return true;
			}
		}
		
		return false;
	}
	else
	{
		if ( element >= iterator->va->m_size )
		{
			return false;
		}

		if ( key )
		{
			key->p2 = INIT_AS_INT;
			key->i = element;
		}

		value->p2 = INIT_AS_REF;
		value->r = iterator->va->m_Vdata + element;

		iterator->p2 = INIT_AS_ITERATOR | ENCODE_ARRAY_ELEMENT_TO_P2(++element);
	}

	return true;
}

//#define D_OPCODE
#ifdef D_OPCODE
 #define PER_INSTRUCTION printf( "S[%d] %d:%s\n", (int)(stackTop - w->stack), (int)READ_8_FROM_PC(pc), c_opcodeName[READ_8_FROM_PC(pc)]);
#else
 #define PER_INSTRUCTION
#endif

#ifdef WRENCH_JUMPTABLE_INTERPRETER
 #define CONTINUE { PER_INSTRUCTION; goto *opcodeJumptable[READ_8_FROM_PC(pc++)];  }
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
int wr_addI( int a, int b ) { return a + b; }
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

int32_t READ_32_FROM_PC_func( const unsigned char* P )
{
	return ( (((int32_t)*(P)) )
			 | (((int32_t)*((P)+1)) << 8)
			 | (((int32_t)*((P)+2)) << 16)
			 | (((int32_t)*((P)+3)) << 24) );
}

int16_t READ_16_FROM_PC_func( const unsigned char* P )
{
	return ( ((int16_t)*(P)) )
			| ((int16_t)*(P+1) << 8 );
}

#endif

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, WRFunction* function, const WRValue* argv, const int argn )
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
		&&CallLibFunctionAndPop,

		&&NewObjectTable,
		&&AssignToObjectTableByOffset,

		&&AssignToHashTableAndPop,
		&&Remove,
		&&HashEntryExists,

		&&PopOne,
		&&ReturnZero,
		&&Return,
		&&Stop,

		&&Dereference,
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

		&&ToInt,
		&&ToFloat,

		&&LoadLibConstant,
		&&InitArray,

		&&DebugInfo,
	};
#endif

	const unsigned char* pc;

	union
	{
		unsigned char findex;
		WRValue* register0;
		const unsigned char *hashLoc;
		uint32_t hashLocInt;
	};
	
	WRValue* register1 = 0;
	WRValue* frameBase = 0;
	WRState* w = context->w;
	const unsigned char* bottom = context->bottom;
	WRValue* stackTop = w->stack;
	WRValue* globalSpace = (WRValue *)(context + 1);

	union
	{
		// never used at the same time..save RAM!
		WRValue* register2;
		uint32_t sizeCheck;
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

#ifdef WRENCH_INCLUDE_DEBUG_CODE
	if ( context->debugInterface && function )
	{
		if ( !function->offset ) // impossible offset, this is a re-entry!
		{
			// pop state
			pc = context->debugInterface->m_pc;
			frameBase = context->debugInterface->m_frameBase;
			stackTop = context->debugInterface->m_stackTop;
			goto debugContinue;
		}
	}
#endif

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

	pc = bottom + 2;

#ifdef WRENCH_JUMPTABLE_INTERPRETER

	CONTINUE;
	
#else

	for(;;)
	{
		switch( READ_8_FROM_PC(pc++) )
		{
#endif
			CASE(RegisterFunction):
			{
				hash = (stackTop -= 3)->i;

				findex = hash;
				WRFunction* localFunctions = context->localFunctions + findex;

				localFunctions->arguments = (unsigned char)(hash >> 8);
				localFunctions->frameSpaceNeeded = (unsigned char)(hash >> 16);

				localFunctions->hash = (stackTop + 1)->i;

				localFunctions->offset = bottom + (stackTop + 2)->i;

				localFunctions->frameBaseAdjustment = 1
													  + localFunctions->frameSpaceNeeded
													  + localFunctions->arguments;

				context->registry.getAsRawValueHashTable(localFunctions->hash)->wrf = localFunctions;

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
				hash = (uint16_t)READ_16_FROM_PC(pc);
				pc += 2;
				
				context->gc( stackTop );
				stackTop->p2 = INIT_AS_ARRAY;
				stackTop->va = context->getSVA( hash, SV_CHAR, false );

				for ( char* to = (char *)(stackTop++)->va->m_data ; hash ; --hash )
				{
					*to++ = READ_8_FROM_PC(pc++);
				}
				
				CONTINUE;
			}

			CASE(LoadLibConstant):
			{
				*stackTop = *(w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)));

				if ( (stackTop->p2 & INIT_AS_LIB_CONST) != INIT_AS_LIB_CONST )
				{
					w->err = WR_ERR_library_constant_not_loaded;
					return 0;
				}
				
				(stackTop++)->p2 &= ~INIT_AS_LIB_CONST;
				pc += 4;
				CONTINUE;
			}

			CASE(InitArray):
			{
				register0 = &(--stackTop)->singleValue();
				register1 = &(stackTop - 1)->deref();
				register1->p2 = INIT_AS_ARRAY;
				register1->va = context->getSVA( register0->ui, SV_VALUE, true );
				CONTINUE;
			}

			CASE(DebugInfo):
			{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
				if ( context->debugInterface )
				{
					context->debugInterface->codewordEncountered( READ_16_FROM_PC(pc), stackTop );
					if ( context->debugInterface->m_brk )
					{
						// push state
						context->debugInterface->m_argv = argv;
						context->debugInterface->m_argn = argn;
						context->debugInterface->m_pc = pc;
						context->debugInterface->m_frameBase = frameBase;
						context->debugInterface->m_stackTop = stackTop;
						context->debugInterface->m_brk = false;
						
						stackTop->p2 = INIT_AS_DEBUG_BREAK;
						return stackTop;
					}
				}

debugContinue:
#endif
				pc += 2;
				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				args = READ_8_FROM_PC(pc++);

				// initialize a return value of 'zero'
				register0 = stackTop;
				register0->p = 0;
				register0->p2 = INIT_AS_INT;

				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb) )
				{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
					if ( context->debugInterface )
					{
						
					}					
#endif
					w->err = WR_ERR_function_hash_signature_not_found;
					return 0;
				}

				register1->ccb( context, stackTop - args, args, *stackTop, register1->usr );

				// DO care about return value, which will be at the top
				// of the stack

				if ( args )
				{
					stackTop -= (args - 1);
					*(stackTop - 1) = *register0;
				}
				else
				{
					++stackTop;
				}
				pc += 4;
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				args = READ_8_FROM_PC(pc++);

				if ( !((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb) )
				{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
					if ( context->debugInterface )
					{

					}					
#endif
					w->err = WR_ERR_function_hash_signature_not_found;
					return 0;
				}

				register1->ccb( context, stackTop - args, args, *stackTop, register1->usr );

				stackTop -= args;
				pc += 4;
				CONTINUE;
			}

			CASE(CallFunctionByIndex):
			{
				args = READ_8_FROM_PC(pc++);

				// function MUST exist or we wouldn't be here, we would
				// be in the "call by hash" above
				function = context->localFunctions + READ_8_FROM_PC(pc++);
				pc += READ_8_FROM_PC(pc);
callFunction:				
				// rectify arg count? hopefully not lets get calling!
				if ( args != function->arguments )
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

				// for speed locals are not guaranteed to be initialized.. but we have to make
				// sure they are not randomly selected to a
				// "collectable" type or the gc will iterate them in some corner cases (ask me how I know)
				// A simple offset add would be so fast.. 
				//           TODO figure out how to use it!
		
//				stackTop += function->frameSpaceNeeded;
				for( int l=0; l<function->frameSpaceNeeded; ++l )
				{
					(stackTop++)->p2 = INIT_AS_INT;
				}
			
				// temp value contains return vector/frame base
				register0 = stackTop++; // return vector
				register0->frame = frameBase;
				register0->returnOffset = pc - bottom; // can't use "pc" because it might set the "gc me" bit, this is guaranteed to be small enough to never do that

				pc = function->offset;

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				assert( stackTop < (w->stack + w->stackSize) );

				CONTINUE;
			}

			CASE(PushIndexFunctionReturnValue):
			{
				*(stackTop++) = (++register0)->deref();
				CONTINUE;
			}

			CASE(CallLibFunction):
			{
				stackTop->p2 = INIT_AS_INT;
				stackTop->p = 0;

				args = READ_8_FROM_PC(pc++); // which have already been pushed

				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb) )
				{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
					if ( context->debugInterface )
					{

					}					
#endif
					w->err = WR_ERR_function_hash_signature_not_found;
					return 0;
				}

				register1->lcb( stackTop, args, context );
				pc += 4;

#ifdef WRENCH_COMPACT
				// this "always works" but is not necessary if args is
				// zero, just a simple stackTop increment is required
				stackTop -= --args;
				*(stackTop - 1) = *(stackTop + args);
#else
				if ( args )
				{
					stackTop -= --args;
					*(stackTop - 1) = *(stackTop + args);
				}
				else
				{
					++stackTop;
				}
#endif

				CONTINUE;
			}

			CASE(CallLibFunctionAndPop):
			{
				args = READ_8_FROM_PC(pc++); // which have already been pushed

				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb) )
				{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
					if ( context->debugInterface )
					{

					}					
#endif
					w->err = WR_ERR_function_hash_signature_not_found;
					return 0;
				}

				register1->lcb( stackTop, args, context );
				pc += 4;

				stackTop -= args;
				
				CONTINUE;
			}

			CASE(NewObjectTable):
			{
				const unsigned char* table = bottom + READ_16_FROM_PC(pc);
				pc += 2;

				if ( table > bottom )
				{
					// if unit was called with no arguments from global
					// level there are not "free" stack entries to
					// gnab, so create it here, but preserve the
					// first value

					// NOTE: we are guaranteed to have at least one
					// value if table > bottom

					unsigned char count = READ_8_FROM_PC(table++);

					register1 = (stackTop + READ_8_FROM_PC(table))->r;
					register2 = (stackTop + READ_8_FROM_PC(table))->r2;

					stackTop->p2 = INIT_AS_STRUCT;

					// table : members in local space
					// table + 1 : arguments + 1 (+1 to save the calculation below)
					// table +2/3 : m_mod
					// table + 4: [static hash table ]

					stackTop->va = context->getSVA( count, SV_VALUE, false );

					stackTop->va->m_ROMHashTable = table + 3;
					stackTop->va->m_mod = READ_16_FROM_PC(table+1);

					register0 = stackTop->va->m_Vdata;
					register0->r = register1;
					(register0++)->r2 = register2;

					if ( --count > 0 )
					{
						memcpy( (char*)register0, stackTop + READ_8_FROM_PC(table) + 1, count*sizeof(WRValue) );
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
				register1 = --stackTop;
				register0 = (stackTop - 1);
				sizeCheck = READ_8_FROM_PC(pc++);
				if ( IS_EXARRAY_TYPE(register0->xtype) && (sizeCheck < register0->va->m_size) )
				{
					register0 = register0->va->m_Vdata + sizeCheck;
#ifdef WRENCH_COMPACT
					goto doAssignToLocalAndPop;
#else
					wr_assign[(register0->type << 2) | register1->type](register0, register1);
#endif
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
			
			CASE(Remove):
			{
				hash = (--stackTop)->getHash();
				register0 = &((stackTop - 1)->deref());
				if ( register0->xtype == WR_EX_HASH_TABLE )
				{
					register0->va->exists( hash, false, true );
				}
				CONTINUE;	
			}

			CASE(HashEntryExists):
			{
				register0 = --stackTop;
				register1 = (stackTop - 1);
				register2 = &(register1->deref());
				register1->p2 = INIT_AS_INT;
				register1->i = ((register2->xtype == WR_EX_HASH_TABLE) && register2->va->exists(register0->getHash(), false, false)) ? 1 : 0;
				
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

				pc = bottom + register0->returnOffset; // grab return PC

				stackTop = frameBase;
				frameBase = register0->frame;
				CONTINUE;
			}

			CASE(ToInt):
			{
				register0 = stackTop - 1;
				register0->i = register0->asInt();
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(ToFloat):
			{
				register0 = stackTop - 1;
				register0->f = register0->asFloat();
				register0->p2 = INIT_AS_FLOAT;
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
				return &(w->stack)->deref();
			}

			CASE(Dereference):
			{
				register0 = (stackTop - 1);
				*register0 = register0->deref();
				CONTINUE;
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
				wr_countOfArrayElement( register0, register0 );
				CONTINUE;
			}

			CASE(HashOf):
			{
				register0 = stackTop - 1;
				register0->ui = register0->getHash();
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(GlobalIndexHash):
			{
				register0 = &(globalSpace + READ_8_FROM_PC(pc++))->deref();
				register1 = stackTop++;
				goto hashIndexJump;
			}

			CASE(LocalIndexHash):
			{
				register0 = &(frameBase + READ_8_FROM_PC(pc++))->deref();
				register1 = stackTop++;
				goto hashIndexJump;
			}

			CASE(StackIndexHash):
			{
				register0 = &(stackTop - 1)->deref();
				register1 = register0;
hashIndexJump:				
				stackTop->ui = READ_32_FROM_PC(pc);
				pc += 4;
				if ( !EXPECTS_HASH_INDEX(register0->xtype) )
				{
					register0->p2 = INIT_AS_HASH_TABLE;
					register0->va = context->getSVA( 0, SV_HASH_TABLE, false );
				}
						
#ifdef WRENCH_COMPACT
				goto indexTempLiteralPostLoad;
#else
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT << 2) | register0->type](context, stackTop, register0, stackTop - 1);
				CONTINUE;
#endif
			}

			CASE(StackSwap):
			{
				register0 = stackTop - 1;
				register1 = stackTop - READ_8_FROM_PC(pc++);
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
				register0 = stackTop - READ_8_FROM_PC(pc++);

				uint32_t t = (stackTop - 1)->p2;
				const void* p = (stackTop - 1)->p;

				(stackTop - 1)->p2 = register0->p2;
				(stackTop - 1)->p = register0->p;

				register0->p = p;
				register0->p2 = t;

				register0 = stackTop - READ_8_FROM_PC(pc++);

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
				stackTop->p = frameBase + READ_8_FROM_PC(pc++);
				(stackTop++)->p2 = INIT_AS_REF;
				CONTINUE;
			}

			CASE(LoadFromGlobal):
			{
				stackTop->p = globalSpace + READ_8_FROM_PC(pc++);
 				(stackTop++)->p2 = INIT_AS_REF;
				CONTINUE;
			}

			CASE(LLValues): { register0 = frameBase + READ_8_FROM_PC(pc++); register1 = frameBase + READ_8_FROM_PC(pc++); CONTINUE; }
			CASE(LGValues): { register0 = frameBase + READ_8_FROM_PC(pc++); register1 = globalSpace + READ_8_FROM_PC(pc++); CONTINUE; }
			CASE(GLValues): { register0 = globalSpace + READ_8_FROM_PC(pc++); register1 = frameBase + READ_8_FROM_PC(pc++); CONTINUE; }
			CASE(GGValues):	{ register0 = globalSpace + READ_8_FROM_PC(pc++); register1 = globalSpace + READ_8_FROM_PC(pc++); CONTINUE; }

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
				pc += (int8_t)READ_8_FROM_PC(pc);
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
				pc += wr_LogicalNot[register0->type](register0) ? (int8_t)READ_8_FROM_PC(pc) : 2;
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
				wr_negate[ register0->type ]( register0, register0 );
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
				stackTop->i = READ_8_FROM_PC(pc++);
indexLiteral:
				stackTop->p2 = INIT_AS_INT;
				register0 = stackTop - 1;
				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, register0 );
				CONTINUE;
			}

			CASE(IndexLocalLiteral16):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				(++stackTop)->i = READ_16_FROM_PC(pc);
				pc += 2;
#ifdef WRENCH_COMPACT
				goto indexTempLiteralPostLoad;
#else
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT << 2) | register0->type](context, stackTop, register0, stackTop - 1);
				CONTINUE;
#endif
			}
			
			CASE(IndexLocalLiteral8):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
#ifdef WRENCH_COMPACT
indexTempLiteral:
#endif
				(++stackTop)->i = READ_8_FROM_PC(pc++);
#ifdef WRENCH_COMPACT
indexTempLiteralPostLoad:
#endif
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, stackTop - 1 );
				CONTINUE;
			}
			
			CASE(IndexGlobalLiteral16):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				(++stackTop)->i = READ_16_FROM_PC(pc);
				pc += 2;
#ifdef WRENCH_COMPACT
				goto indexTempLiteralPostLoad;
#else
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT << 2) | register0->type](context, stackTop, register0, stackTop - 1);
				CONTINUE;
#endif
			}

			CASE(IndexGlobalLiteral8):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
#ifdef WRENCH_COMPACT
				goto indexTempLiteral;
#else
				(++stackTop)->i = READ_8_FROM_PC(pc++);
				stackTop->p2 = INIT_AS_INT;
				wr_index[(WR_INT << 2) | register0->type](context, stackTop, register0, stackTop - 1);
				CONTINUE;
#endif
			}
			
			CASE(AssignToGlobalAndPop):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
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
				register0 = frameBase + READ_8_FROM_PC(pc++);

#ifdef WRENCH_COMPACT
doAssignToLocalAndPopPreLoad:
#endif
				register1 = --stackTop;

#ifdef WRENCH_COMPACT
doAssignToLocalAndPop:
#endif
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

				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, stackTop + 1 );
				register0 = stackTop-- + 1;

#ifdef WRENCH_COMPACT
				goto doAssignToLocalAndPop;
#else
				wr_assign[(register0->type << 2) | register1->type](register0, register1);
				CONTINUE;
#endif
			}

			CASE(LiteralInt8):
			{
				stackTop->i = (int32_t)(int8_t)READ_8_FROM_PC(pc++);
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

			CASE(LiteralInt8ToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->i = (int32_t)(int8_t)READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->i = READ_16_FROM_PC(pc);
				register0->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralInt32ToLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_INT;
				goto load32ToTemp;
			}

			CASE(LiteralInt8ToLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register0->i = (int32_t)(int8_t)READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_INT;
				CONTINUE;
			}

			CASE(LiteralInt16ToLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register0->i = READ_16_FROM_PC(pc);
				register0->p2 = INIT_AS_INT;
				pc += 2;
				CONTINUE;
			}

			CASE(LiteralFloatToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralFloatToLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_FLOAT;
				goto load32ToTemp;
			}

			CASE(LiteralInt32ToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_INT;
load32ToTemp:
				register0->i = READ_32_FROM_PC(pc);
				pc += 4;
				CONTINUE;
			}

			CASE(GC_Command):
			{
				context->gcPauseCount = READ_16_FROM_PC(pc);
				pc += 2;
				CONTINUE;
			}
			
			CASE(GPushIterator):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_pushIterator[register0->type]( register0, globalSpace + READ_8_FROM_PC(pc++) );
				CONTINUE;
			}

			CASE(LPushIterator):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_pushIterator[register0->type]( register0, globalSpace + READ_8_FROM_PC(pc++) );
				CONTINUE;
			}

			CASE(GGNextKeyValueOrJump): { register1 = globalSpace + READ_8_FROM_PC(pc++); register0 = globalSpace + READ_8_FROM_PC(pc++); goto NextIterator; }
			CASE(GLNextKeyValueOrJump):	{ register1 = globalSpace + READ_8_FROM_PC(pc++); register0 = frameBase + READ_8_FROM_PC(pc++); goto NextIterator; }
			CASE(LGNextKeyValueOrJump): { register1 = frameBase + READ_8_FROM_PC(pc++); register0 = globalSpace + READ_8_FROM_PC(pc++); goto NextIterator; }
			CASE(LLNextKeyValueOrJump): { register1 = frameBase + READ_8_FROM_PC(pc++); register0 = frameBase + READ_8_FROM_PC(pc++); goto NextIterator; }
			CASE(GNextValueOrJump): { register0 = globalSpace + READ_8_FROM_PC(pc++); register1 = 0; goto NextIterator; }
			CASE(LNextValueOrJump):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register1 = 0;
NextIterator:
				register2 = globalSpace + READ_8_FROM_PC(pc++);
				pc += wr_getNextValue( register2, register0, register1) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}
			
			CASE(Switch):
			{
				hash = (--stackTop)->getHash(); // hash has been loaded
				hashLoc = pc + 4 + 6*(hash % (uint16_t)READ_16_FROM_PC(pc)); // jump into the table

				if ( (uint32_t)READ_32_FROM_PC(hashLoc) != hash )
				{
					hashLoc = pc - 2; // nope, point it at default vector
				}

				hashLoc += 4; // yup, point hashLoc to jump vector
				
				pc += READ_16_FROM_PC(hashLoc);
				CONTINUE;
			}

			CASE(SwitchLinear):
			{
				hashLocInt = (--stackTop)->getHash(); // the "hashes" were all 0<=h<256
				
				if ( hashLocInt < READ_8_FROM_PC(pc++) ) // catch selecting > size
				{
					hashLoc = pc + (hashLocInt<<1) + 2; // jump to vector
					pc += READ_16_FROM_PC( hashLoc ); // and read it
				}
				else
				{
					pc += READ_16_FROM_PC( pc );
				}
				CONTINUE;
			}

//-------------------------------------------------------------------------------------------------------------
#ifdef WRENCH_COMPACT //---------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

			CASE(PostIncrement):
			{
				register0 = stackTop - 1;
				m_unaryPost[ register0->type ]( register0, register0, 1 );
				CONTINUE;
			}

			CASE(PostDecrement):
			{
				register0 = stackTop - 1;
				m_unaryPost[ register0->type ]( register0, register0, -1 );
				CONTINUE;
			}
			
			CASE(BinaryRightShiftSkipLoad): { intCall = rightShiftI; goto targetFuncOpSkipLoad; }
			CASE(BinaryLeftShiftSkipLoad): { intCall = leftShiftI; goto targetFuncOpSkipLoad; }
			CASE(BinaryAndSkipLoad): { intCall = andI; goto targetFuncOpSkipLoad; }
			CASE(BinaryOrSkipLoad): { intCall = orI; goto targetFuncOpSkipLoad; }
			CASE(BinaryXORSkipLoad): { intCall = xorI; goto targetFuncOpSkipLoad; }
			CASE(BinaryModSkipLoad):
			{
				intCall = modI;
targetFuncOpSkipLoad:
				floatCall = blankF;
targetFuncOpSkipLoadNoClobberF:
				register2 = stackTop++;
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
				intCall = wr_addI;
targetFuncOp:
				register1 = --stackTop;
				register0 = stackTop - 1;
				register2 = register0;
				goto targetFuncOpSkipLoadAndReg2;
			}
			
			
			CASE(SubtractAssign): { floatCall = subtractionF; intCall = subtractionI; goto binaryTableOp; }
			CASE(AddAssign): { floatCall = addF; intCall = wr_addI; goto binaryTableOp; }
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
			CASE(AddAssignAndPop): { floatCall = addF; intCall = wr_addI; goto binaryTableOpAndPop; }
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
			
			CASE(BinaryAdditionAndStoreGlobal) : { floatCall = addF; intCall = wr_addI; goto targetFuncStoreGlobalOp; }
			CASE(BinarySubtractionAndStoreGlobal): { floatCall = subtractionF; intCall = subtractionI; goto targetFuncStoreGlobalOp; }
			CASE(BinaryMultiplicationAndStoreGlobal): { floatCall = multiplicationF; intCall = multiplicationI; goto targetFuncStoreGlobalOp; }
			CASE(BinaryDivisionAndStoreGlobal):
			{
				floatCall = divisionF;
				intCall = divisionI;
				
targetFuncStoreGlobalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				wr_funcBinary[(register0->type<<2)|register1->type]( register0, register1, globalSpace + READ_8_FROM_PC(pc++), intCall, floatCall );
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreLocal): { floatCall = addF; intCall = wr_addI; goto targetFuncStoreLocalOp; }
			CASE(BinarySubtractionAndStoreLocal): { floatCall = subtractionF; intCall = subtractionI; goto targetFuncStoreLocalOp; }
			CASE(BinaryMultiplicationAndStoreLocal): { floatCall = multiplicationF; intCall = multiplicationI; goto targetFuncStoreLocalOp; }
			CASE(BinaryDivisionAndStoreLocal):
			{
				floatCall = divisionF;
				intCall = divisionI;
				
targetFuncStoreLocalOp:
				register0 = --stackTop;
				register1 = --stackTop;
				wr_funcBinary[(register0->type<<2)|register1->type]( register0, register1, frameBase + READ_8_FROM_PC(pc++), intCall, floatCall );
				CONTINUE;
			}
			
			CASE(PreIncrement):
			{
				register0 = stackTop - 1;
compactPreIncrement:
				intCall = wr_addI;
				floatCall = addF;

compactIncrementWork:
				register1 = stackTop + 1;
				register1->i = 1;
				register1->p2 = INIT_AS_INT;
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactPreIncrement;
			}
			
			CASE(DecGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactPreDecrement;
			}
			CASE(IncLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactPreIncrement;
			}

			CASE(DecLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				goto targetFuncOpSkipLoadNoClobberF;
			}

			CASE(GLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactGLFunc:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				goto targetFuncOpSkipLoadNoClobberF;
			}

			CASE(LLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactFFFunc:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register1 = frameBase + READ_8_FROM_PC(pc++);
				goto targetFuncOpSkipLoadNoClobberF;
			}

			CASE(GGBinaryAddition):
			{
				floatCall = addF;
				intCall = wr_addI;
				goto CompactGGFunc;
			}

			CASE(GLBinaryAddition):
			{
				floatCall = addF;
				intCall = wr_addI;
				goto CompactGLFunc;
			}

			CASE(LLBinaryAddition):
			{
				floatCall = addF;
				intCall = wr_addI;
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register1 = frameBase + READ_8_FROM_PC(pc++);
				goto targetFuncOpSkipLoadNoClobberF;
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

			CASE(GSCompareEQ): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareNE): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncInvertedPostLoad; }
			CASE(GSCompareGT): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareLT): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncPostLoad; }
			CASE(GSCompareGE): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncInvertedPostLoad; }
			CASE(GSCompareLE): { register0 = globalSpace + READ_8_FROM_PC(pc++); boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncInvertedPostLoad; }
							   
			CASE(LSCompareEQ): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareNE): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactReturnFuncInvertedPostLoad; }
			CASE(LSCompareGT): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareLT): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncPostLoad; }
			CASE(LSCompareGE): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnFuncInvertedPostLoad; }
			CASE(LSCompareLE): { register0 = frameBase + READ_8_FROM_PC(pc++); boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnFuncInvertedPostLoad; }

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
				pc += wr_Compare[(register0->type<<2)|register1->type]( register0, register1, boolIntCall, boolFloatCall ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}

			CASE(GGCompareLE): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnCompareGGNEPost; }
			CASE(GGCompareGE): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnCompareGGNEPost; }
			CASE(GGCompareNE):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnCompareGGNEPost:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnCompareNEPost;
			}

			CASE(GGCompareGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactEQCompareGGReg; }
			CASE(GGCompareEQ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactEQCompareGGReg; }
			CASE(GGCompareLT):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactEQCompareGGReg:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnCompareEQPost;
			}

			CASE(LLCompareGT): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactReturnCompareEQ; }
			CASE(LLCompareLT): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactReturnCompareEQ; }
			CASE(LLCompareEQ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactReturnCompareEQ:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInvertedPreReg1;
			}
			
			CASE(GSCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGNormal; }
			CASE(GSCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGNormal; }
			CASE(GSCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGNormal:
				register0 = globalSpace + READ_8_FROM_PC(pc++);
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
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInverted8PreReg1;
			}
			
			CASE(GSCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareG8Normal; }
			CASE(GSCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareG8Normal; }
			CASE(GSCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareG8Normal:
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactBLA8PreReg1;
			}
			
			CASE(LSCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareL8Inverted; }
			CASE(LSCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareL8Inverted; }
			CASE(LSCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareL8Inverted:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInverted8PreReg1;
			}
			
			CASE(LSCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareL8Normal; }
			CASE(LSCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareL8Normal; }
			CASE(LSCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareL8Normal:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactBLA8PreReg1;
			}

			CASE(LLCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLLInv; }
			CASE(LLCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareLLInv; }
			CASE(LLCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareLLInv:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInvertedPostReg1;
			}
			CASE(LLCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareLL; }
			CASE(LLCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLL; }
			CASE(LLCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareLL:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactBLA;
			}
			
			CASE(LLCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLLInv8; }
			CASE(LLCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareLLInv8; }
			CASE(LLCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareLLInv8:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInverted8Post;
			}
			CASE(LLCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareLL8; }
			CASE(LLCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareLL8; }
			CASE(LLCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareLL8:	
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				goto compactBLA8;
			}

			CASE(GGCompareGEBZ): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareGGInv; }
			CASE(GGCompareLEBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGGInv; }
			CASE(GGCompareNEBZ):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareGGInv:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInvertedPostReg1;
			}
			
			CASE(GGCompareEQBZ): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGG; }
			CASE(GGCompareGTBZ): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGG; }
			CASE(GGCompareLTBZ):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGG:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactBLA;
			}
			
			CASE(GGCompareGEBZ8): { boolIntCall = CompareLTI; boolFloatCall = CompareLTF; goto compactCompareGGInv8; }
			CASE(GGCompareLEBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGGInv8; }
			CASE(GGCompareNEBZ8):
			{
				boolIntCall = CompareEQI;
				boolFloatCall = CompareEQF;
compactCompareGGInv8:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactReturnFuncBInverted8Post;
			}
			CASE(GGCompareEQBZ8): { boolIntCall = CompareEQI; boolFloatCall = CompareEQF; goto compactCompareGG8; }
			CASE(GGCompareGTBZ8): { boolIntCall = CompareGTI; boolFloatCall = CompareGTF; goto compactCompareGG8; }
			CASE(GGCompareLTBZ8):
			{
				boolIntCall = CompareLTI;
				boolFloatCall = CompareLTF;
compactCompareGG8:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto compactBLA8;
			}

			
//-------------------------------------------------------------------------------------------------------------
#else 
//-------------------------------------------------------------------------------------------------------------
// NON-COMPACT version
			
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

			CASE(PreIncrement): { register0 = stackTop - 1; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrement): { register0 = stackTop - 1; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreIncrementAndPop): { register0 = --stackTop; wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(PreDecrementAndPop): { register0 = --stackTop; wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncGlobal): { register0 = globalSpace + READ_8_FROM_PC(pc++); wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecGlobal): { register0 = globalSpace + READ_8_FROM_PC(pc++); wr_predec[ register0->type ]( register0 ); CONTINUE; }
			CASE(IncLocal): { register0 = frameBase + READ_8_FROM_PC(pc++); wr_preinc[ register0->type ]( register0 ); CONTINUE; }
			CASE(DecLocal): { register0 = frameBase + READ_8_FROM_PC(pc++); wr_predec[ register0->type ]( register0 ); CONTINUE; }

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
				pc += wr_LogicalAND[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
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
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(GGBinaryMultiplication):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryMultiplication):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryMultiplication):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			
			CASE(GGBinaryAddition):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryAddition):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++); 
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryAddition):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			
			CASE(GGBinarySubtraction):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinarySubtraction):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinarySubtraction):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinarySubtraction):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}


			CASE(GGBinaryDivision):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(GLBinaryDivision):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LGBinaryDivision):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CONTINUE;
			}

			CASE(LLBinaryDivision):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
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

			CASE(GSCompareEQ): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareEQ; goto returnFuncPostLoad; }
			CASE(GSCompareNE): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareEQ; goto returnFuncInvertedPostLoad; }
			CASE(GSCompareGT): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareGT; goto returnFuncPostLoad; }
			CASE(GSCompareLT): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareLT; goto returnFuncPostLoad; }
			CASE(GSCompareGE): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareLT; goto returnFuncInvertedPostLoad; }
			CASE(GSCompareLE): { register0 = globalSpace + READ_8_FROM_PC(pc++); returnFunc = wr_CompareGT; goto returnFuncInvertedPostLoad; }
							   
			CASE(LSCompareEQ): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareEQ; goto returnFuncPostLoad; }
			CASE(LSCompareNE): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareEQ; goto returnFuncInvertedPostLoad; }
			CASE(LSCompareGT): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareGT; goto returnFuncPostLoad; }
			CASE(LSCompareLT): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareLT; goto returnFuncPostLoad; }
			CASE(LSCompareGE): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareLT; goto returnFuncInvertedPostLoad; }
			CASE(LSCompareLE): { register0 = frameBase + READ_8_FROM_PC(pc++); returnFunc = wr_CompareGT; goto returnFuncInvertedPostLoad; }

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
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CONTINUE;
			}
			
			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}

			CASE(GGCompareEQ):
			{
				returnFunc = wr_CompareEQ;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareEQPost;
			}
			CASE(GGCompareNE):
			{
				returnFunc = wr_CompareEQ;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareNEPost;
			}
			CASE(GGCompareGT):
			{
				returnFunc = wr_CompareGT;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareEQPost;
			}
			CASE(GGCompareGE):
			{
				returnFunc = wr_CompareLT;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareNEPost;
			}
			CASE(GGCompareLT):
			{
				returnFunc = wr_CompareLT;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareEQPost;
			}
			CASE(GGCompareLE):
			{
				returnFunc = wr_CompareGT;
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				goto returnCompareNEPost;
			}

			
			CASE(LLCompareGT): { returnFunc = wr_CompareGT; goto returnCompareEQ; }
			CASE(LLCompareLT): { returnFunc = wr_CompareLT; goto returnCompareEQ; }
			CASE(LLCompareEQ):
			{
				returnFunc = wr_CompareEQ;
returnCompareEQ:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
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
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = frameBase + READ_8_FROM_PC(pc++);
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
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}
			
			CASE(GSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareG8Normal; }
			CASE(GSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareG8Normal; }
			CASE(GSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareG8Normal:
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CONTINUE;
			}
			
			CASE(LSCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareL8Inverted; }
			CASE(LSCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareL8Inverted; }
			CASE(LSCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareL8Inverted:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}
			
			CASE(LSCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareL8Normal; }
			CASE(LSCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareL8Normal; }
			CASE(LSCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareL8Normal:
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CONTINUE;
			}

			CASE(LLCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareLLInv; }
			CASE(LLCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareLLInv; }
			CASE(LLCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareLLInv:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}
			CASE(LLCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareLL; }
			CASE(LLCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareLL; }
			CASE(LLCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareLL:	
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(LLCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareLL8Inv; }
			CASE(LLCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareLL8Inv; }
			CASE(LLCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareLL8Inv:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}
			CASE(LLCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareLL8; }
			CASE(LLCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareLL8; }
			CASE(LLCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareLL8:
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(GGCompareGEBZ): { returnFunc = wr_CompareLT; goto CompareGGInv; }
 		    CASE(GGCompareLEBZ): { returnFunc = wr_CompareGT; goto CompareGGInv; }
			CASE(GGCompareNEBZ):
			{
				returnFunc = wr_CompareEQ;
CompareGGInv:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
				CONTINUE;
			}

			CASE(GGCompareEQBZ): { returnFunc = wr_CompareEQ; goto CompareGG; }
			CASE(GGCompareGTBZ): { returnFunc = wr_CompareGT; goto CompareGG; }
			CASE(GGCompareLTBZ):
			{
				returnFunc = wr_CompareLT;
CompareGG:	
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CONTINUE;
			}

			
			CASE(GGCompareGEBZ8): { returnFunc = wr_CompareLT; goto CompareGG8Inv; }
			CASE(GGCompareLEBZ8): { returnFunc = wr_CompareGT; goto CompareGG8Inv; }
			CASE(GGCompareNEBZ8):
			{
				returnFunc = wr_CompareEQ;
CompareGG8Inv:
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CONTINUE;
			}
			CASE(GGCompareEQBZ8): { returnFunc = wr_CompareEQ; goto CompareGG8; }
			CASE(GGCompareGTBZ8): { returnFunc = wr_CompareGT; goto CompareGG8; }
			CASE(GGCompareLTBZ8):
			{
				returnFunc = wr_CompareLT;
CompareGG8:	
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
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
				targetFunc[(register0->type<<2)|register1->type]( register0, register1, globalSpace + READ_8_FROM_PC(pc++) );
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
				targetFunc[(register0->type<<2)|register1->type]( register0, register1, frameBase + READ_8_FROM_PC(pc++) );
				CONTINUE;
			}

			
//-------------------------------------------------------------------------------------------------------------
#endif//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------


#ifndef WRENCH_JUMPTABLE_INTERPRETER
	#ifdef _MSC_VER
			default: __assume(0); // tells the compiler to make this a jump table
	#endif
		}
	}
#endif
}

