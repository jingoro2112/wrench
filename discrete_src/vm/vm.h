/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

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

#define WR_FUNCTION_CORE_SIZE 11
//------------------------------------------------------------------------------
struct WRFunction
{
	uint16_t namespaceOffset;
	uint16_t functionOffset;
	
	uint32_t hash;

	uint8_t arguments;
	uint8_t frameSpaceNeeded;
	uint8_t frameBaseAdjustment;
};

//------------------------------------------------------------------------------
enum WRContextFlags
{
	WRC_OwnsMemory = 1<<0, // if this is true, 'bottom' must be freed upon destruction
	WRC_ForceYielded = 1<<1, // if this is true, 'bottom' must be freed upon destruction
};

//------------------------------------------------------------------------------
struct WRContext
{
	uint16_t globals;

	uint32_t allocatedMemoryHint; // _approximately_ how much memory has been allocated since last gc
	
	const unsigned char* bottom;
	const unsigned char* codeStart;
	int32_t bottomSize;

	WRValue* stack;
	
	const unsigned char* stopLocation;
	
	WRGCBase* svAllocated;

#ifdef WRENCH_INCLUDE_DEBUG_CODE
	WRDebugServerInterface* debugInterface;
#endif

	WRState* w;

	WRGCObject registry; // the 'next' pointer in this registry is used as the context LL next

	const unsigned char* yield_pc;
	WRValue* yield_stackTop;
	WRValue* yield_frameBase;
	const WRValue* yield_argv;
	uint8_t yield_argn;
	uint8_t yieldArgs;
	uint8_t stackOffset;
	uint8_t flags;

	WRFunction* localFunctions;
	uint8_t numLocalFunctions;
	
	WRContext* imported; // linked list of contexts this one imported

	WRContext* nextStateContextLink;

	void mark( WRValue* s );
	void gc( WRValue* stackTop );
	
	WRGCObject* getSVA( int size, WRGCObjectType type, bool init );
};

//------------------------------------------------------------------------------
struct WRLibraryCleanup
{
	void (*cleanupFunction)(WRState *w, void* param);
	void* param;
	WRLibraryCleanup* next;
};

//------------------------------------------------------------------------------
struct WRState
{
#ifdef WRENCH_TIME_SLICES
	int instructionsPerSlice;
	int sliceInstructionCount;
	int yieldEnabled;
#endif
	
	WRContext* contextList;

	WRLibraryCleanup* libCleanupFunctions;
	
	WRGCObject globalRegistry;

	uint16_t allocatedMemoryLimit; // WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT by default
	uint16_t stackSize; // how much stack to give each context
	int8_t err;

};

void wr_addLibraryCleanupFunction( WRState* w, void(*function)(WRState *w, void* param), void* param );

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

void wr_doIndexHash( WRValue* index, WRValue* value, WRValue* target );
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

typedef void (*WRSingleTargetFunc)( WRValue* value, WRValue* target );
extern WRSingleTargetFunc wr_negate[4];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_preinc[4];
extern WRUnaryFunc wr_predec[4];
extern WRUnaryFunc wr_toInt[4];
extern WRUnaryFunc wr_toFloat[4];

typedef uint32_t (*WRUint32Call)( WRValue* value );
extern WRUint32Call wr_bitwiseNot[4];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[4];

void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table );

extern WRReturnFunc wr_CompareEQ[16];

uint32_t wr_hash_read8( const void* dat, const int len );
uint32_t wr_hashStr_read8( const char* dat );

bool wr_concatStringCheck( WRValue* to, WRValue* from, WRValue* target );
void wr_valueToEx( const WRValue* ex, WRValue* value );

#define WR_FLOATS_EQUAL(f1,f2) (fabsf((f1) - (f2)) <= (fabsf((f1)*.0000005f)));

// if the current + native match then great it's a simple read, it's
// only when they differ that we need bitshiftiness
#ifdef WRENCH_LITTLE_ENDIAN

 #define wr_x32(P) (P)
 #define wr_x16(P) (P)

#ifndef READ_32_FROM_PC
  #ifdef WRENCH_UNALIGNED_READS
    #define READ_32_FROM_PC(P) ((int32_t)(*(int32_t *)(P)))
  #else
    #define READ_32_FROM_PC(P) ((int32_t)((int32_t)*(P) | ((int32_t)*(P+1))<<8 | ((int32_t)*(P+2))<<16 | ((int32_t)*(P+3))<<24))
  #endif
#endif
#ifndef READ_16_FROM_PC
  #ifdef WRENCH_UNALIGNED_READS
    #define READ_16_FROM_PC(P) ((int16_t)(*(int16_t *)(P)))
  #else
    #define READ_16_FROM_PC(P) ((int16_t)((int16_t)*(P) | ((int16_t)*(P+1))<<8))
  #endif
#endif

#else

 int32_t wr_x32( const int32_t val );
 int16_t wr_x16( const int16_t val );

 #ifdef WRENCH_COMPACT

  #ifndef READ_32_FROM_PC
   int32_t READ_32_FROM_PC_func( const unsigned char* P );
   #define READ_32_FROM_PC(P) READ_32_FROM_PC_func(P)
  #endif
  #ifndef READ_16_FROM_PC
   int16_t READ_16_FROM_PC_func( const unsigned char* P );
   #define READ_16_FROM_PC(P) READ_16_FROM_PC_func(P)
  #endif
   
 #else

  #ifndef READ_32_FROM_PC
   #define READ_32_FROM_PC(P) ((int32_t)((int32_t)*(P) | ((int32_t)*(P+1))<<8 | ((int32_t)*(P+2))<<16 | ((int32_t)*(P+3))<<24))
  #endif
  #ifndef READ_16_FROM_PC
   #define READ_16_FROM_PC(P) ((int16_t)((int16_t)*(P) | ((int16_t)*(P+1))<<8))
  #endif
   
 #endif
#endif

 #ifndef READ_8_FROM_PC
  #define READ_8_FROM_PC(P) (*(P))
 #endif

#endif
