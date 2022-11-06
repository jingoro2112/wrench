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

#define IS_REFARRAY(X) (((X)&0xE0)==WR_EX_REFARRAY)
#define IS_ITERATOR(X) (((X)&0xE0)==WR_EX_ITERATOR)

#endif
