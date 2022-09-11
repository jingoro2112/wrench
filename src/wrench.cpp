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
const uint16_t c_primeTable[] =
{
//	2,
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
	6151,

	12289,
	24593,
	49157,
};
/*
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
};
*/

//------------------------------------------------------------------------------
template <class T> class WRHashTable
{
public:
	WRHashTable( int sizeHint =0 )
	{
		for( uint16_t i=0; c_primeTable[i]; ++i )
		{
			if ( sizeHint < c_primeTable[i] )
			{
				m_mod = (int)c_primeTable[i];
				m_list = new Node[m_mod];
				break;
			}
		}
	}
	~WRHashTable() { delete[] m_list; }

	//------------------------------------------------------------------------------
	void clear()
	{
		delete[] m_list;
		m_mod = (int)c_primeTable[0];
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

			if ( newMod == 0 )
			{
				return false;
			}

			// this causes a bad fragmentation on small memory systems
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
		int32_t hash = wr_hashStr(key);
		
		UDNode* node = m_index.getItem( hash );
		if ( !node )
		{
			node = new UDNode;
			node->next = m_nodeOnlyHead;
			m_nodeOnlyHead = node;
			m_index.set( wr_hashStr(key), node );
		}

		node->val = value;
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

	WRUserData( int sizeHint =0 ) : m_head(0), m_nodeOnlyHead(0), m_index(sizeHint) {}
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
struct WRFunction
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
struct WRContext
{
	WRFunction* localFunctions;
	WRHashTable<WRFunction*> localFunctionRegistry;
	WRValue* globalSpace;
	int globals;

	const unsigned char* bottom;
	int32_t stopLocation;

	WRStaticValueArray* svAllocated;
	WRStaticValueArray* getSVA( int size, WRStaticValueArrayType type = SV_VALUE );
	void gcArray( WRStaticValueArray* sva );

	WRState* w;
	
	WRContext* next;
	
	WRContext( WRState* state );
	~WRContext();
};

//------------------------------------------------------------------------------
struct WRState
{
	WRHashTable<WRCFunctionCallback> c_functionRegistry;
	WRHashTable<WR_LIB_CALLBACK> c_libFunctionRegistry;

	WRError err;

	WRValue* stack;
	int stackSize;
	WRValue* stackTop;
	
	WR_LOAD_BLOCK_FUNC loader;
	void* usr;
	
	WRValue* returnValue;
	WRContext* contextList;

	WRState( int EntriesInStack =WRENCH_DEFAULT_STACK_SIZE );
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

void wr_arrayToValue( const WRValue* array, WRValue* value );
void wr_intValueToArray( const WRValue* array, int32_t I );
void wr_floatValueToArray( const WRValue* array, float F );

typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[36];

extern WRVoidFunc wr_SubtractAssign[36];
extern WRVoidFunc wr_AddAssign[36];
extern WRVoidFunc wr_ModAssign[36];
extern WRVoidFunc wr_MultiplyAssign[36];
extern WRVoidFunc wr_DivideAssign[36];
extern WRVoidFunc wr_ORAssign[36];
extern WRVoidFunc wr_ANDAssign[36];
extern WRVoidFunc wr_XORAssign[36];
extern WRVoidFunc wr_RightShiftAssign[36];
extern WRVoidFunc wr_LeftShiftAssign[36];
extern WRVoidFunc wr_postinc[6];
extern WRVoidFunc wr_postdec[6];


typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_binaryAddition[36];

extern WRTargetFunc wr_binaryAddition2[36];


extern WRTargetFunc wr_binaryMultiply[36];
extern WRTargetFunc wr_binarySubtract[36];
extern WRTargetFunc wr_binaryDivide[36];
extern WRTargetFunc wr_binaryLeftShift[36];
extern WRTargetFunc wr_binaryRightShift[36];
extern WRTargetFunc wr_binaryMod[36];
extern WRTargetFunc wr_binaryAND[36];
extern WRTargetFunc wr_binaryOR[36];
extern WRTargetFunc wr_binaryXOR[36];

typedef void (*WRStateFunc)( WRContext* c, WRValue* to, WRValue* from );
extern WRStateFunc wr_index[36];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[36];
extern WRReturnFunc wr_CompareGT[36];
extern WRReturnFunc wr_CompareLT[36];
extern WRReturnFunc wr_LogicalAND[36];
extern WRReturnFunc wr_LogicalOR[36];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_negate[6];
extern WRUnaryFunc wr_preinc[6];
extern WRUnaryFunc wr_predec[6];
extern WRUnaryFunc wr_toInt[6];
extern WRUnaryFunc wr_toFloat[6];
extern WRUnaryFunc wr_bitwiseNot[6];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[6];

typedef bool (*WRValueCheckFunc)( WRValue* value );
extern WRValueCheckFunc wr_ZeroCheck[6];

typedef void (*WRUserHashFunc)( WRValue* value, WRValue* target, int32_t hash );
extern WRUserHashFunc wr_UserHash[6];


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

	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,
	O_CallLibFunction,
	O_CallLibFunctionAndPop,

	O_Index,
	O_IndexLiteral8,
	O_IndexLiteral32,
	O_StackIndexHash,
	O_GlobalIndexHash,
	O_LocalIndexHash,
	
	O_Assign,
	O_AssignAndPop,
	O_AssignToGlobalAndPop,
	O_AssignToLocalAndPop,
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
	O_CoerceToFloat,

	O_RelativeJump,
	O_RelativeJump8,
	
	O_BZ,
	O_BZ8,
	O_BNZ,
	O_BNZ8,

	O_CompareEQ, 
	O_CompareNE, 
	O_CompareGE,
	O_CompareLE,
	O_CompareGT,
	O_CompareLT,

	O_CompareBEQ,
	O_CompareBNE,
	O_CompareBGE,
	O_CompareBLE,
	O_CompareBGT,
	O_CompareBLT,

	O_CompareBEQ8,
	O_CompareBNE8,
	O_CompareBGE8,
	O_CompareBLE8,
	O_CompareBGT8,
	O_CompareBLT8,

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

	O_LiteralInt8ToGlobal,
	O_LiteralInt32ToLocal,
	O_LiteralInt8ToLocal,
	O_LiteralFloatToGlobal,
	O_LiteralFloatToLocal,
	O_LiteralInt32ToGlobal,
	
	O_LAST,
	
	O_HASH_PLACEHOLDER,
};

//#define DEBUG_OPCODE_NAMES
#ifdef DEBUG_OPCODE_NAMES
#define D_OPCODE
extern const char* c_opcodeName[];
#else
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
	WRstr( const char* s ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, (unsigned int)strlen(s)); }
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
	WRstr& operator += ( const char* s ) { return append(s, (unsigned int)strlen(s)); }
	WRstr& operator += ( const char c ) { return append(c); }

	WRstr& operator = ( const WRstr& str ) { if ( &str != this ) set(str, str.size()); return *this; }
	WRstr& operator = ( const WRstr* str ) { if ( !str ) { clear(); } else if ( this != this ) { set(*str, str->size()); } return *this; }
	WRstr& operator = ( const char* c ) { set(c, (unsigned int)strlen(c)); return *this; }
	WRstr& operator = ( const char c ) { set(&c, 1); return *this; }

	friend bool operator == ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len == s2.m_len && (strncmp(s1.m_str, s2.m_str, s1.m_len) == 0); }
	friend bool operator == ( const char* z, const WRstr& s ) { return s.isMatch( z ); }
	friend bool operator == ( const WRstr& s, const char* z ) { return s.isMatch( z ); }
	friend bool operator != ( const WRstr& s1, const WRstr& s2 ) { return s1.m_len != s2.m_len || (strncmp(s1.m_str, s2.m_str, s1.m_len) != 0); }
	friend bool operator != ( const WRstr& s, const char* z ) { return !s.isMatch( z ); }
	friend bool operator != ( const char* z, const WRstr& s ) { return !s.isMatch( z ); }

	friend WRstr operator + ( const WRstr& str, const char* s) { WRstr T(str); T += s; return T; }
	friend WRstr operator + ( const WRstr& str, const char c) { WRstr T(str); T += c; return T; }
	friend WRstr operator + ( const char* s, const WRstr& str ) { WRstr T(s, (unsigned int)strlen(s)); T += str; return T; }
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

	{ ".",    2, O_HASH_PLACEHOLDER,    true,  WR_OPER_BINARY },

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
	
	BytecodeJumpOffset() : offset(0) {}
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
	WRstr prefix;
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
	WRError compile( const char* data, const int size, unsigned char** out, int* outLen, char* erroMsg =0 );
	
private:
	
	bool isReserved( const char* token );
	bool isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix );
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

// fake type for compiling only
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
	"if",
	"int",
	"return",
//	"switch",
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
bool WRCompilationContext::isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix )
{
	prefix.clear();

	if ( !token.size() || (!isalpha(token[0]) && token[0] != '_' && token[0] != ':') ) // non-zero size and start with alpha or '_' ?
	{
		return false;
	}

	isGlobal = false;

	if ( token[0] == ':' )
	{
		if ( (token.size() > 2) && (token[1] == ':') )
		{
			isGlobal = true;
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

	for( unsigned int i=0; i<token.size(); i++ ) // entire token alphanumeric or '_'?
	{
		if ( token[i] == ':' )
		{
			if ( token[++i] != ':' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			if ( i != 1 )
			{
				for( unsigned int p=0; p<i - 1; ++p )
				{
					prefix += token[p];
				}

				for( unsigned int t=2; t<token.size(); ++t)
				{
					token[t-2] = token[t+prefix.size()];
				}
				token.shave( prefix.size() + 2 );
				i -= (prefix.size() + 1);
			}
			
			isGlobal = true;
			continue;
		}
		
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
				if ( isdigit(m_source[m_pos]) || m_source[m_pos] == '.' )
				{
					goto parseAsNumber;
				}
				else if ( m_source[m_pos] == '-' )
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

parseAsNumber:
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
		// some keyhole optimizations

		--o;
		unsigned int a = bytecode.all.size() - 1;

		if ( opcode == O_Index )
		{
			if ( bytecode.opcodes[o] == O_LiteralInt32 ) // indexing with a literal? we have an app for that
			{
				bytecode.all[a-5] = O_IndexLiteral32;
				bytecode.opcodes[o] = O_IndexLiteral32;
				return;
				
			}
			else if ( bytecode.opcodes[o] == O_LiteralInt8 )
			{
				bytecode.all[a-1] = O_IndexLiteral8;
				bytecode.opcodes[o] = O_IndexLiteral8;
				return;
			}
		}
		else if ( opcode == O_BZ )
		{
			if ( bytecode.opcodes[o] == O_CompareEQ ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBEQ;
				bytecode.opcodes[o] = O_CompareBEQ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLT ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBLT;
				bytecode.opcodes[o] = O_CompareBLT;
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGT ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBGT;
				bytecode.opcodes[o] = O_CompareBGT;
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGE ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBGE;
				bytecode.opcodes[o] = O_CompareBGE;
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLE ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBLE;
				bytecode.opcodes[o] = O_CompareBLE;
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareNE ) // assign+pop is very common
			{
				bytecode.all[a] = O_CompareBNE;
				bytecode.opcodes[o] = O_CompareBNE;
				return;
			}
		}
		else if ( opcode == O_PopOne )
		{
			if ( bytecode.opcodes[o] == O_CallLibFunction )
			{
				bytecode.all[a-5] = O_CallLibFunctionAndPop;
				bytecode.opcodes[o] = O_CallLibFunctionAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_Assign ) // assign+pop is very common
			{
				if ( o > 0 )
				{
					// save three opcodes if its just an assignment to
					// a variable that is not going to be used.. this is super common
					if ( bytecode.opcodes[ o - 1 ] == O_LoadFromGlobal )
					{
						if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt8 )
						{
							// Lit8     a - 4
							// [val]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 4 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							// Lit8     a - 7
							// [val]    a - 6
							// [val]    a - 5
							// [val]    a - 4
							// [val]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 7 ] = O_LiteralInt32ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							// Lit8     a - 7
							// [Fval]    a - 6
							// [Fval]    a - 5
							// [Fval]    a - 4
							// [Fval]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 7 ] = O_LiteralFloatToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							// LitZ     a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 3 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;

							// Lit8     a - 3
							// [index]  a - 2
							// [0]    a - 1
							// Assign   a     X

							bytecode.all.shave(1);
							bytecode.opcodes.shave(1);
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.shave(1);
						}

						return;
					}
					else if ( bytecode.opcodes[ o - 1 ] == O_LoadFromLocal )
					{
						if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt8 )
						{
							// Lit8     a - 4
							// [val]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 4 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							// Lit8     a - 7
							// [val]    a - 6
							// [val]    a - 5
							// [val]    a - 4
							// [val]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 7 ] = O_LiteralInt32ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							// Lit8     a - 7
							// [Fval]    a - 6
							// [Fval]    a - 5
							// [Fval]    a - 4
							// [Fval]    a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 7 ] = O_LiteralFloatToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];

							// Lit8     a - 4
							// [index]  a - 3
							// [val]    a - 2
							// [index]  a - 1 X
							// Assign   a     X

							bytecode.all.shave(2);
							bytecode.opcodes.shave(2);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							// LitZ     a - 3
							// LoadGlob a - 2
							// [index]  a - 1
							// Assign   a

							bytecode.all[ a - 3 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;

							// Lit8     a - 3
							// [index]  a - 2
							// [0]    a - 1
							// Assign   a     X

							bytecode.all.shave(1);
							bytecode.opcodes.shave(1);
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToLocalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.shave(1);
						}
						return;
					}
				}
								
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
	pushData( bytecode, "0xE1E2", 2 );
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveRelativeJumps( WRBytecode& bytecode )
{
	for( unsigned int j=0; j<bytecode.jumpOffsetTargets.count(); ++j )
	{
		for( unsigned int t=0; t<bytecode.jumpOffsetTargets[j].references.count(); ++t )
		{
			int16_t diff = bytecode.jumpOffsetTargets[j].offset - bytecode.jumpOffsetTargets[j].references[t];
			
			int offset = bytecode.jumpOffsetTargets[j].references[t];
			WROpcode o = (WROpcode) * (bytecode.all.c_str(offset - 1));

			char* i1 = bytecode.all.p_str(offset);

			if ( (diff < 128) && (diff > -129) )
			{
				switch( o )
				{
					case O_RelativeJump: *bytecode.all.p_str(offset - 1) = O_RelativeJump8; break;
					case O_BZ: *bytecode.all.p_str(offset - 1) = O_BZ8; break;
					case O_BNZ: *bytecode.all.p_str(offset - 1) = O_BNZ8; break;

					case O_CompareBEQ: *bytecode.all.p_str(offset - 1) = O_CompareBEQ8; break;
					case O_CompareBNE: *bytecode.all.p_str(offset - 1) = O_CompareBNE8; break;
					case O_CompareBGE: *bytecode.all.p_str(offset - 1) = O_CompareBGE8; break;
					case O_CompareBLE: *bytecode.all.p_str(offset - 1) = O_CompareBLE8; break;
					case O_CompareBGT: *bytecode.all.p_str(offset - 1) = O_CompareBGT8; break;
					case O_CompareBLT: *bytecode.all.p_str(offset - 1) = O_CompareBLT8; break;
						
					// no work to be done, already visited
					case O_RelativeJump8:
					case O_BZ8:
					case O_BNZ8:
					case O_CompareBLE8:
					case O_CompareBGE8:
					case O_CompareBGT8:
					case O_CompareBLT8:
					case O_CompareBEQ8:
					case O_CompareBNE8:
						break;

					default:
						m_err = WR_ERR_compiler_panic;
						return;
				}

				*i1 = (int8_t)diff;
			}
			else
			{
				switch( o )
				{
					// check to see if any were pushed into 16-bit land
					// that were previously optimized
					case O_RelativeJump8: *bytecode.all.p_str(offset - 1) = O_RelativeJump; break;
					case O_BZ8: *bytecode.all.p_str(offset - 1) = O_BZ; break;
					case O_BNZ8: *bytecode.all.p_str(offset - 1) = O_BNZ; break;
					case O_CompareBEQ8: *bytecode.all.p_str(offset - 1) = O_CompareBEQ; break;
					case O_CompareBNE8: *bytecode.all.p_str(offset - 1) = O_CompareBNE; break;
					case O_CompareBGE8: *bytecode.all.p_str(offset - 1) = O_CompareBGE; break;
					case O_CompareBLE8: *bytecode.all.p_str(offset - 1) = O_CompareBLE; break;
					case O_CompareBGT8: *bytecode.all.p_str(offset - 1) = O_CompareBGT; break;
					case O_CompareBLT8: *bytecode.all.p_str(offset - 1) = O_CompareBLT; break;
									   
					default:
						break;
				}
				
				pack16( diff, bytecode.all.p_str(bytecode.jumpOffsetTargets[j].references[t]) );
			}
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::appendBytecode( WRBytecode& bytecode, WRBytecode& addMe )
{
	if ( addMe.all.size() == 1 && addMe.opcodes[0] == O_HASH_PLACEHOLDER )
	{
		if ( bytecode.opcodes.size() > 1
			 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
			 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromLocal )
		{
			// O_literalint32
			// 0
			// 1
			// 2
			// 3
			// O_loadlocal
			// index

			int o = bytecode.all.size() - 7;

			bytecode.all[o] = O_LocalIndexHash;
			bytecode.all[o + 5] = bytecode.all[o + 4];
			bytecode.all[o + 4] = bytecode.all[o + 3];
			bytecode.all[o + 3] = bytecode.all[o + 2];
			bytecode.all[o + 2] = bytecode.all[o + 1];
			bytecode.all[o + 1] = bytecode.all[o + 6];

			bytecode.opcodes.shave(2);
			bytecode.all.shave(1);

			// O_LocalIndexHash
			// index
			// 0
			// 1
			// 2
			// 3
		}
		else if (bytecode.opcodes.size() > 1
				 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
				 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromGlobal )
		{
			// O_literalint32
			// 0
			// 1
			// 2
			// 3
			// O_LoadGlobal
			// index

			int o = bytecode.all.size() - 7;

			bytecode.all[o] = O_GlobalIndexHash;
			bytecode.all[o + 5] = bytecode.all[o + 4];
			bytecode.all[o + 4] = bytecode.all[o + 3];
			bytecode.all[o + 3] = bytecode.all[o + 2];
			bytecode.all[o + 2] = bytecode.all[o + 1];
			bytecode.all[o + 1] = bytecode.all[o + 6];

			bytecode.opcodes.shave(2);
			bytecode.all.shave(1);

			// O_GlobalIndexHash
			// index
			// 0
			// 1
			// 2
			// 3
		}
		else if (bytecode.opcodes.size() > 1
				 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
				 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_StackSwap )
		{
			// it arrived from a stack swap

			// O_literalint32
			// 0
			// 1
			// 2
			// 3
			// O_StackSwap
			// index

			int o = bytecode.all.size() - 7;

			bytecode.all[o] = O_StackIndexHash;

			bytecode.opcodes.shave(2);
			bytecode.all.shave(2);


			// O_StackIndexHash
			// 0
			// 1
			// 2
			// 3
		}
		else
		{
			m_err = WR_ERR_compiler_panic;
		}

		return;
	}

	
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
		 && expression.context[operation].operation->opcode == O_HASH_PLACEHOLDER
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

		case WR_OPER_BINARY_COMMUTE:
		{
			// for operations like + and * the operands can be in any
			// order, so don't go to any great lengths to shift the stack
			// around for them

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
					loadExpressionContext( expression, first, o ); 
				}
				else if ( expression.context[first].stackPosition != 0 )
				{
					// otherwise swap first to the top and load the second
					expression.swapWithTop( expression.context[first].stackPosition );
				}

				loadExpressionContext( expression, second, o );
			}
			else if ( expression.context[first].stackPosition == -1 )
			{
				if ( expression.context[second].stackPosition != 0 )
				{
					expression.swapWithTop( expression.context[second].stackPosition );
				}

				// just load the second to top
				loadExpressionContext( expression, first, o );
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
				WRstr prefix = expression.context[depth].prefix;
				
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

				if ( prefix.size() )
				{
					prefix += "::";
					prefix += functionName;

					char buf[4];
					pack32( wr_hashStr(prefix), buf );

					pushOpcode( expression.context[depth].bytecode, O_CallLibFunction );
					pushData( expression.context[depth].bytecode, buf, 4 );
					pushData( expression.context[depth].bytecode, &argsPushed, 1 );
				}
				else
				{
					// push the number of args
					addFunctionToHashSpace( expression.context[depth].bytecode, functionName );
					pushData( expression.context[depth].bytecode, &argsPushed, 1 );
				}
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
		WRstr prefix;
		if ( isValidLabel(token, isGlobal, prefix) )
		{
			expression.context[depth].type = EXTYPE_LABEL;
			expression.context[depth].global = isGlobal;
			expression.context[depth].prefix = prefix;
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
	WRstr prefix;
	
	// get the function name
	if ( !getToken(ex)
		 || !isValidLabel(token, isGlobal, prefix)
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

		if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
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
	if ( m_units[0].bytecode.localSpace.count() )
	{
		code += O_ReserveGlobalFrame;
		code += (unsigned char)(m_units[0].bytecode.localSpace.count());
	}

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
WRError WRCompilationContext::compile( const char* source,
									   const int size,
									   unsigned char** out,
									   int* outLen,
									   char* errorMsg )
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

		if ( errorMsg )
		{
			strncpy( errorMsg, msg, msg.size() + 1 );
		}

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
int wr_compile( const char* source, const int size, unsigned char** out, int* outLen, char* errMsg )
{
	assert( sizeof(float) == 4 );
	assert( sizeof(int) == 4 );
	assert( sizeof(char) == 1 );
	assert( O_LAST < 255 );

	// create a compiler context that has all the necessary stuff so it's completely unloaded when complete
	WRCompilationContext comp; 

	return comp.compile( source, size, out, outLen, errMsg );
}

//------------------------------------------------------------------------------
#ifdef DEBUG_OPCODE_NAMES
const char* c_opcodeName[] = 
{
	"RegisterFunction",
	"FunctionListSize",
	"LiteralZero",
	"LiteralInt8",
	"LiteralInt32",
	"LiteralFloat",
	"LiteralString",
	"CallFunctionByHash",
	"CallFunctionByHashAndPop",
	"CallFunctionByIndex",
	"CallLibFunction",
	"CallLibFunctionAndPop",
	"Index",
	"IndexLiteral8",
	"IndexLiteral32",
	"StackIndexHash",
	"GlobalIndexHash",
	"LocalIndexHash",
	"Assign",
	"AssignAndPop",
	"AssignToGlobalAndPop",
	"AssignToLocalAndPop",
	"StackSwap",
	"ReserveFrame",
	"ReserveGlobalFrame",
	"LoadFromLocal",
	"LoadFromGlobal",
	"PopOne",
	"Return",
	"Stop",
	"BinaryAddition",
	"BinarySubtraction",
	"BinaryMultiplication",
	"BinaryDivision",
	"BinaryRightShift",
	"BinaryLeftShift",
	"BinaryMod",
	"BinaryAnd",
	"BinaryOr",
	"BinaryXOR",
	"BitwiseNOT",
	"CoerceToInt",
	"CoerceToFloat",
	"RelativeJump",
	"RelativeJump8",
	"BZ",
	"BZ8",
	"BNZ",
	"BNZ8",
	"CompareEQ",
	"CompareNE",
	"CompareGE",
	"CompareLE",
	"CompareGT",
	"CompareLT",
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
	"PostIncrement",
	"PostDecrement",
	"PreIncrement",
	"PreDecrement",
	"Negate",
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
	"LogicalAnd",
	"LogicalOr",
	"LogicalNot",
};
#endif

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

/*
{
  I_I, I_R, I_F, I_U, I_A, I_Y,
  R_I, R_R, R_F, R_U, R_A, R_Y,
  F_I, F_R, F_F, F_U, F_A, F_Y,
  U_I, U_R, U_F, U_U, U_A, U_Y,
  A_I, A_R, A_F, A_U, A_A, A_Y,
  Y_I, Y_R, Y_F, Y_U, Y_A, Y_Y,
}
*/


static void doVoidFuncBlank( WRValue* to, WRValue* from ) {}
static bool doReturnFuncBlank( WRValue* to, WRValue* from ) { return false; }
static void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}
static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value ) {}
static bool doSingleBlank( WRValue* value ) { return false; }
static void doSingleVoidBlank( WRValue* value ) {}

//------------------------------------------------------------------------------
int wr_arrayValueAsInt( const WRValue* array )
{
	unsigned int index = array->arrayElement >> 8;
	int s = index < array->r->va->m_size ? index : array->r->va->m_size - 1;

	switch( array->r->va->m_type&0x3 )
	{
		case SV_VALUE: { return ((WRValue *)array->r->va->m_data)[s].asInt(); }
		case SV_CHAR: { return ((unsigned char *)array->r->va->m_data)[s]; }
		case SV_INT: { return ((int *)array->r->va->m_data)[s]; }
		case SV_FLOAT: { return (int)((float *)array->r->va->m_data)[s]; }
		default: return 0;
	}
}

//------------------------------------------------------------------------------
float wr_arrayValueAsFloat( const WRValue* array )
{
	unsigned int index = array->arrayElement >> 8;
	int s = index < array->r->va->m_size ? index : array->r->va->m_size - 1;

	switch( array->r->va->m_type&0x3 )
	{
		case SV_VALUE: { return ((WRValue *)array->r->va->m_data)[s].asFloat(); }
		case SV_CHAR: { return ((unsigned char *)array->r->va->m_data)[s]; }
		case SV_INT: { return (float)((int *)array->r->va->m_data)[s]; }
		case SV_FLOAT: { return ((float *)array->r->va->m_data)[s]; }
		default: return 0;
	}
}

//------------------------------------------------------------------------------
void wr_arrayToValue( const WRValue* array, WRValue* value )
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
void wr_intValueToArray( const WRValue* array, int32_t I )
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
void wr_floatValueToArray( const WRValue* array, float F )
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

//==============================================================================
static void doAssign_R_R( WRValue* to, WRValue* from )
{
	wr_assign[to->type*6+from->r->type](to, from->r);
}

static void doAssign_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_assign[WR_REFARRAY*6+element.type](to, &element);
}

static void doAssign_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_assign[to->type*6+element.type](to, &element);
}

static void doAssign_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_assign[element.type*6+from->r->type](&element, from->r);
}

static void doAssign_R_X( WRValue* to, WRValue* from ) { *to->r = *from; }
static void doAssign_X_R( WRValue* to, WRValue* from ) { *to = *from->r; }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
static void doAssign_Y_F( WRValue* to, WRValue* from ) { wr_floatValueToArray( to, from->f ); }
static void doAssign_Y_I( WRValue* to, WRValue* from ) { wr_intValueToArray( to, from->i ); }

WRVoidFunc wr_assign[36] = 
{
	doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doAssign_R_X,    doAssign_R_R,     doAssign_R_X,  doVoidFuncBlank,  doVoidFuncBlank,     doAssign_R_Y,
	doAssign_X_X,    doAssign_X_R,     doAssign_X_X,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
 doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
 doVoidFuncBlank, doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,
	doAssign_Y_I,    doAssign_Y_R,     doAssign_Y_F,  doVoidFuncBlank,  doVoidFuncBlank,     doAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doSubtractAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_SubtractAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doSubtractAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_SubtractAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doSubtractAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_SubtractAssign[element.type*6+WR_FLOAT]( &element, from );

	wr_floatValueToArray( to, element.f );
	*from = element;
}

static void doSubtractAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_SubtractAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doSubtractAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_SubtractAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doSubtractAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_SubtractAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

static void doSubtractAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doSubtractAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doSubtractAssign_R_I( WRValue* to, WRValue* from ) { wr_SubtractAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doSubtractAssign_R_F( WRValue* to, WRValue* from ) { wr_SubtractAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
static void doSubtractAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
static void doSubtractAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_SubtractAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

static void doSubtractAssign_F_F( WRValue* to, WRValue* from ) { to->f -= from->f; }
static void doSubtractAssign_I_I( WRValue* to, WRValue* from ) { to->i -= from->i; }
static void doSubtractAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i - from->f; }
static void doSubtractAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f -= (float)from->i; }

WRVoidFunc wr_SubtractAssign[36] = 
{
	doSubtractAssign_I_I,  doSubtractAssign_I_R,  doSubtractAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_I_Y,
	doSubtractAssign_R_I,  doSubtractAssign_R_R,  doSubtractAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_R_Y,
	doSubtractAssign_F_I,  doSubtractAssign_F_R,  doSubtractAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_F_Y,
	     doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank,      doVoidFuncBlank,
	     doVoidFuncBlank,       doVoidFuncBlank,       doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank,      doVoidFuncBlank,
	doSubtractAssign_Y_I,  doSubtractAssign_Y_R,  doSubtractAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doSubtractAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doAddAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_AddAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doAddAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_AddAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doAddAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_AddAssign[element.type*6+WR_FLOAT]( &element, from );

	wr_floatValueToArray( to, element.f );
	*from = element;
}

static void doAddAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_AddAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doAddAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_AddAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doAddAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_AddAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

static void doAddAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doAddAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doAddAssign_R_I( WRValue* to, WRValue* from ) { wr_AddAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doAddAssign_R_F( WRValue* to, WRValue* from ) { wr_AddAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
static void doAddAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
static void doAddAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

static void doAddAssign_F_F( WRValue* to, WRValue* from ) { to->f += from->f; }
static void doAddAssign_I_I( WRValue* to, WRValue* from ) { to->i += from->i; }
static void doAddAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i + from->f; }
static void doAddAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f += (float)from->i; }

WRVoidFunc wr_AddAssign[36] = 
{
	doAddAssign_I_I,  doAddAssign_I_R,  doAddAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_I_Y,
	doAddAssign_R_I,  doAddAssign_R_R,  doAddAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_R_Y,
	doAddAssign_F_I,  doAddAssign_F_R,  doAddAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doAddAssign_Y_I,  doAddAssign_Y_R,  doAddAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doAddAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doMultiplyAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_MultiplyAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doMultiplyAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_MultiplyAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doMultiplyAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_MultiplyAssign[element.type*6+WR_FLOAT]( &element, from );

	wr_floatValueToArray( to, element.f );
	*from = element;
}

static void doMultiplyAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_MultiplyAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doMultiplyAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_MultiplyAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doMultiplyAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_MultiplyAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

static void doMultiplyAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doMultiplyAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doMultiplyAssign_R_I( WRValue* to, WRValue* from ) { wr_MultiplyAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doMultiplyAssign_R_F( WRValue* to, WRValue* from ) { wr_MultiplyAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
static void doMultiplyAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
static void doMultiplyAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_MultiplyAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

static void doMultiplyAssign_F_F( WRValue* to, WRValue* from ) { to->f *= from->f; }
static void doMultiplyAssign_I_I( WRValue* to, WRValue* from ) { to->i *= from->i; }
static void doMultiplyAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i * from->f; }
static void doMultiplyAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f *= (float)from->i; }

WRVoidFunc wr_MultiplyAssign[36] = 
{
	doMultiplyAssign_I_I,  doMultiplyAssign_I_R,  doMultiplyAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_I_Y,
	doMultiplyAssign_R_I,  doMultiplyAssign_R_R,  doMultiplyAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_R_Y,
	doMultiplyAssign_F_I,  doMultiplyAssign_F_R,  doMultiplyAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doMultiplyAssign_Y_I,  doMultiplyAssign_Y_R,  doMultiplyAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doMultiplyAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doDivideAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_DivideAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doDivideAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_DivideAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doDivideAssign_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_DivideAssign[element.type*6+WR_FLOAT]( &element, from );

	wr_floatValueToArray( to, element.f );
	*from = element;
}

static void doDivideAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_DivideAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doDivideAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_DivideAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doDivideAssign_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_DivideAssign[WR_FLOAT*6+element.type](to, &element);
	*from = *to;
}

static void doDivideAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doDivideAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doDivideAssign_R_I( WRValue* to, WRValue* from ) { wr_DivideAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doDivideAssign_R_F( WRValue* to, WRValue* from ) { wr_DivideAssign[to->r->type*6+WR_FLOAT](to->r, from); *from = *to->r; }
static void doDivideAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }
static void doDivideAssign_F_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[WR_FLOAT*6+from->r->type](to, from->r); *from = *to; }

static void doDivideAssign_F_F( WRValue* to, WRValue* from ) { to->f /= from->f; }
static void doDivideAssign_I_I( WRValue* to, WRValue* from ) { to->i /= from->i; }
static void doDivideAssign_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; to->f = (float)to->i / from->f; }
static void doDivideAssign_F_I( WRValue* to, WRValue* from ) { from->type = WR_FLOAT; to->f /= (float)from->i; }

WRVoidFunc wr_DivideAssign[36] = 
{
	doDivideAssign_I_I,  doDivideAssign_I_R,  doDivideAssign_I_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_I_Y,
	doDivideAssign_R_I,  doDivideAssign_R_R,  doDivideAssign_R_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_R_Y,
	doDivideAssign_F_I,  doDivideAssign_F_R,  doDivideAssign_F_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_F_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doDivideAssign_Y_I,  doDivideAssign_Y_R,  doDivideAssign_Y_F,  doVoidFuncBlank, doVoidFuncBlank, doDivideAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doModAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ModAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doModAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_ModAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doModAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_ModAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doModAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ModAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doModAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doModAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doModAssign_R_I( WRValue* to, WRValue* from ) { wr_ModAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doModAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ModAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doModAssign_I_I( WRValue* to, WRValue* from ) { to->i %= from->i; }

WRVoidFunc wr_ModAssign[36] = 
{
	doModAssign_I_I,  doModAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_I_Y,
	doModAssign_R_I,  doModAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doModAssign_Y_I,  doModAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doModAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doORAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ORAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doORAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_ORAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doORAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_ORAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doORAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ORAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doORAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doORAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doORAssign_R_I( WRValue* to, WRValue* from ) { wr_ORAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doORAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ORAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doORAssign_I_I( WRValue* to, WRValue* from ) { to->i |= from->i; }

WRVoidFunc wr_ORAssign[36] = 
{
	doORAssign_I_I,  doORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_I_Y,
	doORAssign_R_I,  doORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doORAssign_Y_I,  doORAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doORAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doANDAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ANDAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doANDAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_ANDAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doANDAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_ANDAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doANDAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_ANDAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doANDAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doANDAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doANDAssign_R_I( WRValue* to, WRValue* from ) { wr_ANDAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doANDAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_ANDAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doANDAssign_I_I( WRValue* to, WRValue* from ) { to->i &= from->i; }

WRVoidFunc wr_ANDAssign[36] = 
{
	doANDAssign_I_I,  doANDAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_I_Y,
	doANDAssign_R_I,  doANDAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doANDAssign_Y_I,  doANDAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doANDAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doXORAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_XORAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doXORAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_XORAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doXORAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_XORAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doXORAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_XORAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doXORAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doXORAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doXORAssign_R_I( WRValue* to, WRValue* from ) { wr_XORAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doXORAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_XORAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doXORAssign_I_I( WRValue* to, WRValue* from ) { to->i ^= from->i; }

WRVoidFunc wr_XORAssign[36] = 
{
	doXORAssign_I_I,  doXORAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_I_Y,
	doXORAssign_R_I,  doXORAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doXORAssign_Y_I,  doXORAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doXORAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doRightShiftAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_RightShiftAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doRightShiftAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_RightShiftAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doRightShiftAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_RightShiftAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doRightShiftAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_RightShiftAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doRightShiftAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doRightShiftAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doRightShiftAssign_R_I( WRValue* to, WRValue* from ) { wr_RightShiftAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doRightShiftAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_RightShiftAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doRightShiftAssign_I_I( WRValue* to, WRValue* from ) { to->i >>= from->i; }

WRVoidFunc wr_RightShiftAssign[36] = 
{
	doRightShiftAssign_I_I,  doRightShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_I_Y,
	doRightShiftAssign_R_I,  doRightShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doRightShiftAssign_Y_I,  doRightShiftAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doRightShiftAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doLeftShiftAssign_R_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_LeftShiftAssign[to->r->type*6+element.type](to->r, &element);
	*from = *to->r;
}

static void doLeftShiftAssign_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );

	wr_LeftShiftAssign[element.type*6+WR_INT]( &element, from );

	wr_intValueToArray( to, element.i );
	*from = element;
}

static void doLeftShiftAssign_Y_Y( WRValue* to, WRValue* from ) 
{
	WRValue element;
	wr_arrayToValue( from, &element );

	wr_LeftShiftAssign[WR_REFARRAY*6+element.type]( to, &element );
	wr_arrayToValue( to, from );
}

static void doLeftShiftAssign_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_LeftShiftAssign[WR_INT*6+element.type](to, &element);
	*from = *to;
}

static void doLeftShiftAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[to->r->type*6+temp.type](to->r, &temp); *from = *to->r; }
static void doLeftShiftAssign_Y_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[WR_REFARRAY*6+temp.type]( to, &temp ); wr_arrayToValue( to, from ); }
static void doLeftShiftAssign_R_I( WRValue* to, WRValue* from ) { wr_LeftShiftAssign[to->r->type*6+WR_INT](to->r, from); *from = *to->r; }
static void doLeftShiftAssign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_LeftShiftAssign[WR_INT*6+temp.type](to, &temp); *from = *to; }

static void doLeftShiftAssign_I_I( WRValue* to, WRValue* from ) { to->i <<= from->i; }

WRVoidFunc wr_LeftShiftAssign[36] = 
{
	doLeftShiftAssign_I_I,  doLeftShiftAssign_I_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_I_Y,
	doLeftShiftAssign_R_I,  doLeftShiftAssign_R_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_R_Y,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doVoidFuncBlank,
	doLeftShiftAssign_Y_I,  doLeftShiftAssign_Y_R,  doVoidFuncBlank,  doVoidFuncBlank, doVoidFuncBlank, doLeftShiftAssign_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryAddition_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+from->type](&element, from, target);
}

static void doBinaryAddition_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryAddition[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryAddition_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryAddition_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryAddition[element.type*6+WR_FLOAT](&element, from, target);
}

static void doBinaryAddition_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryAddition[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryAddition_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryAddition[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryAddition_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryAddition[WR_FLOAT*6+element.type](to, &element, target);
}

static void doBinaryAddition_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryAddition_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+WR_FLOAT](to->r, from, target); }
static void doBinaryAddition_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryAddition_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[to->r->type*6+WR_INT](to->r, from, target); }
static void doBinaryAddition_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAddition[WR_FLOAT*6+from->r->type](to, from->r, target); }

static void doBinaryAddition_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i + from->i; }
static void doBinaryAddition_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)to->i + from->f; }
static void doBinaryAddition_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f + (float)from->i; }
static void doBinaryAddition_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f + from->f; }

WRTargetFunc wr_binaryAddition[36] = 
{
	doBinaryAddition_I_I,  doBinaryAddition_I_R,  doBinaryAddition_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_I_Y,
	doBinaryAddition_R_I,  doBinaryAddition_R_R,  doBinaryAddition_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_R_Y,
	doBinaryAddition_F_I,  doBinaryAddition_F_R,  doBinaryAddition_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryAddition_Y_I,  doBinaryAddition_Y_R,  doBinaryAddition_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAddition_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryMultiply_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+from->type](&element, from, target);
}

static void doBinaryMultiply_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryMultiply[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryMultiply_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryMultiply_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryMultiply[element.type*6+WR_FLOAT](&element, from, target);
}

static void doBinaryMultiply_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryMultiply[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryMultiply_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryMultiply[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryMultiply_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryMultiply[WR_FLOAT*6+element.type](to, &element, target);
}

static void doBinaryMultiply_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryMultiply_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+WR_FLOAT](to->r, from, target); }
static void doBinaryMultiply_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryMultiply_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[to->r->type*6+WR_INT](to->r, from, target); }
static void doBinaryMultiply_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMultiply[WR_FLOAT*6+from->r->type](to, from->r, target); }

static void doBinaryMultiply_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i * from->i; }
static void doBinaryMultiply_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)to->i * from->f; }
static void doBinaryMultiply_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f * (float)from->i; }
static void doBinaryMultiply_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f * from->f; }

WRTargetFunc wr_binaryMultiply[36] = 
{
	doBinaryMultiply_I_I,  doBinaryMultiply_I_R,  doBinaryMultiply_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_I_Y,
	doBinaryMultiply_R_I,  doBinaryMultiply_R_R,  doBinaryMultiply_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_R_Y,
	doBinaryMultiply_F_I,  doBinaryMultiply_F_R,  doBinaryMultiply_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryMultiply_Y_I,  doBinaryMultiply_Y_R,  doBinaryMultiply_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMultiply_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinarySubtract_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+from->type](&element, from, target);
}

static void doBinarySubtract_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binarySubtract[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinarySubtract_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+WR_INT](&element, from, target);
}

static void doBinarySubtract_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binarySubtract[element.type*6+WR_FLOAT](&element, from, target);
}

static void doBinarySubtract_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binarySubtract[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinarySubtract_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binarySubtract[WR_INT*6+element.type](to, &element, target);
}

static void doBinarySubtract_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binarySubtract[WR_FLOAT*6+element.type](to, &element, target);
}

static void doBinarySubtract_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinarySubtract_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+WR_FLOAT](to->r, from, target); }
static void doBinarySubtract_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinarySubtract_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[to->r->type*6+WR_INT](to->r, from, target); }
static void doBinarySubtract_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binarySubtract[WR_FLOAT*6+from->r->type](to, from->r, target); }

static void doBinarySubtract_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i - from->i; }
static void doBinarySubtract_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)to->i - from->f; }
static void doBinarySubtract_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - (float)from->i; }
static void doBinarySubtract_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f - from->f; }

WRTargetFunc wr_binarySubtract[36] = 
{
	doBinarySubtract_I_I,  doBinarySubtract_I_R,  doBinarySubtract_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_I_Y,
	doBinarySubtract_R_I,  doBinarySubtract_R_R,  doBinarySubtract_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_R_Y,
	doBinarySubtract_F_I,  doBinarySubtract_F_R,  doBinarySubtract_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_F_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinarySubtract_Y_I,  doBinarySubtract_Y_R,  doBinarySubtract_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinarySubtract_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryDivide_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+from->type](&element, from, target);
}

static void doBinaryDivide_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryDivide[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryDivide_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryDivide_Y_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryDivide[element.type*6+WR_FLOAT](&element, from, target);
}

static void doBinaryDivide_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryDivide[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryDivide_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryDivide[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryDivide_F_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryDivide[WR_FLOAT*6+element.type](to, &element, target);
}

static void doBinaryDivide_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryDivide_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+WR_FLOAT](to->r, from, target); }
static void doBinaryDivide_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryDivide_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[to->r->type*6+WR_INT](to->r, from, target); }
static void doBinaryDivide_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryDivide[WR_FLOAT*6+from->r->type](to, from->r, target); }

static void doBinaryDivide_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i / from->i; }
static void doBinaryDivide_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = (float)to->i / from->f; }
static void doBinaryDivide_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / (float)from->i; }
static void doBinaryDivide_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_FLOAT; target->f = to->f / from->f; }

WRTargetFunc wr_binaryDivide[36] = 
{
	doBinaryDivide_I_I,  doBinaryDivide_I_R,  doBinaryDivide_I_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_I_Y,
	doBinaryDivide_R_I,  doBinaryDivide_R_R,  doBinaryDivide_R_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_R_Y,
	doBinaryDivide_F_I,  doBinaryDivide_F_R,  doBinaryDivide_F_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_F_Y,
  	 doTargetFuncBlank,   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,   doTargetFuncBlank,
	 doTargetFuncBlank,   doTargetFuncBlank,   doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,   doTargetFuncBlank,
	doBinaryDivide_Y_I,  doBinaryDivide_Y_R,  doBinaryDivide_Y_F,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryDivide_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryMod_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryMod[element.type*6+from->type](&element, from, target);
}

static void doBinaryMod_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryMod[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryMod_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryMod[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryMod_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryMod[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryMod_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryMod[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryMod_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryMod_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryMod_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryMod[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryMod_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i % from->i; }

WRTargetFunc wr_binaryMod[36] = 
{
	doBinaryMod_I_I,  doBinaryMod_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_I_Y,
	doBinaryMod_R_I,  doBinaryMod_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryMod_Y_I,  doBinaryMod_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryMod_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryLeftShift_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryLeftShift[element.type*6+from->type](&element, from, target);
}

static void doBinaryLeftShift_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryLeftShift[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryLeftShift_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryLeftShift[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryLeftShift_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryLeftShift[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryLeftShift_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryLeftShift[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryLeftShift_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryLeftShift_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryLeftShift_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryLeftShift[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryLeftShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i << from->i; }

WRTargetFunc wr_binaryLeftShift[36] = 
{
	doBinaryLeftShift_I_I,  doBinaryLeftShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_I_Y,
	doBinaryLeftShift_R_I,  doBinaryLeftShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryLeftShift_Y_I,  doBinaryLeftShift_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryLeftShift_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryRightShift_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryRightShift[element.type*6+from->type](&element, from, target);
}

static void doBinaryRightShift_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryRightShift[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryRightShift_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryRightShift[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryRightShift_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryRightShift[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryRightShift_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryRightShift[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryRightShift_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryRightShift_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryRightShift_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryRightShift[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryRightShift_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i >> from->i; }

WRTargetFunc wr_binaryRightShift[36] = 
{
	doBinaryRightShift_I_I,  doBinaryRightShift_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_I_Y,
	doBinaryRightShift_R_I,  doBinaryRightShift_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryRightShift_Y_I,  doBinaryRightShift_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryRightShift_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryOR_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryOR[element.type*6+from->type](&element, from, target);
}

static void doBinaryOR_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryOR[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryOR_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryOR[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryOR_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryOR[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryOR_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryOR[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryOR_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryOR_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryOR_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryOR[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryOR_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i | from->i; }

WRTargetFunc wr_binaryOR[36] = 
{
	doBinaryOR_I_I,  doBinaryOR_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_I_Y,
	doBinaryOR_R_I,  doBinaryOR_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryOR_Y_I,  doBinaryOR_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryOR_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryAND_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryAND[element.type*6+from->type](&element, from, target);
}

static void doBinaryAND_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryAND[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryAND_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryAND[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryAND_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryAND[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryAND_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryAND[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryAND_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryAND_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryAND_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryAND[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryAND_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i & from->i; }

WRTargetFunc wr_binaryAND[36] = 
{
	doBinaryAND_I_I,  doBinaryAND_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_I_Y,
	doBinaryAND_R_I,  doBinaryAND_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryAND_Y_I,  doBinaryAND_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryAND_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static void doBinaryXOR_Y_R( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryXOR[element.type*6+from->type](&element, from, target);
}

static void doBinaryXOR_R_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryXOR[to->r->type*6+element.type]( to->r, &element, target);
}

static void doBinaryXOR_Y_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	wr_binaryXOR[element.type*6+WR_INT](&element, from, target);
}

static void doBinaryXOR_Y_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	wr_binaryXOR[element1.type*6+element2.type](&element1, &element2, target);
}

static void doBinaryXOR_I_Y( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	wr_binaryXOR[WR_INT*6+element.type](to, &element, target);
}

static void doBinaryXOR_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[WR_INT*6+from->r->type](to, from->r, target); }
static void doBinaryXOR_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[to->r->type*6+from->r->type](to->r, from->r, target); }
static void doBinaryXOR_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_binaryXOR[to->r->type*6+WR_INT](to->r, from, target); }

static void doBinaryXOR_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->type = WR_INT; target->i = to->i ^ from->i; }

WRTargetFunc wr_binaryXOR[36] = 
{
	doBinaryXOR_I_I,  doBinaryXOR_I_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_I_Y,
	doBinaryXOR_R_I,  doBinaryXOR_R_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_R_Y,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doTargetFuncBlank,     doTargetFuncBlank,     doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,     doTargetFuncBlank,
	doBinaryXOR_Y_I,  doBinaryXOR_Y_R,  doTargetFuncBlank,  doTargetFuncBlank,   doTargetFuncBlank,  doBinaryXOR_Y_Y,
};

//==============================================================================


//------------------------------------------------------------------------------
static bool doCompareEQ_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	return wr_CompareEQ[element1.type*6+element2.type](&element1, &element2);
}

static bool doCompareEQ_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareEQ[to->type*6+element.type](to, &element);
}

static bool doCompareEQ_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+from->type](&element, from);
}

static bool doCompareEQ_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+WR_INT](&element, from);
}

static bool doCompareEQ_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareEQ[element.type*6+WR_FLOAT](&element, from);
}

static bool doCompareEQ_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareEQ[WR_INT*6+element.type](to, &element);
}
static bool doCompareEQ_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareEQ[WR_FLOAT*6+element.type](to, &element);
}

static bool doCompareEQ_R_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+from->r->type](to->r, from->r); }
static bool doCompareEQ_R_I( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+WR_INT](to->r, from); }
static bool doCompareEQ_R_F( WRValue* to, WRValue* from ) { return wr_CompareEQ[to->r->type*6+WR_FLOAT](to->r, from); }
static bool doCompareEQ_I_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[WR_INT*6+from->r->type](to, from->r); }
static bool doCompareEQ_F_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[WR_FLOAT*6+from->r->type](to, from->r); }

static bool doCompareEQ_I_I( WRValue* to, WRValue* from ) { return to->i == from->i; }
static bool doCompareEQ_I_F( WRValue* to, WRValue* from ) { to->type = WR_FLOAT; return to->f == from->f; }
static bool doCompareEQ_F_I( WRValue* to, WRValue* from ) { return to->f == (float)from->i; }
static bool doCompareEQ_F_F( WRValue* to, WRValue* from ) { return to->f == from->f; }

WRReturnFunc wr_CompareEQ[36] = 
{
	doCompareEQ_I_I,   doCompareEQ_I_R,   doCompareEQ_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_I_Y,
	doCompareEQ_R_I,   doCompareEQ_R_R,   doCompareEQ_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_R_Y,
	doCompareEQ_F_I,   doCompareEQ_F_R,   doCompareEQ_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareEQ_Y_I,   doCompareEQ_Y_R,   doCompareEQ_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareEQ_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static bool doCompareGT_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	return wr_CompareGT[element1.type*6+element2.type](&element1, &element2);
}

static bool doCompareGT_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareGT[to->type*6+element.type](to, &element);
}

static bool doCompareGT_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+from->type](&element, from);
}

static bool doCompareGT_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+WR_INT](&element, from);
}

static bool doCompareGT_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareGT[element.type*6+WR_FLOAT](&element, from);
}

static bool doCompareGT_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareGT[WR_INT*6+element.type](to, &element);
}
static bool doCompareGT_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareGT[WR_FLOAT*6+element.type](to, &element);
}

static bool doCompareGT_R_R( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+from->r->type](to->r, from->r); }
static bool doCompareGT_R_I( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+WR_INT](to->r, from); }
static bool doCompareGT_R_F( WRValue* to, WRValue* from ) { return wr_CompareGT[to->r->type*6+WR_FLOAT](to->r, from); }
static bool doCompareGT_I_R( WRValue* to, WRValue* from ) { return wr_CompareGT[WR_INT*6+from->r->type](to, from->r); }
static bool doCompareGT_F_R( WRValue* to, WRValue* from ) { return wr_CompareGT[WR_FLOAT*6+from->r->type](to, from->r); }

static bool doCompareGT_I_I( WRValue* to, WRValue* from ) { return to->i > from->i; }
static bool doCompareGT_I_F( WRValue* to, WRValue* from ) { return (float)to->i > from->f; }
static bool doCompareGT_F_I( WRValue* to, WRValue* from ) { return to->f > (float)from->i; }
static bool doCompareGT_F_F( WRValue* to, WRValue* from ) { return to->f > from->f; }

WRReturnFunc wr_CompareGT[36] = 
{
	doCompareGT_I_I,   doCompareGT_I_R,   doCompareGT_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_I_Y,
	doCompareGT_R_I,   doCompareGT_R_R,   doCompareGT_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_R_Y,
	doCompareGT_F_I,   doCompareGT_F_R,   doCompareGT_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareGT_Y_I,   doCompareGT_Y_R,   doCompareGT_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareGT_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static bool doCompareLT_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	return wr_CompareLT[element1.type*6+element2.type](&element1, &element2);
}

static bool doCompareLT_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareLT[to->type*6+element.type](to, &element);
}

static bool doCompareLT_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+from->type](&element, from);
}

static bool doCompareLT_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+WR_INT](&element, from);
}

static bool doCompareLT_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_CompareLT[element.type*6+WR_FLOAT](&element, from);
}

static bool doCompareLT_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareLT[WR_INT*6+element.type](to, &element);
}
static bool doCompareLT_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_CompareLT[WR_FLOAT*6+element.type](to, &element);
}

static bool doCompareLT_R_R( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+from->r->type](to->r, from->r); }
static bool doCompareLT_R_I( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+WR_INT](to->r, from); }
static bool doCompareLT_R_F( WRValue* to, WRValue* from ) { return wr_CompareLT[to->r->type*6+WR_FLOAT](to->r, from); }
static bool doCompareLT_I_R( WRValue* to, WRValue* from ) { return wr_CompareLT[WR_INT*6+from->r->type](to, from->r); }
static bool doCompareLT_F_R( WRValue* to, WRValue* from ) { return wr_CompareLT[WR_FLOAT*6+from->r->type](to, from->r); }

static bool doCompareLT_I_I( WRValue* to, WRValue* from ) { return to->i < from->i; }
static bool doCompareLT_I_F( WRValue* to, WRValue* from ) { return (float)to->i < from->f; }
static bool doCompareLT_F_I( WRValue* to, WRValue* from ) { return to->f < (float)from->i; }
static bool doCompareLT_F_F( WRValue* to, WRValue* from ) { return to->f < from->f; }

WRReturnFunc wr_CompareLT[36] = 
{
	doCompareLT_I_I,   doCompareLT_I_R,   doCompareLT_I_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_I_Y,
	doCompareLT_R_I,   doCompareLT_R_R,   doCompareLT_R_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_R_Y,
	doCompareLT_F_I,   doCompareLT_F_R,   doCompareLT_F_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doCompareLT_Y_I,   doCompareLT_Y_R,   doCompareLT_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doCompareLT_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static bool doLogicalAND_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	return wr_LogicalAND[element1.type*6+element2.type](&element1, &element2);
}

static bool doLogicalAND_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalAND[to->type*6+element.type](to, &element);
}

static bool doLogicalAND_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+from->type](&element, from);
}

static bool doLogicalAND_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+WR_INT](&element, from);
}

static bool doLogicalAND_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalAND[element.type*6+WR_FLOAT](&element, from);
}

static bool doLogicalAND_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalAND[WR_INT*6+element.type](to, &element);
}
static bool doLogicalAND_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalAND[WR_FLOAT*6+element.type](to, &element);
}

static bool doLogicalAND_R_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+from->r->type](to->r, from->r); }
static bool doLogicalAND_R_I( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+WR_INT](to->r, from); }
static bool doLogicalAND_R_F( WRValue* to, WRValue* from ) { return wr_LogicalAND[to->r->type*6+WR_FLOAT](to->r, from); }
static bool doLogicalAND_I_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[WR_INT*6+from->r->type](to, from->r); }
static bool doLogicalAND_F_R( WRValue* to, WRValue* from ) { return wr_LogicalAND[WR_FLOAT*6+from->r->type](to, from->r); }

static bool doLogicalAND_I_I( WRValue* to, WRValue* from ) { return to->i && from->i; }
static bool doLogicalAND_I_F( WRValue* to, WRValue* from ) { return (float)to->i && from->f; }
static bool doLogicalAND_F_I( WRValue* to, WRValue* from ) { return to->f && (float)from->i; }
static bool doLogicalAND_F_F( WRValue* to, WRValue* from ) { return to->f && from->f; }

WRReturnFunc wr_LogicalAND[36] = 
{
	doLogicalAND_I_I,   doLogicalAND_I_R,   doLogicalAND_I_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_I_Y,
	doLogicalAND_R_I,   doLogicalAND_R_R,   doLogicalAND_R_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_R_Y,
	doLogicalAND_F_I,   doLogicalAND_F_R,   doLogicalAND_F_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doLogicalAND_Y_I,   doLogicalAND_Y_R,   doLogicalAND_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalAND_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static bool doLogicalOR_Y_Y( WRValue* to, WRValue* from )
{
	WRValue element1;
	wr_arrayToValue( to, &element1 );
	WRValue element2;
	wr_arrayToValue( from, &element2 );
	return wr_LogicalOR[element1.type*6+element2.type](&element1, &element2);
}

static bool doLogicalOR_R_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalOR[to->type*6+element.type](to, &element);
}

static bool doLogicalOR_Y_R( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+from->type](&element, from);
}

static bool doLogicalOR_Y_I( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+WR_INT](&element, from);
}

static bool doLogicalOR_Y_F( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( to, &element );
	return wr_LogicalOR[element.type*6+WR_FLOAT](&element, from);
}

static bool doLogicalOR_I_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalOR[WR_INT*6+element.type](to, &element);
}
static bool doLogicalOR_F_Y( WRValue* to, WRValue* from )
{
	WRValue element;
	wr_arrayToValue( from, &element );
	return wr_LogicalOR[WR_FLOAT*6+element.type](to, &element);
}

static bool doLogicalOR_R_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+from->r->type](to->r, from->r); }
static bool doLogicalOR_R_I( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+WR_INT](to->r, from); }
static bool doLogicalOR_R_F( WRValue* to, WRValue* from ) { return wr_LogicalOR[to->r->type*6+WR_FLOAT](to->r, from); }
static bool doLogicalOR_I_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[WR_INT*6+from->r->type](to, from->r); }
static bool doLogicalOR_F_R( WRValue* to, WRValue* from ) { return wr_LogicalOR[WR_FLOAT*6+from->r->type](to, from->r); }

static bool doLogicalOR_I_I( WRValue* to, WRValue* from ) { return to->i || from->i; }
static bool doLogicalOR_I_F( WRValue* to, WRValue* from ) { return (float)to->i || from->f; }
static bool doLogicalOR_F_I( WRValue* to, WRValue* from ) { return to->f || (float)from->i; }
static bool doLogicalOR_F_F( WRValue* to, WRValue* from ) { return to->f || from->f; }

WRReturnFunc wr_LogicalOR[36] = 
{
	doLogicalOR_I_I,   doLogicalOR_I_R,   doLogicalOR_I_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_I_Y,
	doLogicalOR_R_I,   doLogicalOR_R_R,   doLogicalOR_R_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_R_Y,
	doLogicalOR_F_I,   doLogicalOR_F_R,   doLogicalOR_F_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_F_Y,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank, doReturnFuncBlank,
	doLogicalOR_Y_I,   doLogicalOR_Y_R,   doLogicalOR_Y_F, doReturnFuncBlank, doReturnFuncBlank,   doLogicalOR_Y_Y,
};
//==============================================================================


//------------------------------------------------------------------------------
static void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value )
{
	// indexing with an int, but what we are indexing is NOT an array,
	// make it one and return a ref
	value->type = WR_ARRAY;
	value->va = c->getSVA( index->i+1 );

	value->r = value->asValueArray() + index->i;
	value->type = WR_REF;

}
static void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value ) 
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
			value->type = WR_REFARRAY;
		}
	}
}

static void doIndex_I_A( WRContext* c, WRValue* index, WRValue* value )
{
	if ( (value->va->m_type&0x3) == SV_VALUE )
	{
		value->r = value->asValueArray() + index->i;
		value->type = WR_REF;
	}
}

static void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_INT](c, index->r, value);
}

static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value )
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

		wr_index[index->r->type*6+WR_REF](c, index->r, value);
	}
}
static void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_FLOAT](c, index->r, value);
}
static void doIndex_R_A( WRContext* c, WRValue* index, WRValue* value )
{
	wr_index[index->r->type*6+WR_ARRAY](c, index->r, value);
}
WRStateFunc wr_index[36] = 
{
	    doIndex_I_X,     doIndex_I_R,     doIndex_I_X, doVoidIndexFunc,     doIndex_I_A,     doIndex_I_R,
	    doIndex_R_I,     doIndex_R_R,     doIndex_R_F, doVoidIndexFunc,     doIndex_R_A, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
};

//------------------------------------------------------------------------------
static void doPreInc_I( WRValue* value ) { ++value->i; }
static void doPreInc_Y( WRValue* value )
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

static void doPreInc_R( WRValue* value )
{
	wr_preinc[ value->r->type ]( value->r );
	*value = *value->r;
}
static void doPreInc_F( WRValue* value )
{
	++value->f;
}

WRUnaryFunc wr_preinc[6] = 
{
	doPreInc_I,  doPreInc_R,  doPreInc_F,  doSingleVoidBlank,  doSingleVoidBlank,  doPreInc_Y
};

//------------------------------------------------------------------------------
static void doPreDec_I( WRValue* value ) { --value->i; }
static void doPreDec_Y( WRValue* value )
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
static void doPreDec_R( WRValue* value )
{
	wr_predec[ value->r->type ]( value->r );
	*value = *value->r;
}

static void doPreDec_F( WRValue* value ) { --value->f; }
WRUnaryFunc wr_predec[6] = 
{
	doPreDec_I,  doPreDec_R,  doPreDec_F,  doSingleVoidBlank,  doSingleVoidBlank,  doPreDec_Y
};


//------------------------------------------------------------------------------
static void doToInt_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void doToInt_F( WRValue* value ) { value->type = WR_INT; value->i = (int32_t)value->f; }
WRUnaryFunc wr_toInt[6] = 
{
	doSingleVoidBlank, doToInt_R,  doToInt_F,  doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank
};

static void doToFloat_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void doToFloat_I( WRValue* value ) { value->type = WR_FLOAT; value->f = (float)value->i; }
WRUnaryFunc wr_toFloat[6] = 
{
	doToFloat_I, doToFloat_R,  doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank
};

static void bitwiseNOT_R( WRValue* value ) { wr_toInt[ value->r->type ]( value->r ); }
static void bitwiseNOT_I( WRValue* value ) { value->i = ~value->i; }
WRUnaryFunc wr_bitwiseNOT[6] = 
{
	bitwiseNOT_I, bitwiseNOT_R,  doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank
};


//------------------------------------------------------------------------------
static void doPostInc_I( WRValue* value, WRValue* stack ) { *stack = *value; ++value->i; }
static void doPostInc_Y( WRValue* value, WRValue* stack )
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

static void doPostInc_R( WRValue* value, WRValue* stack ) { wr_postinc[ value->r->type ]( value->r, stack ); }
static void doPostInc_F( WRValue* value, WRValue* stack ) { *stack = *value; ++value->f; }

WRVoidFunc wr_postinc[6] = 
{
	doPostInc_I,  doPostInc_R,  doPostInc_F,  doVoidFuncBlank,  doVoidFuncBlank, doPostInc_Y
};

//------------------------------------------------------------------------------
static void doPostDec_I( WRValue* value, WRValue* stack ) { *stack = *value; --value->i; }
static void doPostDec_Y( WRValue* value, WRValue* stack )
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

static void doPostDec_R( WRValue* value, WRValue* stack ) { wr_postdec[ value->r->type ]( value->r, stack ); }
static void doPostDec_F( WRValue* value, WRValue* stack ) { *stack = *value; --value->f; }

WRVoidFunc wr_postdec[6] = 
{
	doPostDec_I, doPostDec_R, doPostDec_F, doVoidFuncBlank, doVoidFuncBlank, doPostDec_Y
};

//------------------------------------------------------------------------------
static void doUserHash_X( WRValue* value, WRValue* target, int32_t hash ) { }
static void doUserHash_R( WRValue* value, WRValue* target, int32_t hash ) { wr_UserHash[ value->r->type ]( value->r, target, hash ); }
static void doUserHash_U( WRValue* value, WRValue* target, int32_t hash )
{
	target->type = WR_REF;
	if ( !(target->r = value->u->get(hash)) )
	{
		target->p = 0;
		target->p2 = 0;
	}
}
WRUserHashFunc wr_UserHash[6] = 
{
	doUserHash_X,  doUserHash_R,  doUserHash_X,  doUserHash_U,  doUserHash_X, doUserHash_X
};

//------------------------------------------------------------------------------
static bool doZeroCheck_I( WRValue* value ) { return value->i == 0; }
static bool doZeroCheck_F( WRValue* value ) { return value->f == 0; }
static bool doZeroCheck_R( WRValue* value ) { return wr_ZeroCheck[ value->r->type ]( value->r ); }
static bool doZeroCheck_Y( WRValue* value )
{
	WRValue element;
	wr_arrayToValue( value, &element );
	return wr_ZeroCheck[ element.type ]( &element );
}

WRValueCheckFunc wr_ZeroCheck[6] = 
{
	doZeroCheck_I,  doZeroCheck_R,  doZeroCheck_F,  doSingleBlank, doSingleBlank, doZeroCheck_Y
};


//------------------------------------------------------------------------------
static bool doLogicalNot_I( WRValue* value ) { return value->i == 0; }
static bool doLogicalNot_F( WRValue* value ) { return value->f == 0; }
static bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
static bool doLogicalNot_Y( WRValue* value )
{
	WRValue element;
	wr_arrayToValue( value, &element );
	return wr_LogicalNot[ element.type ]( &element );
}
WRReturnSingleFunc wr_LogicalNot[6] = 
{
	doLogicalNot_I,  doLogicalNot_R,  doLogicalNot_F,  doSingleBlank, doSingleBlank, doLogicalNot_Y
};

//------------------------------------------------------------------------------
static void doNegate_I( WRValue* value ) { value->i = -value->i; }
static void doNegate_Y( WRValue* value )
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
static void doNegate_R( WRValue* value )
{
	wr_negate[ value->r->type ]( value->r );
	*value = *value->r;
}

static void doNegate_F( WRValue* value ) { value->f = -value->f; }
WRUnaryFunc wr_negate[6] = 
{
	doNegate_I,  doNegate_R,  doNegate_F,  doSingleVoidBlank,  doSingleVoidBlank, doNegate_Y
};

//------------------------------------------------------------------------------
static void doBitwiseNot_I( WRValue* value ) { value->i = ~value->i; }
static void doBitwiseNot_R( WRValue* value )
{
	wr_bitwiseNot[ value->r->type ]( value->r );
	*value = *value->r;
}
static void doBitwiseNot_Y( WRValue* value )
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
				val->i = ~val->i;
				*value = *val;
			}
			break;
		}

		case SV_CHAR:
		{
			value->type = WR_INT;
			value->i = (((((char*)value->r->va->m_data)[s]) = ~(((char*)value->r->va->m_data)[s])));
			break;
		}

		case SV_INT:
		{
			value->type = WR_INT;
			value->i = (((((int*)value->r->va->m_data)[s]) = ~(((int*)value->r->va->m_data)[s])));
			break;
		}

		case SV_FLOAT:
		{
			break;
		}
	}
}

WRUnaryFunc wr_bitwiseNot[6] = 
{
	doBitwiseNot_I,  doBitwiseNot_R, doSingleVoidBlank,  doSingleVoidBlank,  doSingleVoidBlank, doBitwiseNot_Y
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

//------------------------------------------------------------------------------
void wr_std_rand( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_INT;

	int32_t	k = wr_Seed / 127773;
	wr_Seed = 16807 * ( wr_Seed - k * 127773 ) - 2836 * k;
	stackTop->i = (uint32_t)wr_Seed % (uint32_t)(stackTop - argn)->asInt();
}

//------------------------------------------------------------------------------
void wr_std_srand( WRValue* stackTop, const int argn )
{
	wr_Seed = (stackTop - argn)->asInt();
}

#include <math.h>

//------------------------------------------------------------------------------
void wr_math_sin( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = sinf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_cos( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = cosf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_tan( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = tanf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_sinh( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = sinhf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_cosh( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = coshf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_tanh( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = tanhf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_asin( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = asinf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_acos( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = acosf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_atan( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = atanf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_atan2( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		stackTop->type = WR_FLOAT;
		stackTop->f = atan2f( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_log( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = logf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_log10( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = log10f( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_exp( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = expf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_sqrt( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = sqrtf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_ceil( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = ceilf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_floor( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = floorf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_abs( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = (float)fabs( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_pow( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		stackTop->type = WR_FLOAT;
		stackTop->f = powf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_fmod( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		stackTop->type = WR_FLOAT;
		stackTop->f = fmodf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_trunc( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = truncf( (stackTop - argn)->asFloat() );
}

//------------------------------------------------------------------------------
void wr_math_ldexp( WRValue* stackTop, const int argn )
{
	if ( argn == 2 )
	{
		stackTop->type = WR_FLOAT;
		stackTop->f = ldexpf( (stackTop - 1)->asFloat(), (stackTop - 2)->asInt() );
	}
}

//------------------------------------------------------------------------------
const float wr_PI = 3.14159265358979323846264338327950288419716939937510582f;
const float wr_toDegrees = (180.f / wr_PI);
const float wr_toRadians = (1.f / wr_toDegrees);

//------------------------------------------------------------------------------
void wr_math_rad2deg( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = wr_toDegrees * (stackTop - argn)->asFloat();
}

//------------------------------------------------------------------------------
void wr_math_deg2rad( WRValue* stackTop, const int argn )
{
	stackTop->type = WR_FLOAT;
	stackTop->f = wr_toRadians * (stackTop - argn)->asFloat();
}

//------------------------------------------------------------------------------
void wr_loadAllLibs( WRState* w )
{
	wr_loadMathLib( w );
	wr_loadStdLib( w );
}

//------------------------------------------------------------------------------
void wr_loadMathLib( WRState* w )
{
	wr_registerLibraryFunction( w, "math::sin", wr_math_sin );
	wr_registerLibraryFunction( w, "math::cos", wr_math_cos );
	wr_registerLibraryFunction( w, "math::tan", wr_math_tan );
	wr_registerLibraryFunction( w, "math::sinh", wr_math_sinh );
	wr_registerLibraryFunction( w, "math::cosh", wr_math_cosh );
	wr_registerLibraryFunction( w, "math::tanh", wr_math_tanh );
	wr_registerLibraryFunction( w, "math::asin", wr_math_asin );
	wr_registerLibraryFunction( w, "math::acos", wr_math_acos );
	wr_registerLibraryFunction( w, "math::atan", wr_math_atan );
	wr_registerLibraryFunction( w, "math::atan2", wr_math_atan2 );
	wr_registerLibraryFunction( w, "math::log", wr_math_log );
	wr_registerLibraryFunction( w, "math::ln", wr_math_log );
	wr_registerLibraryFunction( w, "math::log10", wr_math_log10 );
	wr_registerLibraryFunction( w, "math::exp", wr_math_exp );
	wr_registerLibraryFunction( w, "math::pow", wr_math_pow );
	wr_registerLibraryFunction( w, "math::fmod", wr_math_fmod );
	wr_registerLibraryFunction( w, "math::trunc", wr_math_trunc );
	wr_registerLibraryFunction( w, "math::sqrt", wr_math_sqrt );
	wr_registerLibraryFunction( w, "math::ceil", wr_math_ceil );
	wr_registerLibraryFunction( w, "math::floor", wr_math_floor );
	wr_registerLibraryFunction( w, "math::abs", wr_math_abs );
	wr_registerLibraryFunction( w, "math::ldexp", wr_math_ldexp );

	wr_registerLibraryFunction( w, "math::deg2rad", wr_math_deg2rad );
	wr_registerLibraryFunction( w, "math::rad2deg", wr_math_rad2deg );

}

//------------------------------------------------------------------------------
void wr_loadStdLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::rand", wr_std_rand );
	wr_registerLibraryFunction( w, "std::srand", wr_std_srand );
}
