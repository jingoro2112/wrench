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

#ifndef _VM_H
#define _VM_H
/*------------------------------------------------------------------------------*/

int wr_itoa( int i, char* string, size_t len );
int wr_ftoa( float f, char* string, size_t len );

//------------------------------------------------------------------------------
struct WRFunction
{
	char arguments;
	char frameSpaceNeeded;
	char frameBaseAdjustment;
	uint32_t hash;
	union
	{
		const unsigned char* offset;
		int offsetI;
	};
};

//------------------------------------------------------------------------------
struct WRContext
{
	uint16_t gcPauseCount;
	uint8_t globals;

	union
	{
		WRFunction* localFunctions;
		uint32_t hashOffset;
	};
	
	const unsigned char* bottom;
	const unsigned char* stopLocation;
	WRGCObject* svAllocated;
	
	void mark( WRValue* s );
	void gc( WRValue* stackTop );
	WRGCObject* getSVA( int size, WRGCObjectType type, bool init );
	
	WRState* w;

	WRContext* next;
};

//------------------------------------------------------------------------------
struct WRState
{
	uint16_t stackSize;
	int8_t err;

	WRGCObject c_functionRegistry;

	WRValue* stack;

	WRContext* contextList;
};


void wr_arrayToValue( const WRValue* array, WRValue* value, int index =-1 );
void wr_intValueToArray( const WRValue* array, int32_t I );
void wr_floatValueToArray( const WRValue* array, float F );
void wr_countOfArrayElement( WRValue* array, WRValue* target );


typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[16];
extern WRVoidFunc wr_SubtractAssign[16];
extern WRVoidFunc wr_AddAssign[16];
extern WRVoidFunc wr_ModAssign[16];
extern WRVoidFunc wr_MultiplyAssign[16];
extern WRVoidFunc wr_DivideAssign[16];
extern WRVoidFunc wr_ORAssign[16];
extern WRVoidFunc wr_ANDAssign[16];
extern WRVoidFunc wr_XORAssign[16];
extern WRVoidFunc wr_RightShiftAssign[16];
extern WRVoidFunc wr_LeftShiftAssign[16];
extern WRVoidFunc wr_postinc[4];
extern WRVoidFunc wr_postdec[4];
extern WRVoidFunc wr_pushIterator[4];


typedef void (*WRVoidPlusFunc)( WRValue* to, WRValue* from, int add );
extern WRVoidPlusFunc m_unaryPost[4];


typedef int (*WRFuncIntCall)( int a, int b );
typedef float (*WRFuncFloatCall)( float a, float b );
typedef void (*WRTargetCallbackFunc)( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall I, WRFuncFloatCall F );
typedef void (*WRFuncAssignFunc)( WRValue* to, WRValue* from, WRFuncIntCall I, WRFuncFloatCall F );
extern WRFuncAssignFunc wr_FuncAssign[16];
extern WRTargetCallbackFunc wr_funcBinary[16];

typedef bool (*WRCompareFuncIntCall)( int a, int b );
typedef bool (*WRCompareFuncFloatCall)( float a, float b );
typedef bool (*WRBoolCallbackReturnFunc)( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall );
extern WRBoolCallbackReturnFunc wr_Compare[16];


typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_AdditionBinary[16];
extern WRTargetFunc wr_MultiplyBinary[16];
extern WRTargetFunc wr_SubtractBinary[16];
extern WRTargetFunc wr_DivideBinary[16];
extern WRTargetFunc wr_LeftShiftBinary[16];
extern WRTargetFunc wr_RightShiftBinary[16];
extern WRTargetFunc wr_ModBinary[16];
extern WRTargetFunc wr_ANDBinary[16];
extern WRTargetFunc wr_ORBinary[16];
extern WRTargetFunc wr_XORBinary[16];

typedef void (*WRStateFunc)( WRContext* c, WRValue* to, WRValue* from, WRValue* target );
extern WRStateFunc wr_index[16];
extern WRStateFunc wr_assignAsHash[4];
extern WRStateFunc wr_assignToArray[16];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[16];
extern WRReturnFunc wr_CompareGT[16];
extern WRReturnFunc wr_CompareLT[16];
extern WRReturnFunc wr_LogicalAND[16];
extern WRReturnFunc wr_LogicalOR[16];


typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_negate[4];
extern WRUnaryFunc wr_preinc[4];
extern WRUnaryFunc wr_predec[4];
extern WRUnaryFunc wr_toInt[4];
extern WRUnaryFunc wr_toFloat[4];

typedef void (*WRGetValueFunc)( WRValue** value );
extern WRGetValueFunc wr_getValue[4];


typedef uint32_t (*WRUint32Call)( WRValue* value );
extern WRUint32Call wr_bitwiseNot[4];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[4];

typedef void (*WRIndexHashFunc)( WRValue* value, WRValue* target, uint32_t hash );
extern WRIndexHashFunc wr_IndexHash[4];

void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table );
void wr_removeFromHashTable( WRContext* c, WRValue* index, WRValue* table );

extern WRReturnFunc wr_CompareEQ[16];

#ifdef WRENCH_COMPACT
int32_t READ_32_FROM_PC( const unsigned char* P );
int16_t READ_16_FROM_PC( const unsigned char* P );
#else
#define READ_32_FROM_PC(P) ((((int32_t)*(P)) << 24) | (((int32_t)*((P)+1)) << 16) | (((int32_t)*((P)+2)) << 8) | ((int32_t)*((P)+3))) 
#define READ_16_FROM_PC(P) ((int16_t)((((int16_t)*(P)) << 8) | ((int16_t)*(P+1))))
#endif

#endif
