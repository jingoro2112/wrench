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
#ifndef _UTILS_H
#define _UTILS_H
/*------------------------------------------------------------------------------*/

int wr_itoa( int i, char* string, size_t len );
int wr_ftoa( float f, char* string, size_t len );

unsigned char* wr_pack16( int16_t i, unsigned char* buf );
unsigned char* wr_pack32( int32_t l, unsigned char* buf );

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

WRContext* wr_createContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership, WRValue* stack =0 );

#ifndef WRENCH_WITHOUT_COMPILER

#include <stdlib.h>
#include <new>

//-----------------------------------------------------------------------------
template <class T> class WRarray
{
private:

	T* m_list;
	unsigned int m_elementsNewed;
	unsigned int m_elementsAllocated;

public:

	//------------------------------------------------------------------------------
	T* newArray( int size )
	{
		if ( !size ) return 0;
		
		T* t = (T*)g_malloc( sizeof(T) * size );
		for( int i=0; i<size; ++i )
		{
			new (&(t[i])) T();
		}
		
		return t;
	}

	//------------------------------------------------------------------------------
	void deleteArray( T* t, int size )
	{
		if ( !t ) return;
		
		for( int i=0; i<size; ++i )
		{
			t[i].~T();
		}

		g_free( t );
	}

	//------------------------------------------------------------------------------
	void clear()
	{
		deleteArray( m_list, m_elementsNewed );
		m_list = 0;
		m_elementsNewed = 0;
		m_elementsAllocated = 0;
	}
	
	unsigned int count() const { return m_elementsAllocated; }
	void setCount( unsigned int count )
	{
		if ( count >= m_elementsNewed )
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

		T* na = newArray( newCount );

		if ( location == 0 )
		{
			for( unsigned int i=0; i<(unsigned int)newCount; ++i )
			{
				na[i] = m_list[i+count];
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
					na[j++] = m_list[i];
				}
				else if ( i >= skipEnd )
				{
					na[j++] = m_list[i];
				}
			}
		}

		deleteArray( m_list, m_elementsNewed );
		m_list = na;
		m_elementsAllocated = newCount;
		m_elementsNewed = newCount;

		return m_elementsAllocated;
	}

	//------------------------------------------------------------------------------
	void alloc( const unsigned int size )
	{
		if ( size > m_elementsNewed )
		{
			unsigned int newSize = size + (size/2) + 1;
			T* na = newArray( newSize );

			if ( m_list )
			{
				for( unsigned int i=0; i<m_elementsAllocated; ++i )
				{
					na[i] = m_list[i];
				}

				deleteArray( m_list, m_elementsNewed );
			}

			m_elementsNewed = newSize;
			m_list = na;
		}
	}

	//------------------------------------------------------------------------------
	T* push() { return &get( m_elementsAllocated ); }
	T* tail() { return m_elementsAllocated ? m_list + (m_elementsAllocated - 1) : 0; }
	void pop() { if ( m_elementsAllocated > 0 ) { --m_elementsAllocated; }  }

	T& append() { return get( m_elementsAllocated ); }
	T& get( const unsigned int l )
	{
		if ( l >= m_elementsNewed )
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
		m_list = newArray( A.m_elementsAllocated );
		m_elementsNewed = A.m_elementsNewed;
		for( unsigned int i=0; i<A.m_elementsAllocated; ++i )
		{
			m_list[i] = A.m_list[i];
		}
		m_elementsAllocated = A.m_elementsAllocated;
	}

	//------------------------------------------------------------------------------
	WRarray& operator= ( const WRarray& A )
	{
		if ( &A != this )
		{
			clear();
			m_list = newArray( A.m_elementsNewed );
			for( unsigned int i=0; i<A.m_elementsAllocated; ++i )
			{
				m_list[i] = A.m_list[i];
			}
			m_elementsAllocated = A.m_elementsAllocated;
			m_elementsNewed = A.m_elementsNewed;
		}
		return *this;
	}

	const T& operator[]( const unsigned int l ) const { return get(l); }
	T& operator[]( const unsigned int l ) { return get(l); }

	WRarray( const unsigned int initialSize =0 ) { m_list = 0; clear(); alloc(initialSize); }
	~WRarray() { clear(); }
};
class WRstr;

//------------------------------------------------------------------------------
template <class T> class WRHashTable
{
public:
	WRHashTable( unsigned int sizeHint =0 )
	{
		for( uint16_t i=0; c_primeTable[i]; ++i )
		{
			if ( sizeHint < c_primeTable[i] )
			{
				m_mod = (int)c_primeTable[i];
				m_list = newArray( m_mod );
				break;
			}
		}
	}
	~WRHashTable() { deleteArray( m_list, m_mod ); }

	struct Node
	{
		T value;
		uint32_t hash;
	};

	//------------------------------------------------------------------------------
	Node* newArray( int size )
	{
		if ( !size ) return 0;

		Node* n = (Node*)g_malloc( sizeof(Node) * size );
		for( int i=0; i<size; ++i )
		{
			new (&(n[i].value)) T();
			n[i].hash = WRENCH_NULL_HASH;
		}

		return n;
	}

	//------------------------------------------------------------------------------
	void deleteArray( Node* n, int size )
	{
		if ( !n ) return;

		for( int i=0; i<size; ++i )
		{
			n[i].value.~T();
		}

		g_free( n );
	}

	//------------------------------------------------------------------------------
	void clear()
	{
		deleteArray( m_list, m_mod );
		m_mod = (int)c_primeTable[0];
		m_list = newArray( m_mod );
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
			m_list[key].hash = WRENCH_NULL_HASH;
		}
	}

	//------------------------------------------------------------------------------
	bool set( uint32_t hash, T const& value )
	{
		uint32_t key = hash % m_mod;
		// clobber on collide, assume the user knows what they are doing
		if ( (m_list[key].hash == WRENCH_NULL_HASH) || (m_list[key].hash == hash) )
		{
			m_list[key].hash = hash;
			m_list[key].value = value;
			return true;
		}

		// otherwise there was a collision, expand the table
		unsigned int newMod = m_mod;
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
			Node* newList = newArray( newMod );

			int h = 0;
			for( ; h<m_mod; ++h )
			{
				if ( m_list[h].hash == WRENCH_NULL_HASH )
				{
					continue;
				}

				if ( newList[m_list[h].hash % newMod].hash != WRENCH_NULL_HASH )
				{
					break;
				}
				
				newList[m_list[h].hash % newMod] = m_list[h];
			}

			if ( h >= m_mod )
			{
				deleteArray( m_list, m_mod );
				m_mod = newMod;
				m_list = newList;
				return set( hash, value );
			}

			deleteArray( newList, newMod ); // try again
		}
	}

	friend struct WRCompilationContext;

private:
	
	int m_mod;
	Node* m_list;
};

#endif

void wr_growValueArray( WRGCObject* va, int newSize );

#define IS_SVA_VALUE_TYPE(V) ((V)->m_type & 0x1)

#define INIT_AS_LIB_CONST    0xFFFFFFFC
#define INIT_AS_DEBUG_BREAK  (((uint32_t)WR_EX) | ((uint32_t)WR_EX_DEBUG_BREAK<<24))
#define INIT_AS_ARRAY        (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ARRAY<<24))
#define INIT_AS_USR          (((uint32_t)WR_EX) | ((uint32_t)WR_EX_USR<<24))
#define INIT_AS_RAW_ARRAY    (((uint32_t)WR_EX) | ((uint32_t)WR_EX_RAW_ARRAY<<24))
#define INIT_AS_ARRAY_MEMBER (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ARRAY_MEMBER<<24))
#define INIT_AS_STRUCT       (((uint32_t)WR_EX) | ((uint32_t)WR_EX_STRUCT<<24))
#define INIT_AS_HASH_TABLE   (((uint32_t)WR_EX) | ((uint32_t)WR_EX_HASH_TABLE<<24))
#define INIT_AS_ITERATOR     (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ITERATOR<<24))

#define INIT_AS_REF      WR_REF
#define INIT_AS_INT      WR_INT
#define INIT_AS_FLOAT    WR_FLOAT

#define ENCODE_ARRAY_ELEMENT_TO_P2(E) ((E)<<8)
#define DECODE_ARRAY_ELEMENT_FROM_P2(E) (((E)&0x1FFFFF00) >> 8)

#define IS_EXARRAY_TYPE(P)   ((P)&0xC0)
#define IS_EX_RAW_ARRAY_TYPE(P)   (((P)&0xE0) == WR_EX_RAW_ARRAY)
#define EX_RAW_ARRAY_SIZE_FROM_P2(P) (((P)&0x1FFFFF00) >> 8)
#define IS_EX_SINGLE_CHAR_RAW_P2(P) ((P) == (((uint32_t)WR_EX) | (((uint32_t)WR_EX_RAW_ARRAY<<24)) | (1<<8)))

int wr_addI( int a, int b );

#endif
