#include "wrench.h"
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
#ifndef _UTILS_H
#define _UTILS_H
/*------------------------------------------------------------------------------*/

#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

//-----------------------------------------------------------------------------
template <class T> class WRarray
{
private:

	T* m_list;
	unsigned int m_elementsTotal;
	unsigned int m_elementsAllocated;

public:

	//------------------------------------------------------------------------------
	void clear() { delete[] m_list; m_list = 0; m_elementsTotal = 0; m_elementsAllocated = 0; }
	unsigned int count() const { return m_elementsAllocated; }
	void setCount( unsigned int count )
	{
		if ( count >= m_elementsTotal )
		{
			alloc( count + 1 ); 
		}
		m_elementsAllocated = count; 
	}

	//------------------------------------------------------------------------------
	unsigned int remove( unsigned int location, unsigned int count =1 )
	{
		if ( location >= m_elementsAllocated )
		{
			return m_elementsAllocated;
		}

		unsigned int newCount;
		if ( count > m_elementsAllocated )
		{
			if ( location == 0 )
			{
				clear();
				return 0;
			}
			else
			{
				newCount = location + 1;
			}
		}
		else
		{
			newCount = m_elementsAllocated - count;
		}

		T* newArray = new T[newCount];

		if ( location == 0 )
		{
			for( unsigned int i=0; i<(unsigned int)newCount; ++i )
			{
				newArray[i] = m_list[i+count];
			}
		}
		else
		{
			unsigned int j = 0;
			
			unsigned int skipEnd = location + count;
			for( unsigned int i=0; i<m_elementsAllocated; ++i )
			{
				if ( i < location )
				{
					newArray[j++] = m_list[i];
				}
				else if ( i >= skipEnd )
				{
					newArray[j++] = m_list[i];
				}
			}
		}

		delete[] m_list;
		m_list = newArray;
		m_elementsAllocated = newCount;
		m_elementsTotal = newCount;

		return m_elementsAllocated;
	}

	//------------------------------------------------------------------------------
	void alloc( const unsigned int size )
	{
		if ( size > m_elementsTotal )
		{
			int newSize = size + (size/2) + 1;
			T* newArray = new T[newSize];

			if ( m_list )
			{
				for( unsigned int i=0; i<m_elementsTotal; ++i )
				{
					newArray[i] = m_list[i];
				}

				delete[] m_list;
			}

			m_elementsTotal = newSize;
			m_list = newArray;
		}
	}

	//------------------------------------------------------------------------------
	T* push() { return &get( m_elementsAllocated ); }
	T* tail() { return m_elementsAllocated ? m_list + (m_elementsAllocated - 1) : 0; }
	void pop() { if ( m_elementsAllocated > 0 ) { --m_elementsAllocated; }  }

	T& append() { return get( m_elementsAllocated ); }
	T& get( const unsigned int l ) 
	{
		if ( l >= m_elementsTotal )
		{
			alloc(l + 1);
			m_elementsAllocated = l + 1;
		}
		else if ( l >= m_elementsAllocated )
		{
			m_elementsAllocated = l + 1;
		}
		
		return m_list[l];
	}

	//------------------------------------------------------------------------------
	WRarray( const WRarray& A )
	{
		clear();
		m_list = new T[A.m_elementsAllocated];
		for( unsigned int i=0; i<A.m_elementsAllocated; ++i )
		{
			m_list[i] = A.m_list[i];
		}
		m_elementsAllocated = A.m_elementsAllocated;
		m_elementsTotal = A.m_elementsTotal;
	}

	//------------------------------------------------------------------------------
	WRarray& operator= ( const WRarray& A )
	{
		if ( &A != this )
		{
			clear();
			m_list = new T[A.m_elementsTotal];
			for( unsigned int i=0; i<A.m_elementsAllocated; ++i )
			{
				m_list[i] = A.m_list[i];
			}
			m_elementsAllocated = A.m_elementsAllocated;
			m_elementsTotal = A.m_elementsTotal;
		}
		return *this;
	}

	const T& operator[]( const unsigned int l ) const { return get(l); }
	T& operator[]( const unsigned int l ) { return get(l); }

	WRarray( const unsigned int initialSize =0 ) { m_list = 0; clear(); alloc(initialSize); }
	~WRarray() { delete[] m_list; }
};
class WRstr;
const char* wr_asciiDump( const void* d, unsigned int len, WRstr* str =0 );

#endif // WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
const int32_t c_primeTable[] =
{
	2,
	5,
	11,
	17,
	23,
	31,
	53,
	97,
	193,
	389,
	769,
	1543,
	3079,
#ifndef UNLIMITED_HASH_SIZE
	0
#else

	// what are you doing here? This is a tiny embedded scripting
	// language.. if you have RAM to burn then use something mature
	// like lua

	6151,  
	12289, // use wren
	24593,
	49157, // use python
	98317,
	196613, // use java
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917, // use c#
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457, 
	1610612741, // use HAL-9000
#endif
};

//------------------------------------------------------------------------------
template <class T> class WRHashTable
{
public:
	WRHashTable() : m_list(0) { clear(); }
	~WRHashTable() { delete[] m_list; }

	//------------------------------------------------------------------------------
	void clear()
	{
		delete[] m_list;
		m_mod = c_primeTable[0];
		m_list = new Node[m_mod];
	}

	//------------------------------------------------------------------------------
	T* get( uint32_t hash )
	{
		Node& N = m_list[hash % m_mod];
		return (N.hash == hash) ? &N.value : 0;
	}

	//------------------------------------------------------------------------------
	T getItem( uint32_t hash )
	{
		Node& N = m_list[hash % m_mod];
		return (N.hash == hash) ? N.value : 0;
	}

	//------------------------------------------------------------------------------
	void remove( uint32_t hash )
	{
		uint32_t key = hash % m_mod;
		if ( m_list[key].hash == hash ) // at least CHECK.. 
		{
			m_list[key].hash = 0;
		}
	}

	//------------------------------------------------------------------------------
	bool set( uint32_t hash, T const& value )
	{
		uint32_t key = hash % m_mod;
		// clobber on collide, assume the user knows what they are doing
		if ( (m_list[key].hash == 0) || (m_list[key].hash == hash) )
		{
			m_list[key].hash = hash;
			m_list[key].value = value;
			return true;
		}

		// otherwise there was a collision, expand the table
		int newMod = m_mod;
		for(;;)
		{
			for( int t=0; c_primeTable[t]; ++t )
			{
				if ( c_primeTable[t] == newMod )
				{
					newMod = c_primeTable[t + 1];
					break;
				}
			}

			if ( newMod == 0 ) // should not be trying past this point on a memory fotprint this small, just use lua!
			{
				return false;
			}

			Node* newList = new Node[newMod];


			int h = 0;
			for( ; h<m_mod; ++h )
			{
				if ( !m_list[h].hash )
				{
					continue;
				}

				if ( newList[m_list[h].hash % newMod].hash )
				{
					break;
				}

				else
				{
					newList[m_list[h].hash % newMod] = m_list[h];
				}
			}

			if ( h >= m_mod )
			{
				m_mod = newMod;
				delete[] m_list;
				m_list = newList;
				return set( hash, value );
			}

			delete[] newList; // try again
		}
	}

	struct Node
	{
		T value;
		uint32_t hash;
		Node() { hash = 0; }
	};

private:
	Node* m_list;
	int m_mod;
};

#endif
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

#ifndef _VM_H
#define _VM_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
class WRUserData
{
public:

	//------------------------------------------------------------------------------
	void registerValue( const char* key, WRValue* value )
	{
		UDNode* node = new UDNode;
		node->val = value;
		node->next = m_nodeOnlyHead;
		m_nodeOnlyHead = node;

		m_index.set( wr_hashStr(key), node );
	}
	
	//------------------------------------------------------------------------------
	WRValue* addValue( const char* key )
	{
		UDNode* node = new UDNode;
		node->val = new WRValue;
		node->val->type = 0;
		node->next = m_head;
		m_head = node;
		m_index.set( wr_hashStr(key), node );
		return node->val;
	}
	
	//------------------------------------------------------------------------------
	WRValue* get( const char* key ) { UDNode* N = m_index.getItem( wr_hashStr(key) ); return N ? N->val : 0; }
	WRValue* get( const int32_t hash ) { UDNode* N = m_index.getItem(hash); return N ? N->val : 0; }

	WRUserData() : m_head(0), m_nodeOnlyHead(0) {}
	~WRUserData()
	{
		while( m_head )
		{
			UDNode* next = m_head->next;
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

private:
	struct UDNode
	{
		WRValue* val;
		UDNode* next;
	};
	UDNode* m_head;
	UDNode* m_nodeOnlyHead;
	WRHashTable<UDNode*> m_index;
};

//------------------------------------------------------------------------------
struct WRFunctionRegistry
{
	union
	{
		const unsigned char* offset;
		int offsetI;
	};
	
	uint32_t hash;
	char arguments;
	char frameSpaceNeeded;
	char frameBaseAdjustment;
};

//------------------------------------------------------------------------------
struct WRCFunctionCallback
{
	WR_C_CALLBACK function;
	void* usr;
};

//------------------------------------------------------------------------------
class WRGCValueArray;
enum WRStaticValueArrayType
{
	SV_VALUE = 0x00,
	SV_CHAR = 0x01,
	SV_INT = 0x02,
	SV_FLOAT = 0x03,

	SV_PRE_ALLOCATED = 0x10,
};

//------------------------------------------------------------------------------
struct WRRunContext
{
	WRFunctionRegistry* localFunctions;
	WRHashTable<WRFunctionRegistry*> localFunctionRegistry;
	WRValue* globalSpace;
	int globals;

	const unsigned char* bottom;
	int32_t stopLocation;

	WRStaticValueArray* svAllocated;
	WRStaticValueArray* getSVA( int size, WRStaticValueArrayType type = SV_VALUE );
	void gcArray( WRStaticValueArray* sva );

	WRState* w;
	
	WRRunContext* next;
	
	WRRunContext( WRState* state );
	~WRRunContext();
};

//------------------------------------------------------------------------------
struct WRState
{
	int contextIdGenerator;
	WRHashTable<WRRunContext*> contexts;

	WRHashTable<WRCFunctionCallback> c_functionRegistry;

	WRError err;

	WRValue* stack;
	int stackSize;
	WRValue* stackTop;
	
	WR_LOAD_BLOCK_FUNC loader;
	void* usr;
	
	WRValue* returnValue;
	WRRunContext* contextList;

	WRState( int EntriesInStack =DEFAULT_STACK_SIZE );
	~WRState();
};

//------------------------------------------------------------------------------
class WRStaticValueArray
{
public:

	WRStaticValueArray* m_next;
	unsigned int m_size;
	char m_type;
	const void* m_data;
	
	//------------------------------------------------------------------------------
	WRStaticValueArray( const unsigned int size,
						const WRStaticValueArrayType type,
						const void* preAlloc =0 )
	{
		m_type = (char)type;
		m_size = size;
		if ( preAlloc )
		{
			m_type |= (char)SV_PRE_ALLOCATED;
			m_data = preAlloc;
		}
		else
		{
			switch( type )
			{
				case SV_VALUE: { m_data = new WRValue[size]; break; }
				case SV_CHAR: { m_data = new char[size]; break; }
				case SV_INT: { m_data = new int[size]; break; }
				case SV_FLOAT: { m_data = new float[size]; break; }
				default: m_data = 0;
			}
		}
	}
	
	//------------------------------------------------------------------------------
	~WRStaticValueArray()
	{
		if ( !(m_type & (char)SV_PRE_ALLOCATED) )
		{
			switch( m_type & 0x3 )
			{
				case SV_VALUE: { delete[] (WRValue *)m_data; break; }
				case SV_CHAR: { delete[] (char *)m_data; break; }
				case SV_INT: { delete[] (int *)m_data; break; }
				case SV_FLOAT: { delete[] (float *)m_data; break; }
				default: break;
			}
		}
	}
	
	//------------------------------------------------------------------------------
	void* operator[]( const unsigned int l ) { return get( l ); }
		
	void* get( const unsigned int l ) const
	{
		int s = l < m_size ? l : m_size - 1;

		switch( m_type & 0x3 )
		{
			case SV_VALUE: { return ((WRValue *)m_data) + s; }
			case SV_CHAR: { return ((char *)m_data) + s; }
			case SV_INT: { return ((int *)m_data) + s; }
			case SV_FLOAT: { return ((float *)m_data) + s; }
			default: return 0;
		}
	}
};

void arrayToValue( const WRValue* array, WRValue* value );
void intValueToArray( const WRValue* array, int32_t I );
void floatValueToArray( const WRValue* array, float F );

typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[5][5];
extern WRVoidFunc wr_SubtractAssign[5][5];
extern WRVoidFunc wr_AddAssign[5][5];
extern WRVoidFunc wr_ModAssign[5][5];
extern WRVoidFunc wr_MultiplyAssign[5][5];
extern WRVoidFunc wr_DivideAssign[5][5];
extern WRVoidFunc wr_ORAssign[5][5];
extern WRVoidFunc wr_ANDAssign[5][5];
extern WRVoidFunc wr_XORAssign[5][5];
extern WRVoidFunc wr_RightShiftAssign[5][5];
extern WRVoidFunc wr_LeftShiftAssign[5][5];


typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_binaryAddition[5][5];
extern WRTargetFunc wr_binaryMultiply[5][5];
extern WRTargetFunc wr_binarySubtract[5][5];
extern WRTargetFunc wr_binaryDivide[5][5];
extern WRTargetFunc wr_binaryLeftShift[5][5];
extern WRTargetFunc wr_binaryRightShift[5][5];
extern WRTargetFunc wr_binaryMod[5][5];
extern WRTargetFunc wr_binaryAnd[5][5];
extern WRTargetFunc wr_binaryOr[5][5];
extern WRTargetFunc wr_binaryXOR[5][5];

typedef void (*WRStateFunc)( WRRunContext* c, WRValue* to, WRValue* from );
extern WRStateFunc wr_index[5][5];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[5][5];
extern WRReturnFunc wr_CompareGT[5][5];
extern WRReturnFunc wr_CompareLT[5][5];
extern WRReturnFunc wr_LogicalAnd[5][5];
extern WRReturnFunc wr_LogicalOr[5][5];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_BitwiseNot[5];
extern WRUnaryFunc wr_negate[5];
extern WRUnaryFunc wr_preinc[5];
extern WRUnaryFunc wr_predec[5];
extern WRVoidFunc wr_postinc[5];
extern WRVoidFunc wr_postdec[5];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[5];

typedef bool (*WRValueCheckFunc)( WRValue* value );
extern WRValueCheckFunc wr_ZeroCheck[5];

typedef void (*WRUserHashFunc)( WRValue* value, WRValue* target, int32_t hash );
extern WRUserHashFunc wr_UserHash[5];


#endif
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
#ifndef _OPCODE_H
#define _OPCODE_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROpcode
{
	O_RegisterFunction = 0,
	O_FunctionListSize,
	
	O_LiteralZero,
	O_LiteralInt8,
	O_LiteralInt32,
	O_LiteralFloat,
	O_LiteralString,

	O_LoadLabel, // [label key:] push this label onto the stack
	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,

	O_Index,
	O_IndexHash,
	
	O_Assign,
	O_AssignAndPop,
	O_StackSwap,

	O_ReserveFrame,
	O_ReserveGlobalFrame,

	O_LoadFromLocal,
	O_LoadFromGlobal,

	O_PopOne, // pop exactly one value from stack
	O_Return, // end of a unit
	O_Stop, // end of a program

	O_BinaryAddition,
	O_BinarySubtraction,
	O_BinaryMultiplication,
	O_BinaryDivision,
	O_BinaryRightShift,
	O_BinaryLeftShift,
	O_BinaryMod,
	O_BinaryAnd,
	O_BinaryOr,
	O_BinaryXOR,
	O_BitwiseNOT,

	O_CoerceToInt,
	O_CoerceToString,
	O_CoerceToFloat,

	O_RelativeJump,
	O_BZ,
	O_BNZ,

	O_CompareEQ, 
	O_CompareNE, 
	O_CompareGE,
	O_CompareLE,
	O_CompareGT,
	O_CompareLT,

	O_PostIncrement, // a++
	O_PostDecrement, // a--
	O_PreIncrement, // ++a
	O_PreDecrement, // ++a
	O_Negate, // -a

	O_SubtractAssign,
	O_AddAssign,
	O_ModAssign,
	O_MultiplyAssign,
	O_DivideAssign,
	O_ORAssign,
	O_ANDAssign,
	O_XORAssign,
	O_RightShiftAssign,
	O_LeftShiftAssign,

	O_SubtractAssignAndPop,
	O_AddAssignAndPop,
	O_ModAssignAndPop,
	O_MultiplyAssignAndPop,
	O_DivideAssignAndPop,
	O_ORAssignAndPop,
	O_ANDAssignAndPop,
	O_XORAssignAndPop,
	O_RightShiftAssignAndPop,
	O_LeftShiftAssignAndPop,

	O_LogicalAnd, // &&
	O_LogicalOr, // ||
	O_LogicalNot, // !


	O_LAST,
};

//#define DEBUG_OPCODE_NAMES
#ifdef DEBUG_OPCODE_NAMES
#define D_OPCODE(a) a
const char* c_opcodeName[]=
{
	"O_RegisterFunction",
	"O_FunctionListSize",
	"O_LiteralZero",
	"O_LiteralInt8",
	"O_LiteralInt32",
	"O_LiteralFloat",
	"O_LiteralString",
	"O_LoadLabel",
	"O_CallFunctionByHash",
	"O_CallFunctionByHashAndPop",
	"O_CallFunctionByIndex",
	"O_Index",
	"O_IndexHash",
	"O_Assign",
	"O_AssignAndPop",
	"O_StackSwap",
	"O_ReserveFrame",
	"O_ReserveGlobalFrame",
	"O_LoadFromLocal",
	"O_LoadFromGlobal",
	"O_PopOne",
	"O_Return",
	"O_Stop",
	"O_BinaryAddition",
	"O_BinarySubtraction",
	"O_BinaryMultiplication",
	"O_BinaryDivision",
	"O_BinaryRightShift",
	"O_BinaryLeftShift",
	"O_BinaryMod",
	"O_BinaryAnd",
	"O_BinaryOr",
	"O_BinaryXOR",
	"O_BitwiseNOT",
	"O_CoerceToInt",
	"O_CoerceToString",
	"O_CoerceToFloat",
	"O_RelativeJump",
	"O_BZ",
	"O_BNZ",
	"O_CompareEQ",
	"O_CompareNE",
	"O_CompareGE",
	"O_CompareLE",
	"O_CompareGT",
	"O_CompareLT",
	"O_PostIncrement",
	"O_PostDecrement",
	"O_PreIncrement",
	"O_PreDecrement",
	"O_Negate",
	"O_SubtractAssign",
	"O_AddAssign",
	"O_ModAssign",
	"O_MultiplyAssign",
	"O_DivideAssign",
	"O_ORAssign",
	"O_ANDAssign",
	"O_XORAssign",
	"O_RightShiftAssign",
	"O_LeftShiftAssign",
	"O_SubtractAssignAndPop",
	"O_AddAssignAndPop",
	"O_ModAssignAndPop",
	"O_MultiplyAssignAndPop",
	"O_DivideAssignAndPop",
	"O_ORAssignAndPop",
	"O_ANDAssignAndPop",
	"O_XORAssignAndPop",
	"O_RightShiftAssignAndPop",
	"O_LeftShiftAssignAndPop",
	"O_LogicalAnd",
	"O_LogicalOr",
	"O_LogicalNot",
};
#else
#define D_OPCODE(a)
#endif

#endif
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
#ifndef _STR_H
#define _STR_H
/* ------------------------------------------------------------------------- */
#ifndef WRENCH_WITHOUT_COMPILER

#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning (disable : 4996) // remove Windows nagging
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef STR_FILE_OPERATIONS
#include <sys/stat.h>
#endif

const unsigned int c_sizeofBaseString = 15; // this lib tries not to use dynamic RAM unless it has to
const int c_cstrFormatBufferSize = 1024; // when formatting, this much stack space is reserved during the call

//-----------------------------------------------------------------------------
class WRstr
{
public:
	WRstr() { m_len = 0; m_smallbuf[0] = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; }
	WRstr( const WRstr& str) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(str, str.size()); } 
	WRstr( const WRstr* str ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; if ( str ) { set(*str, str->size()); } } 
	WRstr( const char* s, const unsigned int len ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, len); }
	WRstr( const char* s ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, strlen(s)); }
	WRstr( const char c) { m_len = 1; m_str = m_smallbuf; m_smallbuf[0] = c; m_smallbuf[1] = 0; m_buflen = c_sizeofBaseString; }
	
	~WRstr() { if ( m_str != m_smallbuf ) delete[] m_str; }

	WRstr& clear() { m_len = 0; m_str[0] = 0; return *this; }
	
	static WRstr sprintf( const char* format, ... ) { WRstr T; va_list arg; va_start(arg, format); T.formatVA(format, arg); va_end(arg); return T; }
	WRstr& format( const char* format, ... ) { va_list arg; va_start( arg, format ); clear(); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	WRstr& formatVA( const char* format, va_list arg ) { clear(); return appendFormatVA(format, arg); }
	WRstr& appendFormat( const char* format, ... ) { va_list arg; va_start( arg, format ); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	WRstr& appendFormatVA( const char* format, va_list arg )
	{
		char buf[ c_cstrFormatBufferSize + 1 ];
		int len = vsnprintf( buf, c_cstrFormatBufferSize, format, arg );
		if ( len > 0 ) insert( buf, (unsigned int)len, m_len );
		return *this;
	}

	unsigned int release( char** toBuf )
	{
		unsigned int retLen = m_len;

		if ( m_str == m_smallbuf )
		{
			*toBuf = new char[m_len + 1];
			memcpy( *toBuf, m_str, (m_len+1) );
		}
		else
		{
			*toBuf = m_str;
			m_str = m_smallbuf;
			m_buflen = c_sizeofBaseString;
		}
		
		clear();
		return retLen;
	}

	inline WRstr& trim();

	inline WRstr& alloc( const unsigned int characters, const bool preserveContents =true ); 

	unsigned int size() const { return m_len; } // see length

	const char* c_str( const unsigned int offset =0 ) const { return m_str + offset; }
	char* p_str( const unsigned int offset =0 ) const { return m_str + offset; }

	operator const void*() const { return m_str; }
	operator const char*() const { return m_str; }

#ifdef STR_FILE_OPERATIONS
	inline bool fileToBuffer( const char* fileName, const bool appendToBuffer =false );
	inline bool bufferToFile( const char* fileName, const bool append =false ) const;
#endif

	WRstr& set( const char* buf, const unsigned int len ) { m_len = 0; m_str[0] = 0; return insert( buf, len ); }
	WRstr& set( const WRstr& str ) { return set( str.m_str, str.m_len ); }
	WRstr& set( const char c ) { clear(); m_str[0]=c; m_str[1]=0; m_len = 1; return *this; }

	inline WRstr& truncate( const unsigned int newLen ); // reduce size to 'newlen'
	WRstr& shave( const unsigned int e ) { return (e > m_len) ? clear() : truncate(m_len - e); } // remove 'x' trailing characters
	
	// memmove from sourcePosition to end of string down to the start of the string
	
	inline bool isMatch( const char* buf ) const;

	inline WRstr& insert( const char* buf, const unsigned int len, const unsigned int startPos =0 );
	inline WRstr& insert( const WRstr& s, const unsigned int startPos =0 ) { return insert(s.m_str, s.m_len, startPos); }
	
	inline WRstr& append( const char* buf, const unsigned int len ) { return insert(buf, len, m_len); } 
	inline WRstr& append( const char c );
	inline WRstr& append( const WRstr& s ) { return insert(s.m_str, s.m_len, m_len); }

	// define the usual suspects:
	
	const char& operator[]( const int l ) const { return get((unsigned int)l); }
	const char& operator[]( const unsigned int l ) const  { return get(l); }
	char& operator[]( const int l )  { return get((unsigned int)l); }
	char& operator[]( const unsigned int l ) { return get(l); }

	char& get( const unsigned int l )
	{
		assert( l < m_buflen );
		return m_str[l];
	}

	const char& get( const unsigned int l ) const
	{
		assert( l < m_buflen );
		return m_str[l];
	}
	
	WRstr& operator += ( const WRstr& str ) { return append(str.m_str, str.m_len); }
	WRstr& operator += ( const char* s ) { return append(s, strlen(s)); }
	WRstr& operator += ( const char c ) { return append(c); }

	WRstr& operator = ( const WRstr& str ) { if ( &str != this ) set(str, str.size()); return *this; }
	WRstr& operator = ( const WRstr* str ) { if ( !str ) { clear(); } else if ( this != this ) { set(*str, str->size()); } return *this; }
	WRstr& operator = ( const char* c ) { set(c, strlen(c)); return *this; }
	WRstr& operator = ( const char c ) { set(&c, 1); return *this; }

	friend bool operator == ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len == s2.m_len && (strncmp(s1.m_str, s2.m_str, s1.m_len) == 0); }
	friend bool operator == ( const char* z, const WRstr& s ) { return s.isMatch( z ); }
	friend bool operator == ( const WRstr& s, const char* z ) { return s.isMatch( z ); }
	friend bool operator != ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len != s2.m_len || (strncmp(s1.m_str, s2.m_str, s1.m_len) != 0); }
	friend bool operator != ( const WRstr& s, const char* z ) { return !s.isMatch( z ); }
	friend bool operator != ( const char* z, const WRstr& s ) { return !s.isMatch( z ); }

	friend WRstr operator + ( const WRstr& str, const char* s) { WRstr T(str); T += s; return T; }
	friend WRstr operator + ( const WRstr& str, const char c) { WRstr T(str); T += c; return T; }
	friend WRstr operator + ( const char* s, const WRstr& str ) { WRstr T(s, strlen(s)); T += str; return T; }
	friend WRstr operator + ( const char c, const WRstr& str ) { WRstr T(c); T += str; return T; }
	friend WRstr operator + ( const WRstr& str1, const WRstr& str2 ) { WRstr T(str1); T += str2; return T; }

protected:

	operator char*() const { return m_str; } // prevent accidental use

	char *m_str; // first element so if the class is cast as a C and de-referenced it always works
	
	unsigned int m_buflen; // how long the buffer itself is
	unsigned int m_len; // how long the string is in the buffer
	char m_smallbuf[ c_sizeofBaseString + 1 ]; // small temporary buffer so a new/delete is not imposed for small strings
};

//-----------------------------------------------------------------------------
WRstr& WRstr::trim()
{
	unsigned int start = 0;

	// find start
	for( ; start<m_len && isspace( (unsigned char)*(m_str + start) ) ; start++ );

	// is the whole thing whitespace?
	if ( start == m_len )
	{
		clear();
		return *this;
	}

	// copy down the characters one at a time, noting the last
	// non-whitespace character position, which will become the length
	unsigned int pos = 0;
	unsigned int marker = start;
	for( ; start<m_len; start++,pos++ )
	{
		if ( !isspace((unsigned char)(m_str[pos] = m_str[start])) )
		{
			marker = pos;
		}
	}

	m_len = marker + 1;
	m_str[m_len] = 0;

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::alloc( const unsigned int characters, const bool preserveContents )
{
	if ( characters >= m_buflen ) // only need to alloc if more space is requested than we have
	{
		char* newStr = new char[ characters + 1 ]; // create the space
		
		if ( preserveContents ) 
		{
			memcpy( newStr, m_str, m_buflen ); // preserve whatever we had
		}
		
		if ( m_str != m_smallbuf )
		{
			delete[] m_str;
		}
		
		m_str = newStr;
		m_buflen = characters;		
	}

	return *this;
}

#ifdef STR_FILE_OPERATIONS
//-----------------------------------------------------------------------------
bool WRstr::fileToBuffer( const char* fileName, const bool appendToBuffer )
{
	if ( !fileName )
	{
		return false;
	}

#ifdef _WIN32
	struct _stat sbuf;
	int ret = _stat( fileName, &sbuf );
#else
	struct stat sbuf;
	int ret = stat( fileName, &sbuf );
#endif

	if ( ret != 0 )
	{
		return false;
	}

	FILE *infil = fopen( fileName, "rb" );
	if ( !infil )
	{
		return false;
	}
	
	if ( appendToBuffer )
	{
		alloc( sbuf.st_size + m_len, true );
		m_str[ sbuf.st_size + m_len ] = 0;
		ret = (int)fread( m_str + m_len, sbuf.st_size, 1, infil );
		m_len += sbuf.st_size;
	}
	else
	{
		alloc( sbuf.st_size, false );
		m_len = sbuf.st_size;
		m_str[ m_len ] = 0;
		ret = (int)fread( m_str, m_len, 1, infil );
	}

	fclose( infil );
	return ret == 1;
}

//-----------------------------------------------------------------------------
bool WRstr::bufferToFile( const char* fileName, const bool append) const
{
	if ( !fileName )
	{
		return false;
	}

	FILE *outfil = append ? fopen( fileName, "a+b" ) : fopen( fileName, "wb" );
	if ( !outfil )
	{
		return false;
	}

	int ret = (int)fwrite( m_str, m_len, 1, outfil );
	fclose( outfil );

	return (m_len == 0) || (ret == 1);
}
#endif

//-----------------------------------------------------------------------------
WRstr& WRstr::truncate( const unsigned int newLen )
{
	if ( newLen >= m_len )
	{
		return *this;
	}

	if ( newLen < c_sizeofBaseString )
	{
		if ( m_str != m_smallbuf )
		{
			m_buflen = c_sizeofBaseString;
			memcpy( m_smallbuf, m_str, newLen );
			delete[] m_str;
			m_str = m_smallbuf;
		}
	}

	m_str[ newLen ] = 0;
	m_len = newLen;

	return *this;
}

//-----------------------------------------------------------------------------
bool WRstr::isMatch( const char* buf ) const
{
	return strcmp( buf, m_str ) == 0;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::insert( const char* buf, const unsigned int len, const unsigned int startPos /*=0*/ )
{
	if ( len == 0 ) // insert 0? done
	{
		return *this;
	}

	alloc( m_len + len, true ); // make sure there is enough room for the new string
	if ( startPos >= m_len )
	{
		if ( buf )
		{
			memcpy( m_str + m_len, buf, len );
		}
	}
	else
	{
		if ( startPos != m_len )
		{
			memmove( m_str + len + startPos, m_str + startPos, m_len );
		}
		
		if ( buf )
		{
			memcpy( m_str + startPos, buf, len );
		}
	}

	m_len += len;
	m_str[m_len] = 0;

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::append( const char c )
{
	if ( (m_len+1) >= m_buflen )
	{
		alloc( ((m_len * 3) / 2) + 1, true ); // single-character, expect a lot more are coming so alloc some buffer space
	}
	m_str[ m_len++ ] = c;
	m_str[ m_len ] = 0;

	return *this;
}

#endif
#endif
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
#ifndef _CC_H
#define _CC_H
#ifndef WRENCH_WITHOUT_COMPILER
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROperationType
{
	WR_OPER_PRE,
	WR_OPER_BINARY,
	WR_OPER_BINARY_COMMUTE, // binary but operation order doesn't matter
	WR_OPER_POST,
};

//------------------------------------------------------------------------------
struct WROperation
{
	const char* token;
	int precedence; // higher number if lower precedence
	WROpcode opcode;
	bool leftToRight;
	WROperationType type;
};

//------------------------------------------------------------------------------
// reference:
// https://en.cppreference.com/w/cpp/language/operator_precedence
const WROperation c_operations[] =
{
//       precedence                      L2R      type

	{ "==",  10, O_CompareEQ,           true,  WR_OPER_BINARY_COMMUTE },
	{ "!=",  10, O_CompareNE,           true,  WR_OPER_BINARY },
	{ ">=",   9, O_CompareGE,           true,  WR_OPER_BINARY },
	{ "<=",   9, O_CompareLE,           true,  WR_OPER_BINARY },
	{ ">",    9, O_CompareGT,           true,  WR_OPER_BINARY },
	{ "<",    9, O_CompareLT,           true,  WR_OPER_BINARY },
	{ "&&",  14, O_LogicalAnd,          true,  WR_OPER_BINARY_COMMUTE },
	{ "||",  15, O_LogicalOr,           true,  WR_OPER_BINARY_COMMUTE },

	{ "++",   3, O_PreIncrement,        true,  WR_OPER_PRE },
	{ "++",   2, O_PostIncrement,       true,  WR_OPER_POST },

	{ "--",   3, O_PreDecrement,        true,  WR_OPER_PRE },
	{ "--",   2, O_PostDecrement,       true,  WR_OPER_POST },

	{ ".",    2, O_IndexHash,           true,  WR_OPER_BINARY },

	{ "!",    3, O_LogicalNot,         false,  WR_OPER_PRE },
	{ "~",    3, O_BitwiseNOT,         false,  WR_OPER_PRE },
	{ "-",    3, O_Negate,             false,  WR_OPER_PRE },

	{ "+",    6, O_BinaryAddition,      true,  WR_OPER_BINARY_COMMUTE },
	{ "-",    6, O_BinarySubtraction,   true,  WR_OPER_BINARY },
	{ "*",    5, O_BinaryMultiplication,true,  WR_OPER_BINARY_COMMUTE },
	{ "/",    5, O_BinaryDivision,      true,  WR_OPER_BINARY },
	{ "%",    6, O_BinaryMod,           true,  WR_OPER_BINARY },

	{ "|",   13, O_BinaryOr,            true,  WR_OPER_BINARY_COMMUTE },
	{ "&",   11, O_BinaryAnd,           true,  WR_OPER_BINARY_COMMUTE },
	{ "^",   11, O_BinaryXOR,           true,  WR_OPER_BINARY_COMMUTE },

	{ ">>",   7, O_BinaryRightShift,    true,  WR_OPER_BINARY },
	{ "<<",   7, O_BinaryLeftShift,     true,  WR_OPER_BINARY },

	{ "+=",  16, O_AddAssign,           true,  WR_OPER_BINARY },
	{ "-=",  16, O_SubtractAssign,      true,  WR_OPER_BINARY },
	{ "%=",  16, O_ModAssign,           true,  WR_OPER_BINARY },
	{ "*=",  16, O_MultiplyAssign,      true,  WR_OPER_BINARY },
	{ "/=",  16, O_DivideAssign,        true,  WR_OPER_BINARY },
	{ "|=",  16, O_ORAssign,            true,  WR_OPER_BINARY },
	{ "&=",  16, O_ANDAssign,           true,  WR_OPER_BINARY },
	{ "^=",  16, O_XORAssign,           true,  WR_OPER_BINARY },
	{ ">>=", 16, O_RightShiftAssign,   false,  WR_OPER_BINARY },
	{ "<<=", 16, O_LeftShiftAssign,    false,  WR_OPER_BINARY },

	{ "=",   16, O_Assign,             false,  WR_OPER_BINARY },

	{ "@i",   3, O_CoerceToInt,         true,  WR_OPER_PRE },
	{ "@f",   3, O_CoerceToFloat,       true,  WR_OPER_PRE },
	{ "@[]",  2, O_Index,               true,  WR_OPER_POST },

	{ 0, 0, O_LAST, false, WR_OPER_PRE },
};
const int c_highestPrecedence = 17; // one higher than the highest entry above, things that happen absolutely LAST

//------------------------------------------------------------------------------
enum WRExpressionType
{
	EXTYPE_NONE =0,
	EXTYPE_LITERAL,
	EXTYPE_LABEL,
	EXTYPE_OPERATION,
	EXTYPE_RESOLVED,
	EXTYPE_BYTECODE_RESULT,

	EXTYPE_LVALUE = 0x1000, // LValue can be ANDED with anything to indicate it is a reference
};

//------------------------------------------------------------------------------
struct WRNamespaceLookup
{
	uint32_t hash; // hash of symbol
	WRarray<int> references; // where this symbol is referenced (loaded) in the bytecode

	WRNamespaceLookup() { reset(0); }
	void reset( uint32_t h )
	{
		hash = h;
		references.clear();
	}
};

//------------------------------------------------------------------------------
struct BytecodeJumpOffset
{
	int offset;
	WRarray<int> references;
};

//------------------------------------------------------------------------------
struct WRBytecode
{
	WRstr all;
	WRstr opcodes;
	WRarray<WRNamespaceLookup> localSpace;
	WRarray<WRNamespaceLookup> functionSpace;

	void invalidateOpcodeCache() { opcodes.clear(); }
	
	WRarray<BytecodeJumpOffset> jumpOffsetTargets;
	
	void clear() { all.clear(); opcodes.clear(); localSpace.clear(); jumpOffsetTargets.clear(); }
};

//------------------------------------------------------------------------------
struct WRExpressionContext
{
	WRExpressionType type;

	bool spaceBefore;
	bool spaceAfter;
	bool global;
	WRstr token;
	WRValue value;
	const WROperation* operation;
	
	int stackPosition;
	
	WRBytecode bytecode;

	WRExpressionContext() { reset(); }

	void setLocalSpace( WRarray<WRNamespaceLookup>& localSpace )
	{
		bytecode.localSpace.clear();
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
		type = EXTYPE_NONE;
	}

	WRExpressionContext* reset()
	{
		type = EXTYPE_NONE;

		spaceBefore = false;
		spaceAfter = false;
		global = false;
		stackPosition = -1;
		token.clear();
		value.init();
		bytecode.clear();

		return this;
	}
};

//------------------------------------------------------------------------------
struct WRExpression
{
	WRarray<WRExpressionContext> context;

	WRBytecode bytecode;

	//------------------------------------------------------------------------------
	void pushToStack( int index )
	{
		int highest = 0;
		for( unsigned int i=0; i<context.count(); ++i )
		{
			if ( context[i].stackPosition != -1 && (i != (unsigned int)index) )
			{
				++context[i].stackPosition;
				highest = context[i].stackPosition > highest ? context[i].stackPosition : highest;
			}
		}

		context[index].stackPosition = 0;

		// now compact the stack
		for( int h=0; h<highest; ++h )
		{
			unsigned int i = 0;
			bool found = false;
			for( ; i<context.count(); ++i )
			{
				if ( context[i].stackPosition == h )
				{
					found = true;
					break;
				}
			}

			if ( !found && i == context.count() )
			{
				for( unsigned int j=0; j<context.count(); ++j )
				{
					if ( context[j].stackPosition > h )
					{
						--context[j].stackPosition;
					}
				}
				--highest;
				--h;
			}
		}
	}

	//------------------------------------------------------------------------------
	void popFrom( int index )
	{
		context[index].stackPosition = -1;

		for( unsigned int i=0; i<context.count(); ++i )
		{
			if ( context[i].stackPosition != -1 )
			{
				--context[i].stackPosition;
			}
		}
	}

	//------------------------------------------------------------------------------
	void swapWithTop( int stackPosition );
	
	WRExpression() { reset(); }
	WRExpression( WRarray<WRNamespaceLookup>& localSpace )
	{
		reset();
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
	}

	void reset()
	{
		context.clear();
		bytecode.clear();
	}
};

//------------------------------------------------------------------------------
struct WRUnitContext
{
	uint32_t hash;
	int arguments;
	int offsetInBytecode;
	
	WRBytecode bytecode;

	WRUnitContext() { reset(); }
	void reset()
	{
		hash = 0;
		arguments = 0;
		offsetInBytecode = 0;
	}
};

//------------------------------------------------------------------------------
struct WRCompilationContext
{
public:
	WRError compile( const char* data, const int size, unsigned char** out, int* outLen );
	
private:
	
	bool isReserved( const char* token );
	bool isValidLabel( WRstr& token, bool& isGlobal );
	bool getToken( WRExpressionContext& ex, const char* expect =0 );
	char* pack16( int16_t i, char* buf )
	{
		*buf = (i>>8) & 0xFF;
		*(buf + 1) = i & 0xFF;
		return buf;
	}
	char* pack32( int32_t l, char* buf )
	{
		*buf = (l>>24) & 0xFF;
		*(buf + 1) = (l>>16) & 0xFF;
		*(buf + 2) = (l>>8) & 0xFF;
		*(buf + 3) = l & 0xFF;
		return buf;
	}

	friend struct WRExpression;
	static void pushOpcode( WRBytecode& bytecode, WROpcode opcode );
	static void pushData( WRBytecode& bytecode, const char* data, const int len ) { bytecode.all.append( data, len ); }

	int getBytecodePosition( WRBytecode& bytecode ) { return bytecode.all.size(); }
	
	int addRelativeJumpTarget( WRBytecode& bytecode );
	void setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget );
	void addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget );
	void resolveRelativeJumps( WRBytecode& bytecode );

	void appendBytecode( WRBytecode& bytecode, WRBytecode& addMe );
	
	void pushLiteral( WRBytecode& bytecode, WRValue& value );
	void addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly =false );
	void addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token );
	void addFunctionToHashSpace( WRBytecode& result, WRstr& token );
	void loadExpressionContext( WRExpression& expression, int depth, int operation );
	void resolveExpression( WRExpression& expression );
	void resolveExpressionEx( WRExpression& expression, int o, int p );

	bool operatorFound( WRstr& token, WRarray<WRExpressionContext>& context, int depth );
	char parseExpression( WRExpression& expression );
	bool parseUnit();
	bool parseWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseForLoop( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseIf( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn );
	
	void link( unsigned char** out, int* outLen );
	
	const char* m_source;
	int m_sourceLen;
	int m_pos;

	WRstr m_loadedToken;
	WRValue m_loadedValue;

	WRError m_err;
	bool m_EOF;

	int m_unitTop;
	WRarray<WRUnitContext> m_units;

	WRarray<int> m_continueTargets;
	WRarray<int> m_breakTargets;


/*

jumpOffsetTarget-> code
                   code
                   code
fill 1             jump to jumpOffset
                   code
                   code
fill 2             jump to jumpOffset

  jumpOffset is a list of fills

 bytecode has a list of jump offsets it added so it can increment them
  when appended

*/
};

//------------------------------------------------------------------------------
enum ScopeContextType
{
	Unit,
	Switch,
};

//------------------------------------------------------------------------------
struct ScopeContext
{
	int type;
};

#endif // WRENCH_WITHOUT_COMPILER

#endif
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
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

#define WR_COMPILER_LITERAL_STRING 0x10

//------------------------------------------------------------------------------
const char* c_reserved[] =
{
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
	"false",
	"float",
	"for",
	"foreach",
	"if",
	"int",
	"return",
	"switch",
	"true",
	"unit",
	"function",
	"while",
	"",
};

//#define _DUMP
#ifdef _DUMP
//------------------------------------------------------------------------------
const char* wr_asciiDump( const void* d, unsigned int len, WRstr& str )
{
	const unsigned char* data = (char unsigned *)d;
	str.clear();
	for( unsigned int i=0; i<len; i++ )
	{
		str.appendFormat( "0x%08X: ", i );
		char dump[24];
		unsigned int j;
		for( j=0; j<16 && i<len; j++, i++ )
		{
			dump[j] = isgraph((unsigned char)data[i]) ? data[i] : '.';
			dump[j+1] = 0;
			str.appendFormat( "%02X ", (unsigned char)data[i] );
		}

		for( ; j<16; j++ )
		{
			str.appendFormat( "   " );
		}
		i--;
		str += ": ";
		str += dump;
		str += "\n";
	}

	return str;
}
#endif

//------------------------------------------------------------------------------
bool WRCompilationContext::isValidLabel( WRstr& token, bool& isGlobal )
{
	if ( !token.size() || (!isalpha(token[0]) && token[0] != '_' && token[0] != ':') ) // non-zero size and start with alpha or '_' ?
	{
		return false;
	}

	unsigned int checkFrom = 1;
	isGlobal = false;

	if ( token[0] == ':' )
	{
		if ( (token.size() > 2) && (token[1] == ':') )
		{
			isGlobal = true;
			checkFrom = 2;
		}
		else
		{
			return false;
		}
	}

	for( unsigned int i=0; c_reserved[i][0]; ++i )
	{
		if ( token == c_reserved[i] )
		{
			return false;
		}
	}

	for( unsigned int i=checkFrom; i<token.size(); i++ ) // entire token alphanumeric or '_'?
	{
		if ( !isalnum(token[i]) && token[i] != '_' )
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::getToken( WRExpressionContext& ex, const char* expect )
{
	WRValue& value = ex.value;
	WRstr& token = ex.token;

	if ( m_loadedToken.size() || (m_loadedValue.type != WR_REF) )
	{
		token = m_loadedToken;
		value = m_loadedValue;
	}
	else
	{
		value.type = WR_REF;

		ex.spaceBefore = (m_pos < m_sourceLen) && isspace(m_source[m_pos]);
	
		do
		{
			if ( m_pos >= m_sourceLen )
			{
				m_EOF = true;
				return false;
			}
			
		} while( isspace(m_source[m_pos++]) );

		token = m_source[m_pos - 1];

		if ( token[0] == '=' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '!' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '*' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '%' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '<' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
			else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '<' )
			{
				token += '<';
				++m_pos;

				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
		}
		else if ( token[0] == '>' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
			else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '>' )
			{
				token += '>';
				++m_pos;

				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
		}
		else if ( token[0] == '&' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '&' )
			{
				token += '&';
				++m_pos;
			}
			else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '^' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '|' )
		{
			if ( (m_pos < m_sourceLen) && m_source[m_pos] == '|' )
			{
				token += '|';
				++m_pos;
			}
			else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
			{
				token += '=';
				++m_pos;
			}
		}
		else if ( token[0] == '+' )
		{
			if ( m_pos < m_sourceLen )
			{
				if ( m_source[m_pos] == '+' )
				{
					token += '+';
					++m_pos;
				}
				else if ( m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
		}
		else if ( token[0] == '-' )
		{
			if ( m_pos < m_sourceLen )
			{
				if ( m_source[m_pos] == '-' )
				{
					token += '-';
					++m_pos;
				}
				else if ( m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
		}
		else if ( token[0] == '\"' )
		{
			token.clear();
			
			do
			{
				if (m_pos >= m_sourceLen)
				{
					m_err = WR_ERR_unterminated_string_literal;
					m_EOF = true;
					return false;
				}

				char c = m_source[m_pos];
				if (c == '\"') // terminating character
				{
					++m_pos;
					break;
				}
				else if (c == '\n')
				{
					m_err = WR_ERR_newline_in_string_literal;
					return false;
				}
				else if (c == '\\')
				{
					c = m_source[++m_pos];
					if (m_pos >= m_sourceLen)
					{
						m_err = WR_ERR_unterminated_string_literal;
						m_EOF = true;
						return false;
					}

					if (c == '\\') // escaped slash
					{
						token += '\\';
					}
					else if (c == '0')
					{
						token += atoi(m_source + m_pos);
						for (; (m_pos < m_sourceLen) && isdigit(m_source[m_pos]); ++m_pos);
					}
					else if (c == '\"')
					{
						token += '\"';
					}
					else if (c == 'n')
					{
						token += '\n';
					}
					else if (c == 'r')
					{
						token += '\r';
					}
					else if (c == 't')
					{
						token += '\t';
					}
					else
					{
						m_err = WR_ERR_bad_string_escape_sequence;
						return false;
					}
				}
				else
				{
					token += c;
				}

			} while( ++m_pos < m_sourceLen );

			value.type = (WRValueType)WR_COMPILER_LITERAL_STRING;
			value.p = &token;
		}
		else if ( token[0] == '/' ) // might be a comment
		{
			if ( m_pos < m_sourceLen )
			{	
				if ( !isspace(m_source[m_pos]) )
				{
					if ( m_source[m_pos] == '/' )
					{
						for( ; m_pos<m_sourceLen && m_source[m_pos] != '\n'; ++m_pos ); // clear to end EOL
						
						return getToken( ex, expect );
					}
					else if ( m_source[m_pos] == '*' )
					{
						for( ; (m_pos+1)<m_sourceLen
							 && !(m_source[m_pos] == '*' && m_source[m_pos+1] == '/');
							 ++m_pos ); // find end of comment
						
						m_pos += 2;

						return getToken( ex, expect );

					}
					else if ( m_source[m_pos] == '=' )
					{
						token += '=';
						++m_pos;
					}
				}					
				//else // bare '/' 
			}
		}
		else if ( isdigit(token[0]) || token[0] == '.' )
		{
			if ( m_pos >= m_sourceLen )
			{
				return false;
			}

			if ( !(token[0] == '.' && !isdigit(m_source[m_pos]) ) )
			{
				if ( m_source[m_pos] == 'x' ) // interpret as hex
				{
					token += 'x';
					m_pos++;

					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( !isxdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					value.type = WR_INT;
					value.i = strtol( token.c_str(2), 0, 16 );
				}
				else
				{
					bool decimal = token[0] == '.';
					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( m_source[m_pos] == '.' )
						{
							if ( decimal )
							{
								return false;
							}

							decimal = true;
						}
						else if ( !isdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					if ( decimal )
					{
						value.type = WR_FLOAT;
						value.f = (float)atof( token );
					}
					else
					{
						value.type = WR_INT;
						value.i = strtol( token, 0, 10 );
					}
				}
			}
		}
		else if ( isalpha(token[0]) || token[0] == '_' || token[0] == ':' ) // must be a label
		{
			for( ; m_pos < m_sourceLen ; ++m_pos )
			{
				if ( !isalnum(m_source[m_pos]) && m_source[m_pos] != '_' && m_source[m_pos] != ':' )
				{
					break;
				}

				token += m_source[m_pos];
			}
		}

		ex.spaceAfter = (m_pos < m_sourceLen) && isspace(m_source[m_pos]);
	}

	m_loadedToken.clear();
	m_loadedValue.type = WR_REF;

	if ( expect && (token != expect) )
	{
		return false;
	}
	
	return true;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushOpcode( WRBytecode& bytecode, WROpcode opcode )
{
	unsigned int o = bytecode.opcodes.size();
	if ( o )
	{
		--o;
		unsigned int a = bytecode.all.size() - 1;

		// incorporate a keyhole optimizer
		if ( opcode == O_PopOne )
		{
			if ( bytecode.opcodes[o] == O_Assign ) // assign+pop is very common
			{
				bytecode.all[a] = O_AssignAndPop;
				bytecode.opcodes[o] = O_AssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LiteralZero ) // put a zero on just to pop it off..
			{
				bytecode.opcodes.shave(1);
				bytecode.all.shave(1);
				return;
			}
			else if ( bytecode.opcodes[o] == O_SubtractAssign )
			{
				bytecode.all[a] = O_SubtractAssignAndPop;
				bytecode.opcodes[o] = O_SubtractAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_AddAssign )
			{
				bytecode.all[a] = O_AddAssignAndPop;
				bytecode.opcodes[o] = O_AddAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ModAssign )
			{
				bytecode.all[a] = O_ModAssignAndPop;
				bytecode.opcodes[o] = O_ModAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_MultiplyAssign )
			{
				bytecode.all[a] = O_MultiplyAssignAndPop;
				bytecode.opcodes[o] = O_MultiplyAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_DivideAssign )
			{
				bytecode.all[a] = O_DivideAssignAndPop;
				bytecode.opcodes[o] = O_DivideAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ORAssign )
			{
				bytecode.all[a] = O_ORAssignAndPop;
				bytecode.opcodes[o] = O_ORAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ANDAssign )
			{
				bytecode.all[a] = O_ANDAssignAndPop;
				bytecode.opcodes[o] = O_ANDAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_XORAssign )
			{
				bytecode.all[a] = O_XORAssignAndPop;
				bytecode.opcodes[o] = O_XORAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_RightShiftAssign )
			{
				bytecode.all[a] = O_RightShiftAssignAndPop;
				bytecode.opcodes[o] = O_RightShiftAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LeftShiftAssign )
			{
				bytecode.all[a] = O_LeftShiftAssignAndPop;
				bytecode.opcodes[o] = O_LeftShiftAssignAndPop;
				return;
			}
		}
	}
	
	bytecode.all += opcode;
	bytecode.opcodes += opcode;
}

//------------------------------------------------------------------------------
// add a point to jump TO and return a list that should be filled with
// opcode + where to jump FROM
int WRCompilationContext::addRelativeJumpTarget( WRBytecode& bytecode )
{
	bytecode.jumpOffsetTargets.append().references.clear();
	return bytecode.jumpOffsetTargets.count() - 1;
}

//------------------------------------------------------------------------------
void WRCompilationContext::setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget )
{
	bytecode.jumpOffsetTargets[relativeJumpTarget].offset = bytecode.all.size();
}

//------------------------------------------------------------------------------
// add a jump FROM with whatever opcode is supposed to do itw
void WRCompilationContext::addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget )
{
	pushOpcode( bytecode, opcode );
	bytecode.jumpOffsetTargets[relativeJumpTarget].references.append() = bytecode.all.size();
	pushData( bytecode, "0xABCD", 2 );
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveRelativeJumps( WRBytecode& bytecode )
{
	for( unsigned int j=0; j<bytecode.jumpOffsetTargets.count(); ++j )
	{
		for( unsigned int t=0; t<bytecode.jumpOffsetTargets[j].references.count(); ++t )
		{
			int16_t diff = bytecode.jumpOffsetTargets[j].offset - bytecode.jumpOffsetTargets[j].references[t];
			pack16( diff, bytecode.all.p_str(bytecode.jumpOffsetTargets[j].references[t]) );
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::appendBytecode( WRBytecode& bytecode, WRBytecode& addMe )
{
	resolveRelativeJumps( addMe );
	
	// add the namespace, making sure to offset it into the new block properly
	for( unsigned int n=0; n<addMe.localSpace.count(); ++n )
	{
		unsigned int m = 0;
		for ( m=0; m<bytecode.localSpace.count(); ++m )
		{
			if ( bytecode.localSpace[m].hash == addMe.localSpace[n].hash )
			{
				for( unsigned int s=0; s<addMe.localSpace[n].references.count(); ++s )
				{
					bytecode.localSpace[m].references.append() = addMe.localSpace[n].references[s] + bytecode.all.size();
				}
				
				break;
			}
		}

		if ( m >= bytecode.localSpace.count() )
		{
			WRNamespaceLookup* space = &bytecode.localSpace.append();
			*space = addMe.localSpace[n];
			for( unsigned int s=0; s<space->references.count(); ++s )
			{
				space->references[s] += bytecode.all.size();
			}
		}
	}

	// add the function space, making sure to offset it into the new block properly
	for( unsigned int n=0; n<addMe.functionSpace.count(); ++n )
	{
		WRNamespaceLookup* space = &bytecode.functionSpace.append();
		*space = addMe.functionSpace[n];
		for( unsigned int s=0; s<space->references.count(); ++s )
		{
			space->references[s] += bytecode.all.size();
		}
	}

	bytecode.all += addMe.all;
	bytecode.opcodes += addMe.opcodes;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLiteral( WRBytecode& bytecode, WRValue& value )
{
	if ( value.type == WR_INT && value.i == 0 )
	{
		pushOpcode( bytecode, O_LiteralZero );
	}
	else if ( value.type == WR_INT )
	{
		if ( (value.i <= 127) && (value.i >= -128) )
		{
			pushOpcode( bytecode, O_LiteralInt8 );
			char be = (char)value.i;
			pushData( bytecode, &be, 1 );
		}
		else
		{
			pushOpcode( bytecode, O_LiteralInt32 );
			char data[4];
			pushData( bytecode, pack32(value.i, data), 4 );
		}
	}
	else if ( value.type == WR_COMPILER_LITERAL_STRING )
	{
		pushOpcode( bytecode, O_LiteralString );
		char data[2];
		int16_t be = ((WRstr*)value.p)->size();
		pushData( bytecode, pack16(be, data), 2 );
		for( unsigned int i=0; i<((WRstr*)value.p)->size(); ++i )
		{
			pushData( bytecode, ((WRstr*)value.p)->c_str(i), 1 );
		}
	}
	else
	{
		pushOpcode( bytecode, O_LiteralFloat );
		char data[4];
		int32_t be = value.i;
		pushData( bytecode, pack32(be, data), 4 );
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly )
{
	if ( m_unitTop == 0 )
	{
		assert( !addOnly );
		addGlobalSpaceLoad( bytecode, token );
		return;
	}
	
	uint32_t hash = wr_hash( token, token.size() );

	unsigned int i=0;
		
	for( ; i<bytecode.localSpace.count(); ++i )
	{
		if ( bytecode.localSpace[i].hash == hash )
		{
			break;
		}
	}

	if ( i >= bytecode.localSpace.count() && !addOnly )
	{
		for (unsigned int j = 0; j < m_units[0].bytecode.localSpace.count(); ++j)
		{
			if (m_units[0].bytecode.localSpace[j].hash == hash)
			{
				pushOpcode(bytecode, O_LoadFromGlobal);
				char c = j;
				pushData(bytecode, &c, 1);
				return;
			}
		}
	}

	bytecode.localSpace[i].hash = hash;

	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromLocal );
		char c = i;
		pushData( bytecode, &c, 1 );
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token )
{
	uint32_t hash;
	if ( token[0] == ':' && token[1] == ':' )
	{
		hash = wr_hash( token.c_str(2), token.size() - 2 );
	}
	else
	{
		hash = wr_hash( token.c_str(), token.size() );
	}

	unsigned int i=0;

	for( ; i<m_units[0].bytecode.localSpace.count(); ++i )
	{
		if ( m_units[0].bytecode.localSpace[i].hash == hash )
		{
			break;
		}
	}
	
	m_units[0].bytecode.localSpace[i].hash = hash;

	pushOpcode( bytecode, O_LoadFromGlobal );
	char c = i;
	pushData( bytecode, &c, 1 );
}

//------------------------------------------------------------------------------
void WRCompilationContext::addFunctionToHashSpace( WRBytecode& result, WRstr& token )
{
	uint32_t hash = wr_hash( token, token.size() );

	unsigned int i=0;
	for( ; i<result.functionSpace.count(); ++i )
	{
		if ( result.functionSpace[i].hash == hash )
		{
			break;
		}
	}

	result.functionSpace[i].references.append() = getBytecodePosition( result );
	result.functionSpace[i].hash = hash;
	pushData( result, "\t\t\t\t\t", 5 ); // TBD opcode plus index, OR hash if index was not found
	result.invalidateOpcodeCache();
}

//------------------------------------------------------------------------------
void WRCompilationContext::loadExpressionContext( WRExpression& expression, int depth, int operation )
{
	if ( operation > 0
		 && expression.context[operation].operation
		 && expression.context[operation].operation->opcode == O_IndexHash
		 && depth > operation )
	{
		char buf[4];
		pack32( wr_hash( expression.context[depth].token,
						 expression.context[depth].token.size()),
				buf );
		pushOpcode( expression.bytecode, O_LiteralInt32 );
		pushData( expression.bytecode, buf, 4 );
	}
	else
	{
		switch( expression.context[depth].type )
		{
			case EXTYPE_LITERAL:
			{
				pushLiteral( expression.bytecode, expression.context[depth].value );
				break;
			}

			case EXTYPE_LABEL:
			{
				if ( expression.context[depth].global )
				{
					addGlobalSpaceLoad( expression.bytecode,
										expression.context[depth].token );
				}
				else
				{
					addLocalSpaceLoad( expression.bytecode,
									   expression.context[depth].token );
				}
				break;
			}

			case EXTYPE_BYTECODE_RESULT:
			{
				appendBytecode( expression.bytecode,
								expression.context[depth].bytecode );
				break;
			}

			case EXTYPE_RESOLVED:
			case EXTYPE_OPERATION:
			default:
			{
				break;
			}
		}
	}

	expression.pushToStack( depth ); // make sure stack knows something got loaded on top of it
	
	expression.context[depth].type = EXTYPE_RESOLVED; // this slot is now a resolved value sitting on the stack
}

//------------------------------------------------------------------------------
void WRExpression::swapWithTop( int stackPosition )
{
	if ( stackPosition == 0 )
	{
		return;
	}
	
	unsigned int currentTop = -1;
	unsigned int swapWith = -1;
	for( unsigned int i=0; i<context.count(); ++i )
	{
		if ( context[i].stackPosition == stackPosition )
		{
			swapWith = i;
		}
		
		if ( context[i].stackPosition == 0 )
		{
			currentTop = i;
		}
	}

	assert( (currentTop != (unsigned int)-1) && (swapWith != (unsigned int)-1) );

	char pos = stackPosition + 1;
	WRCompilationContext::pushOpcode( bytecode, O_StackSwap );
	WRCompilationContext::pushData( bytecode, &pos, 1 );
	context[currentTop].stackPosition = stackPosition;
	context[swapWith].stackPosition = 0;
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveExpression( WRExpression& expression )
{
	if ( expression.context.count() == 1 ) // single expression is trivial to resolve, load it!
	{
		loadExpressionContext( expression, 0, 0 );
		return;
	}

	for( int p=0; p<c_highestPrecedence && expression.context.count() > 1; ++p )
	{
		// left to right operations
		for( int o=0; (unsigned int)o < expression.context.count(); ++o )
		{
			if ( (expression.context[o].type != EXTYPE_OPERATION)
				 || expression.context[o].operation->precedence > p
				 || !expression.context[o].operation->leftToRight )
			{
				continue;
			}

			resolveExpressionEx( expression, o, p );
			o = (unsigned int)-1;
		}

		// right to left operations
		for( int o = expression.context.count() - 1; o >= 0; --o )
		{
			if ( (expression.context[o].type != EXTYPE_OPERATION)
				 || expression.context[o].operation->precedence > p
				 || expression.context[o].operation->leftToRight )
			{
				continue;
			}

			resolveExpressionEx( expression, o, p );
			o = expression.context.count();
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveExpressionEx( WRExpression& expression, int o, int p )
{
	switch( expression.context[o].operation->type )
	{
		case WR_OPER_PRE:
		{
			if ( expression.context[o + 1].stackPosition == -1 )
			{
				loadExpressionContext( expression, o + 1, 0 ); // load argument
			}
			else if ( expression.context[o + 1].stackPosition != 0 )
			{
				expression.swapWithTop( expression.context[o + 1].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode );  // apply operator
			expression.context.remove( o, 1 ); // knock off operator
			expression.pushToStack(o);

			break;
		}

		// this is a really cool optimization but -=, += etc
		// breaks it in some cases :/ --TODO really want this
		// to work, come back to it
/*
		case WR_OPER_BINARY_COMMUTE:
		{
			// this operation allows the arguments to be pushed
			// in any order :)

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return;
			}

			int second = o + 1; // push first
			int first = o - 1;  // push second
			// so
			// 1 - first
			// [oper]
			// 0 - second

			if ( expression.context[second].stackPosition == -1 )
			{
				if ( expression.context[first].stackPosition == -1 )
				{
					loadExpressionContext( expression, first ); 
				}
				else if ( expression.context[first].stackPosition != 0 )
				{
					// otherwise swap first to the top and load the second
					expression.swapWithTop( expression.context[first].stackPosition );
				}

				loadExpressionContext( expression, second );
			}
			else if ( expression.context[first].stackPosition == -1 )
			{
				if ( expression.context[second].stackPosition != 0 )
				{
					expression.swapWithTop( expression.context[second].stackPosition );
				}

				// just load the second to top
				loadExpressionContext( expression, first );
			}
			else if ( expression.context[second].stackPosition == 1 )
			{
				expression.swapWithTop( expression.context[first].stackPosition );
			}
			else if ( expression.context[first].stackPosition == 1 )
			{
				expression.swapWithTop( expression.context[second].stackPosition );
			}
			else
			{
				// first and second are both loaded but neither
				// is in the correct position, three swaps
				// required

				expression.swapWithTop( expression.context[second].stackPosition );
				expression.swapWithTop( 1 );
				expression.swapWithTop( expression.context[first].stackPosition );
			}


			appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}
*/
		case WR_OPER_BINARY_COMMUTE:

		case WR_OPER_BINARY:
		{
			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return;
			}

			int second = o + 1; // push first
			int first = o - 1;  // push second
			// so
			// 1 - first
			// [oper]
			// 0 - second

			if ( expression.context[second].stackPosition == -1 )
			{
				if ( expression.context[first].stackPosition == -1 )
				{
					loadExpressionContext( expression, second, o ); // nope, grab 'em
					loadExpressionContext( expression, first, o ); 
				}
				else
				{
					// first is in the stack somewhere, we need
					// to swap it to the top, then load, then
					// swap top values
					expression.swapWithTop( expression.context[first].stackPosition );

					loadExpressionContext( expression, second, o );

					expression.swapWithTop( 1 );

				}
			}
			else if ( expression.context[first].stackPosition == -1 )
			{
				// perfect, swap up the second, then load the
				// first

				expression.swapWithTop( expression.context[second].stackPosition );

				loadExpressionContext( expression, first, o );
			}
			else if ( expression.context[second].stackPosition == 1 )
			{
				// second is already where its supposed to be,
				// swap first to top if it's not there already
				if ( expression.context[first].stackPosition != 0 )
				{
					expression.swapWithTop( expression.context[first].stackPosition );
				}
			}
			else if ( expression.context[second].stackPosition == 0 )
			{
				// second is on top of the stack, swap with
				// next level down then swap first up

				expression.swapWithTop( 1 );

				expression.swapWithTop( expression.context[first].stackPosition );
			}
			else
			{
				// first and second are both loaded but neither
				// is in the correct position, three swaps
				// required

				expression.swapWithTop( expression.context[second].stackPosition );

				expression.swapWithTop( 1 );

				expression.swapWithTop( expression.context[first].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}

		case WR_OPER_POST:
		{
			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return;
			}

			if ( expression.context[o - 1].stackPosition == -1 )
			{
				loadExpressionContext( expression, o - 1, o ); // load argument
			}
			else if ( expression.context[o-+ 1].stackPosition != 0 )
			{
				expression.swapWithTop( expression.context[o - 1].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode );
			expression.context.remove( o, 1 ); // knock off operator
			expression.pushToStack(o - 1);

			break;
		}
	}
}

//------------------------------------------------------------------------------
bool WRCompilationContext::operatorFound( WRstr& token, WRarray<WRExpressionContext>& context, int depth )
{
	for( int i=0; c_operations[i].token; i++ )
	{
		if ( token == c_operations[i].token )
		{
			if ( c_operations[i].type == WR_OPER_PRE
				 && depth > 0
				 && (context[depth-1].type != EXTYPE_OPERATION 
					 || (context[depth - 1].type == EXTYPE_OPERATION &&
						 (context[depth - 1].operation->type != WR_OPER_BINARY && context[depth - 1].operation->type != WR_OPER_BINARY_COMMUTE))) )
			{
				continue;
			}
			
			context[depth].operation = c_operations + i;
			context[depth].type = EXTYPE_OPERATION;

			pushOpcode( context[depth].bytecode, c_operations[i].opcode );

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
char WRCompilationContext::parseExpression( WRExpression& expression )
{
	int depth = 0;
	char end = 0;

	for(;;)
	{
		WRValue& value = expression.context[depth].value;
		WRstr& token = expression.context[depth].token;

		expression.context[depth].bytecode.clear();
		expression.context[depth].setLocalSpace( expression.bytecode.localSpace );
		if ( !getToken(expression.context[depth]) )
		{
			return 0;
		}

		if ( value.type != WR_REF )
		{
			// it's a literal
			expression.context[depth].type = EXTYPE_LITERAL;
			
			++depth;
			continue;
		}

		if ( token == ";" || token == ")" || token == "}" || token == "," || token == "]" )
		{
			end = token[0];
			break;
		}

		if ( operatorFound(token, expression.context, depth) )
		{
			++depth;
			continue;
		}

		if ( token == "(" )
		{
			// might be cast, call or sub-expression
			
			if ( (depth > 0) && expression.context[depth - 1].type == EXTYPE_LABEL )
			{
				// always only a call
				
				--depth;
				WRstr functionName = expression.context[depth].token;
				expression.context[depth].reset();

				expression.context[depth].type = EXTYPE_BYTECODE_RESULT;

				char argsPushed = 0;

				WRstr& token2 = expression.context[depth].token;
				WRValue& value2 = expression.context[depth].value;

				for(;;)
				{
					if ( !getToken(expression.context[depth]) )
					{
						m_err = WR_ERR_unexpected_EOF;
						return 0;
					}

					if ( token2 == ")" )
					{
						break;
					}

					++argsPushed;

					WRExpression nex( expression.bytecode.localSpace );
					nex.context[0].token = token2;
					nex.context[0].value = value2;
					m_loadedToken = token2;
					m_loadedValue = value2;

					char end = parseExpression( nex );

					appendBytecode( expression.context[depth].bytecode, nex.bytecode );
							   
					if ( end == ')' )
					{
						break;
					}
					else if ( end != ',' )
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}
				}

				// push the number of args
				addFunctionToHashSpace( expression.context[depth].bytecode, functionName );
				pushData( expression.context[depth].bytecode, &argsPushed, 1 );
			}
			else if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}
			else if ( token == "int" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				WRstr t( "@i" );
				operatorFound( t, expression.context, depth );
			}
			else if ( token == "float" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				WRstr t( "@f" );
				operatorFound( t, expression.context, depth );
			}
			else if (depth < 0)
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}
			else
			{
				WRExpression nex( expression.bytecode.localSpace );
				nex.context[0].token = token;
				nex.context[0].value = value;
				m_loadedToken = token;
				m_loadedValue = value;
				if ( parseExpression(nex) != ')' )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				expression.context[depth].type = EXTYPE_BYTECODE_RESULT;
				expression.context[depth].bytecode = nex.bytecode;
			}

			++depth;
			continue;
		}

		if ( token == "[" )
		{
			if ( depth == 0
				 || (expression.context[depth - 1].type != EXTYPE_LABEL
					 && expression.context[depth - 1].type != EXTYPE_BYTECODE_RESULT) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}


			WRExpression nex( expression.bytecode.localSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;

			if ( parseExpression( nex ) != ']' )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}
			
			expression.context[depth].bytecode = nex.bytecode;

			WRstr t( "@[]" );
			operatorFound( t, expression.context, depth );

			++depth;
			continue;
		}

		bool isGlobal;
		if ( isValidLabel(token, isGlobal) )
		{
			expression.context[depth].type = EXTYPE_LABEL;
			expression.context[depth].global = isGlobal;
			++depth;

			continue;
		}

		m_err = WR_ERR_bad_expression;
		return 0;
	}

	expression.context.setCount( expression.context.count() - 1 );

	resolveExpression( expression );

	return end;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseUnit()
{
	int previousIndex = m_unitTop;
	m_unitTop = m_units.count();
	
	bool isGlobal;

	WRExpressionContext ex;
	WRstr& token = ex.token;
	
	// get the function name
	if ( !getToken(ex)
		 || !isValidLabel(token, isGlobal)
		 || isGlobal )
	{
		m_err = WR_ERR_bad_label;
		return false;
	}

	m_units[m_unitTop].hash = wr_hash( token, token.size() );
	
	// get the function name
	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_bad_label;
		return false;
	}

	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == ")" )
		{
			break;
		}

		if ( !isValidLabel(token, isGlobal) || isGlobal )
		{
			m_err = WR_ERR_bad_label;
			return false;
		}
		
		++m_units[m_unitTop].arguments;

		// register the argument on the hash stack
		addLocalSpaceLoad( m_units[m_unitTop].bytecode, token, true );

		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == ")" )
		{
			break;
		}

		if ( token != "," )
		{
			m_err = WR_ERR_unexpected_token;
			return false;
		}
	}

	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	bool returnCalled;
	parseStatement( m_unitTop, '}', returnCalled, O_Return );

	if ( !returnCalled )
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_LiteralZero );
		pushOpcode( m_units[m_unitTop].bytecode, O_Return );
	}

	m_unitTop = previousIndex;

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseWhile( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	m_loadedToken = token;
	m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );

	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, *m_continueTargets.tail() );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();
	
	resolveRelativeJumps( m_units[m_unitTop].bytecode );
	
	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn )
{
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int jumpToTop = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, jumpToTop );
	
	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}

	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	if ( token != "while" )
	{
		m_err = WR_ERR_expected_while;
		return false;
	}

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	//m_loadedToken = token;
	//m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if (!getToken(ex, ";"))
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BNZ, jumpToTop );

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseForLoop( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	// create the continue and break reference points
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

/*


[setup]
<- condition point
[condition]
-> false jump break
[code]
<- continue point
[post code]
-> always jump condition
<- break point

*/
	
	// [setup]
	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == ";" )
		{
			break;
		}

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;

		char end = parseExpression( nex );
		pushOpcode( nex.bytecode, O_PopOne );

		appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );
		
		if ( end == ';' )
		{
			break;
		}
		else if ( end != ',' )
		{
			m_err = WR_ERR_unexpected_token;
			return 0;
		}
	}

	// <- condition point
	int conditionPoint = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionPoint );

	
	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	// [ condition ]
	if ( token != ";" )
	{
		WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;
		

		if ( parseExpression( nex ) != ';' )
		{
			m_err = WR_ERR_unexpected_token;
			return false;
		}
		
		appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

		// -> false jump break
		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );
	}


	WRExpression post( m_units[m_unitTop].bytecode.localSpace );

	// [ post code ]
	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == ")" )
		{
			break;
		}

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;

		char end = parseExpression( nex );
		pushOpcode( nex.bytecode, O_PopOne );

		appendBytecode( post.bytecode, nex.bytecode );

		if ( end == ')' )
		{
			break;
		}
		else if ( end != ',' )
		{
			m_err = WR_ERR_unexpected_token;
			return 0;
		}
	}

	// [ code ]
	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}

	// <- continue point
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	// [post code]
	appendBytecode( m_units[m_unitTop].bytecode, post.bytecode );

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionPoint );

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseIf( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	m_loadedToken = token;
	m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	int conditionFalseMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, conditionFalseMarker );

	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}

 	if ( !getToken(ex) )
	{
		setRelativeJumpTarget(m_units[m_unitTop].bytecode, conditionFalseMarker);
	}
	else if ( token == "else" )
	{
		int conditionTrueMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode ); // when it hits here it will jump OVER this section

		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionTrueMarker );
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );

		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionTrueMarker );
	}
	else
	{
		m_loadedToken = token;
		m_loadedValue = value;
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );
	}

	resolveRelativeJumps( m_units[m_unitTop].bytecode ); // at least do the ones we added

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	returnCalled = false;

	for(;;)
	{
		WRstr& token = ex.token;

		if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
		{
			if ( end == '}' )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			break;
		}

		if ( token[0] == end )
		{
			break;
		}

		if ( token == "{" )
		{
			return parseStatement( unitIndex, '}', returnCalled, opcodeToReturn );
		}

		if ( token == "return" )
		{
			returnCalled = true;
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( token == ";" ) // special case of a null return, add the null
			{
				pushOpcode( m_units[unitIndex].bytecode, O_LiteralZero );
			}
			else
			{
				WRExpression nex( m_units[unitIndex].bytecode.localSpace );
				nex.context[0].token = token;
				nex.context[0].value = ex.value;
				m_loadedToken = token;
				m_loadedValue = ex.value;

				if ( parseExpression( nex ) != ';')
				{
					m_err = WR_ERR_unexpected_token;
					return false;
				}

				appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			}

			pushOpcode( m_units[unitIndex].bytecode, opcodeToReturn );
		}
		else if ( token == "unit" || token == "function" )
		{
			if ( !parseUnit() )
			{
				return false;
			}
		}
		else if ( token == "if" )
		{
			if ( !parseIf(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "while" )
		{
			if ( !parseWhile(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "for" )
		{
			if ( !parseForLoop(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "do" )
		{
			if ( !parseDoWhile(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "break" )
		{
			if ( !m_breakTargets.count() )
			{
				m_err = WR_ERR_break_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_breakTargets.tail() );
		}
		else if ( token == "continue" )
		{
			if ( !m_continueTargets.count() )
			{
				m_err = WR_ERR_continue_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_continueTargets.tail() );
		}
		else
		{
			WRExpression nex( m_units[unitIndex].bytecode.localSpace);
			nex.context[0].token = token;
			nex.context[0].value = ex.value;
			m_loadedToken = token;
			m_loadedValue = ex.value;
			if ( parseExpression(nex) != ';' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			pushOpcode( m_units[unitIndex].bytecode, O_PopOne );
		}

		if ( end == ';' ) // single statement
		{
			break;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void WRCompilationContext::link( unsigned char** out, int* outLen )
{
	WRstr code;

	if ( m_units.count() > 1 )
	{
		code += O_FunctionListSize;
		code += (char)m_units.count() - 1;
	}

	char data[4];

	// register the function signatures
	for( unsigned int u=1; u<m_units.count(); ++u )
	{
		code += O_LiteralInt8; // index
		code += (char)(u - 1);

		code += O_LiteralInt8; // args
		code += (char)m_units[u].arguments;

		code += O_LiteralInt8; // local frame size
		code += (char)m_units[u].bytecode.localSpace.count();

		code += O_LiteralInt32; // hash
		code.append( pack32(m_units[u].hash, data), 4 );

		// offset placeholder
		code += O_LiteralInt32;
		m_units[u].offsetInBytecode = code.size();
		code.append( data, 4 ); // placeholder, it doesn't matter

		code += O_RegisterFunction;
	}

	// reserve global space
	code += O_ReserveGlobalFrame;
	code += (unsigned char)(m_units[0].bytecode.localSpace.count());

	// append all the unit code
	for( unsigned int u=0; u<m_units.count(); ++u )
	{
		if ( u > 0 ) // for the non-zero unit fill location into the jump table
		{
			int32_t offset = code.size();
			pack32( offset, code.p_str(m_units[u].offsetInBytecode) );
		}

		int base = code.size();

		code.append( m_units[u].bytecode.all, m_units[u].bytecode.all.size() );

		// load function table
		for( unsigned int x=0; x<m_units[u].bytecode.functionSpace.count(); ++x )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.functionSpace[x];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				unsigned int u2 = 1;
				for( ; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						code[base + N.references[r]] = O_CallFunctionByIndex;
						code[base + N.references[r]+1] = (char)(u2 - 1);
						break;
					}
				}

				if ( u2 >= m_units.count() )
				{
					if ( ((int)code.size() > (base + N.references[r] + 6))
						 && code[base + N.references[r] + 6] == O_PopOne )
					{
						code[base + N.references[r]] = O_CallFunctionByHashAndPop;
					}
					else
					{
						code[base + N.references[r]] = O_CallFunctionByHash;
					}
					pack32( N.hash, code.p_str(base + N.references[r]+1) );
				}
			}
		}
	}

	if ( !m_err )
	{
		*outLen = code.size();
		code.release( (char **)out );
	}
}

//------------------------------------------------------------------------------
WRError WRCompilationContext::compile( const char* source, const int size, unsigned char** out, int* outLen )
{
	m_source = source;
	m_sourceLen = size;

	*outLen = 0;
	*out = 0;
	
	m_pos = 0;
	m_err = WR_ERR_None;
	m_EOF = false;
	m_unitTop = 0;

	m_units.setCount(1);
	
	bool returnCalled;

	m_loadedValue.type = WR_REF;

	do
	{
		parseStatement( 0, ';', returnCalled, O_Stop );

	} while ( !m_EOF && (m_err == WR_ERR_None) );

	if ( m_err != WR_ERR_None )
	{			
		int onChar = 0;
		int onLine = 1;
		WRstr line;
		WRstr msg;

		for( int p = 0; p<size && source[p] != '\n'; p++ )
		{
			line += (char)source[p];
		}

		for( int i=0; i<m_pos; ++i )
		{
			if ( source[i] == '\n' )
			{
				onLine++;
				onChar = 0;

				line.clear();
				for( int p = i+1; p<size && source[p] != '\n'; p++ )
				{
					line += (char)source[p];
				}
			}
			else
			{
				onChar++;
			}
		}

		msg.format( "line:%d\n", onLine );
		msg.appendFormat( "err:%d\n", m_err );
		msg.appendFormat( "%-5d %s\n", onLine, line.c_str() );

		for( int i=0; i<onChar; i++ )
		{
			msg.appendFormat(" ");
		}

		msg.appendFormat( "     ^\n" );

		printf( "%s", msg.c_str() );
		
		return m_err;
	}
	
	if ( !returnCalled )
	{
		// pop final return value
		pushOpcode( m_units[0].bytecode, O_LiteralZero );
		pushOpcode( m_units[0].bytecode, O_Stop );
	}

	link( out, outLen );

	return m_err;
}

//------------------------------------------------------------------------------
int wr_compile( const char* source, const int size, unsigned char** out, int* outLen )
{
	assert( sizeof(float) == 4 );
	assert( sizeof(int) == 4 );
	assert( sizeof(char) == 1 );

	// create a compiler context that has all the necessary stuff so it's completely unloaded when complete
	WRCompilationContext comp; 

	return comp.compile( source, size, out, outLen );
}

#else // WRENCH_WITHOUT_COMPILER

int wr_compile( const char* source, const int size, unsigned char** out, int* outLen )
{
	return WR_ERR_compiler_not_loaded;
}
	
#endif
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
WRRunContext::WRRunContext( WRState* state ) : w(state)
{
	localFunctions = 0;
	globalSpace = 0;
	globals = 0;
	svAllocated = 0;
	stopLocation = 0;
	bottom = 0;
}

//------------------------------------------------------------------------------
WRRunContext::~WRRunContext()
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
	contextIdGenerator = 0;
	
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
		WRRunContext* next = contextList->next;
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
WRStaticValueArray* WRRunContext::getSVA( int size, WRStaticValueArrayType type )
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
			array[i].init();
		}
	}

	ret->m_next = svAllocated;
	svAllocated = ret;

	return ret;
}

//------------------------------------------------------------------------------
void WRRunContext::gcArray( WRStaticValueArray* sva )
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
int wr_runEx( WRState* w )
{
	int context_id = ++w->contextIdGenerator;
	if ( context_id == 0 ) // catch wraparound
	{
		++context_id;
	}
	
	WRRunContext* C = new WRRunContext( w );
	C->next = w->contextList;
	w->contextList = C;
	
	w->contexts.set( context_id, C );

	if ( wr_callFunction(w, context_id, (int32_t)0) )
	{
		w->contextIdGenerator--;
		delete C;
		w->contexts.remove( context_id );
		return -1;
	}

	return context_id;
}

//------------------------------------------------------------------------------
int wr_run( WRState* w, const unsigned char* block, const int size )
{
	w->loader = wr_loadSingleBlock;
	w->usr = (void*)block;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
int wr_run( WRState* w, WR_LOAD_BLOCK_FUNC loader, void* usr )
{
	w->loader = loader;
	w->usr = usr;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
void wr_destroyContext( WRState* w, const int contextId )
{
	WRRunContext* context;
	if ( !contextId || !(context = w->contexts.getItem(contextId)) )
	{
		return;
	}

	w->contexts.remove( contextId );

	WRRunContext* prev = 0;

	// unlink it
	for( WRRunContext* c = w->contextList; c; c = c->next )
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
		case WR_REF:
		{
			if ( r->type == WR_ARRAY )
			{
				WRValue temp;
				arrayToValue( this, &temp );
				return temp.asString( string );
			}
			else
			{
				return r->asString( string );
			}
		}
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
	}
	
	return string;
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, const int contextId, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( w, contextId, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, const int contextId, const int32_t hash, const WRValue* argv, const int argn )
{
	const unsigned char* pc;
	WRValue* tempValue;
	WRValue* tempValue2;
	WRValue* frameBase = 0;
	WRFunctionRegistry* F;
	unsigned char args;
	WRValue* stackTop = w->stack;
#ifdef _WIN32
	WROpcode opcode;
#endif

	w->err = WR_ERR_None;
	
	WRRunContext* context = w->contexts.getItem( contextId );
	if ( !context )
	{
		return w->err = WR_ERR_context_not_found;
	}
	
	union
	{
		WRVoidFunc (*voidFunc)[5];
		WRReturnFunc (*returnFunc)[5];
		WRTargetFunc (*targetFunc)[5];
	};

#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
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
		
	if ( hash )
	{
		// if a function name is provided it is meant to be called
		// directly. Set that up with a return vector of "stop"
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_execute_must_be_called_by_itself_first;
		}

		F = context->localFunctionRegistry.getItem( hash );

		if ( !F )
		{
			return w->err = WR_ERR_wrench_function_not_found;
		}
		
		args = 0;
		if ( argv && argn )
		{
			for( ; args < argn; ++args )
			{
				*stackTop++ = argv[args];
			}
		}

#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
		pc = context->bottom + context->stopLocation;
#else
		unsigned int size = loader( absoluteBottom = context->stopLocation, &pc, usr );
		top = pc + (size - 6);
		context->bottom = pc;
#endif
		goto callFunction;
	}

	for(;;)
	{

		if ( w->err )
		{
			if ( w->err > WR_warning_enums_follow )
			{
				//if ( !w->ignoreWarnings )
				{
					// then do something I guess
				}
				//else
				{
				}
			}
			else
			{
				return w->err;
			}

			w->err = WR_ERR_None;
		}

#ifndef SINGLE_COMPLETE_BYTECODE_LOAD
		if ( pc >= top )
		{
			unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
			top = pc + (size - 6);
			context->bottom = pc;
		}
#endif

#ifdef _WIN32
		opcode = (WROpcode)*pc;
#endif

		D_OPCODE(printf( "s[%p] top[%p] size[%d] %d:%s\n", w->stack, stackTop, (int)(stackTop - w->stack), (int)*pc, c_opcodeName[*pc]));
		
		switch( *pc++)
		{
			case O_RegisterFunction:
			{
				int index = (stackTop - 5)->i;
				context->localFunctions[ index ].arguments = (stackTop - 4)->i;
				context->localFunctions[ index ].frameSpaceNeeded = (stackTop - 3)->i;
				context->localFunctions[ index ].hash = (stackTop - 2)->i;
				
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
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
				continue;
			}

			case O_FunctionListSize:
			{
				delete context->localFunctions;
				context->localFunctionRegistry.clear();
				context->localFunctions = new WRFunctionRegistry[ *pc++ ];
				continue;
			}

			case O_LiteralZero:
			{
				(stackTop++)->init();
				continue;
			}
			
			case O_LiteralInt8:
			{
				stackTop->type = WR_INT;
				(stackTop++)->i = (int32_t)(int8_t)*pc++;
				continue;
			}
			
			case O_LiteralFloat:
			{
				stackTop->type = WR_FLOAT;
				(stackTop++)->i = (((int32_t)*pc) << 24)
								  | (((int32_t)*(pc+1)) << 16)
								  | (((int32_t)*(pc+2)) << 8)
								  | ((int32_t)*(pc+3));
				pc += 4;
				continue;
			}
			case O_LiteralInt32:
			{
				stackTop->type = WR_INT;
				(stackTop++)->i = (((int32_t)*pc) << 24)
								  | (((int32_t)*(pc+1)) << 16)
								  | (((int32_t)*(pc+2)) << 8)
								  | ((int32_t)*(pc+3));
				pc += 4;
				continue;
			}

			case O_LiteralString:
			{
				int16_t len = (((int16_t)*pc)<<8) | (int16_t)*(pc + 1);
				pc += 2;
				stackTop->type = WR_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR );
				
				for( int c=0; c<len; ++c )
				{
					((unsigned char *)stackTop->va->m_data)[c] = *pc++;

#ifndef SINGLE_COMPLETE_BYTECODE_LOAD
					if ( pc >= top )
					{
						unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
						top = pc + (size - 6);
						context->bottom = pc;
					}
#endif
				}
				++stackTop;
				continue;
			}
			
			case O_ReserveFrame:
			{
				frameBase = stackTop;
				for( unsigned char i=0; i<*pc; ++i )
				{
					(++stackTop)->init();
				}
				++pc;
				continue;
			}

			case O_ReserveGlobalFrame:
			{
				delete[] context->globalSpace;
				context->globals = *pc++;
				context->globalSpace = new WRValue[ context->globals ];
				for( unsigned char i=0; i<context->globals; ++i )
				{
					context->globalSpace[i].init();
				}
				continue;
			}

			case O_LoadFromLocal:
			{
				stackTop->type = WR_REF;
				(stackTop++)->p = frameBase + *pc++;
				continue;
			}

			case O_LoadFromGlobal:
			{
				/*
				stackTop->type = WR_REF;
				(stackTop++)->p = w->stack + *pc++;
				*/
				stackTop->type = WR_REF;
				(stackTop++)->p = context->globalSpace + *pc++;

				continue;
			}

			case O_Index:
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_index[tempValue->type][tempValue2->type]( context, tempValue, tempValue2 );
				continue;
			}

			case O_IndexHash:
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_UserHash[ tempValue->type ]( tempValue, tempValue2, tempValue2->i );
				continue;
			}

			case O_BinaryMultiplication: { targetFunc = wr_binaryMultiply; goto targetFuncOp; }
			case O_BinarySubtraction: { targetFunc = wr_binarySubtract; goto targetFuncOp; }
			case O_BinaryDivision: { targetFunc = wr_binaryDivide; goto targetFuncOp; }
			case O_BinaryRightShift: { targetFunc = wr_binaryRightShift; goto targetFuncOp; }
			case O_BinaryLeftShift: { targetFunc = wr_binaryLeftShift; goto targetFuncOp; }
			case O_BinaryMod: { targetFunc = wr_binaryMod; goto targetFuncOp; }
			case O_BinaryOr: { targetFunc = wr_binaryOr; goto targetFuncOp; }
			case O_BinaryXOR: { targetFunc = wr_binaryXOR; goto targetFuncOp; }
			case O_BinaryAnd: { targetFunc = wr_binaryAnd; goto targetFuncOp; }
			case O_BinaryAddition:
			{
				targetFunc = wr_binaryAddition;
targetFuncOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				targetFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2, tempValue2 );
				continue;
			}

			case O_SubtractAssign: { voidFunc = wr_SubtractAssign; goto binaryTableOp; }
			case O_AddAssign: { voidFunc = wr_AddAssign; goto binaryTableOp; }
			case O_ModAssign: { voidFunc = wr_ModAssign; goto binaryTableOp; }
			case O_MultiplyAssign: { voidFunc = wr_MultiplyAssign; goto binaryTableOp; }
			case O_DivideAssign: { voidFunc = wr_DivideAssign; goto binaryTableOp; }
			case O_ORAssign: { voidFunc = wr_ORAssign; goto binaryTableOp; }
			case O_ANDAssign: { voidFunc = wr_ANDAssign; goto binaryTableOp; }
			case O_XORAssign: { voidFunc = wr_XORAssign; goto binaryTableOp; }
			case O_RightShiftAssign: { voidFunc = wr_RightShiftAssign; goto binaryTableOp; }
			case O_LeftShiftAssign: { voidFunc = wr_LeftShiftAssign; goto binaryTableOp; }
			case O_Assign:
			{
				voidFunc = wr_assign;
binaryTableOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				voidFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				continue;
			}

			case O_SubtractAssignAndPop: { voidFunc = wr_SubtractAssign; goto binaryTableOpAndPop; }
			case O_AddAssignAndPop: { voidFunc = wr_AddAssign; goto binaryTableOpAndPop; }
			case O_ModAssignAndPop: { voidFunc = wr_ModAssign; goto binaryTableOpAndPop; }
			case O_MultiplyAssignAndPop: { voidFunc = wr_MultiplyAssign; goto binaryTableOpAndPop; }
			case O_DivideAssignAndPop: { voidFunc = wr_DivideAssign; goto binaryTableOpAndPop; }
			case O_ORAssignAndPop: { voidFunc = wr_ORAssign; goto binaryTableOpAndPop; }
			case O_ANDAssignAndPop: { voidFunc = wr_ANDAssign; goto binaryTableOpAndPop; }
			case O_XORAssignAndPop: { voidFunc = wr_XORAssign; goto binaryTableOpAndPop; }
			case O_RightShiftAssignAndPop: { voidFunc = wr_RightShiftAssign; goto binaryTableOpAndPop; }
			case O_LeftShiftAssignAndPop: { voidFunc = wr_LeftShiftAssign; goto binaryTableOpAndPop; }
			case O_AssignAndPop:
			{
				voidFunc = wr_assign;
binaryTableOpAndPop:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				voidFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				continue;
			}

			case O_StackSwap:
			{
				WRValue v = *(stackTop - 1);
				unsigned char offset = *pc++;
				*(stackTop - 1) = *(stackTop - offset);
				*(stackTop - offset) = v;
				continue;
			}

			case O_CallFunctionByHash:
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
						stackTop->init();
						cF->function( w, stackTop - args, args, *stackTop, cF->usr );
						*(stackTop - args) = *stackTop;
						stackTop -= args - 1;
					}
				}
				else
				{
					w->err = WR_WARN_c_function_not_found;
					stackTop -= args;
					(stackTop++)->init(); // push a fake return value
				}
				continue;
			}

			case O_CallFunctionByHashAndPop:
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
				continue;
			}

			case O_CallFunctionByIndex:
			{
				F = context->localFunctions + *pc;
				pc += 4;
				args = *pc++;
callFunction:				
				// rectify arg count
				if ( args != F->arguments )
				{
					if ( args > F->arguments )
					{
						stackTop -= args - F->arguments; // poof
					}
					else
					{
						for( char a=args; a < F->arguments; ++a )
						{
							(stackTop++)->init();
						}
					}
				}

				// the arguments are at framepace 0, add more to make
				// up for the locals in the function
				for( int i=0; i<F->frameSpaceNeeded; ++i )
				{
					(stackTop++)->init();
				}

				// temp value contains return vector
				tempValue = stackTop++; // return vector
				tempValue->type = WR_INT;
				
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				tempValue->p = pc; // simple for big-blob case
				pc = F->offset;
#else
				// relative position in the code to return to
				tempValue->i = absoluteBottom + (pc - context->bottom);

				if ( F->offsetI >= absoluteBottom )
				{
					// easy, function is within this loaded block
					pc = context->bottom + (F->offsetI - absoluteBottom);
				}
				else
				{
					// less easy, have to load the new block
					unsigned int size = loader( absoluteBottom = F->offsetI, &pc, usr );
					top = pc + (size - 6);
					context->bottom = pc;
				}
#endif

				stackTop->type = WR_INT;
				(stackTop++)->p = frameBase; // very top is the old framebase

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - F->frameBaseAdjustment;

				continue;
			}

			case O_Stop:
			{
				// leave the global space allocated at the top alone so
				// functions can be called BACK when necessary, but
				// return the top of the stack which is where the
				// "return value" will be
				w->returnValue = stackTop--;
				context->stopLocation = (pc - 1) - context->bottom;
				return WR_ERR_None;
			}

			case O_Return:
			{
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				// copy the return value
				pc = (unsigned char*)((stackTop - 3)->p); // grab new PC in case function clobbers it
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
					frameBase->type = stackTop->r->type;
					frameBase->p = stackTop->r->p;
				}
				else
				{
					*frameBase = *stackTop; // copy return value down
				}
				
				tempValue = (WRValue*)(stackTop - 1)->p;
				stackTop = frameBase + 1;
				frameBase = tempValue;
				continue;
			}
			
			case O_PopOne:
			{
				--stackTop;
				continue;
			}

			case O_CoerceToInt:
			{
				break;
			}
			
			case O_CoerceToFloat:
			{
				break;
			}

			case O_RelativeJump:
			{
				int16_t offset = *pc;
				offset = (offset<<8) | *(pc+1);
				pc += offset;
				continue;
			}
			
			case O_BZ:
			{
				tempValue = --stackTop;
				if ( wr_ZeroCheck[tempValue->type](tempValue) )
				{
					int16_t offset = *pc;
					offset = (offset<<8) | *(pc+1);
					pc += offset;
					continue;
				}

				pc += 2;
				continue;
			}
			case O_BNZ:
			{
				tempValue = --stackTop;
				if ( wr_ZeroCheck[tempValue->type](tempValue) )
				{
					pc += 2;
					continue;
				}

				int16_t offset = *pc;
				offset = (offset<<8) | *(pc+1);
				pc += offset;
				continue;
			}

			case O_LogicalAnd: { returnFunc = wr_LogicalAnd; goto returnFuncNormal; }
			case O_LogicalOr: { returnFunc = wr_LogicalOr; goto returnFuncNormal; }
			case O_CompareLE: { returnFunc = wr_CompareGT; goto returnFuncInverted; }
			case O_CompareGE: { returnFunc = wr_CompareLT; goto returnFuncInverted; }
			case O_CompareGT: { returnFunc = wr_CompareGT; goto returnFuncNormal; }
			case O_CompareLT: { returnFunc = wr_CompareLT; goto returnFuncNormal; }
			case O_CompareEQ:
			{
				returnFunc = wr_CompareEQ;
returnFuncNormal:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)returnFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				continue;
			}
			
			case O_CompareNE:
			{
				returnFunc = wr_CompareEQ;
returnFuncInverted:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)!returnFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				continue;
			}

			case O_PostIncrement:
			{
				tempValue = stackTop - 1;
				wr_postinc[ tempValue->type ]( tempValue, tempValue );
				continue;
			}

			case O_PostDecrement:
			{
				tempValue = stackTop - 1;
				wr_postdec[ tempValue->type ]( tempValue, tempValue );
				continue;
			}
			
			case O_PreIncrement:
			{
				tempValue = stackTop - 1;
				wr_preinc[ tempValue->type ]( tempValue );
				continue;
			}
			
			case O_PreDecrement:
			{
				tempValue = stackTop - 1;
				wr_predec[ tempValue->type ]( tempValue );
				continue;
			}

			case O_LogicalNot:
			{
				tempValue = stackTop - 1;
				tempValue->i = wr_LogicalNot[ tempValue->type ]( tempValue );
				tempValue->type = WR_INT;
				continue;
			}

			case O_Negate:
			{
				tempValue = stackTop - 1;
				wr_negate[ tempValue->type ]( tempValue );
				continue;
			}

			default:
			{
				return w->err = WR_ERR_unknown_opcode;
			}
		}
	}
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
void wr_makeUserData( WRValue* val )
{
	val->type = WR_USR;
	val->u = new WRUserData;
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
void doBinaryLeftShift_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryLeftShift[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryLeftShift[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryLeftShift[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryLeftShift_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryLeftShift[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryLeftShift[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryLeftShift_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryLeftShift[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryLeftShift[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryLeftShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i << from->i; }
WRTargetFunc wr_binaryLeftShift[5][5] = 
{
	{  doBinaryLeftShift_I_I,  doBinaryLeftShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{  doBinaryLeftShift_R_I,  doBinaryLeftShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,      doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,      doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,      doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
};

//------------------------------------------------------------------------------
void doBinaryRightShift_R_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryRightShift[element.type][from->type](&element, from, target);
	}
	else if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryRightShift[to->r->type][element.type]( to->r, &element, target);
	}
	else
	{
		wr_binaryRightShift[to->r->type][from->r->type](to->r, from->r, target);
	}
}
void doBinaryRightShift_R_I( WRValue* to, WRValue* from, WRValue* target )
{
	if ( to->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( to, &element );
		wr_binaryRightShift[element.type][WR_INT](&element, from, target);
	}
	else
	{
		wr_binaryRightShift[to->r->type][WR_INT](to->r, from, target);
	}
}
void doBinaryRightShift_I_R( WRValue* to, WRValue* from, WRValue* target )
{
	if ( from->r->type == WR_ARRAY )
	{
		WRValue element;
		arrayToValue( from, &element );
		wr_binaryRightShift[WR_INT][element.type](to, &element, target);
	}
	else
	{
		wr_binaryRightShift[WR_INT][from->r->type](to, from->r, target);
	}
}
void doBinaryRightShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i >> from->i; }
WRTargetFunc wr_binaryRightShift[5][5] = 
{
	{ doBinaryRightShift_I_I,  doBinaryRightShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{ doBinaryRightShift_R_I,  doBinaryRightShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,       doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,       doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
	{      doTargetFuncBlank,       doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank,  doTargetFuncBlank },
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
	if ( !(target->r = value->u->get(hash)) )
	{
		target->init();
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

// standard functions that sort of come up a lot

int32_t wr_Seed;

//------------------------------------------------------------------------------
int32_t wr_rand( int32_t range )
{
	int32_t	k = wr_Seed / 127773;
	wr_Seed = 16807 * ( wr_Seed - k * 127773 ) - 2836 * k;
	return wr_Seed % range;
}

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len )
{
	// in-place implementation of murmer
	uint32_t hash = 0x811C9DC5;
	const unsigned char* data = (const unsigned char *)dat;

	for( int i=0; i<len; ++i )
	{
		hash ^= (uint32_t)data[i];
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hashStr( const char* dat )
{
	uint32_t hash = 0x811C9DC5;
	const char* data = dat;
	while ( *data )
	{
		hash ^= (uint32_t)(*data++);
		hash *= 0x1000193;
	}

	return hash;
}
