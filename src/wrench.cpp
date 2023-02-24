#include "wrench.h"
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
#ifndef _UTILS_H
#define _UTILS_H
/*------------------------------------------------------------------------------*/

int wr_itoa( int i, char* string, size_t len );
int wr_ftoa( float f, char* string, size_t len );

#ifndef WRENCH_WITHOUT_COMPILER

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
// "good values" for hash table progression
const uint16_t c_primeTable[] =
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
	6151,
	12289,
	24593,
	49157
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

#ifndef WRENCH_WITHOUT_COMPILER

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
		for( int i=0; i<m_mod; ++i ){ m_list[i].hash = 0; }
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
			for( int i=0; i<m_mod; ++i ){ newList[i].hash = 0; }

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

	friend struct WRCompilationContext;

private:
	
	int m_mod;
	Node* m_list;
};

#endif

//------------------------------------------------------------------------------
enum WRGCObjectType
{
	SV_VALUE = 0x01,           // 0001
	SV_CHAR = 0x02,            // 0010

	SV_VOID_HASH_TABLE = 0x04, // 0100

	SV_HASH_TABLE = 0x03,      // 0011
};

#define IS_SVA_VALUE_TYPE(V) ((V)->m_type & 0x1)

#define INIT_AS_ARRAY      (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ARRAY<<24))
#define INIT_AS_USR        (((uint32_t)WR_EX) | ((uint32_t)WR_EX_USR<<24))
#define INIT_AS_RAW_ARRAY  (((uint32_t)WR_EX) | ((uint32_t)WR_EX_RAW_ARRAY<<24))
#define INIT_AS_REFARRAY   (((uint32_t)WR_EX) | ((uint32_t)WR_EX_REFARRAY<<24))
#define INIT_AS_STRUCT     (((uint32_t)WR_EX) | ((uint32_t)WR_EX_STRUCT<<24))
#define INIT_AS_HASH_TABLE (((uint32_t)WR_EX) | ((uint32_t)WR_EX_HASH_TABLE<<24))
#define INIT_AS_ITERATOR   (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ITERATOR<<24))

#define INIT_AS_REF      WR_REF
#define INIT_AS_INT      WR_INT
#define INIT_AS_FLOAT    WR_FLOAT

#define ARRAY_ELEMENT_FROM_P2(P) (((P)&0x1FFFFF00) >> 8)
#define ARRAY_ELEMENT_TO_P2(P,E) { (P)&=0xE00000FF; (P)|=(E<<8); }

#define IS_EXARRAY_TYPE(P)   ((P)&0xC0)
#define EX_RAW_ARRAY_SIZE_FROM_P2(P) (((P)&0x1FFFFF00) >> 8)
#define IS_EX_SINGLE_CHAR_RAW_P2(P) ((P) == (((uint32_t)WR_EX) | (((uint32_t)WR_EX_RAW_ARRAY<<24)) | (1<<8)))

#define EX_TYPE_MASK   0xE0

#define IS_REFARRAY(X) (((X)&0xE0)==WR_EX_REFARRAY)
#define IS_ITERATOR(X) (((X)&0xE0)==WR_EX_ITERATOR)
#define IS_RAW_ARRAY(X) (((X)&0xE0)==WR_EX_RAW_ARRAY)

#endif
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

#ifndef _GC_ARRAY_H
#define _GC_ARRAY_H

#include "wrench.h"

#include <assert.h>

#define HASH_SCRAMBLER 0x5656ABAB

//------------------------------------------------------------------------------
class WRGCObject
{
public:

	// the order here matters for data alignment
	
	char m_type;
	char m_skipGC;
	uint16_t m_mod;
	uint32_t m_size;

	union
	{
		uint32_t* m_hashTable;
		const unsigned char* m_ROMHashTable;
	};

	union
	{
		const void* m_constData;
		void* m_data;
		int* m_Idata;
		char* m_SCdata;
		unsigned char* m_Cdata;
		WRValue* m_Vdata;
		float* m_Fdata;
	};

	union
	{
		WRGCObject* m_next; // for gc
		void* m_vNext;
	};

	//------------------------------------------------------------------------------
	uint32_t growHash( const uint32_t hash, const uint16_t sizeHint =0 )
	{
		// there was a collision with the passed hash, grow until the
		// collision disappears

		uint16_t start = (sizeHint > m_mod) ? sizeHint : m_mod;
		int t = 0;
		for( ; c_primeTable[t] <= start ; ++t );

		for(;;)
		{
tryAgain:
			int newMod = c_primeTable[t];

			int newSize;
			if ( m_type == SV_VOID_HASH_TABLE )
			{
				newSize = newMod;
			}
			else
			{
				newSize = newMod << 1;
			}

			int total = newMod*sizeof(uint32_t) + newSize*sizeof(WRValue);
			uint32_t* proposed = (uint32_t*)malloc( total );

			memset( (unsigned char *)proposed, 0, total );

			proposed[ hash % newMod ] = hash;

			for( int h=0; h<m_mod; ++h )
			{
				int tries = m_mod >> 3;
				int newEntry = m_hashTable[h] % newMod;
				for(;;)
				{
					if ( proposed[newEntry] == 0 )
					{
						proposed[newEntry] = m_hashTable[h];
						break;
					}
					else if ( tries-- )
					{
						newEntry = (newEntry + 1) % newMod;
					}
					else
					{
						free( proposed );
						++t;
						
						assert( newMod != 49157 );
						
						goto tryAgain;
					}
				}
			}

			WRValue* newValues = (WRValue*)(proposed + newMod);
			
			uint32_t* oldHashTable = m_hashTable;
			m_hashTable = proposed;
			int oldMod = m_mod;
			m_mod = newMod;
			m_size = newSize;
			
			for( int v=0; v<oldMod; ++v )
			{
				if ( !oldHashTable[v] )
				{
					continue;
				}

				unsigned int newPos = getIndexOfHit( oldHashTable[v], true );

				if ( m_type == SV_VOID_HASH_TABLE )
				{
					newValues[newPos] = m_Vdata[v];
				}
				else
				{
					// copy all the new hashes to their new locations
					WRValue* to = newValues + (newPos<<1);
					WRValue* from = m_Vdata + (v<<1);

					// value
					to->p2 = from->p2;
					to->p = from->p;
					from->p2 = INIT_AS_INT;

					// key
					++to;
					++from;
					to->p2 = from->p2;
					to->p = from->p;
					from->p2 = INIT_AS_INT;
				}
			}

			free( oldHashTable );

			m_Vdata = newValues;

			return getIndexOfHit( hash, true );
		}
	}
	
	//------------------------------------------------------------------------------
	void init( const unsigned int size, const WRGCObjectType type )
	{
		memset( (unsigned char*)this, 0, sizeof(WRGCObject) );

		if ( (m_type = (char)type) == SV_VALUE )
		{
			m_size = size;
			m_Cdata = (unsigned char*)malloc( m_size * sizeof(WRValue) );
		}
		else if ( m_type == SV_CHAR )
		{
			m_size = size;
			m_Cdata = (unsigned char*)malloc( m_size + 1 );
			m_Cdata[size] = 0;
		}
		else
		{
			growHash( 0, size );
		}
	}

	//------------------------------------------------------------------------------
	void clear()
	{
		if ( m_type == SV_VALUE || m_type == SV_CHAR )
		{
			free( m_Cdata );
		}
		else
		{
			free( m_hashTable );
		}
	}
	
	//------------------------------------------------------------------------------
	WRValue* getAsRawValueHashTable( const uint32_t hash )
	{
		uint32_t index = getIndexOfHit( hash, false );
		return m_Vdata + index;
	}

	//------------------------------------------------------------------------------
	WRValue* exists( const uint32_t hash, const bool clear )
	{
		uint32_t index = hash % m_mod;
		if ( m_hashTable[index] == hash )
		{
checkReturn:
			if ( clear )
			{
				m_hashTable[index] = 0;
			}
			return m_Vdata + index;
		}

		int tries = m_mod >> 3;
		do
		{
			index = (index + 1) % m_mod;

			if ( m_hashTable[index] == hash )
			{
				goto checkReturn;
			}

		} while( tries-- );

		return 0;
	}

	//------------------------------------------------------------------------------
	uint32_t getIndexOfHit( const uint32_t hash, const bool insert )
	{
		uint32_t index = hash % m_mod;
		if ( m_hashTable[index] == hash )
		{
			return index; // immediate hits should be cheap
		}

		int tries = m_mod >> 3;
		do
		{
			if ( insert && m_hashTable[index] == 0 )
			{
				m_hashTable[index] = hash;
				return index;
			}

			index = (index + 1) % m_mod;

			if ( m_hashTable[index] == hash )
			{
				return index;
			}

		} while( tries-- );

		return insert ? growHash(hash) : getIndexOfHit( hash, true );
	}

	//------------------------------------------------------------------------------
	void* get( const uint32_t l )
	{
		int s = l < m_size ? l : m_size - 1;

		if ( m_type == SV_CHAR )
		{
			return m_Cdata + s;
		}
		else if ( m_type == SV_HASH_TABLE )
		{
			// zero remains "null hash" by scrambling. Which means
			// a hash of HASH_SCRAMBLER will break the system. If you
			// are reading this comment then I'm sorry, the only
			// way out is to re-architect so this hash is not
			// generated by your program
			assert( l != HASH_SCRAMBLER );

			uint32_t hash = l ^ HASH_SCRAMBLER; 

			s = getIndexOfHit(hash, false) << 1;
		}
		else if ( m_type == SV_VOID_HASH_TABLE )
		{
			return getAsRawValueHashTable( l );
		}
		// else it must be SV_VALUE

		return m_Vdata + s;
	}

private:
	WRGCObject& operator= ( WRGCObject& A );
	WRGCObject(WRGCObject& A);
};

#endif
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
	uint16_t globals;

	union
	{
		WRFunction* localFunctions;
		uint32_t contextId;
	};
	
	const unsigned char* bottom;
	const unsigned char* stopLocation;
	
	WRGCObject* svAllocated;
	
	WRState* w;
	
	WRGCObject registry; // the 'next' pointer in this registry is used as the context LL next

	void mark( WRValue* s );
	void gc( WRValue* stackTop );
	WRGCObject* getSVA( int size, WRGCObjectType type, bool init );
};

//------------------------------------------------------------------------------
struct WRState
{
	uint16_t stackSize;
	int8_t err;

	WRValue* stack;

	WRContext* contextList;

	WRGCObject globalRegistry;
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

typedef void (*WRSingleTargetFunc)( WRValue* value, WRValue* target );
extern WRSingleTargetFunc wr_negate[4];

typedef void (*WRUnaryFunc)( WRValue* value );
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
#ifndef _OPCODE_H
#define _OPCODE_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROpcode
{
	O_RegisterFunction = 0,

	O_LiteralInt32,
	O_LiteralZero,
	O_LiteralFloat,
	O_LiteralString,

	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,
	O_PushIndexFunctionReturnValue,

	O_CallLibFunction,

	O_NewObjectTable,
	O_AssignToObjectTableByOffset,

	O_AssignToHashTableAndPop,
	O_RemoveFromHashTable,
	O_HashEntryExists,

	O_PopOne,
	O_ReturnZero,
	O_Return,
	O_Stop,

	O_Index,
	O_IndexSkipLoad,
	O_CountOf,
	O_HashOf,

	O_StackIndexHash,
	O_GlobalIndexHash,
	O_LocalIndexHash,

	O_StackSwap,
	O_SwapTwoToTop,

	O_LoadFromLocal,
	O_LoadFromGlobal,

	O_LLValues,
	O_LGValues,
	O_GLValues,
	O_GGValues,

	O_BinaryRightShiftSkipLoad,
	O_BinaryLeftShiftSkipLoad,
	O_BinaryAndSkipLoad,
	O_BinaryOrSkipLoad,
	O_BinaryXORSkipLoad,
	O_BinaryModSkipLoad,

	O_BinaryMultiplication,
	O_BinarySubtraction,
	O_BinaryDivision,
	O_BinaryRightShift,
	O_BinaryLeftShift,
	O_BinaryMod,
	O_BinaryOr,
	O_BinaryXOR,
	O_BinaryAnd,
	O_BinaryAddition,

	O_BitwiseNOT,

	O_RelativeJump,
	O_RelativeJump8,

	O_BZ,
	O_BZ8,

	O_LogicalAnd,
	O_LogicalOr,
	O_CompareLE,
	O_CompareGE,
	O_CompareGT,
	O_CompareLT,
	O_CompareEQ,
	O_CompareNE,

	O_GGCompareGT,
	O_GGCompareGE,
	O_GGCompareLT,
	O_GGCompareLE,
	O_GGCompareEQ, 
	O_GGCompareNE, 

	O_LLCompareGT,
	O_LLCompareGE,
	O_LLCompareLT,
	O_LLCompareLE,
	O_LLCompareEQ, 
	O_LLCompareNE, 

	O_GSCompareEQ,
	O_LSCompareEQ, 
	O_GSCompareNE, 
	O_LSCompareNE, 
	O_GSCompareGE,
	O_LSCompareGE,
	O_GSCompareLE,
	O_LSCompareLE,
	O_GSCompareGT,
	O_LSCompareGT,
	O_GSCompareLT,
	O_LSCompareLT,

	O_GSCompareEQBZ, 
	O_LSCompareEQBZ, 
	O_GSCompareNEBZ, 
	O_LSCompareNEBZ, 
	O_GSCompareGEBZ,
	O_LSCompareGEBZ,
	O_GSCompareLEBZ,
	O_LSCompareLEBZ,
	O_GSCompareGTBZ,
	O_LSCompareGTBZ,
	O_GSCompareLTBZ,
	O_LSCompareLTBZ,

	O_GSCompareEQBZ8,
	O_LSCompareEQBZ8,
	O_GSCompareNEBZ8,
	O_LSCompareNEBZ8,
	O_GSCompareGEBZ8,
	O_LSCompareGEBZ8,
	O_GSCompareLEBZ8,
	O_LSCompareLEBZ8,
	O_GSCompareGTBZ8,
	O_LSCompareGTBZ8,
	O_GSCompareLTBZ8,
	O_LSCompareLTBZ8,

	O_LLCompareLTBZ,
	O_LLCompareLEBZ,
	O_LLCompareGTBZ,
	O_LLCompareGEBZ,
	O_LLCompareEQBZ,
	O_LLCompareNEBZ,

	O_GGCompareLTBZ,
	O_GGCompareLEBZ,
	O_GGCompareGTBZ,
	O_GGCompareGEBZ,
	O_GGCompareEQBZ,
	O_GGCompareNEBZ,

	O_LLCompareLTBZ8,
	O_LLCompareLEBZ8,
	O_LLCompareGTBZ8,
	O_LLCompareGEBZ8,
	O_LLCompareEQBZ8,
	O_LLCompareNEBZ8,

	O_GGCompareLTBZ8,
	O_GGCompareLEBZ8,
	O_GGCompareGTBZ8,
	O_GGCompareGEBZ8,
	O_GGCompareEQBZ8,
	O_GGCompareNEBZ8,

	O_PostIncrement,
	O_PostDecrement,
	O_PreIncrement,
	O_PreDecrement,

	O_PreIncrementAndPop,
	O_PreDecrementAndPop,

	O_IncGlobal,
	O_DecGlobal,
	O_IncLocal,
	O_DecLocal,

	O_Assign,
	O_AssignAndPop,
	O_AssignToGlobalAndPop,
	O_AssignToLocalAndPop,
	O_AssignToArrayAndPop,

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

	O_LogicalNot, //X
	O_Negate,

	O_LiteralInt8,
	O_LiteralInt16,

	O_IndexLiteral8,
	O_IndexLiteral16,

	O_IndexLocalLiteral8,
	O_IndexGlobalLiteral8,
	O_IndexLocalLiteral16,
	O_IndexGlobalLiteral16,

	O_BinaryAdditionAndStoreGlobal,
	O_BinarySubtractionAndStoreGlobal,
	O_BinaryMultiplicationAndStoreGlobal,
	O_BinaryDivisionAndStoreGlobal,

	O_BinaryAdditionAndStoreLocal,
	O_BinarySubtractionAndStoreLocal,
	O_BinaryMultiplicationAndStoreLocal,
	O_BinaryDivisionAndStoreLocal,

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

	O_BLA,
	O_BLA8,
	O_BLO,
	O_BLO8,

	O_LiteralInt8ToGlobal,
	O_LiteralInt16ToGlobal,
	O_LiteralInt32ToLocal,
	O_LiteralInt8ToLocal,
	O_LiteralInt16ToLocal,
	O_LiteralFloatToGlobal,
	O_LiteralFloatToLocal,
	O_LiteralInt32ToGlobal,

	O_GGBinaryMultiplication,
	O_GLBinaryMultiplication,
	O_LLBinaryMultiplication,

	O_GGBinaryAddition,
	O_GLBinaryAddition,
	O_LLBinaryAddition,

	O_GGBinarySubtraction,
	O_GLBinarySubtraction,
	O_LGBinarySubtraction,
	O_LLBinarySubtraction,

	O_GGBinaryDivision,
	O_GLBinaryDivision,
	O_LGBinaryDivision,
	O_LLBinaryDivision,

	O_GC_Command,

	O_GPushIterator,
	O_LPushIterator,
	O_GGNextKeyValueOrJump,
	O_GLNextKeyValueOrJump,
	O_LGNextKeyValueOrJump,
	O_LLNextKeyValueOrJump,
	O_GNextValueOrJump,
	O_LNextValueOrJump,

	O_Switch,
	O_SwitchLinear,

	O_GlobalStop,

	O_ToInt,
	O_ToFloat,

	O_CallLibFunctionAndPop,

	// non-interpreted opcodes
	O_HASH_PLACEHOLDER,
	O_FUNCTION_CALL_PLACEHOLDER,

	O_LAST,
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
#ifndef _STR_H
#define _STR_H
/* ------------------------------------------------------------------------- */

#ifndef WRENCH_WITHOUT_COMPILER

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
#else
	bool fileToBuffer( const char* fileName, const bool appendToBuffer =false ) { return false; }
	bool bufferToFile( const char* fileName, const bool append =false ) const { return false; }
#endif

	WRstr& set( const char* buf, const unsigned int len ) { m_len = 0; m_str[0] = 0; return insert( buf, len ); }
	WRstr& set( const WRstr& str ) { return set( str.m_str, str.m_len ); }
	WRstr& set( const char c ) { clear(); m_str[0]=c; m_str[1]=0; m_len = 1; return *this; }

	inline WRstr& truncate( const unsigned int newLen ); // reduce size to 'newlen'
	WRstr& shave( const unsigned int e ) { return (e > m_len) ? clear() : truncate(m_len - e); } // remove 'x' trailing characters
	
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
	WRstr& operator = ( const WRstr* str ) { if ( !str ) { clear(); } else if ( this != str ) { set(*str, str->size()); } return *this; }
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
#ifndef _OPCODE_STREAM_H
#define _OPCODE_STREAM_H
#ifndef WRENCH_WITHOUT_COMPILER
/*------------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
class WROpcodeStream
{
public:
	WROpcodeStream() { m_buf = 0; clear(); }
	~WROpcodeStream() { clear(); }
	WROpcodeStream& clear()
	{
		m_len = 0;
		delete[] m_buf;
		m_buf = 0;
		m_bufLen = 0;
		return *this;
	}

	unsigned int size() const 
	{
		return m_len; 
	}

	WROpcodeStream (const WROpcodeStream &other ) { m_buf = 0; *this = other; }
	WROpcodeStream& operator = ( const WROpcodeStream& str )
	{
		clear();
		if ( str.m_len )
		{
			*this += str;
		}
		return *this;
	}

	WROpcodeStream& operator += ( const WROpcodeStream& stream ) { return append(stream.m_buf, stream.m_len); }
	WROpcodeStream& operator += ( const unsigned char data ) { return append(&data, 1); }
	WROpcodeStream& append( const unsigned char* data, const int size )
	{
		if ( (size + m_len) >= m_bufLen )
		{
			unsigned char* buf = m_buf;
			m_bufLen = size + m_len + 16;
			m_buf = new unsigned char[ m_bufLen ];
			if ( m_len )
			{
				memcpy( m_buf, buf, m_len );
				delete[] buf;
			}
		}

		memcpy( m_buf + m_len, data, size );
		m_len += size;
		return *this;

	}

	unsigned char* p_str( int offset =0 ) { return m_buf + offset; }
	operator const unsigned char*() const { return m_buf; }
	unsigned char& operator[]( const int l ) { return *p_str(l); }

	WROpcodeStream& shave( const unsigned int e )
	{
		m_len -= e;
		return *this;
	}

	unsigned int release( unsigned char** toBuf )
	{
		unsigned int retLen = m_len;
		*toBuf = m_buf;
		m_buf = 0;
		clear();
		return retLen;
	}

private:
	unsigned char *m_buf;
	unsigned int m_len;
	unsigned int m_bufLen;
};

#endif

#endif/*******************************************************************************
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
#ifndef _CC_H
#define _CC_H
/*------------------------------------------------------------------------------*/

#ifndef WRENCH_WITHOUT_COMPILER

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
	WROpcode alt;
};

//------------------------------------------------------------------------------
// reference:
// https://en.cppreference.com/w/cpp/language/operator_precedence
const WROperation c_operations[] =
{
//       precedence                      L2R      type             alt

	{ "==",  10, O_CompareEQ,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "!=",  10, O_CompareNE,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ ">=",   9, O_CompareGE,           true,  WR_OPER_BINARY, O_CompareLE },
	{ "<=",   9, O_CompareLE,           true,  WR_OPER_BINARY, O_CompareGE },
	{ ">",    9, O_CompareGT,           true,  WR_OPER_BINARY, O_CompareLT },
	{ "<",    9, O_CompareLT,           true,  WR_OPER_BINARY, O_CompareGT },
	{ "&&",  14, O_LogicalAnd,          true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "||",  15, O_LogicalOr,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },

	{ "++",   3, O_PreIncrement,        true,  WR_OPER_PRE, O_LAST },
	{ "++",   2, O_PostIncrement,       true,  WR_OPER_POST, O_LAST },

	{ "--",   3, O_PreDecrement,        true,  WR_OPER_PRE, O_LAST },
	{ "--",   2, O_PostDecrement,       true,  WR_OPER_POST, O_LAST },

	{ ".",    2, O_HASH_PLACEHOLDER,    true,  WR_OPER_BINARY, O_LAST },

	{ "!",    3, O_LogicalNot,         false,  WR_OPER_PRE, O_LAST },
	{ "~",    3, O_BitwiseNOT,         false,  WR_OPER_PRE, O_LAST },
	{ "-",    3, O_Negate,             false,  WR_OPER_PRE, O_LAST },

	{ "+",    6, O_BinaryAddition,      true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "-",    6, O_BinarySubtraction,   true,  WR_OPER_BINARY, O_LAST },
	{ "*",    5, O_BinaryMultiplication,true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "/",    5, O_BinaryDivision,      true,  WR_OPER_BINARY, O_LAST },
	{ "%",    6, O_BinaryMod,           true,  WR_OPER_BINARY, O_LAST },

	{ "|",   13, O_BinaryOr,            true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "&",   11, O_BinaryAnd,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "^",   11, O_BinaryXOR,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },

	{ ">>",   7, O_BinaryRightShift,    true,  WR_OPER_BINARY, O_LAST },
	{ "<<",   7, O_BinaryLeftShift,     true,  WR_OPER_BINARY, O_LAST },

	{ "+=",  16, O_AddAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "-=",  16, O_SubtractAssign,      true,  WR_OPER_BINARY, O_LAST },
	{ "%=",  16, O_ModAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "*=",  16, O_MultiplyAssign,      true,  WR_OPER_BINARY, O_LAST },
	{ "/=",  16, O_DivideAssign,        true,  WR_OPER_BINARY, O_LAST },
	{ "|=",  16, O_ORAssign,            true,  WR_OPER_BINARY, O_LAST },
	{ "&=",  16, O_ANDAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "^=",  16, O_XORAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ ">>=", 16, O_RightShiftAssign,   false,  WR_OPER_BINARY, O_LAST },
	{ "<<=", 16, O_LeftShiftAssign,    false,  WR_OPER_BINARY, O_LAST },

	{ "=",   16, O_Assign,             false,  WR_OPER_BINARY, O_LAST },

	{ "@i",  3, O_ToInt,               false,  WR_OPER_PRE, O_LAST },
	{ "@f",  3, O_ToFloat,             false,  WR_OPER_PRE, O_LAST },

	{ "@[]",  2, O_Index,               true,  WR_OPER_POST, O_LAST },
	{ "@init", 2, O_Index,              true,  WR_OPER_POST, O_LAST },

	{ "@macroBegin", 0, O_LAST,         true,  WR_OPER_POST, O_LAST },

	{ "._count", 2, O_CountOf,          true,  WR_OPER_POST, O_LAST },
	{ "._hash",  2, O_HashOf,           true,  WR_OPER_POST, O_LAST },
	{ "._remove", 2, O_RemoveFromHashTable, true,  WR_OPER_POST, O_LAST },
	{ "._exists", 2, O_HashEntryExists, true,  WR_OPER_POST, O_LAST },
	
	{ 0, 0, O_LAST, false, WR_OPER_PRE, O_LAST },
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
	uint32_t gotoHash;
	
	BytecodeJumpOffset() : offset(0), gotoHash(0) {}
};

//------------------------------------------------------------------------------
struct GotoSource
{
	uint32_t hash;
	int offset;
};

//------------------------------------------------------------------------------
struct WRBytecode
{
	WROpcodeStream all;
	WROpcodeStream opcodes;

	bool isStructSpace;

	WRarray<WRNamespaceLookup> localSpace;
	WRarray<WRNamespaceLookup> functionSpace;
	WRarray<WRNamespaceLookup> unitObjectSpace;

	void invalidateOpcodeCache() { opcodes.clear(); }
	
	WRarray<BytecodeJumpOffset> jumpOffsetTargets;
	WRarray<GotoSource> gotoSource;
	
	void clear()
	{
		all.clear();
		opcodes.clear();
		isStructSpace = false;
		localSpace.clear();
		functionSpace.clear();
		unitObjectSpace.clear();
		
		jumpOffsetTargets.clear();
		gotoSource.clear();
		
		isStructSpace = false;
	}
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
	WRstr literalString;
	const WROperation* operation;
	
	int stackPosition;
	
	WRBytecode bytecode;

	WRExpressionContext() { reset(); }

	void setLocalSpace( WRarray<WRNamespaceLookup>& localSpace, bool isStructSpace )
	{
		bytecode.localSpace.clear();
		bytecode.isStructSpace = isStructSpace;
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
		operation = 0;

		return this;
	}
};

//------------------------------------------------------------------------------
class WRExpression
{
public:
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
	void swapWithTop( int stackPosition, bool addOpcodes =true );
	
	WRExpression() { reset(); }
	WRExpression( WRarray<WRNamespaceLookup>& localSpace, bool isStructSpace )
	{
		reset();
		bytecode.isStructSpace = isStructSpace;
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
struct ConstantValue
{
	WRValue value;
	WRstr label;
	ConstantValue() { value.init(); }
};

//------------------------------------------------------------------------------
struct WRUnitContext
{
	uint32_t hash; // hashed name of this unit
	int arguments; // how many arguments it expects
	int offsetInBytecode; // where in the bytecode it resides

	WRarray<ConstantValue> constantValues;

	int16_t offsetOfLocalHashMap;
	
	// the code that runs when it loads
	// the locals it has
	WRBytecode bytecode;
	
	WRUnitContext() { reset(); }
	void reset()
	{
		hash = 0;
		arguments = 0;
		offsetInBytecode = 0;
		offsetOfLocalHashMap = 0;
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

	static bool CheckSkipLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool CheckFastLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool IsLiteralLoadOpcode( unsigned char opcode );
	static bool CheckCompareReplace( WROpcode LS, WROpcode GS, WROpcode ILS, WROpcode IGS, WRBytecode& bytecode, unsigned int a, unsigned int o );
	
	unsigned char* pack16( int16_t i, unsigned char* buf )
	{
		*buf = (i>>8) & 0xFF;
		*(buf + 1) = i & 0xFF;
		return buf;
	}
	unsigned char* pack32( int32_t l, unsigned char* buf )
	{
		*buf = (l>>24) & 0xFF;
		*(buf + 1) = (l>>16) & 0xFF;
		*(buf + 2) = (l>>8) & 0xFF;
		*(buf + 3) = l & 0xFF;
		return buf;
	}

	friend class WRExpression;
	static void pushOpcode( WRBytecode& bytecode, WROpcode opcode );
	static void pushData( WRBytecode& bytecode, const unsigned char* data, const int len ) { bytecode.all.append( data, len ); }
	static void pushData( WRBytecode& bytecode, const char* data, const int len ) { bytecode.all.append( (unsigned char*)data, len ); }

	int getBytecodePosition( WRBytecode& bytecode ) { return bytecode.all.size(); }
	
	int addRelativeJumpTarget( WRBytecode& bytecode );
	void setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget );
	void addRelativeJumpSourceEx( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget, const unsigned char* data, const int dataSize );
	void addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget );
	void resolveRelativeJumps( WRBytecode& bytecode );

	void appendBytecode( WRBytecode& bytecode, WRBytecode& addMe );
	
	void pushLiteral( WRBytecode& bytecode, WRExpressionContext& context );
	int addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly =false );
	int addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly =false );
	void addFunctionToHashSpace( WRBytecode& result, WRstr& token );
	void loadExpressionContext( WRExpression& expression, int depth, int operation );
	void resolveExpression( WRExpression& expression );
	unsigned int resolveExpressionEx( WRExpression& expression, int o, int p );

	bool operatorFound( WRstr& token, WRarray<WRExpressionContext>& context, int depth );
	bool parseCallFunction( WRExpression& expression, WRstr functionName, int depth, bool parseArguments );
	bool pushObjectTable( WRExpressionContext& context, WRarray<WRNamespaceLookup>& localSpace, uint32_t hash );
	char parseExpression( WRExpression& expression);
	bool parseUnit( bool isStruct );
	bool parseWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseForLoop( bool& returnCalled, WROpcode opcodeToReturn );
	bool lookupConstantValue( WRstr& prefix, WRValue* value =0 );
	bool parseEnum( int unitIndex );
	uint32_t getSingleValueHash( const char* end );
	bool parseSwitch( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseIf( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn );

	void createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size );
	void link( unsigned char** out, int* outLen );
	
	const char* m_source;
	int m_sourceLen;
	int m_pos;

	WRstr m_loadedToken;
	WRValue m_loadedValue;

	WRError m_err;
	bool m_EOF;
	bool m_LastParsedLabel;
	bool m_parsingFor;

	int m_unitTop;
	WRarray<WRUnitContext> m_units;

	WRarray<int> m_continueTargets;
	WRarray<int> m_breakTargets;

	int m_foreachHash;

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
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

#define WR_COMPILER_LITERAL_STRING 0x10 

//------------------------------------------------------------------------------
const char* c_reserved[] =
{
	"enum",
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
	"switch",
	"true",
	"function",
	"while",
	"new",
	"null",
	"struct",
	"goto",
	"gc_pause",
	""
};

//#define _DUMP
#ifdef _DUMP
#define D_STREAM(a) streamDump(a)
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
void streamDump( WROpcodeStream const& stream )
{
	WRstr str;
	wr_asciiDump( stream, stream.size(), str );
	printf( "%d:\n%s\n", stream.size(), str.c_str() );
}

#else
#define D_STREAM(a)
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
				for( unsigned int p=0; p<i - 1 && p<token.size(); ++p )
				{
					prefix += token[p];
				}

				for( unsigned int t=2; (t+prefix.size())<token.size(); ++t)
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
		value.p2 = INIT_AS_REF;

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

		unsigned int t=0;
		for( ; c_operations[t].token && strncmp( c_operations[t].token, "@macroBegin", 11); ++t );

		int offset = m_pos - 1;

		for( ; c_operations[t].token; ++t )
		{
			int len = (int)strnlen( c_operations[t].token, 20 );
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, c_operations[t].token, len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}
				
				m_pos += len - 1;
				token = c_operations[t].token;
				goto foundMacroToken;
			}
		}

		for( t = 0; t<m_units[0].constantValues.count(); ++t )
		{
			int len = m_units[0].constantValues[t].label.size();
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, m_units[0].constantValues[t].label.c_str(), len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}

				m_pos += len - 1;
				token.clear();
				value = m_units[0].constantValues[t].value;
				goto foundMacroToken;
			}
		}

		for( t = 0; t<m_units[m_unitTop].constantValues.count(); ++t )
		{
			int len = m_units[m_unitTop].constantValues[t].label.size();
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, m_units[m_unitTop].constantValues[t].label.c_str(), len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}

				m_pos += len - 1;
				token.clear();
				value = m_units[m_unitTop].constantValues[t].value;
				goto foundMacroToken;
			}
		}

		if ( token[0] == '-' )
		{
			if ( m_pos < m_sourceLen )
			{
				if ( (isdigit(m_source[m_pos]) && !m_LastParsedLabel) || m_source[m_pos] == '.' )
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
		else
		{
			m_LastParsedLabel = false;
		
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
			else if ( token[0] == '\"' || token[0] == '\'' )
			{
				bool single = token[0] == '\'';
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
					if (c == '\"' && !single ) // terminating character
					{
						if ( single )
						{
							m_err = WR_ERR_bad_expression;
							return false;
						}
						++m_pos;
						break;
					}
					else if ( c == '\'' && single )
					{
						if ( !single || (token.size() > 1) )
						{
							m_err = WR_ERR_bad_expression;
							return false;
						}
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
							--m_pos;
						}
						else if (c == '\"')
						{
							token += '\"';
						}
						else if (c == '\'')
						{
							token += '\'';
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

				value.p2 = INIT_AS_INT;
				if ( single )
				{
					value.ui = token.size() == 1 ? token[0] : 0;
					token.clear();
				}
				else
				{
					value.p2 = INIT_AS_INT;
					value.type = (WRValueType)WR_COMPILER_LITERAL_STRING;
					value.p = &token;
				}
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
			else if ( isdigit(token[0])
					  || (token[0] == '.' && isdigit(m_source[m_pos])) )
			{
				if ( m_pos >= m_sourceLen )
				{
					return false;
				}

parseAsNumber:

				m_LastParsedLabel = true;

				if ( token[0] == '0' && m_source[m_pos] == 'x' ) // interpret as hex
				{
					token.clear();
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

					value.p2 = INIT_AS_INT;
					value.ui = strtoul( token, 0, 16 );
				}
				else if (token[0] == '0' && m_source[m_pos] == 'b' )
				{
					token.clear();
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

					value.p2 = INIT_AS_INT;
					value.i = strtol( token, 0, 2 );
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
							if ( m_source[m_pos] == 'f' || m_source[m_pos] == 'F')
							{
								decimal = true;
								m_pos++;
							}

							break;
						}

						token += m_source[m_pos++];
					}

					if ( decimal )
					{
						value.p2 = INIT_AS_FLOAT;
						value.f = (float)atof( token );
					}
					else
					{
						value.p2 = INIT_AS_INT;
						value.ui = (uint32_t)strtoul( token, 0, 10 );
					}
				}
			}
			else if ( token[0] == ':' && isspace(m_source[m_pos]) )
			{
				
			}
			else if ( isalpha(token[0]) || token[0] == '_' || token[0] == ':' ) // must be a label
			{
				if ( token[0] != ':' || m_source[m_pos] == ':' )
				{
					m_LastParsedLabel = true;
					if ( m_pos < m_sourceLen
						 && token[0] == ':'
						 && m_source[m_pos] == ':' )
					{
						token += ':';
						++m_pos;
					}

					for (; m_pos < m_sourceLen; ++m_pos)
					{
						if ( m_source[m_pos] == ':' && m_source[m_pos + 1] == ':' && token.size() > 0 )
						{
							token += "::";
							m_pos ++;
							continue;
						}
						
						if (!isalnum(m_source[m_pos]) && m_source[m_pos] != '_' )
						{
							break;
						}

						token += m_source[m_pos];
					}

					if (token == "true")
					{
						value.p2 = INIT_AS_INT;
						value.i = 1;
						token = "1";
					}
					else if (token == "false" || token == "null" )
					{
						value.p2 = INIT_AS_INT;
						value.i = 0;
						token = "0";
					}
				}
			}
		}

foundMacroToken:
	
		ex.spaceAfter = (m_pos < m_sourceLen) && isspace(m_source[m_pos]);
	}

	m_loadedToken.clear();
	m_loadedValue.p2 = INIT_AS_REF;

	if ( expect && (token != expect) )
	{
		return false;
	}
	
	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckSkipLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o )
{
	if ( bytecode.opcodes[o] == O_LoadFromLocal
		 && bytecode.opcodes[o-1] == O_LoadFromLocal )
	{
		bytecode.all[a-3] = O_LLValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += O_IndexSkipLoad;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromGlobal
			  && bytecode.opcodes[o-1] == O_LoadFromLocal )
	{
		bytecode.all[a-3] = O_LGValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromLocal
			  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
	{
		bytecode.all[a-3] = O_GLValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromGlobal
			  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
	{
		bytecode.all[a-3] = O_GGValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckFastLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o )
{
	if ( a <= 2 || o == 0 )
	{
		return false;
	}

	if ( (opcode == O_Index && CheckSkipLoad(O_IndexSkipLoad, bytecode, a, o))
		 || (opcode == O_BinaryMod && CheckSkipLoad(O_BinaryModSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryRightShift && CheckSkipLoad(O_BinaryRightShiftSkipLoad, bytecode, a, o))
		 || (opcode == O_BinaryLeftShift && CheckSkipLoad(O_BinaryLeftShiftSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryAnd && CheckSkipLoad(O_BinaryAndSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryOr && CheckSkipLoad(O_BinaryOrSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryXOR && CheckSkipLoad(O_BinaryXORSkipLoad, bytecode, a, o)) )
	{
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::IsLiteralLoadOpcode( unsigned char opcode )
{
	return opcode == O_LiteralInt32
			|| opcode == O_LiteralZero
			|| opcode == O_LiteralFloat
			|| opcode == O_LiteralInt8
			|| opcode == O_LiteralInt16;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckCompareReplace( WROpcode LS, WROpcode GS, WROpcode ILS, WROpcode IGS, WRBytecode& bytecode, unsigned int a, unsigned int o )
{
	if ( IsLiteralLoadOpcode(bytecode.opcodes[o-1]) )
	{
		if ( bytecode.opcodes[o] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 1 ] = GS;
			bytecode.opcodes[o] = GS;
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LoadFromLocal )
		{
			bytecode.all[ a - 1 ] = LS;
			bytecode.opcodes[o] = LS;
			return true;
		}
	}
	else if ( IsLiteralLoadOpcode(bytecode.opcodes[o]) )
	{
		if ( (bytecode.opcodes[o] == O_LiteralInt32 || bytecode.opcodes[o] == O_LiteralFloat)
			 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 6 ] = bytecode.all[ a ]; // store i3
			bytecode.all[ a ] = bytecode.all[ a - 5 ]; // move index

			bytecode.all[ a - 5 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 4 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a - 6 ];
			bytecode.all[ a - 6 ] = bytecode.opcodes[o];
			bytecode.all[ a - 1 ] = IGS;

			bytecode.opcodes[o] = IGS; // reverse the logic

			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt8
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a ];
			bytecode.all[ a ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.all[ a - 3 ] = O_LiteralInt8;

			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt16
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = bytecode.all[ a ];

			bytecode.all[ a - 4 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a ] = bytecode.all[ a - 4 ];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.all[ a - 4 ] = O_LiteralInt16;

			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		if ( (bytecode.opcodes[o] == O_LiteralInt32 || bytecode.opcodes[o] == O_LiteralFloat)
			 && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 6 ] = bytecode.all[ a ]; // store i3
			bytecode.all[ a ] = bytecode.all[ a - 5 ]; // move index

			bytecode.all[ a - 5 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 4 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a - 6 ];
			bytecode.all[ a - 6 ] = bytecode.opcodes[o];
			bytecode.all[ a - 1 ] = ILS;

			bytecode.opcodes[o] = ILS; // reverse the logic

			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt8
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a ];
			bytecode.all[ a ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.all[ a - 3 ] = O_LiteralInt8;

			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt16
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = bytecode.all[ a ];

			bytecode.all[ a - 4 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a ] = bytecode.all[ a - 4 ];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.all[ a - 4 ] = O_LiteralInt16;

			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushOpcode( WRBytecode& bytecode, WROpcode opcode )
{
	unsigned int o = bytecode.opcodes.size();
	if ( o )
	{
		// keyhole optimizations

		--o;
		unsigned int a = bytecode.all.size() - 1;

		if ( opcode == O_Return
			 && bytecode.opcodes[o] == O_LiteralZero )
		{
			bytecode.all[a] = O_ReturnZero;
			bytecode.opcodes[o] = O_ReturnZero;
			return;
		}
		else if ( opcode == O_CompareEQ && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareEQ;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_LLCompareLE;
			return;
		}
		else if ( opcode == O_CompareEQ && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareEQ;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.shave( 2 );
			bytecode.opcodes += O_GGCompareLE;
			return;
		}
		else if ( (opcode == O_CompareEQ && (o>0))
				  && CheckCompareReplace(O_LSCompareEQ, O_GSCompareEQ, O_LSCompareEQ, O_GSCompareEQ, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareNE && (o>0))
				  && CheckCompareReplace(O_LSCompareNE, O_GSCompareNE, O_LSCompareNE, O_GSCompareNE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareGE && (o>0))
			 && CheckCompareReplace(O_LSCompareGE, O_GSCompareGE, O_LSCompareLE, O_GSCompareLE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareLE && (o>0))
				  && CheckCompareReplace(O_LSCompareLE, O_GSCompareLE, O_LSCompareGE, O_GSCompareGE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareGT && (o>0))
				  && CheckCompareReplace(O_LSCompareGT, O_GSCompareGT, O_LSCompareLT, O_GSCompareLT, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareLT && (o>0))
				  && CheckCompareReplace(O_LSCompareLT, O_GSCompareLT, O_LSCompareGT, O_GSCompareGT, bytecode, a, o) )
		{
			return;
		}
		else if ( CheckFastLoad(opcode, bytecode, a, o) )
		{
			return;
		}
		else if ( opcode == O_BinaryMultiplication && (a>2) )
		{
			
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				// LoadFromGlobal   a - 3
				// [index]          a - 2
				// LoadFromGlobal   a - 1
				// [index]          a

				bytecode.all[ a - 3 ] = O_GGBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
				bytecode.all[ a - 2 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}
			
			return;
		}
		else if ( opcode == O_BinaryAddition && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
				bytecode.all[ a - 2 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_BinarySubtraction && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_BinaryDivision && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.shave(2);
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_Index )
		{
			if ( bytecode.opcodes[o] == O_LiteralInt16 )
			{
				bytecode.all[a-2] = O_IndexLiteral16;
				bytecode.opcodes[o] = O_IndexLiteral16;
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
			if ( bytecode.opcodes[o] == O_LLCompareLT )
			{
				bytecode.opcodes[o] = O_LLCompareLTBZ;
				bytecode.all[ a - 2 ] = O_LLCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareGT )
			{
				bytecode.opcodes[o] = O_LLCompareGTBZ;
				bytecode.all[ a - 2 ] = O_LLCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareGE )
			{
				bytecode.opcodes[o] = O_LLCompareGEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareLE )
			{
				bytecode.opcodes[o] = O_LLCompareLEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareEQ )
			{
				bytecode.opcodes[o] = O_LLCompareEQBZ;
				bytecode.all[ a - 2 ] = O_LLCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareNE )
			{
				bytecode.opcodes[o] = O_LLCompareNEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareLT )
			{
				bytecode.opcodes[o] = O_GGCompareLTBZ;
				bytecode.all[ a - 2 ] = O_GGCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareGT )
			{
				bytecode.opcodes[o] = O_GGCompareGTBZ;
				bytecode.all[ a - 2 ] = O_GGCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareLE )
			{
				bytecode.opcodes[o] = O_GGCompareLEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareGE )
			{
				bytecode.opcodes[o] = O_GGCompareGEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareEQ )
			{
				bytecode.opcodes[o] = O_GGCompareEQBZ;
				bytecode.all[ a - 2 ] = O_GGCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareNE )
			{
				bytecode.opcodes[o] = O_GGCompareNEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareEQ )
			{
				bytecode.opcodes[o] = O_GSCompareEQBZ;
				bytecode.all[ a - 1 ] = O_GSCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareEQ )
			{
				bytecode.opcodes[o] = O_LSCompareEQBZ;
				bytecode.all[ a - 1 ] = O_LSCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareNE )
			{
				bytecode.opcodes[o] = O_GSCompareNEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareNE )
			{
				bytecode.opcodes[o] = O_LSCompareNEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareGE )
			{
				bytecode.opcodes[o] = O_GSCompareGEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareGE )
			{
				bytecode.opcodes[o] = O_LSCompareGEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareLE )
			{
				bytecode.opcodes[o] = O_GSCompareLEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareLE )
			{
				bytecode.opcodes[o] = O_LSCompareLEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareGT )
			{
				bytecode.opcodes[o] = O_GSCompareGTBZ;
				bytecode.all[ a - 1 ] = O_GSCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareGT )
			{
				bytecode.opcodes[o] = O_LSCompareGTBZ;
				bytecode.all[ a - 1 ] = O_LSCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareLT )
			{
				bytecode.opcodes[o] = O_GSCompareLTBZ;
				bytecode.all[ a - 1 ] = O_GSCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareLT )
			{
				bytecode.opcodes[o] = O_LSCompareLTBZ;
				bytecode.all[ a - 1 ] = O_LSCompareLTBZ;
				return;
			}			
			else if ( bytecode.opcodes[o] == O_CompareEQ ) // assign+pop is very common
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareEQBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareEQBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareEQBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareEQBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBEQ;
					bytecode.opcodes[o] = O_CompareBEQ;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLT )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareLTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareLTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBLT;
					bytecode.opcodes[o] = O_CompareBLT;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGT )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareGTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareGTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBGT;
					bytecode.opcodes[o] = O_CompareBGT;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGE )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareGEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareLTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBGE;
					bytecode.opcodes[o] = O_CompareBGE;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLE )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareLEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareGTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBLE;
					bytecode.opcodes[o] = O_CompareBLE;
				}

				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareNE ) // assign+pop is very common
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareNEBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_LLCompareNEBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareNEBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.shave(2);
					bytecode.opcodes[o-2] = O_GGCompareNEBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBNE;
					bytecode.opcodes[o] = O_CompareBNE;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_LogicalOr ) // assign+pop is very common
			{
				bytecode.all[a] = O_BLO;
				bytecode.opcodes[o] = O_BLO;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LogicalAnd ) // assign+pop is very common
			{
				bytecode.all[a] = O_BLA;
				bytecode.opcodes[o] = O_BLA;
				return;
			}
		}
		else if ( opcode == O_PopOne )
		{
			if ( bytecode.opcodes[o] == O_LoadFromLocal || bytecode.opcodes[o] == O_LoadFromGlobal ) 
			{
				bytecode.all.shave(2);
				bytecode.opcodes.shave(1);
				return;
			}
			else if ( bytecode.opcodes[o] == O_CallLibFunction && (a > 4) )
			{
				bytecode.all[a-5] = O_CallLibFunctionAndPop;
				bytecode.opcodes[o] = O_CallLibFunctionAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_FUNCTION_CALL_PLACEHOLDER )
			{
				bytecode.all[ a ] = O_PopOne;
				return;
			}
			else if ( bytecode.opcodes[o] == O_PreIncrement || bytecode.opcodes[o] == O_PostIncrement )
			{	
				if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromGlobal )
				{
					bytecode.all[ a - 2 ] = O_IncGlobal;
										
					bytecode.all.shave(1);
					bytecode.opcodes.shave(2);
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_IncLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.shave(2);
				}
				else
				{
					bytecode.all[a] = O_PreIncrementAndPop;
					bytecode.opcodes[o] = O_PreIncrementAndPop;
				}

				return;
			}
			else if ( bytecode.opcodes[o] == O_PreDecrement || bytecode.opcodes[o] == O_PostDecrement )
			{	
				if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromGlobal )
				{
					bytecode.all[ a - 2 ] = O_DecGlobal;
					bytecode.all.shave(1);
					bytecode.opcodes.shave(2);
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_DecLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.shave(2);
				}
				else
				{
					bytecode.all[a] = O_PreDecrementAndPop;
					bytecode.opcodes[o] = O_PreDecrementAndPop;
				}
				
				return;
			}
			else if ( bytecode.opcodes[o] == O_Assign ) // assign+pop is very common
			{
				if ( o > 0 )
				{
					if ( bytecode.opcodes[o-1] == O_LoadFromGlobal )
					{
						if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt8 )
						{
							// a - 4: O_literalInt8
							// a - 3: val
							// a - 2: load from global
							// a - 1: index
							// a -- assign

							bytecode.all[ a - 4 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							bytecode.all[ a - 7 ] = O_LiteralInt32ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							bytecode.all[ a - 7 ] = O_LiteralFloatToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[ a - 3 ] = O_BinaryDivisionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[ a - 3 ] = O_BinaryAdditionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[ a - 3 ] = O_BinaryMultiplicationAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[ a - 3 ] = O_BinarySubtractionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_FUNCTION_CALL_PLACEHOLDER )
						{
							//bytecode.all[a] = bytecode.all[a-1];
							bytecode.all[a-2] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes[o] = O_AssignToGlobalAndPop;
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
							bytecode.all[ a - 4 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							bytecode.all[ a - 7 ] = O_LiteralInt32ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							bytecode.all[ a - 7 ] = O_LiteralFloatToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[a - 3] = O_BinaryDivisionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[a - 3] = O_BinaryAdditionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[a - 3] = O_BinaryMultiplicationAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[a - 3] = O_BinarySubtractionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.shave(3);
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
void WRCompilationContext::addRelativeJumpSourceEx( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget, const unsigned char* data, const int dataSize )
{
	pushOpcode( bytecode, opcode );

	int offset = bytecode.all.size();
	
	if ( dataSize ) // additional data
	{
		pushData( bytecode, data, dataSize );
	}

	bytecode.jumpOffsetTargets[relativeJumpTarget].references.append() = offset;

	pushData( bytecode, "\t\t", 2 ); // 16-bit relative vector
}


//------------------------------------------------------------------------------
// add a jump FROM with whatever opcode is supposed to do it
void WRCompilationContext::addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget )
{
	pushOpcode( bytecode, opcode );

	int offset = bytecode.all.size();
	switch( bytecode.opcodes[bytecode.opcodes.size() - 1] )
	{
		case O_GSCompareEQBZ:
		case O_LSCompareEQBZ:
		case O_GSCompareNEBZ:
		case O_LSCompareNEBZ:
		case O_GSCompareGEBZ:
		case O_LSCompareGEBZ:
		case O_GSCompareLEBZ:
		case O_LSCompareLEBZ:
		case O_GSCompareGTBZ:
		case O_LSCompareGTBZ:
		case O_GSCompareLTBZ:
		case O_LSCompareLTBZ:
		{
			--offset;
			break;
		}

		case O_LLCompareLTBZ:
		case O_LLCompareGTBZ:
		case O_LLCompareLEBZ:
		case O_LLCompareGEBZ:
		case O_LLCompareEQBZ:
		case O_LLCompareNEBZ:
		case O_GGCompareLTBZ:
		case O_GGCompareGTBZ:
		case O_GGCompareLEBZ:
		case O_GGCompareGEBZ:
		case O_GGCompareEQBZ:
		case O_GGCompareNEBZ:
		{
			offset -= 2;
			break;
		}

		default: break;
	}
	
	bytecode.jumpOffsetTargets[relativeJumpTarget].references.append() = offset;
	pushData( bytecode, "\t\t", 2 );
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
			WROpcode o = (WROpcode)bytecode.all[offset - 1];
			bool no8version = false;

			switch( o )
			{
				case O_GSCompareEQBZ8:
				case O_LSCompareEQBZ8:
				case O_GSCompareNEBZ8:
				case O_LSCompareNEBZ8:
				case O_GSCompareGEBZ8:
				case O_LSCompareGEBZ8:
				case O_GSCompareLEBZ8:
				case O_LSCompareLEBZ8:
				case O_GSCompareGTBZ8:
				case O_LSCompareGTBZ8:
				case O_GSCompareLTBZ8:
				case O_LSCompareLTBZ8:
				case O_GSCompareEQBZ:
				case O_LSCompareEQBZ:
				case O_GSCompareNEBZ:
				case O_LSCompareNEBZ:
				case O_GSCompareGEBZ:
				case O_LSCompareGEBZ:
				case O_GSCompareLEBZ:
				case O_LSCompareLEBZ:
				case O_GSCompareGTBZ:
				case O_LSCompareGTBZ:
				case O_GSCompareLTBZ:
				case O_LSCompareLTBZ:
				{
					--diff; // these instructions are offset
					break;
				}

				case O_LLCompareLTBZ8:
				case O_LLCompareGTBZ8:
				case O_LLCompareLEBZ8:
				case O_LLCompareGEBZ8:
				case O_LLCompareEQBZ8:
				case O_LLCompareNEBZ8:
				case O_GGCompareLTBZ8:
				case O_GGCompareGTBZ8:
				case O_GGCompareLEBZ8:
				case O_GGCompareGEBZ8:
				case O_GGCompareEQBZ8:
				case O_GGCompareNEBZ8:
				case O_LLCompareLTBZ:
				case O_LLCompareGTBZ:
				case O_LLCompareLEBZ:
				case O_LLCompareGEBZ:
				case O_LLCompareEQBZ:
				case O_LLCompareNEBZ:
				case O_GGCompareLTBZ:
				case O_GGCompareGTBZ:
				case O_GGCompareLEBZ:
				case O_GGCompareGEBZ:
				case O_GGCompareEQBZ:
				case O_GGCompareNEBZ:
				{
					diff -= 2;
					break;
				}

				case O_GGNextKeyValueOrJump:
				case O_GLNextKeyValueOrJump:
				case O_LGNextKeyValueOrJump:
				case O_LLNextKeyValueOrJump:
				{
					no8version = true;
					diff -= 3;
					break;
				}

				case O_GNextValueOrJump:
				case O_LNextValueOrJump:
				{
					no8version = true;
					diff -= 2;
					break;
				}

				default:
					break;
			}

			
			if ( (diff < 128) && (diff > -129) && !no8version )
			{
				switch( o )
				{
					case O_RelativeJump: *bytecode.all.p_str(offset - 1) = O_RelativeJump8; break;
					case O_BZ: *bytecode.all.p_str(offset - 1) = O_BZ8; break;

					case O_CompareBEQ: *bytecode.all.p_str(offset - 1) = O_CompareBEQ8; break;
					case O_CompareBNE: *bytecode.all.p_str(offset - 1) = O_CompareBNE8; break;
					case O_CompareBGE: *bytecode.all.p_str(offset - 1) = O_CompareBGE8; break;
					case O_CompareBLE: *bytecode.all.p_str(offset - 1) = O_CompareBLE8; break;
					case O_CompareBGT: *bytecode.all.p_str(offset - 1) = O_CompareBGT8; break;
					case O_CompareBLT: *bytecode.all.p_str(offset - 1) = O_CompareBLT8; break;

					case O_GSCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareEQBZ8; ++offset; break;
					case O_LSCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareEQBZ8; ++offset; break;
					case O_GSCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareNEBZ8; ++offset; break;
					case O_LSCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareNEBZ8; ++offset; break;
					case O_GSCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareGEBZ8; ++offset; break;
					case O_LSCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareGEBZ8; ++offset; break;
					case O_GSCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareLEBZ8; ++offset; break;
					case O_LSCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareLEBZ8; ++offset; break;
					case O_GSCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareGTBZ8; ++offset; break;
					case O_LSCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareGTBZ8; ++offset; break;
					case O_GSCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareLTBZ8; ++offset; break;
					case O_LSCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareLTBZ8; ++offset; break;
										  
					case O_LLCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareLTBZ8; offset += 2; break;
					case O_LLCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareGTBZ8; offset += 2; break;
					case O_LLCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareLEBZ8; offset += 2; break;
					case O_LLCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareGEBZ8; offset += 2; break;
					case O_LLCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareEQBZ8; offset += 2; break;
					case O_LLCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareNEBZ8; offset += 2; break;
					case O_GGCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareLTBZ8; offset += 2; break;
					case O_GGCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareGTBZ8; offset += 2; break;
					case O_GGCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareLEBZ8; offset += 2; break;
					case O_GGCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareGEBZ8; offset += 2; break;
					case O_GGCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareEQBZ8; offset += 2; break;
					case O_GGCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareNEBZ8; offset += 2; break;

					case O_BLA: *bytecode.all.p_str(offset - 1) = O_BLA8; break;
					case O_BLO: *bytecode.all.p_str(offset - 1) = O_BLO8; break;

					// no work to be done
					case O_RelativeJump8:
					case O_BZ8:
					case O_BLA8:
					case O_BLO8:
					case O_CompareBLE8:
					case O_CompareBGE8:
					case O_CompareBGT8:
					case O_CompareBLT8:
					case O_CompareBEQ8:
					case O_CompareBNE8:
						break;
						
					case O_GSCompareEQBZ8:
					case O_LSCompareEQBZ8:
					case O_GSCompareNEBZ8:
					case O_LSCompareNEBZ8:
					case O_GSCompareGEBZ8:
					case O_LSCompareGEBZ8:
					case O_GSCompareLEBZ8:
					case O_LSCompareLEBZ8:
					case O_GSCompareGTBZ8:
					case O_LSCompareGTBZ8:
					case O_GSCompareLTBZ8:
					case O_LSCompareLTBZ8:
						++offset;
						break;

					case O_LLCompareLTBZ8:
					case O_LLCompareGTBZ8:
					case O_LLCompareLEBZ8:
					case O_LLCompareGEBZ8:
					case O_LLCompareEQBZ8:
					case O_LLCompareNEBZ8:
					case O_GGCompareLTBZ8:
					case O_GGCompareGTBZ8:
					case O_GGCompareLEBZ8:
					case O_GGCompareGEBZ8:
					case O_GGCompareEQBZ8:
					case O_GGCompareNEBZ8:
					{
						offset += 2;
						break;
					}

					default:
						m_err = WR_ERR_compiler_panic;
						return;
				}

				*bytecode.all.p_str(offset) = (int8_t)diff;
			}
			else
			{
				switch( o )
				{
					// check to see if any were pushed into 16-bit land
					// that were previously optimized
					case O_RelativeJump8: *bytecode.all.p_str(offset - 1) = O_RelativeJump; break;
					case O_BZ8: *bytecode.all.p_str(offset - 1) = O_BZ; break;
					case O_CompareBEQ8: *bytecode.all.p_str(offset - 1) = O_CompareBEQ; break;
					case O_CompareBNE8: *bytecode.all.p_str(offset - 1) = O_CompareBNE; break;
					case O_CompareBGE8: *bytecode.all.p_str(offset - 1) = O_CompareBGE; break;
					case O_CompareBLE8: *bytecode.all.p_str(offset - 1) = O_CompareBLE; break;
					case O_CompareBGT8: *bytecode.all.p_str(offset - 1) = O_CompareBGT; break;
					case O_CompareBLT8: *bytecode.all.p_str(offset - 1) = O_CompareBLT; break;
					case O_GSCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareEQBZ; ++offset; break;
					case O_LSCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareEQBZ; ++offset; break;
					case O_GSCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareNEBZ; ++offset; break;
					case O_LSCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareNEBZ; ++offset; break;
					case O_GSCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareGEBZ; ++offset; break;
					case O_LSCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareGEBZ; ++offset; break;
					case O_GSCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareLEBZ; ++offset; break;
					case O_LSCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareLEBZ; ++offset; break;
					case O_GSCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareGTBZ; ++offset; break;
					case O_LSCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareGTBZ; ++offset; break;
					case O_GSCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareLTBZ; ++offset; break;
					case O_LSCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareLTBZ; ++offset; break;

					case O_LLCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareLTBZ; offset += 2; break;
					case O_LLCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareGTBZ; offset += 2; break;
					case O_LLCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareLEBZ; offset += 2; break;
					case O_LLCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareGEBZ; offset += 2; break;
					case O_LLCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareEQBZ; offset += 2; break;
					case O_LLCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareNEBZ; offset += 2; break;
					case O_GGCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareLTBZ; offset += 2; break;
					case O_GGCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareGTBZ; offset += 2; break;
					case O_GGCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareLEBZ; offset += 2; break;
					case O_GGCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareGEBZ; offset += 2; break;
					case O_GGCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareEQBZ; offset += 2; break;
					case O_GGCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareNEBZ; offset += 2; break;

					case O_GGNextKeyValueOrJump:
					case O_GLNextKeyValueOrJump:
					case O_LGNextKeyValueOrJump:
					case O_LLNextKeyValueOrJump:
					{
						offset += 3;
						break;
					}

					case O_GNextValueOrJump:
					case O_LNextValueOrJump:
					{
						offset += 2;
						break;
					}

					case O_BLA8: *bytecode.all.p_str(offset - 1) = O_BLA; break;
					case O_BLO8: *bytecode.all.p_str(offset - 1) = O_BLO; break;

					 // no work to be done, already visited
					case O_RelativeJump:
					case O_BZ:
					case O_BLA:
					case O_BLO:
					case O_CompareBLE:
					case O_CompareBGE:
					case O_CompareBGT:
					case O_CompareBLT:
					case O_CompareBEQ:
					case O_CompareBNE:
						break;

					case O_GSCompareEQBZ:
					case O_LSCompareEQBZ:
					case O_GSCompareNEBZ:
					case O_LSCompareNEBZ:
					case O_GSCompareGEBZ:
					case O_LSCompareGEBZ:
					case O_GSCompareLEBZ:
					case O_LSCompareLEBZ:
					case O_GSCompareGTBZ:
					case O_LSCompareGTBZ:
					case O_GSCompareLTBZ:
					case O_LSCompareLTBZ:
						++offset;
						break;
					
					case O_LLCompareLTBZ:
					case O_LLCompareGTBZ:
					case O_LLCompareLEBZ:
					case O_LLCompareGEBZ:
					case O_LLCompareEQBZ:
					case O_LLCompareNEBZ:
					case O_GGCompareLTBZ:
					case O_GGCompareGTBZ:
					case O_GGCompareLEBZ:
					case O_GGCompareGEBZ:
					case O_GGCompareEQBZ:
					case O_GGCompareNEBZ:
					{
						offset += 2;
						break;
					}
						
					default:
					{
						m_err = WR_ERR_compiler_panic;
						return;
					}
				}
				
				pack16( diff, bytecode.all.p_str(offset) );
			}
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::appendBytecode( WRBytecode& bytecode, WRBytecode& addMe )
{
	for (unsigned int n = 0; n < addMe.jumpOffsetTargets.count(); ++n)
	{
		if (addMe.jumpOffsetTargets[n].gotoHash)
		{
			int index = addRelativeJumpTarget(bytecode);
			bytecode.jumpOffsetTargets[index].gotoHash = addMe.jumpOffsetTargets[n].gotoHash;
			bytecode.jumpOffsetTargets[index].offset = addMe.jumpOffsetTargets[n].offset + bytecode.all.size();
		}
	}

	// add the namespace, making sure to offset it into the new block properly
	for (unsigned int n = 0; n < addMe.localSpace.count(); ++n)
	{
		unsigned int m = 0;
		for (m = 0; m < bytecode.localSpace.count(); ++m)
		{
			if (bytecode.localSpace[m].hash == addMe.localSpace[n].hash)
			{
				for (unsigned int s = 0; s < addMe.localSpace[n].references.count(); ++s)
				{
					bytecode.localSpace[m].references.append() = addMe.localSpace[n].references[s] + bytecode.all.size();
				}

				break;
			}
		}

		if (m >= bytecode.localSpace.count())
		{
			WRNamespaceLookup* space = &bytecode.localSpace.append();
			*space = addMe.localSpace[n];
			for (unsigned int s = 0; s < space->references.count(); ++s)
			{
				space->references[s] += bytecode.all.size();
			}
		}
	}

	// add the function space, making sure to offset it into the new block properly
	for (unsigned int n = 0; n < addMe.functionSpace.count(); ++n)
	{
		WRNamespaceLookup* space = &bytecode.functionSpace.append();
		*space = addMe.functionSpace[n];
		for (unsigned int s = 0; s < space->references.count(); ++s)
		{
			space->references[s] += bytecode.all.size();
		}
	}

	// add the function space, making sure to offset it into the new block properly
	for (unsigned int u = 0; u < addMe.unitObjectSpace.count(); ++u)
	{
		WRNamespaceLookup* space = &bytecode.unitObjectSpace.append();
		*space = addMe.unitObjectSpace[u];
		for (unsigned int s = 0; s < space->references.count(); ++s)
		{
			space->references[s] += bytecode.all.size();
		}
	}

	// add the goto targets, making sure to offset it into the new block properly
	for (unsigned int u = 0; u < addMe.gotoSource.count(); ++u)
	{
		GotoSource* G = &bytecode.gotoSource.append();
		G->hash = addMe.gotoSource[u].hash;
		G->offset = addMe.gotoSource[u].offset + bytecode.all.size();
	}

	if ( bytecode.all.size() > 1
		 && bytecode.opcodes.size() > 0
		 && addMe.opcodes.size() == 1
		 && addMe.all.size() > 2
		 && addMe.gotoSource.count() == 0
		 && addMe.opcodes[0] == O_IndexLiteral16
		 && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexLocalLiteral16;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.shave(1);
		bytecode.opcodes += O_IndexLocalLiteral16;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 2
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral16
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexGlobalLiteral16;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.shave(1);
		bytecode.opcodes += O_IndexGlobalLiteral16;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 1
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral8
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexLocalLiteral8;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.shave(1);
		bytecode.opcodes += O_IndexLocalLiteral8;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 1
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral8
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexGlobalLiteral8;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.shave(1);
		bytecode.opcodes += O_IndexGlobalLiteral8;
		return;
	}
	else if ( bytecode.all.size() > 0
			  && addMe.opcodes.size() == 2
			  && addMe.gotoSource.count() == 0
			  && addMe.all.size() == 3
			  && addMe.opcodes[1] == O_Index )
	{
		if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal
			 && addMe.opcodes[0] == O_LoadFromLocal )
		{
			int a = bytecode.all.size() - 1;
			
			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_LLValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;
			
			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal
			 && addMe.opcodes[0] == O_LoadFromGlobal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_GGValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal
				  && addMe.opcodes[0] == O_LoadFromGlobal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_GLValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal
				  && addMe.opcodes[0] == O_LoadFromLocal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_LGValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
	}
	else if ( addMe.all.size() == 1 )
	{
		if ( addMe.opcodes[0] == O_HASH_PLACEHOLDER )
		{
			if ( bytecode.opcodes.size() > 1
				 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
				 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromLocal )
			{
				int o = bytecode.all.size() - 7;

				bytecode.all[o] = O_LocalIndexHash;
				bytecode.all[o + 5] = bytecode.all[o + 4];
				bytecode.all[o + 4] = bytecode.all[o + 3];
				bytecode.all[o + 3] = bytecode.all[o + 2];
				bytecode.all[o + 2] = bytecode.all[o + 1];
				bytecode.all[o + 1] = bytecode.all[o + 6];

				bytecode.opcodes.shave(2);
				bytecode.all.shave(1);
			}
			else if (bytecode.opcodes.size() > 1
					 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
					 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromGlobal )
			{
				int o = bytecode.all.size() - 7;

				bytecode.all[o] = O_GlobalIndexHash;
				bytecode.all[o + 5] = bytecode.all[o + 4];
				bytecode.all[o + 4] = bytecode.all[o + 3];
				bytecode.all[o + 3] = bytecode.all[o + 2];
				bytecode.all[o + 2] = bytecode.all[o + 1];
				bytecode.all[o + 1] = bytecode.all[o + 6];

				bytecode.opcodes.shave(2);
				bytecode.all.shave(1);
			}
			else if (bytecode.opcodes.size() > 1
					 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
					 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_StackSwap )
			{
				int a = bytecode.all.size() - 7;

				bytecode.all[a] = O_StackIndexHash;

				bytecode.opcodes.shave(2);
				bytecode.all.shave(2);

			}
			else
			{
				m_err = WR_ERR_compiler_panic;
			}

			return;
		}

		pushOpcode( bytecode, (WROpcode)addMe.opcodes[0] );
		return;
	}

	resolveRelativeJumps( addMe );

	bytecode.all += addMe.all;
	bytecode.opcodes += addMe.opcodes;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLiteral( WRBytecode& bytecode, WRExpressionContext& context )
{
	WRValue& value = context.value;
	if ( value.type == WR_INT && value.i == 0 )
	{
		pushOpcode( bytecode, O_LiteralZero );
	}
	else if ( value.type == WR_INT )
	{
		if ( (value.i <= 127) && (value.i >= -128) )
		{
			pushOpcode( bytecode, O_LiteralInt8 );
			unsigned char be = (char)value.i;
			pushData( bytecode, &be, 1 );
		}
		else if ( (value.i <= 32767) && (value.i >= -32768) )
		{
			pushOpcode( bytecode, O_LiteralInt16 );
			unsigned char be = (char)(value.i>>8);
			pushData( bytecode, &be, 1 );
			be = (char)value.i;
			pushData( bytecode, &be, 1 );
		}
		else
		{
			pushOpcode( bytecode, O_LiteralInt32 );
			unsigned char data[4];
			pushData( bytecode, pack32(value.i, data), 4 );
		}
	}
	else if ( value.type == WR_COMPILER_LITERAL_STRING )
	{
		pushOpcode( bytecode, O_LiteralString );
		unsigned char data[2];
		int16_t be = context.literalString.size();
		pushData( bytecode, pack16(be, data), 2 );
		for( unsigned int i=0; i<context.literalString.size(); ++i )
		{
			pushData( bytecode, context.literalString.c_str(i), 1 );
		}
	}
	else
	{
		pushOpcode( bytecode, O_LiteralFloat );
		unsigned char data[4];
		int32_t be = value.i;
		pushData( bytecode, pack32(be, data), 4 );
	}
}

//------------------------------------------------------------------------------
int WRCompilationContext::addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly )
{
	if ( m_unitTop == 0 )
	{
		return addGlobalSpaceLoad( bytecode, token, addOnly );
	}

	uint32_t hash = wr_hashStr(token);
	
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
		WRstr t2;
		if (token[0] == ':' && token[1] == ':')
		{
			t2 = token;
		}
		else
		{
			t2.format("::%s", token.c_str());
		}

		uint32_t ghash = wr_hashStr(t2);

		// was NOT found locally which is possible for a "global" if
		// the argument list names it, now check global with the global
		// hash
		if ( !bytecode.isStructSpace ) // structs are immune to this
		{
			for (unsigned int j = 0; j < m_units[0].bytecode.localSpace.count(); ++j)
			{
				if (m_units[0].bytecode.localSpace[j].hash == ghash)
				{
					pushOpcode(bytecode, O_LoadFromGlobal);
					unsigned char c = j;
					pushData(bytecode, &c, 1);
					return j;
				}
			}
		}
	}

	bytecode.localSpace[i].hash = hash;

	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromLocal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}
	
	return i;
}

//------------------------------------------------------------------------------
int WRCompilationContext::addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly )
{
	uint32_t hash;
	WRstr t2;
	if ( token[0] == ':' && token[1] == ':' )
	{
		t2 = token;
	}
	else
	{
		t2.format( "::%s", token.c_str() );
	}
	hash = wr_hash( t2, t2.size() );

	unsigned int i=0;

	for( ; i<m_units[0].bytecode.localSpace.count(); ++i )
	{
		if ( m_units[0].bytecode.localSpace[i].hash == hash )
		{
			break;
		}
	}
	
	m_units[0].bytecode.localSpace[i].hash = hash;

	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromGlobal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}

	return i;
}

//------------------------------------------------------------------------------
void WRCompilationContext::loadExpressionContext( WRExpression& expression, int depth, int operation )
{
	if ( operation > 0
		 && expression.context[operation].operation
		 && expression.context[operation].operation->opcode == O_HASH_PLACEHOLDER
		 && depth > operation )
	{
		unsigned char buf[4];
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
				pushLiteral( expression.bytecode, expression.context[depth] );
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
void WRExpression::swapWithTop( int stackPosition, bool addOpcodes )
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

	if ( addOpcodes )
	{
		unsigned char pos = stackPosition + 1;
		WRCompilationContext::pushOpcode( bytecode, O_StackSwap );
		WRCompilationContext::pushData( bytecode, &pos, 1 );
	}
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

	unsigned int resolves = 1;
	unsigned int startedWith = expression.context.count();

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

			resolves += resolveExpressionEx( expression, o, p );
			if (m_err)
			{
				return;
			}
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

			resolves += resolveExpressionEx( expression, o, p );
			if (m_err)
			{
				return;
			}
			o = expression.context.count();
		}
	}

	if ( startedWith && (resolves != startedWith) )
	{
		m_err = WR_ERR_bad_expression;
		return;
	}
}

//------------------------------------------------------------------------------
unsigned int WRCompilationContext::resolveExpressionEx( WRExpression& expression, int o, int p )
{
	unsigned int ret = 0;
	switch( expression.context[o].operation->type )
	{
		case WR_OPER_PRE:
		{
			ret = 1;

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

			ret = 2;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
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
				// is in the correct position

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = expression.context[second].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( expression.context[second].stackPosition, false );
				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}

		case WR_OPER_BINARY:
		{
			ret = 2;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			int second = o + 1; // push first
			int first = o - 1;  // push second
			bool useAlt = false;
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

					if ( expression.context[o].operation->alt == O_LAST )
					{
						expression.swapWithTop( 1 );
					}
					else // otherwise top shuffle is NOT required because we have an equal-but-opposite operation
					{
						useAlt = true;
					}
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

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = 2;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}
			else
			{
				// first and second are both loaded but neither is in the correct position

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = expression.context[second].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( expression.context[second].stackPosition, false );
				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}

			if ( useAlt )
			{
				pushOpcode( expression.bytecode, expression.context[o].operation->alt );
			}
			else
			{
				appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator
			}

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}

		case WR_OPER_POST:
		{
			ret = 1;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( expression.context[o - 1].stackPosition == -1 )
			{
				loadExpressionContext( expression, o - 1, o ); // load argument
			}
			else if ( expression.context[o - 1].stackPosition != 0 )
			{
				expression.swapWithTop( expression.context[o - 1].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode );
			expression.context.remove( o, 1 ); // knock off operator
			expression.pushToStack( o - 1 );

			break;
		}
	}
	
	return ret;
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
bool WRCompilationContext::parseCallFunction( WRExpression& expression, WRstr functionName, int depth, bool parseArguments )
{
	WRstr prefix = expression.context[depth].prefix;

	expression.context[depth].type = EXTYPE_BYTECODE_RESULT;

	unsigned char argsPushed = 0;

	if ( parseArguments )
	{
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

			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
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
	}

	if ( prefix.size() )
	{
		prefix += "::";
		prefix += functionName;

		unsigned char buf[4];
		pack32( wr_hashStr(prefix), buf );

		pushOpcode( expression.context[depth].bytecode, O_CallLibFunction );
		pushData( expression.context[depth].bytecode, &argsPushed, 1 ); // arg #
		pushData( expression.context[depth].bytecode, buf, 4 ); // hash id

		// normal lib will copy down the result, otherwise
		// ignore it, it will be AFTER args
	}
	else
	{
		// push the number of args
		uint32_t hash = wr_hashStr( functionName );

		unsigned int i=0;
		for( ; i<expression.context[depth].bytecode.functionSpace.count(); ++i )
		{
			if ( expression.context[depth].bytecode.functionSpace[i].hash == hash )
			{
				break;
			}
		}

		expression.context[depth].bytecode.functionSpace[i].references.append() = getBytecodePosition( expression.context[depth].bytecode );
		expression.context[depth].bytecode.functionSpace[i].hash = hash;

		pushOpcode( expression.context[depth].bytecode, O_FUNCTION_CALL_PLACEHOLDER );

		pushData( expression.context[depth].bytecode, &argsPushed, 1 );
		pushData( expression.context[depth].bytecode, "0123", 4 ); // TBD opcode plus index, OR hash if index was not found

		// hash will copydown result same as lib, unless
		// copy/pop which case does nothing

		// normal call does nothing special, to preserve
		// the return value a call to copy-down must be
		// inserted
	}

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::pushObjectTable( WRExpressionContext& context,
											WRarray<WRNamespaceLookup>& localSpace,
											uint32_t hash )
{
	WRstr& token2 = context.token;
	WRValue& value2 = context.value;

	unsigned int i=0;
	for( ; i<context.bytecode.unitObjectSpace.count(); ++i )
	{
		if ( context.bytecode.unitObjectSpace[i].hash == hash )
		{
			break;
		}
	}

	pushOpcode( context.bytecode, O_NewObjectTable );

	context.bytecode.unitObjectSpace[i].references.append() = getBytecodePosition( context.bytecode );
	context.bytecode.unitObjectSpace[i].hash = hash;

	pushData( context.bytecode, "\0\0", 2 );

	context.type = EXTYPE_BYTECODE_RESULT;

	if ( token2 == "{" )
	{
		unsigned char offset = 0;
		for(;;)
		{
			WRExpression nex( localSpace, context.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;
			char end = parseExpression( nex );

			if ( nex.bytecode.all.size() )
			{
				appendBytecode( context.bytecode, nex.bytecode );
				pushOpcode( context.bytecode, O_AssignToObjectTableByOffset );
				pushData( context.bytecode, &offset, 1 );
			}

			++offset;

			if ( end == '}' )
			{
				break;
			}
			else if ( end != ',' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
		}
	}

	return true;
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
		expression.context[depth].setLocalSpace( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
		if ( !getToken(expression.context[depth]) )
		{
			return 0;
		}

		if ( value.type != WR_REF )
		{
			if ( (depth > 0) 
				 && ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					 || (expression.context[depth - 1].type == EXTYPE_LITERAL)) )
			{
				// two labels/literals cannot follow each other
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			// it's a literal
			expression.context[depth].type = EXTYPE_LITERAL;
			if ( value.type == WR_COMPILER_LITERAL_STRING )
			{
				expression.context[depth].literalString = *(WRstr*)value.p;
			}
			
			++depth;
			continue;
		}

		if ( token == ";" || token == ")" || token == "}" || token == "," || token == "]" || token == ":" )
		{
			end = token[0];
			break;
		}

		if ( token == "{" )
		{
			if ( depth == 0 )
			{
				if ( parseExpression(expression) != '}' )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				return ';';
			}
			else
			{
				// it's an initializer

				if ( depth < 2 )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}
				if ( (expression.context[depth - 1].operation
					  && expression.context[ depth - 1 ].operation->opcode != O_Assign) )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				if ( (expression.context[depth - 2].operation
					  && expression.context[ depth - 2 ].operation->opcode != O_Index) )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				expression.context.remove( depth - 1, 1 ); // knock off the equate
				depth--;

				WRstr t("@init");
				for( int o=0; c_operations[o].token; o++ )
				{
					if ( t == c_operations[o].token )
					{
						expression.context[depth].operation = c_operations + o;
						expression.context[depth].type = EXTYPE_OPERATION;
						break;
					}
				}


				WRstr& token2 = expression.context[depth].token;
				WRValue& value2 = expression.context[depth].value;

				uint16_t initializer = 0;


				// TODO - more effiecient way to do this, it allocates
				// twice for the case of
				// a[500] = { 1 };
				
/*
				// make sure the array we are about to initialize is
				// actually empty (set it to a nonsense int value)
				unsigned char offset[3];
				offset[2] = 0xEE;

				int target = depth - 1;
				if ( (depth > 1)
					 && expression.context[target].operation
					 && !strncmp( expression.context[target].operation->token, "@[]", 3) )
				{
					--target;
				}
								
				if ( expression.context[target].global || (m_unitTop == 0) )
				{
					offset[0] = O_LiteralInt8ToGlobal;
					offset[1] = addGlobalSpaceLoad( expression.bytecode,
													expression.context[target].token,
													true );
				}
				else
				{
					offset[0] = O_LiteralInt8ToLocal;
					offset[1] = addLocalSpaceLoad( expression.bytecode,
												   expression.context[target].token,
												   true );
				}

				pushData( expression.context[depth].bytecode, offset, 3 );
*/






				for(;;)
				{
					if ( !getToken(expression.context[depth]) )
					{
						m_err = WR_ERR_unexpected_EOF;
						return 0;
					}

					if ( token2 == "}" )
					{
						break;
					}

					WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
					nex.context[0].token = token2;
					nex.context[0].value = value2;
					m_loadedToken = token2;
					m_loadedValue = value2;

					char end = parseExpression( nex );

					unsigned char data[2];
					pack16( initializer, data );

					if ( end == '}' )
					{
						appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
						pushData( expression.context[depth].bytecode, data, 2 );
						break;
					}
					else if ( end == ',' )
					{
						appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
						pushData( expression.context[depth].bytecode, data, 2 );
					}
					else if ( end == ':' )
					{
						if ( nex.bytecode.all.size() == 0 )
						{
							pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						}
						else
						{
							appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						}

						WRExpression nex2( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
						nex2.context[0].token = token2;
						nex2.context[0].value = value2;
						char end = parseExpression( nex2 );

						if ( nex2.bytecode.all.size() == 0 )
						{
							pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						}
						else
						{
							appendBytecode( expression.context[depth].bytecode, nex2.bytecode );
						}

						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );

						if ( end == '}' )
						{
							break;
						}
						else if ( end != ',' )
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}
					}
					else
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					++initializer;
				}

				if ( !getToken(expression.context[depth]) || token2 != ";" )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}

				m_loadedToken = token2;
				m_loadedValue = value2;

				++depth;

				continue;
			}
		}
		
		if ( operatorFound(token, expression.context, depth) )
		{
			++depth;
			continue;
		}
		
		if ( token == "new" )
		{
			if ( (depth < 2) || 
				 (expression.context[depth - 1].operation
				  && expression.context[ depth - 1 ].operation->opcode != O_Assign) )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}

			if ( (depth < 2) 
				 || (expression.context[depth - 2].operation
					 && expression.context[ depth - 2 ].operation->opcode != O_Index) )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
			
			WRstr& token2 = expression.context[depth].token;
			WRValue& value2 = expression.context[depth].value;

			bool isGlobal;
			WRstr prefix;

			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !isValidLabel(token2, isGlobal, prefix) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			WRstr functionName = token2;
			uint32_t hash = wr_hashStr( functionName );
			
			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( token2 == ";" )
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}

				m_loadedToken = ";";
				m_loadedValue.p2 = INIT_AS_REF;
			}
			else if ( token2 == "(" )
			{
				if ( !parseCallFunction(expression, functionName, depth, true) )
				{
					return 0;
				}

				if ( !getToken(expression.context[depth]) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}

				if ( token2 != "{" )
				{
					m_loadedToken = token2;
					m_loadedValue = value2;
				}
			}
			else if (token2 == "{")
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}
				token2 = "{";
			}
			else if ( token2 == "[" )
			{
				// must be "array" directive
				if ( !getToken(expression.context[depth], "]") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				expression.context.remove( depth - 1, 1 ); // knock off the equate
				depth--;

				WRstr& token3 = expression.context[depth].token;
				WRValue& value3 = expression.context[depth].value;

				// we know we're about to create at least X new
				// commands in a row, unless the function calls
				// allocate memory (a distinct possibility) no GC will
				// be required, so pause it by the number of
				// allocations (TBD) 
				pushOpcode( expression.context[depth].bytecode, O_GC_Command );
				unsigned int gcOffset = expression.context[depth].bytecode.all.size();
				pushData( expression.context[depth].bytecode, "\0\0", 2 );

				if ( !getToken(expression.context[depth], "{") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				uint16_t initializer = 0;

				if ( token3 == "{" )
				{
					for(;;)
					{
						if ( !parseCallFunction(expression, functionName, depth, false) )
						{
							return 0;
						}

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( token3 == "}" )
						{
							break;
						}
						else if (token3 != "{")
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}

						if ( !pushObjectTable(expression.context[depth], expression.bytecode.localSpace, hash) )
						{
							m_err = WR_ERR_bad_expression;
							return 0;
						}

						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );

						unsigned char data[2];
						pack16( initializer, data );
						++initializer;

						pushData( expression.context[depth].bytecode, data, 2 );

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( token3 == "," )
						{
							continue;
						}
						else if ( token3 != "}" )
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}
						else
						{
							break;
						}
					}
					
					if ( !getToken(expression.context[depth], ";") )
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					WRstr t("@init");
					for( int o=0; c_operations[o].token; o++ )
					{
						if ( t == c_operations[o].token )
						{
							expression.context[depth].type = EXTYPE_OPERATION;
							expression.context[depth].operation = c_operations + o;
							break;
						}
					}

					m_loadedToken = token3;
					m_loadedValue = value3;

				}
				else if ( token2 != ";" )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				pack16( initializer, expression.context[depth].bytecode.all.p_str(gcOffset) );

				++depth;
				continue;
			}
			else if ( token2 != "{" )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !pushObjectTable(expression.context[depth], expression.bytecode.localSpace, hash) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

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
				if ( !parseCallFunction(expression, expression.context[depth].token, depth, true) )
				{
					return 0;
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
				WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
				nex.context[0].token = token;
				nex.context[0].value = value;
				m_loadedToken = token;
				m_loadedValue = value;
				if ( parseExpression(nex) != ')' )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}


				if ( depth > 0 && expression.context[depth - 1].operation
					 && expression.context[depth - 1].operation->opcode == O_RemoveFromHashTable )
				{
					--depth;
					WRstr t( "._remove" );
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( t, expression.context, depth );
				}
				else if ( depth > 0 && expression.context[depth - 1].operation
						  && expression.context[depth - 1].operation->opcode == O_HashEntryExists )
				{
					--depth;
					WRstr t( "._exists" );
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( t, expression.context, depth );
				}
				else
				{
					expression.context[depth].type = EXTYPE_BYTECODE_RESULT;
					expression.context[depth].bytecode = nex.bytecode;
				}
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


			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;

			if ( parseExpression(nex) != ']' )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}
			
			if ( nex.bytecode.all.size() == 0 )
			{
				WRstr t( "@[]" );
				operatorFound( t, expression.context, depth );
				expression.context[depth].bytecode.all.shave(1);
				expression.context[depth].bytecode.opcodes.shave(1);
				pushOpcode( expression.context[depth].bytecode, O_IndexLiteral8 );
				unsigned char c = 0;
				pushData( expression.context[depth].bytecode, &c, 1 );
			}
			else 
			{
				WRstr t( "@[]" );
				expression.context[depth].bytecode = nex.bytecode;
				operatorFound( t, expression.context, depth );
			}


			++depth;
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		if ( isValidLabel(token, isGlobal, prefix) )
		{
			if ( (depth > 0) 
				&& ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					|| (expression.context[depth - 1].type == EXTYPE_LITERAL)) )
			{
				// two labels/literals cannot follow each other
				m_err = WR_ERR_bad_expression;
				return 0;
			}


			WRstr label = token;
			if ( depth == 0 && !m_parsingFor )
			{
				if ( !getToken(expression.context[depth]) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}
				
				if ( token == ":" )
				{
					uint32_t hash = wr_hashStr( label );
					for( unsigned int i=0; i<expression.bytecode.jumpOffsetTargets.count(); ++i )
					{
						if ( expression.bytecode.jumpOffsetTargets[i].gotoHash == hash )
						{
							m_err = WR_ERR_bad_goto_label;
							return false;
						}
					}
					
					int index = addRelativeJumpTarget( expression.bytecode );
					expression.bytecode.jumpOffsetTargets[index].gotoHash = hash;
					expression.bytecode.jumpOffsetTargets[index].offset = expression.bytecode.all.size() + 1;

					// this always return a value
					pushOpcode( expression.bytecode, O_LiteralZero );

					
					return ';';
				}
				else
				{
					m_loadedToken = token;
					m_loadedValue = value;
				}
			}
			
			
			expression.context[depth].type = EXTYPE_LABEL;
			expression.context[depth].token = label;
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
bool WRCompilationContext::parseUnit( bool isStruct )
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
	m_units[m_unitTop].bytecode.isStructSpace = isStruct;
	
	// get the function name
	if ( getToken(ex, "(") )
	{
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
	}
	else if ( token != "{" )
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

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
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

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
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

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, jumpToTop );
	
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
	push iterator
B:
	get next or jump A
	.
	.
	.
	goto B
A:
*/					


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

	bool foreachPossible = true;
	bool foreachKV = false;
	bool foreachV = false;
	int foreachLoadI = 0;
	unsigned char foreachLoad[4];
	unsigned char g;

	m_parsingFor = true;

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

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;

		char end = parseExpression( nex );

		if ( foreachPossible )
		{
			if ( foreachLoadI < 3
				 && nex.bytecode.opcodes.size() == 1
				 && nex.bytecode.all.size() == 2
				 && ((nex.bytecode.all[0] == O_LoadFromLocal) || (nex.bytecode.all[0] == O_LoadFromGlobal) ) )
			{
				foreachLoad[foreachLoadI++] = nex.bytecode.all[0];
				foreachLoad[foreachLoadI++] = nex.bytecode.all[1];
			}
			else
			{
				foreachPossible = false;
			}
		}

		pushOpcode( nex.bytecode, O_PopOne );

		appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );
		
		if ( end == ';' )
		{
			foreachPossible = false;
			break;
		}
		else if ( end == ':' )
		{
			if ( foreachPossible )
			{
				WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
				nex.context[0].token = token;
				nex.context[0].value = value;
				end = parseExpression( nex );
				if ( end == ')'
					 && nex.bytecode.opcodes.size() == 1
					 && nex.bytecode.all.size() == 2 )
				{

					WRstr T;
					T.format( "foreach:%d", m_foreachHash++ );
					g = (unsigned char)(addGlobalSpaceLoad( m_units[0].bytecode, T, true ));

					if ( nex.bytecode.opcodes[0] == O_LoadFromLocal )
					{
						m_units[m_unitTop].bytecode.all += O_LPushIterator;
					}
					else
					{
						m_units[m_unitTop].bytecode.all += O_GPushIterator;
					}

					m_units[m_unitTop].bytecode.all += nex.bytecode.all[1];
					pushData( m_units[m_unitTop].bytecode, &g, 1 );

					if ( foreachLoadI == 4 )
					{
						foreachKV = true;
					}
					else if ( foreachLoadI == 2 )
					{
						foreachV = true;
					}
					else
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					break;
				}
				else
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}
			}
			else
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
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
	
	if ( foreachV || foreachKV )
	{
		if ( foreachV )
		{
			unsigned char load[2] = { foreachLoad[1], g };

			if ( foreachLoad[0] == O_LoadFromLocal )
			{
				addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LNextValueOrJump, *m_breakTargets.tail(), load, 2 );
			}
			else
			{
				addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GNextValueOrJump, *m_breakTargets.tail(), load, 2 );
			}
		}
		else
		{
			unsigned char load[3] = { foreachLoad[1], foreachLoad[3], g };
			
			if ( foreachLoad[0] == O_LoadFromLocal )
			{
				if ( foreachLoad[2] == O_LoadFromLocal )
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LLNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
				else
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LGNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
			}
			else
			{
				if ( foreachLoad[2] == O_LoadFromLocal )
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GLNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
				else
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GGNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
			}
		}
		
		m_parsingFor = false;

		// [ code ]
		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}
	}
	else
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		// [ condition ]
		if ( token != ";" )
		{
			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
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


		WRExpression post( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );

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

			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
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

		m_parsingFor = false;

		// [ code ]
		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}

		// <- continue point
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

		// [post code]
		appendBytecode( m_units[m_unitTop].bytecode, post.bytecode );
	}

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionPoint );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::lookupConstantValue( WRstr& prefix, WRValue* value )
{
	for( unsigned int v=0; v<m_units[m_unitTop].constantValues.count(); ++v )
	{
		if ( m_units[m_unitTop].constantValues[v].label == prefix )
		{
			if ( value )
			{
				*value = m_units[m_unitTop].constantValues[v].value;
			}
			return true;
		}
	}

	for( unsigned int v=0; v<m_units[0].constantValues.count(); ++v )
	{
		if ( m_units[0].constantValues[v].label == prefix )
		{
			if ( value )
			{
				*value = m_units[0].constantValues[v].value;
			}
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseEnum( int unitIndex )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	unsigned int index = 0;

	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_token;
			return false;
		}

		if ( token == "}" )
		{
			break;
		}
		else if ( token == "," )
		{
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
		{
			m_err = WR_ERR_bad_label;
			return false;
		}

		prefix = token;
		
		WRValue defaultValue;
		defaultValue.init();
		defaultValue.ui = index++;

		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == "=" )
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			if ( value.type == WR_REF )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}
		}
		else
		{
			m_loadedToken = token;
			m_loadedValue = value;

			value = defaultValue;
		}

		if ( value.type == WR_INT )
		{
			index = value.ui + 1;
		}
		else if ( value.type != WR_FLOAT )
		{
			m_err = WR_ERR_bad_label;
			return false;
		}
		
		if ( lookupConstantValue(prefix) )
		{
			m_err = WR_ERR_constant_refined;
			return false;
		}

		ConstantValue& newVal = m_units[m_unitTop].constantValues.append();
		newVal.label = prefix;
		newVal.value = value;
	}

	if ( getToken(ex) && token != ";" )
	{
		m_loadedToken = token;
		m_loadedValue = value;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WRCompilationContext::getSingleValueHash( const char* end )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;
	getToken( ex );
	
	if ( token == "(" )
	{
		return getSingleValueHash( ")" );
	}

	if ( value.type == WR_REF )
	{
		m_err = WR_ERR_switch_bad_case_hash;
		return 0;
	}
	
	uint32_t hash = (value.type == WR_COMPILER_LITERAL_STRING) ? wr_hashStr(token) : value.getHash();

	if ( !getToken(ex, end) )
	{
		m_err = WR_ERR_unexpected_token;
		return 0;
	}

	return hash;
}

/*


    continue target:
    targetswitchLinear
    8-bit max
pc> 16-bit default location
    16 bit case offset
    16 bit case offset
    16 bit case offset
    cases...
    [cases]
    break target:


continue target:
switch ins
16-bit mod
16-bit default location
32 hash case mod 0 : 16 bit case offset
32 hash case mod 1 : 16 bit case offset
32 hash case mod 2 : 16 bit case offset
...
[cases]
break target:

*/

//------------------------------------------------------------------------------
struct WRSwitchCase
{
	uint32_t hash; // hash to get this case
	bool occupied; // null hash is legal and common, must mark occupation of a node with extra flag
	bool defaultCase;
	int16_t jumpOffset; // where to go to get to this case
};

//------------------------------------------------------------------------------
bool WRCompilationContext::parseSwitch( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	WRarray<WRSwitchCase> cases;
	int16_t defaultOffset = -1;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
	
	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	
	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	WRBytecode bytecodeSnapshot; // snapshot up to now

	bytecodeSnapshot = m_units[m_unitTop].bytecode;
	m_units[m_unitTop].bytecode.clear();

	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	WRSwitchCase* swCase = 0;

	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if( token == "}" )
		{
			break;
		}
		else if ( token == "case" )
		{
			swCase = &cases.append();
			swCase->jumpOffset = m_units[m_unitTop].bytecode.all.size();
			swCase->hash = getSingleValueHash(":");
			swCase->occupied = true;
			swCase->defaultCase = false;

			if (m_err)
			{
				return false;
			}

			if ( cases.count() > 1 )
			{
				for( unsigned int h = 0; h < cases.count() - 1 ; ++h )
				{
					if ( cases[h].occupied && !cases[h].defaultCase && (swCase->hash == cases[h].hash) )
					{
						m_err = WR_ERR_switch_duplicate_case;
						return false;
					}
				}
			}
		}
		else if ( token == "default" )
		{
			if ( defaultOffset != -1 )
			{
				m_err = WR_ERR_switch_duplicate_case;
				return false;
			}
			
			swCase = &cases.append();
			swCase->jumpOffset = m_units[m_unitTop].bytecode.all.size();
			swCase->occupied = true;
			swCase->defaultCase = true;

			if ( !getToken(ex, ":") )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			defaultOffset = swCase->jumpOffset;
		}
		else
		{
			if (swCase == 0)
			{
				m_err = WR_ERR_switch_case_or_default_expected;
				return false;
			}
			
			m_loadedToken = token;
			m_loadedValue = value;

			if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
	}

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	if ( cases.count() == 1 && defaultOffset != -1 )
	{
		// single default case? no switch here just always execute the default case
		appendBytecode( bytecodeSnapshot, m_units[m_unitTop].bytecode );
	}
	else if ( cases.count() > 0 )
	{
		// first try the easy way

		// find the highest hash value, and size an array to that
		unsigned int size = 0;
		for( unsigned int d=0; d<cases.count(); ++d )
		{
			if ( cases[d].defaultCase )
			{
				continue;
			}

			if ( cases[d].hash > size )
			{
				size = cases[d].hash;
			}

			if ( size >= 254 )
			{
				break;
			}
		}

		// first try the easy way

		++size;
		
		WRSwitchCase* table = 0;
		unsigned char packbuf[4];
		
		if ( size < 254 )
		{
			pushOpcode( bytecodeSnapshot, O_SwitchLinear );

			packbuf[0] = size;
			pushData( bytecodeSnapshot, packbuf, 1 );

			table = new WRSwitchCase[size];
			memset( table, 0, size*sizeof(WRSwitchCase) );

			for( unsigned int i = 0; i<size; ++i ) // for each of the possible entries..
			{
				for( unsigned int hash = 0; hash<cases.count(); ++hash ) // if a hash matches it, populate that table entry
				{
					if ( cases[hash].occupied && !cases[hash].defaultCase && (cases[hash].hash == i) )
					{
						table[cases[hash].hash].jumpOffset = cases[hash].jumpOffset + 2*size + 2;
						table[cases[hash].hash].occupied = true;
						break;
					}
				}
			}

			// default case is jumping to the end of the code segment
			if ( defaultOffset == -1 )
			{
				defaultOffset = m_units[m_unitTop].bytecode.all.size() + 2*size + 2;
			}
			else
			{
				defaultOffset += 2*size + 2;
			}

			pushData( bytecodeSnapshot, pack16(defaultOffset, packbuf), 2 );

			for( unsigned int i=0; i<size; ++i )
			{
				if ( table[i].occupied )
				{
					pushData( bytecodeSnapshot, pack16(table[i].jumpOffset, packbuf), 2 );
				}
				else
				{
					pushData( bytecodeSnapshot, pack16(defaultOffset, packbuf), 2 );
				}
			}
		}
		else
		{
			// find a suitable mod
			uint16_t mod = 1;
			for( ; mod<0x7FFE; ++mod )
			{
				table = new WRSwitchCase[mod];
				memset( table, 0, sizeof(WRSwitchCase)*mod );

				unsigned int c=0;
				for( ; c<cases.count(); ++c )
				{
					if ( cases[c].defaultCase )
					{
						continue;
					}

					if ( table[cases[c].hash % mod].occupied )
					{
						break;
					}

					table[cases[c].hash % mod].hash = cases[c].hash;
					table[cases[c].hash % mod].jumpOffset = 2 + 2 + (mod * 6) + cases[c].jumpOffset;
					table[cases[c].hash % mod].occupied = true;
				}

				if ( c >= cases.count() )
				{
					break;
				}
				else
				{
					delete[] table;
					table = 0;
				} 
			}

			if ( mod >= 0x7FFE )
			{
				m_err = WR_ERR_switch_construction_error;
				return false;
			}

			pushOpcode( bytecodeSnapshot, O_Switch ); // add switch command
			unsigned char packbuf[4];

			pushData( bytecodeSnapshot, pack16(mod, packbuf), 2 ); // mod value

			if ( defaultOffset == -1 ) // no default case? point it to the end
			{
				defaultOffset = 2 + 2 + (mod * 6) + m_units[m_unitTop].bytecode.all.size();
			}
			else
			{
				defaultOffset += 2 + 2 + (mod * 6);
			}

			pushData( bytecodeSnapshot, pack16(defaultOffset, packbuf), 2 );

			for( uint16_t m = 0; m<mod; ++m )
			{
				pushData( bytecodeSnapshot, pack32(table[m].hash, packbuf), 4 );

				if ( !table[m].occupied )
				{
					pushData( bytecodeSnapshot, pack16(defaultOffset, packbuf), 2 );
				}
				else
				{
					pushData( bytecodeSnapshot, pack16(table[m].jumpOffset, packbuf), 2 );
				}
			}
		}

		appendBytecode( bytecodeSnapshot, m_units[m_unitTop].bytecode );

		delete[] table;
	}
	// else 0 cases, skip switch parsing nothing happens!

	m_units[m_unitTop].bytecode = bytecodeSnapshot;
		
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode ); // at least do the ones we added

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

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
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
		WRValue& value = ex.value;

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
				WRExpression nex( m_units[unitIndex].bytecode.localSpace, m_units[unitIndex].bytecode.isStructSpace );
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
		else if ( token == "struct" )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}

			if ( !parseUnit(true) )
			{
				return false;
			}
		}
		else if ( token == "function" )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}
			
			if ( !parseUnit(false) )
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
		else if ( token == "enum" )
		{
			if ( !parseEnum(unitIndex) )
			{
				return false;
			}
		}
		else if ( token == "switch" )
		{
			if ( !parseSwitch(returnCalled, opcodeToReturn) )
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
		else if ( token == "gc_pause" )
		{
			if ( !getToken(ex, "(") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}

			if ( !getToken(ex) || value.type != WR_INT )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}

			uint16_t cycle = value.ui;

			pushOpcode( m_units[unitIndex].bytecode, O_GC_Command );
			unsigned char count[2];
			pack16( cycle, count );
			pushData( m_units[unitIndex].bytecode, count, 2 );

			if ( !getToken(ex, ")") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}
			
			if ( !getToken(ex, ";") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}
		}
		else if ( token == "goto" )
		{
			if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			bool isGlobal;
			WRstr prefix;
			if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
			{
				m_err = WR_ERR_bad_goto_label;
				return false;
			}

			GotoSource& G = m_units[unitIndex].bytecode.gotoSource.append();
			G.hash = wr_hashStr( token );
			G.offset = m_units[unitIndex].bytecode.all.size();
			pushData( m_units[unitIndex].bytecode, "\0\0\0", 3 );

			if ( !getToken(ex, ";"))
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
		}
		else
		{
			WRExpression nex( m_units[unitIndex].bytecode.localSpace, m_units[unitIndex].bytecode.isStructSpace );
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
void WRCompilationContext::createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size )
{
	if ( unit.bytecode.localSpace.count() == 0 )
	{
		*size = 0;
		*buf = 0;
		return;
	}

	WRHashTable<unsigned char> offsets;
	for( unsigned char i=unit.arguments; i<unit.bytecode.localSpace.count(); ++i )
	{
		offsets.set( unit.bytecode.localSpace[i].hash, i - unit.arguments );
	}
	
	*size = 2;
	*buf = new unsigned char[ (offsets.m_mod * 5) + 4 ];
	(*buf)[(*size)++] = (offsets.m_mod>>8) & 0xFF;
	(*buf)[(*size)++] = offsets.m_mod & 0xFF;

	for( int i=0; i<offsets.m_mod; ++i )
	{
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>24) & 0xFF;
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>16) & 0xFF;
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>8) & 0xFF;
		(*buf)[(*size)++] = offsets.m_list[i].hash & 0xFF;
		(*buf)[(*size)++] = offsets.m_list[i].hash ? offsets.m_list[i].value : (unsigned char)-1;
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::link( unsigned char** out, int* outLen )
{
	WROpcodeStream code;

	code += (unsigned char)(m_units.count() - 1);
	code += (unsigned char)(m_units[0].bytecode.localSpace.count());

	unsigned char data[4];

	// register the function signatures
	for( unsigned int u=1; u<m_units.count(); ++u )
	{
		data[3] = u - 1; // index
		data[2] = m_units[u].arguments; // args
		data[1] = m_units[u].bytecode.localSpace.count(); // local frame size
		data[0] = 0;
		code += O_LiteralInt32;
		code.append( data, 4 ); // placeholder, it doesn't matter

		code += O_LiteralInt32; // hash
		code.append( pack32(m_units[u].hash, data), 4 );

		// offset placeholder
		code += O_LiteralInt16;
		m_units[u].offsetInBytecode = code.size();
		code.append( data, 2 ); // placeholder, it doesn't matter

		code += O_RegisterFunction;
	}

	// append all the unit code
	for( unsigned int u=0; u<m_units.count(); ++u )
	{
		if ( u > 0 ) // for the non-zero unit fill location into the jump table
		{
			int16_t offset = code.size();
			pack16( offset, code.p_str(m_units[u].offsetInBytecode) );
		}

		// fill in relative jumps for the gotos
		for( unsigned int g=0; g<m_units[u].bytecode.gotoSource.count(); ++g )
		{
			unsigned int j=0;
			for( ; j<m_units[u].bytecode.jumpOffsetTargets.count(); j++ )
			{
				if ( m_units[u].bytecode.jumpOffsetTargets[j].gotoHash == m_units[u].bytecode.gotoSource[g].hash )
				{
					int diff = m_units[u].bytecode.jumpOffsetTargets[j].offset - m_units[u].bytecode.gotoSource[g].offset;
					diff -= 2;
					if ( (diff < 128) && (diff > -129) )
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump8;
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset + 1 ) = diff;
					}
					else
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump;
						pack16( diff, m_units[u].bytecode.all.p_str(m_units[u].bytecode.gotoSource[g].offset + 1) );
					}

					break;
				}
			}

			if ( j >= m_units[u].bytecode.jumpOffsetTargets.count() )
			{
				m_err = WR_ERR_goto_target_not_found;
				return;
			}
		}

		int base = code.size();

		code.append( m_units[u].bytecode.all, m_units[u].bytecode.all.size() );

		// load new's
		for( unsigned int f=0; f<m_units[u].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash != N.hash )
					{
						continue;
					}

					if ( m_units[u2].offsetOfLocalHashMap == 0 )
					{
						int size;
						unsigned char* buf = 0;

						createLocalHashMap( m_units[u2], &buf, &size );
						
						if ( size )
						{
							buf[0] = (unsigned char)(m_units[u2].bytecode.localSpace.count() - m_units[u2].arguments); // number of entries
							buf[1] = m_units[u2].arguments;
							m_units[u2].offsetOfLocalHashMap = code.size();
							code.append( buf, size );
						}

						delete[] buf;
					}

					if ( m_units[u2].offsetOfLocalHashMap != 0 )
					{
						int index = base + N.references[r];

						code[index] = (m_units[u2].offsetOfLocalHashMap>>8) & 0xFF;
						code[index+1] = m_units[u2].offsetOfLocalHashMap & 0xFF;
					}

					break;
				}
			}
		}
		
		// load function table
		for( unsigned int x=0; x<m_units[u].bytecode.functionSpace.count(); ++x )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.functionSpace[x];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				unsigned int u2 = 1;
				int index = base + N.references[r];

				for( ; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						code[index] = O_CallFunctionByIndex;

						code[index+2] = (char)(u2 - 1);

						// r+1 = args
						// r+2345 = hash
						// r+6

						if ( code[index+5] == O_PopOne || code[index + 6] == O_NewObjectTable)
						{
							code[index+3] = 3; // skip past the pop, or TO the NewObjectTable
						}
						else 
						{
							code[index+3] = 2; // skip by this push is seen
							code[index+5] = O_PushIndexFunctionReturnValue;
						}
						
						break;
					}
				}

				if ( u2 >= m_units.count() )
				{
					if ( code[index+5] == O_PopOne )
					{
						code[index] = O_CallFunctionByHashAndPop;
					}
					else
					{
						code[index] = O_CallFunctionByHash;
					}

					pack32( N.hash, code.p_str(index+2) );
				}
			}
		}
	}

	uint32_t hash = wr_hash( code, code.size() );

	pack32( hash, data );
	code.append( data, 4 );

	if ( !m_err )
	{
		*outLen = code.size();
		code.release( out );
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
	m_parsingFor = false;
	m_unitTop = 0;
	m_foreachHash = 0;

	m_units.setCount(1);
	
	bool returnCalled = false;

	m_loadedValue.p2 = INIT_AS_REF;

	do
	{

		WRExpressionContext ex;
		WRstr& token = ex.token;
		WRValue& value = ex.value;
		if ( !getToken(ex) )
		{
			break;
		}
		m_loadedToken = token;
		m_loadedValue = value;
		parseStatement( 0, ';', returnCalled, O_GlobalStop );

	} while ( !m_EOF && (m_err == WR_ERR_None) );

	WRstr msg;
	
	if ( m_err != WR_ERR_None )
	{			
		int onChar = 0;
		int onLine = 1;
		WRstr line;

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
		pushOpcode( m_units[0].bytecode, O_GlobalStop );
	}

	pushOpcode( m_units[0].bytecode, O_Stop );
	
	link( out, outLen );
	if ( m_err )
	{
		printf( "link error [%d]\n", m_err );
		if ( errorMsg )
		{
			snprintf( errorMsg, 32, "link error [%d]\n", m_err );
		}

	}
		 
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

	"LiteralInt32",
	"LiteralZero",
	"LiteralFloat",
	"LiteralString",

	"CallFunctionByHash",
	"CallFunctionByHashAndPop",
	"CallFunctionByIndex",
	"PushIndexFunctionReturnValue",

	"CallLibFunction",

	"NewObjectTable",
	"AssignToObjectTableByOffset",

	"AssignToHashTableAndPop",
	"RemoveFromHashTable",
	"HashEntryExists",

	"PopOne",
	"ReturnZero",
	"Return",
	"Stop",

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

	"CallLibFunctionAndPop",
};
#endif

#else // WRENCH_WITHOUT_COMPILER

int wr_compile( const char* source, const int size, unsigned char** out, int* outLen, char* errMsg )
{
	return WR_ERR_compiler_not_loaded;
}

	
#endif
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
void WRContext::mark( WRValue* s )
{
	if ( IS_REFARRAY(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		// we don't c mark this type, but we might mark it's target
		mark( s->r );
		return;
	}

	if ( !IS_EXARRAY_TYPE(s->xtype) || s->va->m_skipGC || (s->va->m_size & 0x40000000) )
	{
		return;
	}

	assert( !IS_RAW_ARRAY(s->xtype) );

	WRGCObject* sva = s->va;

	if ( IS_SVA_VALUE_TYPE(sva) && !(sva->m_size & 0x40000000) )
	{
		// this is an array of values, check them for array-ness too

		WRValue* top = sva->m_Vdata + sva->m_size;
		for( WRValue* V = sva->m_Vdata; V<top; ++V )
		{
			mark( V );
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
		memset( (char*)ret->m_Cdata, 0, sizeof(WRValue) * size);
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

	uint32_t element = ARRAY_ELEMENT_FROM_P2( iterator->p2 );

	if ( iterator->va->m_type == SV_HASH_TABLE )
	{
		for( ; element<iterator->va->m_mod && !iterator->va->m_hashTable[element]; ++element );

		if ( element >= iterator->va->m_mod )
		{
			return false;
		}

		ARRAY_ELEMENT_TO_P2( iterator->p2, (element + 1) );

		element <<= 1;

		value->p2 = INIT_AS_REF;
		value->r = iterator->va->m_Vdata + element;
		if ( key )
		{
			key->p2 = INIT_AS_REF;
			key->r = iterator->va->m_Vdata + element + 1;
		}
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

		&&CallLibFunctionAndPop,
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
	
	union
	{
		WRValue* register1;
		uint16_t switchMod;
	};
	WRValue* frameBase = 0;
	WRValue* stackTop = w->stack;
	WRValue* globalSpace = (WRValue *)(context + 1);

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
		switch( *pc++ )
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
				
				context->registry.getAsRawValueHashTable(context->localFunctions[findex].hash)->wrf = context->localFunctions + findex;

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

				if ( (register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb )
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
					++stackTop;
				}
				pc += 4;
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				args = *pc++;

				if ( (register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->ccb )
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

				register0->p = pc;
				pc = function->offset;

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				assert( stackTop < (w->stack + w->stackSize) );

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

			CASE(CallLibFunction):
			{
				stackTop->p2 = INIT_AS_INT;
				stackTop->p = 0;

				args = *pc++; // which have already been pushed

				if ( (register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb )
				{
					register1->lcb( stackTop, args, context );
				}
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
				args = *pc++; // which have already been pushed

				if ( (register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb )
				{
					register1->lcb( stackTop, args, context );
				}
				pc += 4;

				stackTop -= args;
				
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
				register1 = --stackTop;
				register0 = (stackTop - 1);
				
				if ( !IS_EXARRAY_TYPE(register0->xtype) || *pc < register0->va->m_size )
				{
					register0 = register0->va->m_Vdata + *pc++;
					goto doAssignToLocalAndPop;
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

			CASE(ToInt):
			{
				register0 = stackTop - 1;
				register1 = register0;
				wr_getValue[register1->type]( &register1 );
				if ( register1->type == WR_FLOAT )
				{
					register0->p2 = INIT_AS_INT;
					register0->i = (int)register1->f;
				}
				CONTINUE;
			}

			CASE(ToFloat):
			{
				register0 = stackTop - 1;
				register1 = register0;
				wr_getValue[register1->type]( &register1 );
				if ( register1->type == WR_INT )
				{
					register0->p2 = INIT_AS_FLOAT;
					register0->f = (float)register1->i;
				}
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
				stackTop->i = *pc++;
indexLiteral:
				stackTop->p2 = INIT_AS_INT;
				register0 = stackTop - 1;
				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, register0 );
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
				wr_index[(WR_INT<<2)|register0->type]( context, stackTop, register0, stackTop - 1 );
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
				pc += 2;
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

				if ( (uint32_t)READ_32_FROM_PC(hashLoc) != hash )
				{
					hashLoc = pc - 2; // nope, point it at default vector
				}

				hashLoc += 4; // yup, point hashLoc to jump vector
				
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
				goto targetFuncOpSkipLoadNoClobberF;
			}

			CASE(GLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactGLFunc:
				register0 = globalSpace + *pc++;
				register1 = frameBase + *pc++;
				goto targetFuncOpSkipLoadNoClobberF;
			}

			CASE(LLBinaryMultiplication):
			{
				floatCall = multiplicationF;
				intCall = multiplicationI;
CompactFFFunc:
				register0 = frameBase + *pc++;
				register1 = frameBase + *pc++;
				goto targetFuncOpSkipLoadNoClobberF;
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

			
//-------------------------------------------------------------------------------------------------------------
#else //-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

			
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

			
//-------------------------------------------------------------------------------------------------------------
#endif//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------


#ifndef WRENCH_JUMPTABLE_INTERPRETER
		}
	}
#endif
}

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
WRState* wr_newState( int stackSize )
{
	WRState* state = (WRState *)malloc( stackSize*sizeof(WRValue) + sizeof(WRState) );
	memset( (unsigned char*)state, 0, stackSize*sizeof(WRValue) + sizeof(WRState) );

	state->stackSize = stackSize;
	state->stack = (WRValue *)((unsigned char *)state + sizeof(WRState));

	state->globalRegistry.init( 0, SV_VOID_HASH_TABLE );

	return state;
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	while( w->contextList )
	{
		wr_destroyContext( w->contextList );
	}

	w->globalRegistry.clear();

	free( w );
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return (WRError)w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize )
{
	const unsigned char* p = block + (blockSize - 4);
	uint32_t hash = READ_32_FROM_PC( p );
	if ( hash != wr_hash(block, (blockSize - 4)) )
	{
		w->err = WR_ERR_bad_bytecode_CRC;
		return 0;
	}

	int needed = sizeof(WRContext) // class
				 + block[1] * sizeof(WRValue)  // globals
				 + block[0] * sizeof(WRFunction); // local functions

	WRContext* C = (WRContext *)malloc( needed );

	memset((char*)C, 0, needed);

	C->globals = block[1];
	C->w = w;

	C->localFunctions = (WRFunction*)((unsigned char *)C + sizeof(WRContext) + block[1] * sizeof(WRValue));

	C->registry.init( 0, SV_VOID_HASH_TABLE );
	C->registry.m_vNext = w->contextList;
	C->bottom = block;

	w->contextList = C;

	if ( wr_callFunction(w, C, (int32_t)0) )
	{
		wr_destroyContext( C );
		return 0;
	}

	return C;
}


//------------------------------------------------------------------------------
void wr_destroyContext( WRContext* context )
{
	if ( !context )
	{
		return;
	}

	WRContext* prev = 0;

	// unlink it
	for( WRContext* c = context->w->contextList; c; c = (WRContext*)c->registry.m_vNext )
	{
		if ( c == context )
		{
			if ( prev )
			{
				prev->registry.m_vNext = c->registry.m_vNext;
			}
			else
			{
				context->w->contextList = (WRContext*)context->w->contextList->registry.m_vNext;
			}

			while ( context->svAllocated )
			{
				WRGCObject* next = context->svAllocated->m_next;
				context->svAllocated->clear();
				free( context->svAllocated );
				context->svAllocated = next;
			}

			context->registry.clear();

			free( context );

			break;
		}
		prev = c;
	}
}

//------------------------------------------------------------------------------
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRValue* V = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(name) );
	V->usr = usr;
	V->ccb = function;
}

//------------------------------------------------------------------------------
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function )
{
	w->globalRegistry.getAsRawValueHashTable(wr_hashStr(signature))->lcb = function;
}

//------------------------------------------------------------------------------
const int WRValue::asInt() const
{
	if ( type == WR_INT )
	{
		return i;
	}
	else if ( type == WR_REF )
	{
		return r->asInt();
	}
	else if ( type == WR_FLOAT )
	{
		return (int)f;
	}
	else if ( IS_REFARRAY(xtype) )
	{
		WRValue temp;
		arrayValue( &temp );
		return temp.asInt();
	}

	return 0;
}

//------------------------------------------------------------------------------
const float WRValue::asFloat() const
{
	if ( type == WR_FLOAT )
	{
		return f;
	}
	else if ( type == WR_REF )
	{
		return r->asFloat();
	}
	else if ( type == WR_INT )
	{
		return (float)i;
	}
	else if ( IS_REFARRAY(xtype) )
	{
		WRValue temp;
		arrayValue( &temp );
		return temp.asFloat();
	}

	return 0;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, size_t len ) const
{
	if ( xtype )
	{
		switch( xtype & EX_TYPE_MASK )
		{
			case WR_EX_ARRAY:
			{
				if ( va->m_type == SV_CHAR )
				{
					unsigned int s = 0;
					while( (string[s]=va->m_Cdata[s]) )
					{
						s++;
					}

				}
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
			case WR_FLOAT: { wr_ftoa( f, string, len ); break; }
			case WR_INT: { wr_itoa( i, string, len ); break; }
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
	WRValue* cF = 0;
	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_run_must_be_called_by_itself_first;
		}

		cF = context->registry.getAsRawValueHashTable( hash );
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
WRFunction* wr_getFunction( WRContext* context, const char* functionName )
{
	return context->registry.getAsRawValueHashTable(wr_hashStr(functionName))->wrf;
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
void wr_makeString( WRContext* context, WRValue* val, const unsigned char* data, const int len )
{
	val->p2 = INIT_AS_ARRAY;
	val->va = (WRGCObject*)malloc( sizeof(WRGCObject) );
	val->va->init( len, SV_CHAR );
	val->va->m_skipGC = 1;
	memcpy( (unsigned char *)val->va->m_data, data, len );
}

//------------------------------------------------------------------------------
void wr_freeString( WRValue* val )
{
	val->va->clear();
	free( val->va );
}

//------------------------------------------------------------------------------
void wr_makeContainer( WRValue* val, const uint16_t sizeHint )
{
	val->p2 = INIT_AS_HASH_TABLE;
	val->va = (WRGCObject*)malloc( sizeof(WRGCObject) );
	val->va->init( sizeHint, SV_VOID_HASH_TABLE );
	val->va->m_skipGC = 1;
}

//------------------------------------------------------------------------------
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value )
{
	WRValue* entry = container->va->getAsRawValueHashTable( wr_hashStr(name) );
	entry->r = value;
	entry->p2 = INIT_AS_REF;
}

//------------------------------------------------------------------------------
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size )
{
	assert( size <= 0x1FFFFF );

	WRValue* entry = container->va->getAsRawValueHashTable( wr_hashStr(name) );
	entry->c = array;
	entry->p2 = INIT_AS_RAW_ARRAY | (size<<8);
}

//------------------------------------------------------------------------------
void wr_destroyContainer( WRValue* val )
{
	if ( val->xtype != WR_EX_HASH_TABLE )
	{
		return;
	}

	val->va->clear();
	free( val->va );
	val->init();
}

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
void* WRValue::array( unsigned int* len, char* arrayType ) const
{
	if ( type == WR_REF )
	{
		return r->array( len, arrayType );
	}

	if ( xtype != WR_EX_ARRAY )
	{
		return 0;
	}

	if ( arrayType )
	{
		*arrayType = va->m_type;
	}

	if ( len )
	{
		*len = va->m_size;
	}

	return va->m_data;
}

//------------------------------------------------------------------------------
const char* WRValue::c_str( unsigned int* len ) const
{
	char arrayType = 0;
	void* ret = array( len, &arrayType );
	return (arrayType == SV_CHAR) ? (char*)ret : 0;
}

//------------------------------------------------------------------------------
WRGCObject* growValueArray( WRGCObject* va, int newSize )
{
	WRGCObject* newArray = (WRGCObject*)malloc( sizeof(WRGCObject) );
	newArray->init( newSize + 1, (WRGCObjectType)va->m_type );
	
	newArray->m_next = va->m_next;
	va->m_next = newArray;

	int size_of = sizeof(WRValue);
	if ( va->m_type == SV_CHAR )
	{
		size_of = 1;
	}
	
	memcpy( newArray->m_Cdata, va->m_Cdata, va->m_size * size_of );
	memset( newArray->m_Cdata + (va->m_size*size_of), 0, (newArray->m_size - va->m_size) * size_of );
	return newArray;
}

//------------------------------------------------------------------------------
void WRValue::arrayValue( WRValue* val ) const
{
	unsigned int s = ARRAY_ELEMENT_FROM_P2(p2);

	if ( IS_RAW_ARRAY(r->xtype) )
	{
		val->p2 = INIT_AS_INT;
		val->ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(r->p2))) ? (uint32_t)(unsigned char)(r->c[s]) : 0;
	}
	else if ( r->va->m_type == SV_VALUE )
	{
		if ( s < r->va->m_size )
		{
			*val = r->va->m_Vdata[s];
		}
	}
	else if ( r->va->m_type == SV_CHAR )
	{
		val->p2 = INIT_AS_INT;
		val->ui = (s < r->va->m_size) ? (uint32_t)(unsigned char)r->va->m_Cdata[s] : 0;
	}
	else
	{
		val->init();
	}
}

//------------------------------------------------------------------------------
void wr_arrayToValue( const WRValue* array, WRValue* value, int index )
{
	uint32_t s = index == -1 ? ARRAY_ELEMENT_FROM_P2(array->p2) : index;

	if ( IS_RAW_ARRAY(array->r->xtype) )
	{
		value->p2 = INIT_AS_INT;
		value->ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(array->r->p2))) ? (uint32_t)(unsigned char)(array->r->c[s]) : 0;
	}
	else if( array->r->va->m_type == SV_VALUE )
	{
		if ( s >= array->r->va->m_size )
		{
			if ( array->r->va->m_skipGC )
			{
				value->init();
				return;
			}

			array->r->va = growValueArray( array->r->va, s );
		}
		*value = array->r->va->m_Vdata[s];
	}
	else if ( array->r->va->m_type == SV_CHAR )
	{
		value->p2 = INIT_AS_INT;
		value->i = (s < array->r->va->m_size) ? (uint32_t)(unsigned char)(array->r->va->m_Cdata[s]) : 0;
	}
	else
	{
		value->init();
	}
}

//------------------------------------------------------------------------------
uint32_t WRValue::getHashEx() const
{
	if ( type <= (int)WR_FLOAT )
	{
		return ui;
	}
	else if ( type == WR_REF )
	{
		return r->getHashEx();
	}

	if ( IS_REFARRAY(xtype) )
	{
		WRValue element;
		wr_arrayToValue( this, &element );
		return element.getHash();
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		return wr_hash( va->m_Cdata, va->m_size );
	}

	return 0;
}


//------------------------------------------------------------------------------
void wr_valueToArray( const WRValue* array, WRValue* value )
{
	uint32_t s = ARRAY_ELEMENT_FROM_P2(array->p2);

	if ( IS_RAW_ARRAY(array->r->xtype ) )
	{
		if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(array->r->p2)) )
		{
			array->r->c[s] = value->ui;
		}
	}
	else if ( array->r->va->m_type == SV_CHAR)
	{
		if ( s < array->r->va->m_size ) 
		{
			array->r->va->m_Cdata[s] = value->ui;
		}
	}
	else
	{
		if ( s >= array->r->va->m_size )
		{
			array->r->va = growValueArray(array->r->va, s);
		}
		WRValue* V = array->r->va->m_Vdata + s;
		wr_assign[(V->type<<2)+value->type](V, value);
	}
}

//------------------------------------------------------------------------------
void wr_countOfArrayElement( WRValue* array, WRValue* target )
{
#ifdef WRENCH_COMPACT
	if ( array->type == WR_REF )
	{
		return wr_countOfArrayElement( array->r, target );
	}
	else
#endif
	if ( IS_EXARRAY_TYPE(array->xtype) )
	{
		if ( IS_REFARRAY(array->xtype) )
		{
			wr_arrayToValue( array, target );
			wr_countOfArrayElement( target, target );
		}
		else
		{
			target->i = array->va->m_size;
			target->p2 = INIT_AS_INT;
		}
	}
}

//------------------------------------------------------------------------------
// put from into TO as a hash table, if to is not a hash table, make it
// one, leave the result in target
void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table )
{
	if ( value->type == WR_REF )
	{
		value = value->r;
	}
	if ( table->type == WR_REF )
	{
		table = table->r;
	}

	if ( table->xtype != WR_EX_HASH_TABLE )
	{
		c->gc( value + 1 );
		
		table->p2 = INIT_AS_HASH_TABLE;
		table->va = c->getSVA( 0, SV_HASH_TABLE, false );
	}

	WRValue *entry = (WRValue *)table->va->get( index->getHash() );

	*entry++ = *value;
	*entry = index->type == WR_REF ? *index->r : *index;
}

//------------------------------------------------------------------------------
static void doIndexHash_X( WRValue* value, WRValue* target, uint32_t hash ) { target->init(); }
static void doIndexHash_R( WRValue* value, WRValue* target, uint32_t hash ) { wr_IndexHash[ value->r->type ]( value->r, target, hash ); }
static void doIndexHash_E( WRValue* value, WRValue* target, uint32_t hash )
{
	if (IS_REFARRAY(value->xtype) )
	{
		if ( !IS_RAW_ARRAY(value->r->xtype)
			 && (value->r->va->m_type == SV_VALUE) )
		{
			unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);
			if ( s >= value->r->va->m_size )
			{
				value->r->va = growValueArray( value->r->va, s );
			}

			WRValue* val = value->r->va->m_Vdata + s;
			wr_IndexHash[ val->type ]( val, target, hash );
		}
	}
	else if ( value->xtype == WR_EX_STRUCT )
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( (uint32_t)READ_32_FROM_PC(table) == hash )
		{
			target->p2 = INIT_AS_REF;
			target->p = ((WRValue*)(value->va->m_data)) + *(table + 4);
		}
		else
		{
			target->init();
		}
	}
	else if ( value->xtype == WR_EX_HASH_TABLE )
	{
		if ( IS_EX_SINGLE_CHAR_RAW_P2( (target->r = (WRValue*)value->va->get(hash))->p2) )
		{
			target->p2 = INIT_AS_REFARRAY;
			//ARRAY_ELEMENT_TO_P2( target->p2, 1 );
		}
		else
		{
			target->p2 = INIT_AS_REF;
		}
	}
}
WRIndexHashFunc wr_IndexHash[4] = 
{
	doIndexHash_X,  doIndexHash_X,  doIndexHash_R,  doIndexHash_E
};

//------------------------------------------------------------------------------
static bool doLogicalNot_X( WRValue* value ) { return value->i == 0; } // float also uses 0x0000000 as 'zero'
static bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
static bool doLogicalNot_E( WRValue* value )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		return wr_LogicalNot[ element.type ]( &element );
	}
	return false;
}
WRReturnSingleFunc wr_LogicalNot[4] = 
{
	doLogicalNot_X,  doLogicalNot_X,  doLogicalNot_R,  doLogicalNot_E
};


//------------------------------------------------------------------------------
static void doNegate_I( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = -value->i; }
static void doNegate_F( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = -value->f; }
static void doNegate_E( WRValue* value, WRValue* target )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2(value->p2);

		if ( IS_RAW_ARRAY(value->r->xtype) )
		{
			target->p2 = INIT_AS_INT;
			target->i = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(value->r->p2))) ? -value->r->c[s] : 0;
		}
		else if (value->r->va->m_type == SV_VALUE)
		{
			if ( s < value->r->va->m_size )
			{
				wr_negate[ (value->r->va->m_Vdata + s)->type ]( value->r->va->m_Vdata + s, target );
			}
			else
			{
				target->init();
			}
		}
		else if ( value->r->va->m_type == SV_CHAR )
		{
			target->p2 = INIT_AS_INT;
			target->i = (s < value->r->va->m_size) ? -value->r->va->m_Cdata[s] : 0;
		}
	}
}
static void doNegate_R( WRValue* value, WRValue* target )
{
	wr_negate[ value->r->type ]( value->r, target );
}

WRSingleTargetFunc wr_negate[4] = 
{
	doNegate_I,  doNegate_F,  doNegate_R,  doNegate_E
};

//------------------------------------------------------------------------------
static void doGetValue_E( WRValue** value )
{
	if ( IS_REFARRAY( (*value)->xtype) )
	{
		unsigned int s = ARRAY_ELEMENT_FROM_P2((*value)->p2);

		if ( IS_RAW_ARRAY((*value)->r->xtype) )
		{
			if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2((*value)->r->p2)) )
			{
				(*value)->p2 = INIT_AS_INT;
				(*value)->i = (*value)->r->c[s];
			}
		}
		else if ((*value)->r->va->m_type == SV_VALUE)
		{
			if ( s < (*value)->r->va->m_size )
			{
				*value = (*value)->r->va->m_Vdata + s;
				wr_getValue[ (*value)->type ]( value );
			}
		}
		else if ((*value)->r->va->m_type == SV_CHAR )
		{
			(*value)->p2 = INIT_AS_INT;
			(*value)->i = (s >= (*value)->r->va->m_size) ? 0 : (*value)->r->va->m_Cdata[s];
		}
	}
}
static void doGetValue_R( WRValue** value )
{
	wr_getValue[ (*value = (*value)->r)->type ]( value );
}
static void doGetValue_X( WRValue** value ) { }

WRGetValueFunc wr_getValue[4] = 
{
	doGetValue_X,  doGetValue_X,  doGetValue_R,  doGetValue_E
};

//------------------------------------------------------------------------------
static uint32_t doBitwiseNot_I( WRValue* value ) { return ~value->ui; }
static uint32_t doBitwiseNot_F( WRValue* value ) { return 0; }
static uint32_t doBitwiseNot_R( WRValue* value ) { return wr_bitwiseNot[ value->r->type ]( value->r ); }
static uint32_t doBitwiseNot_E( WRValue* value )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		return wr_bitwiseNot[ element.type ]( &element );
	}
	return 0;
}
WRUint32Call wr_bitwiseNot[4] = 
{
	doBitwiseNot_I,  doBitwiseNot_F,  doBitwiseNot_R,  doBitwiseNot_E
};

//------------------------------------------------------------------------------
void pushIterator_X( WRValue* on, WRValue* to ) { on->init(); }
void pushIterator_R( WRValue* on, WRValue* to ) { wr_pushIterator[on->r->type]( on->r, to ); }
void pushIterator_E( WRValue* on, WRValue* to )
{
	if ( on->xtype == WR_EX_ARRAY || on->xtype == WR_EX_HASH_TABLE )
	{
		to->va = on->va;
		to->p2 = INIT_AS_ITERATOR;
	}
}

WRVoidFunc wr_pushIterator[4] =
{
	pushIterator_X, pushIterator_X, pushIterator_R, pushIterator_E
};

//------------------------------------------------------------------------------
static void doAssign_X_E( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_assign[(WR_EX<<2)+element.type](to, &element);
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_X( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		wr_valueToArray( to, from );
	}
	else
	{
		*to = *from;
	}
}
static void doAssign_E_E( WRValue* to, WRValue* from )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_assign[(WR_EX<<2)+element.type](to, &element);
		return;
	}
	else if ( IS_REFARRAY(to->xtype)
			  && IS_EXARRAY_TYPE(to->r->xtype)
			  && (to->r->va->m_type == SV_VALUE) )
	{
		unsigned int index = ARRAY_ELEMENT_FROM_P2(to->p2);

		if ( index > to->r->va->m_size )
		{
			if ( to->r->va->m_skipGC )
			{
				return;
			}

			to->r->va = growValueArray( to->r->va, index );	
		}

		to->r->va->m_Vdata[index] = *from;
		return;
	}

	*to = *from;
}



static void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	c->gc( target );

	// all we know is the value is not an array, so make it one
	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target->p2, index->i );

	value->p2 = INIT_AS_ARRAY;
	value->va = c->getSVA( index->i+1, SV_VALUE, true );
}

static void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	if ( value->xtype == WR_EX_HASH_TABLE )
	{
		target->r = (WRValue*)value->va->get( index->ui );
		target->p2 = INIT_AS_REF;
		return;
	}
	else if ( IS_RAW_ARRAY(value->xtype) )
	{
		if ( index->ui >= (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(value->p2)) )
		{
			goto boundsFailed;
		}
	}
	else if ( !(value->xtype == WR_EX_ARRAY) )
	{
		c->gc( target );

		// nope, make it one of this size and return a ref
		value->p2 = INIT_AS_ARRAY;
		value->va = c->getSVA( index->ui+1, SV_VALUE, true );
	}
	else if ( index->ui >= value->va->m_size )
	{
		if ( value->va->m_skipGC )
		{
boundsFailed:
			target->init();
			return;
		}

		value->va = growValueArray( value->va, index->ui );
	}

	target->r = value;
	target->p2 = INIT_AS_REFARRAY;
	ARRAY_ELEMENT_TO_P2( target->p2, index->ui );
}
static void doIndex_E_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	// the specific case of a string indexing a hash table
	if ( value->xtype == WR_EX_HASH_TABLE )
	{
		target->p2 = INIT_AS_REF;
		target->r = (WRValue*)value->va->get( index->getHash() );
	}
}

//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================

#ifdef WRENCH_COMPACT


static void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
static void doAssign_R_X( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->type](to->r, from); }
static void doAssign_X_R( WRValue* to, WRValue* from ) { wr_assign[(to->type<<2)|from->r->type](to, from->r); }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_X_R,  doAssign_X_E,
	doAssign_R_X,  doAssign_R_X,  doAssign_R_R,  doAssign_R_X,
	doAssign_E_X,  doAssign_E_X,  doAssign_X_R,  doAssign_E_E,
};

static void doIndex_X_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->type<<2)|value->r->type](c, index, value->r, target); }
static void doIndex_R_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->type](c, index->r, value, target); }
static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }
static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_X_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_X,     doIndex_R_X,     doIndex_R_R,     doIndex_R_X, 
	doVoidIndexFunc, doVoidIndexFunc, doIndex_X_R,     doIndex_E_E,
};

extern bool CompareEQI( int a, int b );
extern bool CompareEQF( float a, float b );

//------------------------------------------------------------------------------
static void unaryPost_E( WRValue* value, WRValue* stack, int add )
{
	if ( IS_REFARRAY(value->xtype) )
	{
		WRValue element;
		wr_arrayToValue( value, &element );
		WRValue temp;
		m_unaryPost[ element.type ]( &element, &temp, add );
		wr_valueToArray( value, &element );
		*stack = temp;
	}
}
static void unaryPost_I( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_INT; stack->i = value->i; value->i += add; }
static void unaryPost_R( WRValue* value, WRValue* stack, int add ) { m_unaryPost[ value->r->type ]( value->r, stack, add ); }
static void unaryPost_F( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f; value->f += add; }
WRVoidPlusFunc m_unaryPost[4] = 
{
	unaryPost_I,  unaryPost_F,  unaryPost_R, unaryPost_E
};

static void FuncAssign_R_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	wr_FuncAssign[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
static void FuncAssign_E_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );

		wr_FuncAssign[(element.type<<2)|from->type]( &element, from, intCall, floatCall );

		wr_valueToArray( to, &element );
		*from = element;
	}
}
static void FuncAssign_E_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );

		wr_FuncAssign[(WR_EX<<2)|element.type]( to, &element, intCall, floatCall );
		wr_arrayToValue( to, from );
	}
}
static void FuncAssign_X_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_FuncAssign[(to->type<<2)|element.type](to, &element, intCall, floatCall);
		*from = *to;
	}
}
static void FuncAssign_E_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}

static void FuncAssign_R_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue temp = *from->r; wr_FuncAssign[(to->r->type<<2)|temp.type](to->r, &temp, intCall, floatCall); *from = *to->r;
}

static void FuncAssign_R_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); *from = *to->r;
}

static void FuncAssign_X_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); *from = *to;
}

static void FuncAssign_F_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->f = floatCall( to->f, from->f );
}

static void FuncAssign_I_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->i = intCall( to->i, from->i );
}

static void FuncAssign_I_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->p2 = INIT_AS_FLOAT;
	to->f = floatCall( (float)to->i, from->f );
}
static void FuncAssign_F_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
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
static void FuncBinary_E_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );
		wr_funcBinary[(element.type<<2)|from->type](&element, from, target, intCall, floatCall);
	}
}
static void FuncBinary_E_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )
	{
		WRValue element1;
		wr_arrayToValue( to, &element1 );
		WRValue element2;
		wr_arrayToValue( from, &element2 );
		wr_funcBinary[(element1.type<<2)|element2.type](&element1, &element2, target, intCall, floatCall );
	}
}
static void FuncBinary_X_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		wr_funcBinary[(to->type<<2)|element.type](to, &element, target, intCall, floatCall);
	}
}
static void FuncBinary_E_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(WR_EX<<2)|from->r->type](to, from->r, target, intCall, floatCall );
}

static void FuncBinary_R_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(to->r->type<<2)|WR_EX](to->r, from, target, intCall, floatCall );
}

static void FuncBinary_X_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->type<<2)+from->r->type](to, from->r, target, intCall, floatCall);
}

static void FuncBinary_R_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->type](to->r, from, target, intCall, floatCall);
}

static void FuncBinary_R_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target, intCall, floatCall);
}

static void FuncBinary_I_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_INT; target->i = intCall( to->i, from->i );
}

static void FuncBinary_I_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( (float)to->i, from->f );
}

static void FuncBinary_F_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( to->f, (float)from->i );
}

static void FuncBinary_F_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
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

static bool Compare_E_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )
	{
		WRValue element1;
		wr_arrayToValue( to, &element1 );
		WRValue element2;
		wr_arrayToValue( from, &element2 );
		return wr_Compare[(element1.type<<2)|element2.type](&element1, &element2, intCall, floatCall);
	}
	return intCall(0,0) && to->getHash() == from->getHash();
}
static bool Compare_E_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(to->xtype) )
	{
		WRValue element;
		wr_arrayToValue( to, &element );
		return wr_Compare[(element.type<<2)|from->type](&element, from, intCall, floatCall);
	}
	return false;
}
static bool Compare_X_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	if ( IS_REFARRAY(from->xtype) )
	{
		WRValue element;
		wr_arrayToValue( from, &element );
		return wr_Compare[(to->type<<2)|element.type](to, &element, intCall, floatCall );
	}
	return false;
}
static bool Compare_R_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	return wr_Compare[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
static bool Compare_E_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	return wr_Compare[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}
static bool Compare_R_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->r->type](to->r, from->r, intCall, floatCall); }
static bool Compare_R_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); }
static bool Compare_X_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); }
static bool Compare_I_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return intCall( to->i, from->i ); }
static bool Compare_I_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( (float)to->i, from->f); }
static bool Compare_F_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, (float)from->i); }
static bool Compare_F_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, from->f); }
WRBoolCallbackReturnFunc wr_Compare[16] = 
{
	Compare_I_I, Compare_I_F, Compare_X_R, Compare_X_E,
	Compare_F_I, Compare_F_F, Compare_X_R, Compare_X_E,
	Compare_R_X, Compare_R_X, Compare_R_R, Compare_R_E,
	Compare_E_X, Compare_E_X, Compare_E_R, Compare_E_E,
};

//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================
#else 

// NOT COMPACT

static void doAssign_R_E( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_EX](to->r, from); }
static void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
static void doAssign_E_R( WRValue* to, WRValue* from ) { wr_assign[(WR_EX<<2)|from->r->type](to, from->r); }
static void doAssign_R_I( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_INT](to->r, from); }
static void doAssign_R_F( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_FLOAT](to->r, from); }
static void doAssign_I_R( WRValue* to, WRValue* from ) { wr_assign[(WR_INT<<2)|from->r->type](to, from->r); }
static void doAssign_F_R( WRValue* to, WRValue* from ) { wr_assign[(WR_FLOAT<<2)|from->r->type](to, from->r); }
static void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_I_R,  doAssign_X_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_F_R,  doAssign_X_E,
	doAssign_R_I,  doAssign_R_F,  doAssign_R_R,  doAssign_R_E,
	doAssign_E_X,  doAssign_E_X,  doAssign_E_R,  doAssign_E_E,
};

static void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_INT<<2)|value->r->type](c, index, value->r, target); }
static void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_INT](c, index->r, value, target); }
static void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target); }
static void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_FLOAT](c, index->r, value, target); }
static void doIndex_R_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_EX](c, index->r, value, target); }
static void doIndex_E_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_EX<<2)|value->r->type](c, index, value->r, target); }

static void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_I_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_I,     doIndex_R_F,     doIndex_R_R,     doIndex_R_E, 
	doVoidIndexFunc, doVoidIndexFunc, doIndex_E_R,     doIndex_E_E,
};

static void doVoidFuncBlank( WRValue* to, WRValue* from ) {}

#define X_INT_ASSIGN( NAME, OPERATION ) \
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)+temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		\
		NAME##Assign[(WR_EX<<2)+element.type]( to, &element );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_INT<<2)+element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
static void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
static void NAME##Assign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_INT<<2)+temp.type](to, &temp); *from = *to; }\
static void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,   doVoidFuncBlank,   NAME##Assign_I_R,  NAME##Assign_I_E,\
	doVoidFuncBlank,    doVoidFuncBlank,    doVoidFuncBlank,   doVoidFuncBlank,\
	NAME##Assign_R_I,   doVoidFuncBlank,   NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,   doVoidFuncBlank,   NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_INT_ASSIGN( wr_Mod, % );
X_INT_ASSIGN( wr_OR, | );
X_INT_ASSIGN( wr_AND, & );
X_INT_ASSIGN( wr_XOR, ^ );
X_INT_ASSIGN( wr_RightShift, >> );
X_INT_ASSIGN( wr_LeftShift, << );


#define X_ASSIGN( NAME, OPERATION ) \
static void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(to->r->type<<2)|element.type](to->r, &element);\
		*from = *to->r;\
	}\
}\
static void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_INT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_F( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		\
		NAME##Assign[(element.type<<2)|WR_FLOAT]( &element, from );\
		\
		wr_valueToArray( to, &element );\
		*from = element;\
	}\
}\
static void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		\
		NAME##Assign[(WR_EX<<2)|element.type]( to, &element );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_INT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_F_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Assign[(WR_FLOAT<<2)|element.type](to, &element);\
		*from = *to;\
	}\
}\
static void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue temp = *from->r;\
		NAME##Assign[(WR_EX<<2)|temp.type]( to, &temp );\
		wr_arrayToValue( to, from );\
	}\
}\
static void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
static void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
static void NAME##Assign_R_F( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }\
static void NAME##Assign_I_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }\
static void NAME##Assign_F_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }\
static void NAME##Assign_F_F( WRValue* to, WRValue* from ) { to->f OPERATION##= from->f; }\
static void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
static void NAME##Assign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i OPERATION from->f; }\
static void NAME##Assign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f OPERATION##= (float)from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,  NAME##Assign_I_F,  NAME##Assign_I_R,  NAME##Assign_I_E,\
	NAME##Assign_F_I,  NAME##Assign_F_F,  NAME##Assign_F_R,  NAME##Assign_F_E,\
	NAME##Assign_R_I,  NAME##Assign_R_F,  NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,  NAME##Assign_E_F,  NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_ASSIGN( wr_Subtract, - );
X_ASSIGN( wr_Add, + );
X_ASSIGN( wr_Multiply, * );
X_ASSIGN( wr_Divide, / );

//------------------------------------------------------------------------------
#define X_BINARY( NAME, OPERATION ) \
static void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->r->type](&element, from->r, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_F( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_FLOAT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		NAME##Binary[(element1.type<<2)|element2.type](&element1, &element2, target);\
	}\
}\
static void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_INT<<2)|element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_F_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_FLOAT<<2)|element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_R_F( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }\
static void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
static void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
static void NAME##Binary_F_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
static void NAME##Binary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i OPERATION from->f; }\
static void NAME##Binary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION (float)from->i; }\
static void NAME##Binary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION from->f; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I,  NAME##Binary_I_F,  NAME##Binary_I_R,  NAME##Binary_I_E,\
	NAME##Binary_F_I,  NAME##Binary_F_F,  NAME##Binary_F_R,  NAME##Binary_F_E,\
	NAME##Binary_R_I,  NAME##Binary_R_F,  NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I,  NAME##Binary_E_F,  NAME##Binary_E_R,  NAME##Binary_E_E,\
};\

X_BINARY( wr_Addition, + );
X_BINARY( wr_Multiply, * );
X_BINARY( wr_Subtract, - );
X_BINARY( wr_Divide, / );


static void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}

//------------------------------------------------------------------------------
#define X_INT_BINARY( NAME, OPERATION ) \
static void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|from->r->type](&element, from->r, target);\
	}\
}\
static void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(to->r->type<<2)|element.type]( to->r, &element, target);\
	}\
}\
static void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		NAME##Binary[(element.type<<2)|WR_INT](&element, from, target);\
	}\
}\
static void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		NAME##Binary[(element1.type<<2)|element2.type](&element1, &element2, target);\
	}\
}\
static void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		NAME##Binary[(WR_INT<<2)+element.type](to, &element, target);\
	}\
}\
static void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
static void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
static void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
static void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I, doTargetFuncBlank,   NAME##Binary_I_R,  NAME##Binary_I_E,\
   doTargetFuncBlank, doTargetFuncBlank,  doTargetFuncBlank, doTargetFuncBlank,\
	NAME##Binary_R_I, doTargetFuncBlank,   NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I, doTargetFuncBlank,   NAME##Binary_E_R,  NAME##Binary_E_E,\
};\


X_INT_BINARY( wr_LeftShift, << );
X_INT_BINARY( wr_RightShift, >> );
X_INT_BINARY( wr_Mod, % );
X_INT_BINARY( wr_AND, & );
X_INT_BINARY( wr_OR, | );
X_INT_BINARY( wr_XOR, ^ );


#define X_COMPARE( NAME, OPERATION ) \
static bool NAME##_E_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) && IS_REFARRAY(from->xtype) )\
	{\
		WRValue element1;\
		wr_arrayToValue( to, &element1 );\
		WRValue element2;\
		wr_arrayToValue( from, &element2 );\
		return NAME[(element1.type<<2)|element2.type](&element1, &element2);\
	}\
	return (0 OPERATION 0) && to->getHash() == from->getHash(); \
}\
static bool NAME##_R_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(to->r->type<<2)|element.type](to->r, &element);\
	}\
	return (0 OPERATION 0) && to->r->getHash() == from->getHash(); \
}\
static bool NAME##_E_R( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|from->r->type](&element, from->r);\
	}\
	return (0 OPERATION 0) && to->getHash() == from->r->getHash(); \
}\
static bool NAME##_E_I( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_INT](&element, from);\
	}\
	return false;\
}\
static bool NAME##_E_F( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(to->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( to, &element );\
		return NAME[(element.type<<2)|WR_FLOAT](&element, from);\
	}\
	return false;\
}\
static bool NAME##_I_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(WR_INT<<2)|element.type](to, &element);\
	}\
	return false;\
}\
static bool NAME##_F_E( WRValue* to, WRValue* from )\
{\
	if ( IS_REFARRAY(from->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( from, &element );\
		return NAME[(WR_FLOAT<<2)|element.type](to, &element);\
	}\
	return false;\
}\
static bool NAME##_R_R( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|from->r->type](to->r, from->r); }\
static bool NAME##_R_I( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_INT](to->r, from); }\
static bool NAME##_R_F( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_FLOAT](to->r, from); }\
static bool NAME##_I_R( WRValue* to, WRValue* from ) { return NAME[(WR_INT<<2)+from->r->type](to, from->r); }\
static bool NAME##_F_R( WRValue* to, WRValue* from ) { return NAME[(WR_FLOAT<<2)+from->r->type](to, from->r); }\
static bool NAME##_I_I( WRValue* to, WRValue* from ) { return to->i OPERATION from->i; }\
static bool NAME##_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->f OPERATION from->f; }\
static bool NAME##_F_I( WRValue* to, WRValue* from ) { return to->f OPERATION (float)from->i; }\
static bool NAME##_F_F( WRValue* to, WRValue* from ) { return to->f OPERATION from->f; }\
WRReturnFunc NAME[16] = \
{\
	NAME##_I_I, NAME##_I_F, NAME##_I_R, NAME##_I_E,\
	NAME##_F_I, NAME##_F_F, NAME##_F_R, NAME##_F_E,\
	NAME##_R_I, NAME##_R_F, NAME##_R_R, NAME##_R_E,\
	NAME##_E_I, NAME##_E_F, NAME##_E_R, NAME##_E_E,\
};\

X_COMPARE( wr_CompareGT, > );
X_COMPARE( wr_CompareLT, < );
X_COMPARE( wr_LogicalAND, && );
X_COMPARE( wr_LogicalOR, || );
X_COMPARE( wr_CompareEQ, == );

//------------------------------------------------------------------------------
#define X_UNARY_PRE( NAME, OPERATION ) \
static void NAME##_E( WRValue* value )\
{\
	if ( IS_REFARRAY(value->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( value, &element );\
		NAME [ element.type ]( &element );\
		wr_valueToArray( value, &element );\
	}\
}\
static void NAME##_I( WRValue* value ) { OPERATION value->i; }\
static void NAME##_F( WRValue* value ) { OPERATION value->f; }\
static void NAME##_R( WRValue* value ) { NAME [ value->r->type ]( value->r ); *value = *value->r; }\
WRUnaryFunc NAME[4] = \
{\
	NAME##_I, NAME##_F, NAME##_R, NAME##_E\
};\

X_UNARY_PRE( wr_preinc, ++ );
X_UNARY_PRE( wr_predec, -- );

//------------------------------------------------------------------------------
#define X_UNARY_POST( NAME, OPERATION ) \
static void NAME##_E( WRValue* value, WRValue* stack )\
{\
	if ( IS_REFARRAY(value->xtype) )\
	{\
		WRValue element;\
		wr_arrayToValue( value, &element );\
		WRValue temp; \
		NAME [ element.type ]( &element, &temp );\
		wr_valueToArray( value, &element );\
		*stack = temp; \
	}\
}\
static void NAME##_I( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_INT; stack->i = value->i OPERATION; }\
static void NAME##_R( WRValue* value, WRValue* stack ) { NAME[ value->r->type ]( value->r, stack ); }\
static void NAME##_F( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f OPERATION; }\
WRVoidFunc NAME[4] = \
{\
	NAME##_I,  NAME##_F,  NAME##_R, NAME##_E\
};\

X_UNARY_POST( wr_postinc, ++ );
X_UNARY_POST( wr_postdec, -- );

#endif
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

// standard functions that sort of come up a lot

int32_t wr_Seed;

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len )
{
	// fnv-1
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
int wr_itoa( int i, char* string, size_t len )
{
	char buf[12];
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
int wr_ftoa( float f, char* string, size_t len )
{
	size_t pos = 0;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		string[pos++] = '-';
	}

	if ( pos > len )
	{
		string[0] = 0;
		return 0;
	}

	f += 5.f / (float)10e6; // round value to 5 places

	int i = (int)f;

	if ( i )
	{
		f -= i;
		pos += wr_itoa( i, string + pos, len - pos );
	}
	else
	{
		string[pos++] = '0';
	}

	string[pos++] = '.';

	for( int p=0; pos < len && p<5; ++p ) // convert non-integer to 5 digits of precision
	{
		f *= 10.0;
		char c = (char)f;
		string[pos++] = '0' + c;
		f -= c;
	}

	for( --pos; pos > 0 && string[pos] == '0' && string[pos] != '.' ; --pos ); // knock off trailing zeros and decimal if appropriate

	if ( string[pos] != '.' )
	{
		++pos;
	}

	string[pos] = 0;
	return pos;
}

//------------------------------------------------------------------------------
void wr_std_rand( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->p2 = INIT_AS_INT;

		int32_t	k = wr_Seed / 127773;
		wr_Seed = 16807 * ( wr_Seed - k * 127773 ) - 2836 * k;
		stackTop->i = (uint32_t)wr_Seed % (uint32_t)(stackTop - 1)->asInt();
	}
}

//------------------------------------------------------------------------------
void wr_std_srand( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		wr_Seed = (stackTop - 1)->asInt();
	}
}

//------------------------------------------------------------------------------
void wr_loadStdLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::rand", wr_std_rand );
	wr_registerLibraryFunction( w, "std::srand", wr_std_srand );
}

//------------------------------------------------------------------------------
void wr_loadAllLibs( WRState* w )
{
	wr_loadMathLib( w );
	wr_loadStdLib( w );
	wr_loadFileLib( w );
	wr_loadStringLib( w );
	wr_loadMessageLib( w );
}

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
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

#ifdef WRENCH_STD_FILE
	if ( argn == 1 )
	{
		WRValue* arg = stackTop - 1;
		const char* fileName = arg->c_str();

#ifdef _WIN32
		struct _stat sbuf;
		int ret = _stat( fileName, &sbuf );
#else
		struct stat sbuf;
		int ret = stat( fileName, &sbuf );
#endif

		if ( ret == 0 )
		{
			FILE *infil = fopen( fileName, "rb" );
			if ( infil )
			{
				stackTop->p2 = INIT_AS_ARRAY;
				stackTop->va = c->getSVA( (int)sbuf.st_size, SV_CHAR, false );
				if ( fread( stackTop->va->m_Cdata, sbuf.st_size, 1, infil ) != 1 )
				{
					stackTop->init();
					return;
				}
			}

			fclose( infil );
		}
	}
#endif
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
#ifdef WRENCH_STD_FILE
	if ( argn == 2 )
	{
		WRValue* arg1 = stackTop - 2;
		unsigned int len;
		char type;
		const char* data = (char*)((stackTop - 1)->array(&len, &type));
		if ( !data || type != SV_CHAR )
		{
			return;
		}
		
		const char* fileName = arg1->c_str();
		if ( !fileName )
		{
			return;
		}

		FILE *outfil = fopen( fileName, "wb" );
		if ( !outfil )
		{
			return;
		}

		stackTop->i = (int)fwrite( data, len, 1, outfil );
		fclose( outfil );
	}
#endif
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
#ifdef WRENCH_STD_FILE
	char buf[256];
	int pos = 0;
	for (;;)
	{
		int in = fgetc( stdin );

		if ( in == EOF || in == '\n' || in == '\r' || pos >= 256 )   //@review: this isn't right - need to check for cr and lf
		{ 
			stackTop->p2 = INIT_AS_ARRAY;
			stackTop->va = c->getSVA( pos, SV_CHAR, false );
			memcpy( stackTop->va->m_Cdata, buf, pos );
			break;
		}

		buf[pos++] = in;
	}
#endif
}

#ifdef WRENCH_STD_TIME
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
#endif

//------------------------------------------------------------------------------
void wr_clock( WRValue* stackTop, const int argn, WRContext* c )
{
#ifdef WRENCH_STD_TIME
	stackTop->i = (int)clock();
#endif
}

//------------------------------------------------------------------------------
void wr_milliseconds(WRValue* stackTop, const int argn, WRContext* c )
{
#ifdef WRENCH_STD_TIME
#ifdef _WIN32
	stackTop->ui = (uint32_t)GetTickCount();
#else
	struct timeval tv;
	gettimeofday( &tv, NULL );
	stackTop->ui = (uint32_t)((tv.tv_usec/1000) + (tv.tv_sec * 1000));
#endif
#endif
}

//------------------------------------------------------------------------------
void wr_loadFileLib( WRState* w )
{
	wr_registerLibraryFunction( w, "file::read", wr_read_file );
	wr_registerLibraryFunction( w, "file::write", wr_write_file );

	wr_registerLibraryFunction( w, "io::getline", wr_getline );

	wr_registerLibraryFunction( w, "time::clock", wr_clock );
	wr_registerLibraryFunction( w, "time::ms", wr_clock );
}

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

#include <string.h>
#include <ctype.h>

//------------------------------------------------------------------------------
int wr_strlenEx( WRValue* val )
{
	if ( val->type == WR_REF )
	{
		return wr_strlenEx( val->r );
	}

	return (val->xtype == WR_EX_ARRAY && val->va->m_type == SV_CHAR) ? val->va->m_size :  0;
}

//------------------------------------------------------------------------------
void wr_strlen( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->i = argn == 1 ? wr_strlenEx( stackTop - 1 ) : 0;
}

//------------------------------------------------------------------------------
int wr_sprintfEx( char* outbuf, const char* fmt, WRValue* args, const int argn )
{
	if ( !fmt )
	{
		return 0;
	}
	
	enum
	{
		zeroPad         = 1<<0,
		negativeJustify = 1<<1,
		secondPass      = 1<<2,
		negativeSign    = 1<<3,
		parsingSigned   = 1<<4,
		altRep			= 1<<5,
	};

	char* out = outbuf;
	int listPtr = 0;

resetState:

	char padChar = ' ';
	unsigned char columns = 0;
	char flags = 0;

	for(;;)
	{
		char c = *fmt;
		fmt++;

		if ( !c )
		{
			*out = 0;
			break;
		}

		if ( !(secondPass & flags) )
		{
			if ( c != '%' ) // literal
			{
				*out++ = c;
			}
			else // possibly % format specifier
			{
				flags |= secondPass;
			}
		}
		else if ( c >= '0' && c <= '9' ) // width
		{
			columns *= 10;
			columns += c - '0';
			if ( !columns ) // leading zero
			{
				flags |= zeroPad;
				padChar = '0';
			}
		}
		else if ( c == '#' )
		{
			flags |= altRep;
		}
		else if ( c == '-' ) // left-justify
		{
			flags |= negativeJustify;
		}
		else if ( c == 'c' ) // character
		{
			if ( listPtr < argn )
			{
				*out++ = (char)(args[listPtr++].asInt());
			}
			goto resetState;
		}
		else if ( c == '%' ) // literal %
		{
			*out++ = c;
			goto resetState;
		}
		else // string or integer
		{
			char buf[20]; // buffer for integer

			const char *ptr; // pointer to first char of integer

			if ( c == 's' ) // string
			{
				buf[0] = 0;
				if ( listPtr < argn )
				{
					ptr = args[listPtr].c_str();
					if ( !ptr )
					{
						return 0;
					}
					++listPtr;
				}
				else
				{
					ptr = buf;
				}

				padChar = ' '; // in case some joker uses a 0 in their column spec

copyToString:

				// get the string length so it can be formatted, don't
				// copy it, just count it
				unsigned char len = 0;
				for ( ; *ptr; ptr++ )
				{
					len++;
				}
				
				ptr -= len;
				
				// Right-justify
				if ( !(flags & negativeJustify) )
				{
					for ( ; columns > len; columns-- )
					{
						*out++ = padChar;
					}
				}
				
				if ( flags & negativeSign )
				{
					*out++ = '-';
				}

				if ( flags & altRep || c == 'p' )
				{
					*out++ = '0';
					*out++ = 'x';
				}

				for (unsigned char l = 0; l < len; ++l )
				{
					*out++ = *ptr++;
				}

				// Left-justify
				for ( ; columns > len; columns-- )
				{
					*out++ = ' ';
				}

				goto resetState;
			}
			else
			{
				unsigned char base;
				unsigned char width;
				unsigned int val;

				if ( c == 'd' || c == 'i' )
				{
					flags |= parsingSigned;
					goto parseDecimal;
				}
				else if ( c == 'u' ) // decimal
				{
parseDecimal:
					base  = 10;
					width = 10;
					goto convertBase;
				}
				else if ( c == 'b' ) // binary
				{
					base  = 2;
					width = 16;
					goto convertBase;
				}
				else if ( c == 'o' ) // octal
				{
					base  = 8;
					width = 5;
					goto convertBase;
				}
				else if ( c == 'x' || c == 'X' || c == 'p' ) // hexadecimal or pointer (pointer is treated as 'X')
				{
					base = 16;
					width = 8;
convertBase:
					if ( listPtr < argn )
					{
						val = args[listPtr++].asInt();
					}
					else
					{
						val = 0;
					}

					if ( (flags & parsingSigned) && (val & 0x80000000) )
					{
						flags |= negativeSign;
						val = -(int)val;
					}

					// Convert to given base, filling buffer backwards from least to most significant
					char* p = buf + width;
					*p = 0;
					ptr = p; // keep track of one past left-most non-zero digit
					do
					{
						char d = val % base;
						val /= base;

						if ( d )
						{
							ptr = p;
						}

						d += '0';
						if ( d > '9' ) // handle bases higher than 10
						{
							d += 'A' - ('9' + 1);
							if ( c == 'x' ) // lowercase
							{
								d += 'a' - 'A';
							}
						}

						*--p = d;

					} while ( p != buf );

					ptr--; // was one past char we want

					goto copyToString;
				}
				else // invalid format specifier
				{
					goto resetState;
				}
			}
		}
	}

	return out - outbuf;
}

//------------------------------------------------------------------------------
void wr_format( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if( argn >= 1 )
	{
		WRValue* args = stackTop - argn;
		char outbuf[512];
		
		int size = wr_sprintfEx( outbuf, args[0].c_str(), args + 1, argn - 1 );

		stackTop->p2 = INIT_AS_ARRAY;
		stackTop->va = c->getSVA( size, SV_CHAR, false );
		memcpy( stackTop->va->m_Cdata, outbuf, size );
	}
}

//------------------------------------------------------------------------------
void wr_printf( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

#ifdef WRENCH_STD_FILE
	if( argn >= 1 )
	{
		WRValue* args = stackTop - argn;

		char outbuf[512];
		stackTop->i = wr_sprintfEx( outbuf, args[0].c_str(), args + 1, argn - 1 );
		
		printf( "%s", outbuf );
	}
#endif
}

//------------------------------------------------------------------------------
void wr_sprintf( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	WRValue* args = stackTop - argn;
	
	if ( argn < 2
		 || args[0].type != WR_REF
		 || args[1].xtype != WR_EX_ARRAY
		 || args[1].va->m_type != SV_CHAR )
	{
		return;
	}

	char outbuf[512];
	stackTop->i = wr_sprintfEx( outbuf, args[1].c_str(), args + 2, argn - 2 );

	args[0].r->p2 = INIT_AS_ARRAY;
	args[0].r->va = c->getSVA( stackTop->i, SV_CHAR, false );
	memcpy( args[0].r->va->m_Cdata, outbuf, stackTop->i );
}

//------------------------------------------------------------------------------
void wr_isspace( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)isspace( (char)(stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_isalpha( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)isalpha( (char)(stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_isdigit( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)isdigit( (char)(stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_isalnum( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)isalnum( (char)(stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_mid( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	unsigned int len;
	const char* data = args[0].c_str( &len );

	if( !data || len <= 0 )
	{
		return;
	}
	
	unsigned int start = args[1].asInt();
	stackTop->p2 = INIT_AS_ARRAY;

	unsigned int chars = 0;
	if ( start < len )
	{
		chars = (argn > 2) ? args[2].asInt() : len;
		if ( chars > (len - start) )
		{
			chars = len - start;
		}
	}
	
	stackTop->va = c->getSVA( chars, SV_CHAR, false );
	memcpy( stackTop->va->m_Cdata, data + start, chars );
}

//------------------------------------------------------------------------------
void wr_strchr( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->i = -1;

	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	const char* str = args[0].c_str();
	if ( !str )
	{
		return;
	}
	
	char ch = (char)args[1].asInt();
	const char* found = strchr( str, ch );
	if ( found )
	{
		stackTop->i = found - str;
	}
}

//------------------------------------------------------------------------------
void wr_tolower( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)tolower( (stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_toupper( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		stackTop->i = (int)toupper( (stackTop - 1)->asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_tol( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 2 )
	{
		const char* str = stackTop[-2].c_str();
		if ( str )
		{
			stackTop->i = (int)strtol( str, 0, stackTop[-1].asInt() );
		}
	}
	else if ( argn == 1 )
	{
		const char* str = stackTop[-1].c_str();
		if ( str )
		{
			stackTop->i = (int)strtol( str, 0, 10 );
		}
	}
}

//------------------------------------------------------------------------------
void wr_loadStringLib( WRState* w )
{
	wr_registerLibraryFunction( w, "str::strlen", wr_strlen );
	wr_registerLibraryFunction( w, "str::sprintf", wr_sprintf );
	wr_registerLibraryFunction( w, "str::printf", wr_printf );
	wr_registerLibraryFunction( w, "str::format", wr_format );
	wr_registerLibraryFunction( w, "str::isspace", wr_isspace );
	wr_registerLibraryFunction( w, "str::isdigit", wr_isdigit );
	wr_registerLibraryFunction( w, "str::isalpha", wr_isalpha );
	wr_registerLibraryFunction( w, "str::mid", wr_mid );
	wr_registerLibraryFunction( w, "str::chr", wr_strchr );
	wr_registerLibraryFunction( w, "str::tolower", wr_tolower );
	wr_registerLibraryFunction( w, "str::toupper", wr_toupper );
	wr_registerLibraryFunction( w, "str::tol", wr_tol );
}
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

#include <math.h>

//------------------------------------------------------------------------------
void wr_math_sin( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = sinf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_cos( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = cosf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_tan( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = tanf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_sinh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = sinhf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_cosh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = coshf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_tanh( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = tanhf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_asin( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = asinf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_acos( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = acosf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_atan( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = atanf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_atan2( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
		stackTop->f = atan2f( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_log( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = logf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_log10( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = log10f( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_exp( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = expf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_sqrt( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = sqrtf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_ceil( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = ceilf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_floor( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = floorf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_abs( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = (float)fabs( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_pow( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
		stackTop->f = powf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_fmod( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
		stackTop->f = fmodf( (stackTop - 1)->asFloat(), (stackTop - 2)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_trunc( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = truncf( (stackTop - 1)->asFloat() );
	}
}

//------------------------------------------------------------------------------
void wr_math_ldexp( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 2 )
	{
		stackTop->f = ldexpf( (stackTop - 1)->asFloat(), (stackTop - 2)->asInt() );
	}
}

//------------------------------------------------------------------------------
const float wr_PI = 3.14159265358979323846f;
const float wr_toDegrees = (180.f / wr_PI);
const float wr_toRadians = (1.f / wr_toDegrees);

//------------------------------------------------------------------------------
void wr_math_rad2deg( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = wr_toDegrees * (stackTop - 1)->asFloat();
	}
}

//------------------------------------------------------------------------------
void wr_math_deg2rad( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_FLOAT;
	if ( argn == 1 )
	{
		stackTop->f = wr_toRadians * (stackTop - 1)->asFloat();
	}
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
// key   : anything hashable
// clear : [OPTIONAL] default 0/'false' 
//
// returns : value if it was there
void wr_mboxRead( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		WRValue* args = stackTop - argn;
		int clear = (argn > 1) ? args[1].asInt() : 0;
		WRValue* msg = c->w->globalRegistry.exists( args[0].getHash() ^ HASH_SCRAMBLER, clear );

		if ( msg )
		{
			*stackTop = *msg;
			return;
		}
	}

	stackTop->init();
}

//------------------------------------------------------------------------------
// key  : anything hashable
// value: stores the "hash"
void wr_mboxWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		WRValue* msg = c->w->globalRegistry.getAsRawValueHashTable( args[0].getHash() ^ HASH_SCRAMBLER );
		msg->ui = args[1].getHash();
		msg->p2 = INIT_AS_INT;
	}
}

//------------------------------------------------------------------------------
// key  : anything hashable
void wr_mboxClear( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		c->w->globalRegistry.exists((stackTop - argn)->getHash() ^ HASH_SCRAMBLER, true);
	}
}

//------------------------------------------------------------------------------
// key  : anything hashable
//
// return : true/false
void wr_mboxPeek( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0
		 && c->w->globalRegistry.exists((stackTop - argn)->getHash() ^ HASH_SCRAMBLER, false) )
	{
		stackTop->i = 1;
	}
}

//------------------------------------------------------------------------------
void wr_loadMessageLib( WRState* w )
{
	wr_registerLibraryFunction( w, "msg::read", wr_mboxRead ); // read (true to clear, false to leave)
	wr_registerLibraryFunction( w, "msg::write", wr_mboxWrite ); // write message
	wr_registerLibraryFunction( w, "msg::clear", wr_mboxClear ); // remove if exists
	wr_registerLibraryFunction( w, "msg::peek", wr_mboxPeek ); // does the message exist?
}
