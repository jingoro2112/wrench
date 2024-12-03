#include "wrench.h"
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
#define INIT_AS_CONTAINER_MEMBER (((uint32_t)WR_EX) | ((uint32_t)WR_EX_CONTAINER_MEMBER<<24))
#define INIT_AS_STRUCT       (((uint32_t)WR_EX) | ((uint32_t)WR_EX_STRUCT<<24))
#define INIT_AS_HASH_TABLE   (((uint32_t)WR_EX) | ((uint32_t)WR_EX_HASH_TABLE<<24))
#define INIT_AS_ITERATOR     (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ITERATOR<<24))
#define INIT_AS_INVALID      (((uint32_t)WR_EX) | ((uint32_t)WR_EX_INVALID<<24))

#define INIT_AS_REF      WR_REF
#define INIT_AS_INT      WR_INT
#define INIT_AS_FLOAT    WR_FLOAT

#define ENCODE_ARRAY_ELEMENT_TO_P2(E) ((E)<<8)
#define DECODE_ARRAY_ELEMENT_FROM_P2(E) (((E)&0x1FFFFF00) >> 8)

#define IS_EXARRAY_TYPE(P)   ((P)&0xC0)
#define IS_EX_RAW_ARRAY_TYPE(P)   (((P)&0xE0) == WR_EX_RAW_ARRAY)
#define EX_RAW_ARRAY_SIZE_FROM_P2(P) (((P)&0x1FFFFF00) >> 8)
#define IS_EX_SINGLE_CHAR_RAW_P2(P) ((P) == (((uint32_t)WR_EX) | (((uint32_t)WR_EX_RAW_ARRAY<<24)) | (1<<8)))
#define IS_INVALID(P) ((P) == INIT_AS_INVALID)

int wr_addI( int a, int b );

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
#ifndef _SERIALIZER_H
#define _SERIALIZER_H
/*------------------------------------------------------------------------------*/

struct WRValue;
struct WRContext;
class WRValueSerializer;

//------------------------------------------------------------------------------
bool wr_serializeEx( WRValueSerializer& serializer, const WRValue& val );
bool wr_deserializeEx( WRValue& value, WRValueSerializer& serializer, WRContext* context );

//------------------------------------------------------------------------------
class WRValueSerializer
{
public:
	
	WRValueSerializer() : m_pos(0), m_size(0), m_buf(0) {}
	WRValueSerializer( const char* data, const int size ) : m_pos(0), m_size(size)
	{
		m_buf = (char*)g_malloc(size);
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_buf )
		{
			m_size = 0;
			g_mallocFailed = true;
		}
		else
#endif
		{
			memcpy( m_buf, data, size );
		}
	}
	
	~WRValueSerializer() { g_free(m_buf); }

	void getOwnership( char** buf, int* len )
	{
		*buf = m_buf;
		*len = m_pos;
		m_buf = 0;
	}
	
	int size() const { return m_pos; }
	const char* data() const { return m_buf; }

	bool read( char* data, const int size )
	{
		if ( m_pos + size > m_size )
		{
			return false;
		}

		memcpy( data, m_buf + m_pos, size );
		m_pos += size;
		return true;
	}

	void write( const char* data, const int size )
	{
		if ( m_pos + size >= m_size )
		{
			m_size += (size*2) + 8;
			char* newBuf = (char*)g_malloc( m_size );
			memcpy( newBuf, m_buf, m_pos );
			g_free( m_buf );
			m_buf = newBuf;
		}

		memcpy( m_buf + m_pos, data, size );
		m_pos += size;
	}

private:

	int m_pos;
	int m_size;
	char* m_buf;
};

#endif

#ifndef _SIMPLE_LL_H
#define _SIMPLE_LL_H
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

// only used in debug code, but it is used on the target machine so keep it compact!

#ifdef WRENCH_INCLUDE_DEBUG_CODE

#include <stdlib.h>
#include <new>

//-----------------------------------------------------------------------------
template<class L> class SimpleLL
{
public:

	SimpleLL( void (*clearFunc)( L& item ) =0 ) : m_iter(0), m_head(0), m_tail(0), m_clearFunc(clearFunc) {}
	~SimpleLL() { clear(); }

	//------------------------------------------------------------------------------
	L* first() { return m_head ? &(m_iter = m_head)->item : 0; }
	L* next() { return m_iter ? &((m_iter = m_iter->next)->item) : 0; }
	L* head() { return first(); }
	L* tail() { return m_tail ? &(m_tail->item) : 0; }

	//------------------------------------------------------------------------------
	// brute-force
	uint32_t count()
	{
		uint32_t c=0;
		for( L* item = first(); item; item = next(), ++c );
		return c;
	}

	//------------------------------------------------------------------------------
	L* operator[]( const int i )
	{
		int cur = 0;
		for( Node* N = m_head; N ; N = N->next, ++cur )
		{
			if( cur >= i )
			{
				return &(N->item);
			}
		}

		return 0;
	}
	
	//------------------------------------------------------------------------------
	L* addHead()
	{
		if ( !m_head )
		{
			m_head = (Node*)g_malloc(sizeof(Node));
			m_head->next = 0;
			new (&(m_head->item)) L();
			
			m_tail = m_head;
		}
		else
		{
			Node* N = (Node*)g_malloc(sizeof(Node));
			N->next = m_head;
			new (&(N->item)) L();

			m_head = N;
		}
		return &(m_head->item);
	}
	
	//------------------------------------------------------------------------------
	L* addTail()
	{
		if ( !m_tail )
		{
			return addHead();
		}
		else
		{
			Node* N = (Node*)g_malloc(sizeof(Node));
			N->next = 0;
			new (&(N->item)) L();

			m_tail->next = N;
			m_tail = N;
			return &(m_tail->item);
		}
	}

	//------------------------------------------------------------------------------
	void popAt( const int index, L* item =0 )
	{
		if ( index == 0 )
		{
			return popHead( item );
		}

		Node* prev = 0;
		int c=0;
		for( Node* N = m_head; N; N = N->next, ++c )
		{
			if ( c == index )
			{
				if ( N == m_tail )
				{
					return popTail( item );
				}
				
				prev->next = N->next;
				if ( m_iter == N )
				{
					m_iter = N->next;
				}

				if ( item )
				{
					*item = N->item;
				}
				else if ( m_clearFunc )
				{
					m_clearFunc( N->item );
				}

				N->item.~L();
				g_free( N );
				
				break;
			}

			prev = N;
		}
	}

	//------------------------------------------------------------------------------
	void popItem( L* item )
	{
		if ( !m_head )
		{
			return;
		}
		else if ( &(m_head->item) == item )
		{
			popHead();
		}
		else if ( &(m_tail->item) == item )
		{
			popTail();
		}

		Node* prev = 0;
		for( Node* N = m_head; N; N = N->next )
		{
			if ( &(N->item) == item )
			{
				prev->next = N->next;
				if ( m_iter == N )
				{
					m_iter = N->next;
				}

				if ( m_clearFunc )
				{
					m_clearFunc( N->item );
				}

				N->item.~L();
				g_free( N );
				break;
			}

			prev = N;
		}
	}

	//------------------------------------------------------------------------------
	void popHead( L* item =0 )
	{
		if ( m_head )
		{
			if ( m_iter == m_head )
			{
				m_iter = m_head->next;
			}

			Node* N = m_head;
			if ( m_tail == N )
			{
				m_head = 0;
				m_tail = 0;
			}
			else
			{
				m_head = m_head->next;
			}

			if ( item )
			{
				*item = N->item;
			}
			else if ( m_clearFunc )
			{
				m_clearFunc( N->item );
			}
			N->item.~L();
			g_free( N );
		}
	}

	//------------------------------------------------------------------------------
	void popTail( L* item =0 )
	{
		if ( m_iter == m_tail )
		{
			m_iter = 0;
		}

		if ( m_head == m_tail )
		{
			popHead( item );
		}
		else
		{
			for( Node* N = m_head; N; N = N->next )
			{
				if ( N->next == m_tail )
				{
					if ( item )
					{
						*item = N->next->item;
					}
					else if ( m_clearFunc )
					{
						m_clearFunc( N->next->item );
					}

					N->next->item.~L();
					g_free( N->next );

					
					N->next = 0;
					m_tail = N;
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void clear() { while( m_head ) { popHead(); }  }

private:
	struct Node
	{
//		Node( Node* n=0 ) : next(n) {}
		L item;
		Node *next;
	};

	Node* m_iter;
	Node* m_head;
	Node* m_tail;
	void (*m_clearFunc)( L& item );

public:
	class Iterator
	{
	public:
		Iterator( SimpleLL<L> const& list ) { iterate(list); }
		Iterator() : m_list(0), m_current(0) {}
			
		bool operator!=( const Iterator& other ) { return m_current != other.m_current; }
		L& operator* () const { return m_current->item; }

		const Iterator& operator++() { if ( m_current ) m_current = m_current->next; return *this; }
		Iterator operator++(int) { Iterator ret = *this; this->operator++(); return ret; }
		void iterate( SimpleLL<L> const& list ) { m_list = &list; m_current = m_list->m_head; }

	private:
		SimpleLL<L> const* m_list;
		Node *m_current;
	};

	Iterator begin() const { return Iterator(*this); }
	Iterator end() const { return Iterator(); }
};


#ifdef TEST_LL
#include <assert.h>
#include <stdio.h>
inline void testLL()
{
	SimpleLL<int> list;

	*list.addHead() = 10;
	assert( list.count() == 1 );
	assert( *list.head() == 10 );
	list.popHead();
	assert( list.count() == 0 );

	*list.addHead() = 10;
	*list.addHead() = 5;
	*list.addTail() = 15;

	assert( *list.first() == 5 );
	assert( *list.next() == 10 );
	assert( *list.next() == 15 );
	assert( !list.next() );

	int i;
	list.popTail( &i );
	assert( i == 15 );

	assert( *list.first() == 5 );
	assert( *list.next() == 10 );
	assert( !list.next() );

	list.popHead( &i );
	assert( i == 5 );
	assert( *list.first() == 10 );
	assert( !list.next() );


	list.popTail();
	assert( !list.first() );


	*list.addHead() = 10;
	*list.addHead() = 5;
	*list.addTail() = 15;
	list.popAt( 1 );
	assert( *list.first() == 5 );
	assert( *list.next() == 15 );
	assert( !list.next() );
	list.popAt(0);
	list.popAt(0);

	*list.addHead() = 10;
	*list.addHead() = 5;
	*list.addTail() = 15;

	assert(*list.first() == 5);
	assert(*list.next() == 10);
	assert(*list.next() == 15);
	assert(!list.next());

	list.popAt( 0 );
	assert( *list.first() == 10 );
	assert( *list.next() == 15 );
	assert( !list.next() );
	list.popAt(0);
	list.popAt(0);

	*list.addHead() = 10;
	*list.addHead() = 5;
	*list.addTail() = 15;
	list.popAt( 2 );
	assert( *list.first() == 5 );
	assert( *list.next() == 10 );
	assert( !list.next() );
	list.popAt(0);
	list.popAt(0);
}
#endif

#endif

#endif
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
#ifndef SIMPLE_ARGS_H
#define SIMPLE_ARGS_H
//------------------------------------------------------------------------------

#include <string.h>

//------------------------------------------------------------------------------
namespace SimpleArgs
{

//   by index: 0,1,2... first, second, third  -1, -2, -3... last, second to last..
// returns pointer to argv if found
inline const char* get( int argn, char *argv[], int index, char* param =0, const int maxParamLen=0 );

//   by option
// returns true if found, opt is filled in with whatever follows it,
// so:
// returns pointer to argv found.
// IF param is specified, returns a pointer to param found
// if not found returns null
inline const char* get( int argn, char *argv[], const char* opt, char* param =0, const int maxParamLen=0 );

}

//------------------------------------------------------------------------------
inline const char* SimpleArgs::get( int argn, char *argv[], int index, char* param, const int maxParamLen )
{
	int a = index;
	if ( a < 0 )
	{
		a = argn + a;
	}

	if ( (unsigned int)a >= (unsigned int)argn )
	{
		return 0;
	}

	if ( param )
	{
		strncpy( param, argv[a], maxParamLen ? maxParamLen : SIZE_MAX );
	}

	return argv[a];
}

//------------------------------------------------------------------------------
inline const char* SimpleArgs::get( int argn, char *argv[], const char* opt, char* param, const int maxParamLen )
{
	for( int a=0; a<argn; ++a )
	{
		if ( strncmp(argv[a], opt, strlen(opt))
			 || (strlen(argv[a]) != strlen(opt)) )
		{
			continue;
		}

		if ( param )
		{
			param[0] = 0;
			if ( ++a < argn )
			{
				strncpy( param, argv[a], maxParamLen ? maxParamLen : SIZE_MAX );
			}
			else
			{
				return 0; // no param and we wanted it, so nope
			}
		}

		return argv[a];
	}

	return 0;
}


#endif
#ifndef GC_OBJECT_H
#define GC_OBJECT_H
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

#include <assert.h>

//------------------------------------------------------------------------------
enum WRGCFlags
{
	GCFlag_SkipGC = 1<<0,
	GCFlag_Marked = 1<<1,
};

//------------------------------------------------------------------------------
class WRGCBase
{
public:
	
	// the order here matters for data alignment

#if (__cplusplus <= 199711L)
	int8_t m_type;
#else
	WRGCObjectType m_type;
#endif
	
	int8_t m_flags;
	
	union
	{
		uint16_t m_mod;
		uint16_t m_hashItem;
	};

	union
	{
		void* m_data;
		char* m_SCdata;
		unsigned char* m_Cdata;
		WRValue* m_Vdata;
		WRGCBase* m_referencedTable;
	};

	WRGCBase* m_nextGC;

	void clear()
	{
		if ( m_type >= SV_VALUE )
		{
			g_free( m_Cdata );
		}
	}
};

//------------------------------------------------------------------------------
class WRGCHashReference : public WRGCBase
{
public:
};

//------------------------------------------------------------------------------
class WRGCObject : public WRGCBase
{
public:

	uint32_t m_size;

	union
	{
		uint32_t* m_hashTable;
		const uint8_t* m_ROMHashTable;
		WRContext* m_creatorContext;
	};

	int init( const unsigned int size, const WRGCObjectType type, bool clear );

	WRValue* getAsRawValueHashTable( const uint32_t hash, int* index =0 );

	WRValue* exists( const uint32_t hash, bool removeIfPresent );

	void* get( const uint32_t l, int* index =0 );

	uint32_t growHash( const uint32_t hash, const uint16_t sizeHint =0, int* sizeAllocated =0 );
	uint32_t getIndexOfHit( const uint32_t hash, const bool inserting );

private:

	WRGCObject& operator= ( WRGCObject& A );
	WRGCObject(WRGCObject& A);
};


#endif
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

	// how many bytes wrench must allocate before running the gc, default WRENCH_DEFAULT_ALLOCATED_MEMORY_LIMIT
	uint16_t allocatedMemoryLimit;
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
	uint16_t stackSize; // how much stack to give each context
	int8_t err;

#ifdef WRENCH_TIME_SLICES
	int instructionsPerSlice;
	int sliceInstructionCount;
	int yieldEnabled;
#endif
	
	WRContext* contextList;

	WRLibraryCleanup* libCleanupFunctions;
	
	WRGCObject globalRegistry;
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
	O_Yield =0,

	O_LiteralInt32,
	O_LiteralZero,
	O_LiteralFloat,
	O_LiteralString,

	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,
	O_PushIndexFunctionReturnValue,

	O_CallLibFunction,
	O_CallLibFunctionAndPop,

	O_NewObjectTable,
	O_AssignToObjectTableByOffset,
	O_AssignToObjectTableByHash,

	O_AssignToHashTableAndPop,
	O_Remove,
	O_HashEntryExists,

	O_PopOne,
	O_ReturnZero,
	O_Return,
	O_Stop,

	O_Dereference,
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

	O_LoadLibConstant,
	O_InitArray,
	O_InitVar,

	O_DebugInfo,
				
	// non-interpreted opcodes
	O_HASH_PLACEHOLDER,
	O_FUNCTION_CALL_PLACEHOLDER,

	O_LAST,
};

extern const char* c_opcodeName[];

#endif
#ifndef _STR_H
#define _STR_H
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef STR_FILE_OPERATIONS
#include <sys/stat.h>
#endif

#if __cplusplus > 199711L
#define STR_COPY_ARG
#endif

// same as str.h but for char only so no template overhead, also no
// new/delete just malloc/free

const unsigned int c_sizeofBaseString = 15; // this lib tries not to use dynamic RAM unless it has to
const int c_formatBaseTrySize = 80;

//-----------------------------------------------------------------------------
class WRstr
{
public:
	WRstr() { m_smallbuf[m_len = 0] = 0 ; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; }
	WRstr( const WRstr& str) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(str, str.size()); } 
	WRstr( const WRstr* str ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; if ( str ) { set(*str, str->size()); } } 
	WRstr( const char* s, const unsigned int len ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, len); }
	WRstr( const char* s ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, (unsigned int)strlen(s)); }
	WRstr( const char c ) { m_len = 1; m_str = m_smallbuf; m_smallbuf[0] = c; m_smallbuf[1] = 0; m_buflen = c_sizeofBaseString; } 

#ifdef STR_FILE_OPERATIONS
	inline bool fileToBuffer( const char* fileName, const bool appendToBuffer =false );
	inline bool bufferToFile( const char* fileName, const bool append =false ) const;
#else
	bool fileToBuffer( const char* fileName, const bool appendToBuffer =false ) { return false; }
	bool bufferToFile( const char* fileName, const bool append =false ) const { return false; }
#endif

	WRstr& clear() { m_str[m_len = 0] = 0; return *this; }

#ifdef STR_COPY_ARG
	WRstr& format( const char* format, ... ) { va_list arg; va_start( arg, format ); clear(); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	WRstr& formatVA( const char* format, va_list arg ) { clear(); return appendFormatVA(format, arg); }
	WRstr& appendFormat( const char* format, ... ) { va_list arg; va_start( arg, format ); appendFormatVA( format, arg ); va_end( arg ); return *this; }
	inline WRstr& appendFormatVA( const char* format, va_list arg );
#else
	inline WRstr& format( const char* format, ... );
	inline WRstr& appendFormat( const char* format, ... );
#endif

	inline void release( char** toBuf, unsigned int* len =0 ); // always suceeds and returns dynamic memory
	inline WRstr& giveOwnership( char* str, const unsigned int len );

	static const unsigned int npos = (unsigned int)-1;
	unsigned int find( const char c, const unsigned int from =0 ) const { char buf[2] = { c, 0 }; return find(buf, from); }
	unsigned int rfind( const char c, const unsigned int from =npos ) const { char buf[2] = { c, 0 }; return rfind(buf, from); }
	unsigned int findCase( const char c, const unsigned int from =0 ) const { char buf[2] = { c, 0 }; return findCase(buf, from); }
	inline unsigned int find( const char* str, const unsigned int from =0 ) const;
	inline unsigned int rfind( const char* str, const unsigned int from =npos ) const;
	inline unsigned int findCase( const char* str, const unsigned int from =0 ) const;

	WRstr& setSize( const unsigned int size, const bool preserveContents =true ) { alloc(size, preserveContents); m_len = size; m_str[size] = 0; return *this; }

	inline WRstr& alloc( const unsigned int characters, const bool preserveContents =true );

	inline WRstr& trim();
	inline WRstr& truncate( const unsigned int newLen ); // reduce size to 'newlen'
	WRstr& shave( const unsigned int e ) { return (e > m_len) ? clear() : truncate(m_len - e); } // remove 'x' trailing characters
	inline WRstr& shift( const unsigned int from );
	inline WRstr substr( const unsigned int begin, const unsigned int len ) const;

	unsigned int size() const { return m_len; } // see length

	const char* c_str( const unsigned int offset =0 ) const { return m_str + offset; }
	char* p_str( const unsigned int offset =0 ) const { return m_str + offset; }

	operator const void*() const { return m_str; }
	operator const char*() const { return m_str; }

	WRstr& set( const char* buf, const unsigned int len ) { m_len = 0; m_str[0] = 0; return insert( buf, len ); }
	WRstr& set( const WRstr& str ) { return set( str.m_str, str.m_len ); }
	WRstr& set( const char c ) { clear(); m_str[0]=c; m_str[1]=0; m_len = 1; return *this; }

	bool isMatch( const char* buf ) const { return strcmp(buf, m_str) == 0; }
#ifdef WIN32
	bool isMatchCase( const char* buf ) const { return _strnicmp(buf, m_str, m_len) == 0; }
#else
	bool isMatchCase( const char* buf ) const { return strncasecmp(buf, m_str, m_len) == 0; }
#endif
	static inline bool isWildMatch( const char* pattern, const char* haystack );
	inline bool isWildMatch( const char* pattern ) const { return isWildMatch( pattern, m_str ); }
				  
	static inline bool isWildMatchCase( const char* pattern, const char* haystack );
	inline bool isWildMatchCase( const char* pattern ) const { return isWildMatchCase( pattern, m_str ); }

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

	char& get( const unsigned int l ) { return m_str[l]; }
	const char& get( const unsigned int l ) const { return m_str[l]; }

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
	friend bool operator != ( const WRstr& s, char* z ) { return !s.isMatch( z ); }
	friend bool operator != ( char* z, const WRstr& s ) { return !s.isMatch( z ); }

	friend WRstr operator + ( const WRstr& str, const char* s) { WRstr T(str); T += s; return T; }
	friend WRstr operator + ( const WRstr& str, const char c) { WRstr T(str); T += c; return T; }
	friend WRstr operator + ( const char* s, const WRstr& str ) { WRstr T(s, (unsigned int)strlen(s)); T += str; return T; }
	friend WRstr operator + ( const char c, const WRstr& str ) { WRstr T(c); T += str; return T; }
	friend WRstr operator + ( const WRstr& str1, const WRstr& str2 ) { WRstr T(str1); T += str2; return T; }

	~WRstr() { if ( m_str != m_smallbuf ) g_free(m_str); }

protected:

	operator char*() const { return m_str; } // prevent accidental use

	char *m_str; // first element so if the class is cast as a C and de-referenced it always works

	unsigned int m_buflen; // how long the buffer itself is
	unsigned int m_len; // how long the string is in the buffer
	char m_smallbuf[ c_sizeofBaseString + 1 ]; // small temporary buffer so a malloc/free is not imposed for small strings
};

//------------------------------------------------------------------------------
unsigned int WRstr::rfind( const char* str, const unsigned int from ) const
{
	int f = (int)(from > m_len ? m_len : from);
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f >= 0; --f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}
			if ( str[i] != m_str[f + i] )
			{
				break;
			}
		}
	}
	return npos;
}

//------------------------------------------------------------------------------
unsigned int WRstr::find( const char* str, const unsigned int from ) const
{
	unsigned int f = from > m_len ? 0 : from;
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f < m_len; ++f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}

			char c = m_str[f + i];
			if ( !c )
			{
				return npos;
			}

			if ( str[i] != c )
			{
				break;
			}
		}
	}
	return npos;
}

//------------------------------------------------------------------------------
unsigned int WRstr::findCase( const char* str, const unsigned int from ) const
{
	unsigned int f = from > m_len ? 0 : from;
	if ( !str || !str[0] )
	{
		return 0;
	}

	for( ; f < m_len; ++f )
	{
		for( int i=0;;++i )
		{
			if ( !str[i] )
			{
				return f;
			}

			char c = m_str[f + i];
			if ( !c )
			{
				return npos;
			}

			if ( tolower(str[i]) != tolower(c) )
			{
				break;
			}
		}
	}
	return npos;
}

#ifdef STR_FILE_OPERATIONS
//-----------------------------------------------------------------------------
bool WRstr::fileToBuffer( const char* fileName, const bool appendToBuffer )
{
	if ( !fileName )
	{
		return false;
	}

#ifdef WIN32
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
void WRstr::release( char** toBuf, unsigned int* len )
{
	if ( len )
	{
		*len = m_len;
	}
	
	if ( !m_len )
	{
		*toBuf = 0;
	}
	else if ( m_str == m_smallbuf )
	{
		*toBuf = (char*)g_malloc( m_len + 1 );
		memcpy( *toBuf, m_str, m_len + 1 );
	}
	else
	{
		*toBuf = m_str;
		m_str = m_smallbuf;
		m_buflen = c_sizeofBaseString;
	}

	m_len = 0;
	m_str[0] = 0;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::giveOwnership( char* buf, const unsigned int len )
{
	if ( !buf || !len )
	{
		clear();
		return *this;
	}

	if ( m_str != m_smallbuf )
	{
		g_free( m_str );
	}

	if ( len < c_sizeofBaseString )
	{
		m_str = m_smallbuf;
		memcpy( m_str, buf, len );
		g_free( buf );
		m_len = len;
	}
	else
	{
		m_str = buf;
	}

	m_len = len;
	m_buflen = len;
	m_str[m_len] = 0;
	
	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::trim()
{
	unsigned int start = 0;

	// find start
	for( ; start<m_len && isspace( (char)*(m_str + start) ) ; start++ );

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
		if ( !isspace((char)(m_str[pos] = m_str[start])) )
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
		char* newStr = (char*)g_malloc( characters + 1 ); // create the space

		if ( preserveContents ) 
		{
			memcpy( newStr, m_str, m_buflen ); // preserve whatever we had
		}

		if ( m_str != m_smallbuf )
		{
			g_free( m_str );
		}

		m_str = newStr;
		m_buflen = characters;		
	}

	return *this;
}

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
			g_free( m_str );
			m_str = m_smallbuf;
		}
	}

	m_str[ newLen ] = 0;
	m_len = newLen;

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::shift( const unsigned int from )
{
	if ( from >= m_len )
	{
		return clear();
	}

	m_len -= from;
	memmove( m_str, m_str + from, m_len + 1 );

	return *this;
}

//------------------------------------------------------------------------------
WRstr WRstr::substr( const unsigned int begin, const unsigned int len ) const
{
	WRstr ret;
	if ( begin < m_len )
	{
		unsigned int amount = (begin + len) > m_len ? m_len - begin : len;
		ret.set( m_str + begin, amount );
	}
	return ret;
}

//-----------------------------------------------------------------------------
bool WRstr::isWildMatch( const char* pattern, const char* haystack )
{
	if ( !pattern )
	{
		return false;
	}

	if ( pattern[0] == 0 )
	{
		return haystack[0] == 0;
	}

	const char* after = 0;
	const char* str = haystack;
	char t;
	char w;

	for(;;)
	{
		t = *str;
		w = *pattern;
		if ( !t )
		{
			if ( !w )
			{
				return true; // "x" matches "x"
			}
			else if (w == '*')
			{
				++pattern;
				continue; // "x*" matches "x" or "xy"
			}

			return false; // "x" doesn't match "xy"
		}
		else if ( t != w )
		{
			if (w == '*')
			{
				after = ++pattern;
				continue; // "*y" matches "xy"
			}
			else if (after)
			{
				pattern = after;
				w = *pattern;
				if ( !w )
				{
					return true; // "*" matches "x"
				}
				else if (t == w)
				{
					++pattern;
				}
				++str;
				continue; // "*sip*" matches "mississippi"
			}
			else
			{
				return false; // "x" doesn't match "y"
			}
		}

		++str;
		++pattern;
	}
}

//-----------------------------------------------------------------------------
bool WRstr::isWildMatchCase( const char* pattern, const char* haystack )
{
	if ( !pattern )
	{
		return false;
	}

	if ( pattern[0] == 0 )
	{
		return haystack[0] == 0;
	}

	const char* after = 0;
	const char* str = haystack;
	char t;
	char w;

	for(;;)
	{
		t = *str;
		w = *pattern;
		if ( !t )
		{
			if ( !w )
			{
				return true; // "x" matches "x"
			}
			else if (w == '*')
			{
				++pattern;
				continue; // "x*" matches "x" or "xy"
			}

			return false; // "x" doesn't match "xy"
		}
		else if ( tolower(t) != tolower(w) )
		{
			if (w == '*')
			{
				after = ++pattern;
				continue; // "*y" matches "xy"
			}
			else if (after)
			{
				pattern = after;
				w = *pattern;
				if ( !w )
				{
					return true; // "*" matches "x"
				}
				else if (t == w)
				{
					++pattern;
				}
				++str;
				continue; // "*sip*" matches "mississippi"
			}
			else
			{
				return false; // "x" doesn't match "y"
			}
		}

		++str;
		++pattern;
	}
}

//-----------------------------------------------------------------------------
WRstr& WRstr::insert( const char* buf, const unsigned int len, const unsigned int startPos /*=0*/ )
{
	if ( len != 0 ) // insert 0? done
	{
		alloc( m_len + len + startPos, true ); // make sure there is enough room for the new string

		if ( startPos < m_len ) // text after the insert, move everything up
		{
			memmove( m_str + len + startPos, m_str + startPos, m_len );
		}

		memcpy( m_str + startPos, buf, len );

		m_len += len;
		m_str[m_len] = 0;
	}

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

#ifdef STR_COPY_ARG

//------------------------------------------------------------------------------
WRstr& WRstr::appendFormatVA( const char* format, va_list arg )
{
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );

	if ( len < c_formatBaseTrySize )
	{
		insert( buf, len, m_len );
	}
	else
	{
		char* alloc = (char*)g_malloc( ++len );

		va_list vacopy;
		va_copy( vacopy, arg );
		len = vsnprintf(alloc, len, format, arg);
		va_end( vacopy );

		if ( m_len )
		{
			insert( alloc, len, m_len );
			g_free( alloc );
		}
		else
		{
			giveOwnership( alloc, len );
		}
	}

	return *this;
}

#else

//------------------------------------------------------------------------------
WRstr& WRstr::format( const char* format, ... )
{
	va_list arg;
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	va_start( arg, format );
	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );
	va_end( arg );

	if ( len < c_formatBaseTrySize+1 )
	{
		set( buf, len );
	}
	else
	{
		char* alloc = (char*)g_malloc(++len);

		va_start( arg, format );
		len = vsnprintf( alloc, len, format, arg );
		va_end( arg );

		giveOwnership( alloc, len );
	}

	return *this;
}

//-----------------------------------------------------------------------------
WRstr& WRstr::appendFormat( const char* format, ... )
{
	va_list arg;
	char buf[ c_formatBaseTrySize + 1 ]; // SOME space, malloc if we need a ton more

	va_start( arg, format );
	int len = vsnprintf( buf, c_formatBaseTrySize, format, arg );
	va_end( arg );

	if ( len < c_formatBaseTrySize+1 )
	{
		insert( buf, len, m_len );
	}
	else
	{
		char* alloc = (char*)g_malloc(++len);

		va_start( arg, format );
		len = vsnprintf( alloc, len, format, arg );
		va_end( arg );

		if ( m_len )
		{
			insert( alloc, len, m_len );
			g_free( alloc );
		}
		else
		{
			giveOwnership( alloc, len );
		}
	}

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
		g_free( m_buf );
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
			m_buf = (unsigned char *)g_malloc( m_bufLen );
			if ( m_len )
			{
				memcpy( m_buf, buf, m_len );
			}

			g_free( buf );
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

#define WR_COMPILER_LITERAL_STRING 0x10 

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
struct NamespacePush
{
	int unit;
	int location;
	NamespacePush* next;
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

	{ "+",    6, O_BinaryAddition,      true,  WR_OPER_BINARY, O_LAST },
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

	{ "@[]",  2, O_Index,		        true,  WR_OPER_POST, O_LAST },
	{ "@init", 2, O_InitArray,          true,  WR_OPER_POST, O_LAST },

	{ "@macroBegin", 0, O_LAST,         true,  WR_OPER_POST, O_LAST },

	{ "._count", 2, O_CountOf,          true,  WR_OPER_POST, O_LAST },
	{ "._hash",  2, O_HashOf,           true,  WR_OPER_POST, O_LAST },
	{ "._remove", 2, O_Remove,			true,  WR_OPER_POST, O_LAST },
	{ "._exists", 2, O_HashEntryExists, true,  WR_OPER_POST, O_LAST },
	
	{ 0, 0, O_LAST, false, WR_OPER_PRE, O_LAST },
};
const int c_highestPrecedence = 17; // one higher than the highest entry above, things that happen absolutely LAST

//------------------------------------------------------------------------------
enum WRExpressionType
{
	EXTYPE_NONE =0,
	EXTYPE_LITERAL,
	EXTYPE_LIB_CONSTANT,
	EXTYPE_LABEL,
	EXTYPE_LABEL_AND_NULL,
	EXTYPE_OPERATION,
	EXTYPE_RESOLVED,
	EXTYPE_BYTECODE_RESULT,
};

//------------------------------------------------------------------------------
struct WRNamespaceLookup
{
	uint32_t hash; // hash of symbol
	WRarray<int> references; // where this symbol is referenced (loaded) in the bytecode
	WRstr label;
	
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
	bool varSeen;
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
		varSeen = false;
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
	bool lValue;

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
		lValue = false;
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
	WRstr name;
	uint32_t hash; // hashed name of this unit
	uint32_t arguments; // how many arguments it expects
	int offsetInBytecode; // where in the bytecode it resides

	bool exportNamespace;

	WRarray<ConstantValue> constantValues;

	int16_t offsetOfLocalHashMap;
	
	// the code that runs when it loads
	// the locals it has
	WRBytecode bytecode;

	int parentUnitIndex;
	
	WRUnitContext() { reset(); }
	void reset()
	{
		name = "";
		exportNamespace = false;
		hash = 0;
		arguments = 0;
		arguments = 0;
		parentUnitIndex = 0;
		constantValues.clear();
		offsetOfLocalHashMap = 0;
		bytecode.clear();
		parentUnitIndex = 0;
	}
};

//------------------------------------------------------------------------------
struct WRCompilationContext
{
public:
	WRError compile( const char* data,
					 const int size,
					 unsigned char** out,
					 int* outLen,
					 char* erroMsg,
					 const uint8_t compilerOptionFlags );

private:
	
	bool isReserved( const char* token );
	bool isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix, bool& isLibConstant );

	bool getToken( WRExpressionContext& ex, const char* expect =0 );

	static bool CheckSkipLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool CheckFastLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool IsLiteralLoadOpcode( unsigned char opcode );
	static bool CheckCompareReplace( WROpcode LS, WROpcode GS, WROpcode ILS, WROpcode IGS, WRBytecode& bytecode, unsigned int a, unsigned int o );

	friend class WRExpression;
	static void pushOpcode( WRBytecode& bytecode, WROpcode opcode );
	static void pushData( WRBytecode& bytecode, const unsigned char* data, const int len ) { bytecode.all.append( data, len ); }
	static void pushData( WRBytecode& bytecode, const char* data, const int len ) { bytecode.all.append( (unsigned char*)data, len ); }

	int getBytecodePosition( WRBytecode& bytecode ) { return bytecode.all.size(); }

	bool m_addDebugSymbols;
	bool m_embedGlobalSymbols;
	bool m_embedSourceCode;
	bool m_needVar;
	bool m_exportNextUnit;
	
	uint16_t m_lastCode;
	uint16_t m_lastParam;
	void pushDebug( uint16_t code, WRBytecode& bytecode,int param );
	int getSourcePosition();
	int getSourcePosition( int& onLine, int& onChar, WRstr* line =0 );
	
	int addRelativeJumpTarget( WRBytecode& bytecode );
	void setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget );
	void addRelativeJumpSourceEx( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget, const unsigned char* data, const int dataSize );
	void addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget );
	void resolveRelativeJumps( WRBytecode& bytecode );

	void appendBytecode( WRBytecode& bytecode, WRBytecode& addMe );
	
	void pushLiteral( WRBytecode& bytecode, WRExpressionContext& context );
	void pushLibConstant( WRBytecode& bytecode, WRExpressionContext& context );
	int addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen );
	int addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen );
	void addFunctionToHashSpace( WRBytecode& result, WRstr& token );
	void loadExpressionContext( WRExpression& expression, int depth, int operation );
	void resolveExpression( WRExpression& expression );
	unsigned int resolveExpressionEx( WRExpression& expression, int o, int p );

	bool operatorFound( WRstr const& token, WRarray<WRExpressionContext>& context, int depth );
	bool parseCallFunction( WRExpression& expression, WRstr functionName, int depth, bool parseArguments );
	bool pushObjectTable( WRExpressionContext& context, WRarray<WRNamespaceLookup>& localSpace, uint32_t hash );
	int parseInitializer( WRExpression& expression, int depth );
	char parseExpression( WRExpression& expression );
	bool parseUnit( bool isStruct, int parentUnitIndex );
	bool parseWhile( WROpcode opcodeToReturn );
	bool parseDoWhile( WROpcode opcodeToReturn );
	bool parseForLoop( WROpcode opcodeToReturn );
	bool lookupConstantValue( WRstr& prefix, WRValue* value =0 );
	bool parseEnum( int unitIndex );
	uint32_t getSingleValueHash( const char* end );
	bool parseSwitch( WROpcode opcodeToReturn );
	bool parseIf( WROpcode opcodeToReturn );
	bool parseStatement( int unitIndex, char end, WROpcode opcodeToReturn );

	void createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size );
	void link( unsigned char** out, int* outLen, const uint8_t compilerOptionFlags );

	const char* m_source;
	int m_sourceLen;
	int m_pos;

	bool getChar( char &c ) { c = m_source[m_pos++]; return m_pos < m_sourceLen; }
	bool checkAsComment( char lead );
	bool readCurlyBlock( WRstr& block );
	struct TokenBlock
	{
		WRstr data;
		TokenBlock* next;
	};

	WRstr m_loadedToken;
	WRValue m_loadedValue;
	bool m_loadedQuoted;
	
	WRError m_err;
	bool m_EOF;
	bool m_LastParsedLabel;
	bool m_parsingFor;
	bool m_parsingNew;
	bool m_quoted;

	uint32_t m_newHashValue;
	
	int m_unitTop;
	WRarray<WRUnitContext> m_units;

	WRarray<int> m_continueTargets;
	WRarray<int> m_breakTargets;

	int m_foreachHash;
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

//------------------------------------------------------------------------------
inline const char* wr_asciiDump( const void* d, unsigned int len, WRstr& str, int markByte =-1 )
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
			if ( i == (unsigned int)markByte )
			{
				str.shave(1);
				str.appendFormat( "[%02X]", (unsigned char)data[i] );
			}
			else
			{
				str.appendFormat( "%02X ", (unsigned char)data[i] );
			}
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

#endif // WRENCH_WITHOUT_COMPILER

#endif
#ifndef _PACKET_H
#define _PACKET_H
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
enum WrenchDebugComm
{
	WRD_None = 0,
	WRD_Ok,
	WRD_Run,
	WRD_Load,

	WRD_DebugOut,

	WRD_RequestCallstack,
	WRD_RequestStackDump,
	WRD_RequestSymbolBlock,
	WRD_RequestSourceBlock,
	WRD_RequestSourceHash,
	WRD_RequestValue,
	WRD_RequestStepOver,
	WRD_RequestStepInto,
	WRD_RequestSetBreakpoint,
	WRD_RequestClearBreakpoint,

	WRD_ReplyCallstack,
	WRD_ReplyStackDump,
	WRD_ReplySymbolBlock,
	WRD_ReplySource,
	WRD_ReplySourceHash,
	WRD_ReplyValue,

	WRD_ReplyUnavailable,
	WRD_Err,
};

//------------------------------------------------------------------------------
struct WrenchPacket
{
	uint32_t size; // TOTAL size including any additional payload (always first)

	union
	{
		uint32_t t; // type this is
		WrenchDebugComm _type;
	};
	int32_t param1;
	int32_t param2;

	operator WrenchPacket* () { return this; }

	void setPayloadSize( uint32_t newsize ) { size = sizeof(WrenchPacket) + newsize; }
	uint32_t payloadSize() { return (size <= sizeof(WrenchPacket)) ? 0 : (size - sizeof(WrenchPacket)); }
	uint8_t* payload( uint32_t offset =0 ) { return (uint8_t*)((char *)this + sizeof(WrenchPacket)) + offset; }
	uint8_t operator[] ( const int offset ) { return *(uint8_t*)((char*)this + sizeof(WrenchPacket)) + offset; }
	uint8_t* data() { return (uint8_t*)this; }
	
	WrenchPacket() {}
	WrenchPacket( const int32_t type );
	static WrenchPacket* alloc( WrenchPacket const& base );
	static WrenchPacket* alloc( const uint32_t type, const uint32_t payloadSize =0, const uint8_t* payload =0 );
	uint32_t xlate();
};

//------------------------------------------------------------------------------
class WrenchPacketScoped
{
public:
	WrenchPacket* packet;

	operator WrenchPacket* () const { return packet; }
	operator bool() const { return packet != 0; }
	
	WrenchPacketScoped( WrenchPacket* manage =0 ) : packet(manage) {}
	WrenchPacketScoped( const uint32_t type ) : packet( WrenchPacket::alloc(type) ) {}
	WrenchPacketScoped( const uint32_t type, const int bytes, const uint8_t* data ) : packet( WrenchPacket::alloc(type, bytes, data) ) {}
	~WrenchPacketScoped() { g_free( packet ); }
};

#endif

#endif
#ifndef _DEBUG_H
#define _DEBUG_H
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

void wr_stackDump( const WRValue* bottom, const WRValue* top, WRstr& out );

//------------------------------------------------------------------------------
// to allow the compiler to COMPILE debug code even if the VM does not
// execute it
enum WrenchDebugInfoType
{
	WRD_TypeMask =     0xC000,
	WRD_PayloadMask =  0x3FFF,
	WRD_GlobalStopFunction = 0x0F00,
	WRD_ExternalFunction = 0x0E00,

	WRD_FunctionCall = 0x0000, // payload is function index
	WRD_LineNumber =   0x4000, // 14-bit line number
	WRD_Return =       0x8000, // function has returned
	WRD_RESERVED =     0xC000,
};

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
class WRDebugClientInterfacePrivate
{
public:
	WRDebugClientInterfacePrivate();
	~WRDebugClientInterfacePrivate();

	void init();

	WRDebugClientInterface* m_parent;
	
	char* m_sourceBlock;
	uint32_t m_sourceBlockLen;
	uint32_t m_sourceBlockHash;

	void getValues( SimpleLL<WrenchDebugValue>& values, const int level );
	void populateSymbols();

	void (*outputDebug)( const char* debugText );
			
	bool m_symbolsLoaded;
	SimpleLL<WrenchSymbol>* m_globals;
	SimpleLL<WrenchFunction>* m_functions;

	bool m_callstackDirty;
	SimpleLL<WrenchCallStackEntry>* m_callStack;
	WRP_ProcState m_procState;

	WrenchDebugCommInterface* m_comm;

	WrenchPacket* getPacket( const int timeoutMilliseconds =0 );
	
	WRState* m_scratchState;
	WRContext* m_scratchContext;
};

//------------------------------------------------------------------------------
class WRDebugServerInterfacePrivate
{
public:
	WRDebugServerInterfacePrivate( WRDebugServerInterface* parent );
	~WRDebugServerInterfacePrivate();

	bool codewordEncountered( const uint8_t* pc, uint16_t codeword, WRValue* stackTop );
	void clearBreakpoint( const int bp );
	WrenchPacket* processPacket( WrenchPacket* packet );

	int m_steppingOverReturnVector;
	int m_lineSteps;
	int m_stepOverDepth;
	bool m_stepOut;

	WrenchPacket m_lastPacket;

	SimpleLL<int>* m_lineBreaks;
	SimpleLL<WrenchCallStackEntry>* m_callStack;

	WRState* m_w;
	WrenchDebugCommInterface* m_comm;

	WRDebugServerInterface* m_parent;
	bool m_firstCall;
	int32_t m_stopOnLine;
	
	uint8_t* m_externalCodeBlock;
	uint32_t m_externalCodeBlockSize;	
	
	const uint8_t* m_embeddedSource;
	uint32_t m_embeddedSourceLen;
	uint32_t m_embeddedSourceHash;
	uint8_t m_compilerFlags;

	const uint8_t* m_symbolBlock;
	uint32_t m_symbolBlockLen;

	// for resuming a function call
	WRContext* m_context;
	const WRValue* m_argv;
	int m_argn;
	const unsigned char* m_pc;
	WRValue* m_frameBase;
	WRValue* m_stackTop;

	bool m_halted;
};

#endif

#endif
#ifndef _DEBUGGER_H
#define _DEBUGGER_H
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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

class WrenchDebugTcpInterface;
class WrenchDebugSerialInterface;

//------------------------------------------------------------------------------
class WrenchDebugClient
{
public:

	void enter( const char* sourceFileName, const char* address, const int port =0 );

	WrenchDebugClient( WRState* w =0 ); // use provided state or make a new one
	~WrenchDebugClient();

private:

	bool init( const char* name, const char* address, const int port =0 );
	bool loadSource( const char* name );

	bool printVariable( WRstr& str );
	WrenchCallStackEntry* getCurrentFrame();

	void showGlobals();
	void showLocals();
	void showCallStack();
	void showCodePosition();

	void showBreakpoint( int bp =-1 );

	int splitArgs( const char* str, SimpleLL<WRstr>& args );
	void splitNL( const char* str );

	bool m_printInHex;
	int m_currentDepth;

	SimpleLL<WRstr> m_listing;
	SimpleLL<int> m_breakpoints;
	SimpleLL<WRstr> m_displayPerStep;

	WRState* m_w;
	WRContext* m_context;
	bool m_ownState;

	unsigned char* m_code;
	int m_codeLen;

	WRDebugClientInterface* m_client;
	WRDebugServerInterface* m_server;

	WrenchDebugCommInterface* m_comm;
};

#endif

#endif
#ifndef _STD_SERIAL_H
#define _STD_SERIAL_H
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

#if defined(WRENCH_WIN32_SERIAL) || defined(WRENCH_LINUX_SERIAL) || defined(WRENCH_ARDUINO_SERIAL)

#ifdef WRENCH_WIN32_SERIAL
#include <windows.h>
#include <atlstr.h>
#define WR_BAD_SOCKET INVALID_HANDLE_VALUE
#endif

#ifdef WRENCH_LINUX_SERIAL
#define HANDLE int
#define WR_BAD_SOCKET -1
#endif

#ifdef WRENCH_ARDUINO_SERIAL
#include <Arduino.h>
#define HANDLE HardwareSerial&
#endif

HANDLE wr_serialOpen( const char* name );
void wr_serialClose( HANDLE name );
int wr_serialSend( HANDLE comm, const char* data, const int size );
int wr_serialReceive( HANDLE comm, char* data, const int max );
int wr_serialPeek( HANDLE comm );

#endif

#endif
#ifndef _STD_TCP_H
#define _STD_TCP_H
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

#if (defined(WRENCH_WIN32_TCP) || defined(WRENCH_LINUX_TCP))

// Win32 IO Includes
#ifdef WRENCH_WIN32_TCP
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Wsock32.lib" )

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <time.h>
#include <io.h>
#endif

// Linux IO includes
#ifdef WRENCH_LINUX_TCP
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#endif

struct WRValue;
struct WRContext;

int wr_bindTCPEx( const int port );
int wr_acceptTCPEx( const int socket, sockaddr* info );
int wr_connectTCPEx( const char* address, const int port );

int wr_receiveTCPEx( const int socket, char* data, const int toRead, const int timeoutMilliseconds =0 );
int wr_peekTCPEx( const int socket, const int timeoutMilliseconds =0 );
int wr_sendTCPEx( const int socket, const char* data, const int toSend );
void wr_closeTCPEx( const int socket );


#endif

#endif
#ifndef _DEBUG_COMM_H
#define _DEBUG_COMM_H
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

#include "wrench.h"

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
class WrenchDebugCommInterface
{
public:
	// block until exactly 'bytes' are sent or return false 
	virtual bool send( WrenchPacket* packet );
	virtual WrenchPacket* receive( const int timeoutMilliseconds =0 );

	virtual bool sendEx( const uint8_t* data, const int bytes ) { return false; }
	virtual bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds ) { return false; }
	virtual uint32_t peekEx( const int timeoutMilliseconds ) { return 0; }

	virtual ~WrenchDebugCommInterface() {}
};

//------------------------------------------------------------------------------
class WrenchDebugLoopbackInterface : public WrenchDebugCommInterface
{
public:

	bool send( WrenchPacket* packet ) { return (*m_queue.addHead() = WrenchPacket::alloc( *packet)) != 0; }
	WrenchPacket* receive( const int timeoutMilliseconds )
	{
		WrenchPacket* p = 0;
		m_queue.popTail( &p );
		return p;
	}

private:
	SimpleLL<WrenchPacket*> m_queue;
};

//------------------------------------------------------------------------------
class WrenchDebugTcpInterface : public WrenchDebugCommInterface
{
public:

	bool serve( const int port );
	bool accept();
	bool connect( const char* address, const int port );
	
	bool sendEx( const uint8_t* data, const int bytes );
	bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds );
	uint32_t peekEx( const int timeoutMilliseconds );
	
	WrenchDebugTcpInterface();
	
private:
	int m_socket;
};


//------------------------------------------------------------------------------
class WrenchDebugSerialInterface : public WrenchDebugCommInterface
{
public:

	bool open( const char* name );
	
	bool sendEx( const uint8_t* data, const int bytes );
	bool recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds );
	uint32_t peekEx( const int timeoutMilliseconds );
	
	WrenchDebugSerialInterface();
	
private:
	HANDLE m_interface;
};

#endif

#endif
#ifndef _STD_IO_DEFS_H
#define _STD_IO_DEFS_H
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

struct WRValue;
struct WRContext;
struct WRState;

void wr_read_file( WRValue* stackTop, const int argn, WRContext* c );
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c );
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c );

void wr_getline( WRValue* stackTop, const int argn, WRContext* c );

void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c );
void wr_ioFSync( WRValue* stackTop, const int argn, WRContext* c );

void wr_ioPushConstants( WRState* w );
void wr_stdout( const char* data, const int size );

#endif
#ifndef _STD_TCP_H
#define _STD_TCP_H
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

#if (defined(WRENCH_WIN32_TCP) || defined(WRENCH_LINUX_TCP))

// Win32 IO Includes
#ifdef WRENCH_WIN32_TCP
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Wsock32.lib" )

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <time.h>
#include <io.h>
#endif

// Linux IO includes
#ifdef WRENCH_LINUX_TCP
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#endif

struct WRValue;
struct WRContext;

int wr_bindTCPEx( const int port );
int wr_acceptTCPEx( const int socket, sockaddr* info );
int wr_connectTCPEx( const char* address, const int port );

int wr_receiveTCPEx( const int socket, char* data, const int toRead, const int timeoutMilliseconds =0 );
int wr_peekTCPEx( const int socket, const int timeoutMilliseconds =0 );
int wr_sendTCPEx( const int socket, const char* data, const int toSend );
void wr_closeTCPEx( const int socket );


#endif

#endif
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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

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
	"export",
	"unit",
	""
};

//------------------------------------------------------------------------------
WRError WRCompilationContext::compile( const char* source,
									   const int size,
									   unsigned char** out,
									   int* outLen,
									   char* errorMsg,
									   const uint8_t compilerOptionFlags )
{
	m_source = source;
	m_sourceLen = size;

	*outLen = 0;
	*out = 0;

	m_lastParam = 0;
	m_lastCode = 0;

	m_pos = 0;
	m_err = WR_ERR_None;
	m_EOF = false;
	m_parsingFor = false;
	m_parsingNew = false;
	m_unitTop = 0;
	m_foreachHash = 0;

	m_units.setCount(1);

	m_loadedValue.p2 = INIT_AS_REF;

	m_addDebugSymbols = compilerOptionFlags & WR_EMBED_DEBUG_CODE;
	m_embedSourceCode = compilerOptionFlags & WR_EMBED_SOURCE_CODE;
	m_embedGlobalSymbols = compilerOptionFlags != 0; // (WR_INCLUDE_GLOBALS)
	m_needVar = !(compilerOptionFlags & WR_NON_STRICT_VAR);

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
		m_loadedQuoted = m_quoted;
		
		parseStatement( 0, ';', O_GlobalStop );

	} while ( !m_EOF && (m_err == WR_ERR_None) );

	WRstr msg;

	if ( m_err != WR_ERR_None )
	{
		int onChar;
		int onLine;
		WRstr line;
		getSourcePosition( onLine, onChar, &line );

		msg.format( "line:%d\n", onLine );
		msg.appendFormat( "err: %s[%d]\n", c_errStrings[m_err], m_err );
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

	if ( !m_units[0].bytecode.opcodes.size()
		 || m_units[0].bytecode.opcodes[m_units[0].bytecode.opcodes.size() - 1] != O_GlobalStop )
	{
		// pop final return value
		pushOpcode( m_units[0].bytecode, O_LiteralZero );
		pushOpcode( m_units[0].bytecode, O_GlobalStop );
	}

	pushOpcode( m_units[0].bytecode, O_Stop );

	link( out, outLen, compilerOptionFlags );
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
WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					char* errMsg,
					const uint8_t compilerOptionFlags )
{
	assert( sizeof(float) == 4 );
	assert( sizeof(int) == 4 );
	assert( sizeof(char) == 1 );
	assert( O_LAST < 255 );

	// create a compiler context that has all the necessary stuff so it's completely unloaded when complete
	WRCompilationContext comp; 

	return comp.compile( source, size, out, outLen, errMsg, compilerOptionFlags );
}

//------------------------------------------------------------------------------
void streamDump( WROpcodeStream const& stream )
{
	WRstr str;
	wr_asciiDump( stream, stream.size(), str );
	printf( "%d:\n%s\n", stream.size(), str.c_str() );
}

//------------------------------------------------------------------------------
bool WRCompilationContext::isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix, bool& isLibConstant )
{
	isLibConstant = false;
	
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

	bool foundColon = false;
	for( unsigned int i=0; i<token.size(); i++ ) // entire token alphanumeric or '_'?
	{
		if ( token[i] == ':' )
		{
			if ( token[++i] != ':' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			foundColon = true;

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

	if ( foundColon && token[token.size() - 1] == ':' )
	{
		return false;
	}
	
	if ( foundColon && token[0] != ':' )
	{
		isLibConstant = true;
	}

	return true;
}


//------------------------------------------------------------------------------
void WRCompilationContext::pushDebug( uint16_t code, WRBytecode& bytecode, int param )
{
	if ( !m_addDebugSymbols )
	{
		return;
	}

	if ( code == m_lastCode && (uint16_t)param == m_lastParam )
	{
		return;
	}

	m_lastParam = param;
	m_lastCode = code;
	
	pushOpcode( bytecode, O_DebugInfo );

	uint16_t codeword = code | ((uint16_t)param & WRD_PayloadMask);
	unsigned char data[2];
	pushData( bytecode, wr_pack16(codeword, data), 2 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::getSourcePosition()
{
	int onChar;
	int onLine;
	return getSourcePosition( onLine, onChar, 0 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::getSourcePosition( int& onLine, int& onChar, WRstr* line )
{
	onChar = 0;
	onLine = 1;

	for( int p = 0; line && p<m_sourceLen && m_source[p] != '\n'; p++ )
	{
		(*line) += (char)m_source[p];
	}

	for( int i=0; i<m_pos; ++i )
	{
		if ( m_source[i] == '\n' )
		{
			onLine++;
			onChar = 0;

			if( line )
			{
				line->clear();
				for( int p = i+1; p<m_sourceLen && m_source[p] != '\n'; p++ )
				{
					(*line) += (char)m_source[p];
				}
			}
		}
		else
		{
			onChar++;
		}
	}

	return onLine;
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
				
				wr_pack16( diff, bytecode.all.p_str(offset) );
			}
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLiteral( WRBytecode& bytecode, WRExpressionContext& context )
{
	WRValue& value = context.value;
	unsigned char data[4];
	
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
			int16_t be = value.i;
			pushData( bytecode, wr_pack16(be, data), 2 );
		}
		else
		{
			pushOpcode( bytecode, O_LiteralInt32 );
			unsigned char data[4];
			pushData( bytecode, wr_pack32(value.i, data), 4 );
		}
	}
	else if ( (uint8_t)value.type == WR_COMPILER_LITERAL_STRING )
	{
		pushOpcode( bytecode, O_LiteralString );
		int16_t be = context.literalString.size();
		pushData( bytecode, wr_pack16(be, data), 2 );
		for( unsigned int i=0; i<context.literalString.size(); ++i )
		{
			pushData( bytecode, context.literalString.c_str(i), 1 );
		}
	}
	else
	{
		pushOpcode( bytecode, O_LiteralFloat );
		int32_t be = value.i;
		pushData( bytecode, wr_pack32(be, data), 4 );
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLibConstant( WRBytecode& bytecode, WRExpressionContext& context )
{
	pushOpcode( bytecode, O_LoadLibConstant );
	unsigned char data[4];
	uint32_t hash = wr_hashStr(context.prefix + "::" + context.token);
	pushData( bytecode, wr_pack32(hash, data), 4 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen )
{
	if ( m_unitTop == 0 )
	{
		return addGlobalSpaceLoad( bytecode, token, addOnly, varSeen );
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

	if ( i >= bytecode.localSpace.count() )
	{
		if ( !addOnly )
		{
			WRstr t2;
			if ( varSeen || (token[0] == ':' && token[1] == ':') )
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
			if ( !bytecode.isStructSpace || varSeen ) // structs and explicit 'var' are immune to this
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

		if ( m_needVar && !varSeen )
		{
			m_err = WR_ERR_var_not_seen_before_label;
			return 0;
		}
	}

	bytecode.localSpace[i].hash = hash;
	bytecode.localSpace[i].label = token;
	
	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromLocal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}
	
	return i;
}

//------------------------------------------------------------------------------
int WRCompilationContext::addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen )
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

	if ( m_needVar && !varSeen && i >= m_units[0].bytecode.localSpace.count() )
	{
		m_err = WR_ERR_var_not_seen_before_label;
		return 0;
	}
	
	m_units[0].bytecode.localSpace[i].hash = hash;
	m_units[0].bytecode.localSpace[i].label = t2;

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
		wr_pack32( wr_hash( expression.context[depth].token,
							expression.context[depth].token.size()),
				   buf );
		pushOpcode( expression.bytecode, O_LiteralInt32 );
		pushData( expression.bytecode, buf, 4 );
	}
	else
	{
		switch( expression.context[depth].type )
		{
			case EXTYPE_LIB_CONSTANT:
			{
				pushLibConstant( expression.bytecode, expression.context[depth] );
				break;
			}
			
			case EXTYPE_LITERAL:
			{
				pushLiteral( expression.bytecode, expression.context[depth] );
				break;
			}

			case EXTYPE_LABEL_AND_NULL:
			case EXTYPE_LABEL:
			{
				if ( expression.context[depth].global )
				{
					addGlobalSpaceLoad( expression.bytecode,
										expression.context[depth].token,
										false,
										expression.context[depth].varSeen );
				}
				else
				{
					addLocalSpaceLoad( expression.bytecode,
									   expression.context[depth].token,
									   false,
									   expression.context[depth].varSeen );
				}

				if ( expression.context[depth].type == EXTYPE_LABEL_AND_NULL )
				{
					expression.bytecode.all += O_InitVar;
					expression.bytecode.opcodes += O_InitVar;
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


				if ( expression.context[first].stackPosition == 1 )
				{
					// lucky! a simple single-swap will put them in the
					// right order (turns out this is common)
					expression.swapWithTop( 1 );
				}
				else
				{
					// can still do it in one opcode
					
					WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
					unsigned char pos = expression.context[first].stackPosition + 1;
					WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
					pos = 2;
					WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
					
					expression.swapWithTop( 1, false );
					expression.swapWithTop( expression.context[first].stackPosition, false );
				}
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
bool WRCompilationContext::operatorFound( WRstr const& token, WRarray<WRExpressionContext>& context, int depth )
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

			if ( !m_quoted && token2 == ")" )
			{
				break;
			}

			++argsPushed;

			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;
			m_loadedToken = token2;
			m_loadedValue = value2;
			m_loadedQuoted = m_quoted;
			
			char end = parseExpression( nex );

			if ( nex.bytecode.opcodes.size() > 0 )
			{
				uint8_t op = nex.bytecode.opcodes[nex.bytecode.opcodes.size() - 1];
				if ( op == O_Index
					 || op == O_IndexSkipLoad
					 || op == O_IndexLiteral8
					 || op == O_IndexLiteral16
					 || op == O_IndexLocalLiteral8
					 || op == O_IndexGlobalLiteral8
					 || op == O_IndexLocalLiteral16
					 || op == O_IndexGlobalLiteral16 )
				{
					nex.bytecode.opcodes += O_Dereference;
					nex.bytecode.all += O_Dereference;
				}
			}
			
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

	pushDebug( WRD_LineNumber, expression.context[depth].bytecode, getSourcePosition() );
	pushDebug( WRD_FunctionCall, expression.context[depth].bytecode, WRD_ExternalFunction );

	if ( prefix.size() )
	{
		prefix += "::";
		prefix += functionName;

		unsigned char buf[4];
		wr_pack32( wr_hashStr(prefix), buf );

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

		if ( hash == wr_hashStr("yield") )
		{
			pushOpcode( expression.context[depth].bytecode, O_Yield );
			pushData( expression.context[depth].bytecode, &argsPushed, 1 );
		}
		else
		{
			pushOpcode( expression.context[depth].bytecode, O_FUNCTION_CALL_PLACEHOLDER );
			pushData( expression.context[depth].bytecode, &argsPushed, 1 );
			pushData( expression.context[depth].bytecode, "XXXX", 4 ); // TBD opcode plus index, OR hash if index was not found
		}
		
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

	bool byHash = false;
	bool byOrder = false;
	
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

	if ( !m_quoted && token2 == "{" )
	{
		unsigned char offset = 0;
		for(;;)
		{
			WRExpression nex( localSpace, context.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;

			m_parsingNew = true;

			bool v = m_needVar;
			m_needVar = false;
			char end = parseExpression( nex );
			m_needVar = v;

			m_parsingNew = false;

			if ( nex.bytecode.all.size() )
			{
				appendBytecode( context.bytecode, nex.bytecode );

				if ( m_newHashValue )
				{
					if ( byOrder )
					{
						m_err = WR_ERR_new_assign_by_label_or_offset_not_both;
						return false;
					}
					byHash = true;
					
					pushOpcode( context.bytecode, O_AssignToObjectTableByHash );
					uint8_t dat[4];
					pushData( context.bytecode, wr_pack32(m_newHashValue, dat), 4 );
				}
				else
				{
					if ( byHash )
					{
						m_err = WR_ERR_new_assign_by_label_or_offset_not_both;
						return false;
					}
					byOrder = true;

					pushOpcode( context.bytecode, O_AssignToObjectTableByOffset );
					pushData( context.bytecode, &offset, 1 );
				}
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
int WRCompilationContext::parseInitializer( WRExpression& expression, int depth )
{
	bool inHashTable = false;
	bool inArray = false;
	
	// start a value ... was something declared in front of us?
	if ( depth == 3
		 && expression.context[0].type == EXTYPE_LABEL
		 && expression.context[1].type == EXTYPE_OPERATION
		 && expression.context[1].operation->opcode
		 && expression.context[1].operation->opcode == O_Index )
	{
		expression.context.remove( depth - 2, 1 );
		depth -= 1;
	}

	WRValue& value = expression.context[depth].value;
	WRstr& token = expression.context[depth].token;

	expression.context[depth].type = EXTYPE_BYTECODE_RESULT;

	pushOpcode( expression.context[depth].bytecode, O_LiteralZero ); // push the value we're going to create

	// knock off the initial create
	uint16_t initializer = 0;
	unsigned char idat[2];
	for(;;)
	{
		if ( !getToken(expression.context[depth]) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return -1;
		}

		bool subTableValue = false;

		if ( token == '{' )
		{
			subTableValue = true;
			if ( parseInitializer(expression, depth) == -1 )
			{
				return -1;
			}

			pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
			wr_pack16( initializer++, idat );
			pushData( expression.context[depth].bytecode, idat, 2 );

			continue;
		}
	
		if ( token == ',' )
		{
			if ( initializer == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return -1;
			}
		}
		else
		{
			m_loadedToken = token;
			m_loadedValue = value;
			m_loadedQuoted = m_quoted;
		}

		WRExpression val( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
		val.context[0].token = token;
		val.context[0].value = value;

		char end = parseExpression( val );
		if ( end == ':' )
		{
			bool nullKey = false;
			
			// we are in a hash table!
			inHashTable = true;
			if ( inArray )
			{
				m_err = WR_ERR_hash_declaration_in_array;
				return -1;
			}

			if ( !val.bytecode.all.size() )
			{
				nullKey = true;
				pushOpcode( val.bytecode, O_LiteralZero );
			}
			
			if ( subTableValue ) // sub-table keys don't make sense
			{
				m_err = WR_ERR_hash_table_invalid_key;
				return -1;
			}

			appendBytecode( expression.context[depth].bytecode, val.bytecode );
			
			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return -1;
			}

			if ( token == '{' )
			{
				if ( parseInitializer(expression, depth) == -1 )
				{
					return -1;
				}

				pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
				continue;
			}
			else
			{
				m_loadedToken = token;
				m_loadedValue = value;
				m_loadedQuoted = m_quoted;
				
				WRExpression key( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
				key.context[0].token = token;
				key.context[0].value = value;
				end = parseExpression( key );
				
				if ( key.bytecode.all.size() ) // nor do null keys
				{
					if ( nullKey )
					{
						m_err = WR_ERR_hash_table_invalid_key;
						return -1;
					}
					
					appendBytecode( expression.context[depth].bytecode, key.bytecode );
					pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
				}
				else
				{
					if ( nullKey )
					{
						// null key AND value means null table, just create a zero-size table, by indexing as
						// hash, which creates the table, then removing the entry we just added, nulling it
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_Remove );
					}
					else
					{
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
					}
				}
			}
		}
		else
		{
			if ( val.bytecode.all.size() )
			{
				inArray = true;
				if ( inHashTable )
				{
					m_err = WR_ERR_array_declaration_in_hash;
					return -1;
				}

				appendBytecode( expression.context[depth].bytecode, val.bytecode );
				pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
				wr_pack16( initializer++, idat );
				pushData( expression.context[depth].bytecode, idat, 2 );
			}
		}
		
		if ( end == '}' )
		{
			break;
		}
		else if ( end == ',' )
		{
			continue;
		}
		else
		{
			m_err = WR_ERR_bad_expression;
			return -1;
		}
	}

	return depth;
}


//------------------------------------------------------------------------------
bool WRCompilationContext::parseUnit( bool isStruct, int parentUnitIndex )
{
	int previousIndex = m_unitTop;
	m_unitTop = m_units.count();

	if ( parentUnitIndex && isStruct )
	{
		m_err = WR_ERR_struct_in_struct;
		return false;
	}

	bool isGlobal;

	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRstr prefix;
	bool isLibConstant;

	// get the function name
	if ( !getToken(ex)
		 || !isValidLabel(token, isGlobal, prefix, isLibConstant)
		 || isGlobal
		 || isLibConstant )
	{
		m_err = WR_ERR_bad_label;
		return false;
	}

	m_units[m_unitTop].exportNamespace = m_exportNextUnit;
	m_exportNextUnit = false;
	m_units[m_unitTop].name = token;
	m_units[m_unitTop].hash = wr_hash( token, token.size() );
	m_units[m_unitTop].bytecode.isStructSpace = isStruct;

	m_units[m_unitTop].parentUnitIndex = parentUnitIndex; // if non-zero, must be called from a new'ed structure!
	
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
			
			if ( !m_quoted && token == ")" )
			{
				break;
			}
			
			if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			++m_units[m_unitTop].arguments;

			// register the argument on the hash stack
			addLocalSpaceLoad( m_units[m_unitTop].bytecode,
							   token,
							   true,
							   true );

			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( !m_quoted && token == ")" )
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
		
	parseStatement( m_unitTop, '}', O_Return );

	if ( !m_units[m_unitTop].bytecode.opcodes.size()
		 || (m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_Return
			 && m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_ReturnZero) )
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_ReturnZero );
	}

	m_unitTop = previousIndex;

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseWhile( WROpcode opcodeToReturn )
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
	m_loadedQuoted = m_quoted;

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

	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
bool WRCompilationContext::parseDoWhile( WROpcode opcodeToReturn )
{
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int jumpToTop = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, jumpToTop );
	
	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
bool WRCompilationContext::parseForLoop( WROpcode opcodeToReturn )
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

		if ( !m_quoted && token == ";" )
		{
			break;
		}

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;
		m_loadedQuoted = m_quoted;

		char end = parseExpression( nex );

		if ( foreachPossible )
		{
			if ( foreachLoadI < 3 )
			{
				if (nex.bytecode.opcodes.size() == 1
					  && nex.bytecode.all.size() == 2
					  && ((nex.bytecode.all[0] == O_LoadFromLocal) || (nex.bytecode.all[0] == O_LoadFromGlobal)) )
				{
					foreachLoad[foreachLoadI++] = nex.bytecode.all[0];
					foreachLoad[foreachLoadI++] = nex.bytecode.all[1];
				}
				else if ( nex.bytecode.opcodes.size() == 2
						  && nex.bytecode.all.size() == 5
						  && nex.bytecode.all[0] == O_DebugInfo
						  && ((nex.bytecode.all[3] == O_LoadFromLocal) || (nex.bytecode.all[3] == O_LoadFromGlobal)) )
				{
					foreachLoad[foreachLoadI++] = nex.bytecode.all[3];
					foreachLoad[foreachLoadI++] = nex.bytecode.all[4];
				}
				else
				{
					foreachPossible = false;
				}
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
					g = (unsigned char)(addGlobalSpaceLoad(m_units[0].bytecode, T, true, true)); // #25 force "var seen" true since we are runtime adding the temporary ourselves

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
		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
			m_loadedQuoted = m_quoted;

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

			if ( !m_quoted && token == ")" )
			{
				break;
			}

			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;
			m_loadedToken = token;
			m_loadedValue = value;
			m_loadedQuoted = m_quoted;

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
		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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

		if ( !m_quoted && token == "}" )
		{
			break;
		}
		else if ( !m_quoted && token == "," )
		{
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		bool isLibConstant;
		if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
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

		if ( !m_quoted && token == "=" )
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
			m_loadedQuoted = m_quoted;

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
			m_err = WR_ERR_constant_redefined;
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
		m_loadedQuoted = m_quoted;
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
	
	if ( !m_quoted && token == "(" )
	{
		return getSingleValueHash( ")" );
	}

	if ( value.type == WR_REF )
	{
		m_err = WR_ERR_switch_bad_case_hash;
		return 0;
	}
	
	uint32_t hash = ((uint8_t)value.type == WR_COMPILER_LITERAL_STRING) ? wr_hashStr(token) : value.getHash();

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
bool WRCompilationContext::parseSwitch( WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
	
	WRExpression selectionCriteria( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	selectionCriteria.context[0].token = token;
	selectionCriteria.context[0].value = value;
	
	if ( parseExpression(selectionCriteria) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, selectionCriteria.bytecode );


	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	WRarray<WRSwitchCase> cases;
	int16_t defaultOffset = -1;
	WRSwitchCase* swCase = 0; // current case

	int defaultCaseJumpTarget = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int selectionLogicPoint = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, selectionLogicPoint );

	unsigned int startingBytecodeMarker = m_units[m_unitTop].bytecode.all.size();
	
	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if( !m_quoted && token == "}" )
		{
			break;
		}

		if ( !m_quoted && token == "case" )
		{
			swCase = &cases.append();
			swCase->jumpOffset = m_units[m_unitTop].bytecode.all.size();
			swCase->hash = getSingleValueHash(":");
			if ( m_err )
			{
				return false;
			}

			swCase->occupied = true;
			swCase->defaultCase = false;

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
		else if ( !m_quoted && token == "default" )
		{
			if ( defaultOffset != -1 )
			{
				m_err = WR_ERR_switch_duplicate_case;
				return false;
			}

			if ( !getToken(ex, ":") )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			defaultOffset = m_units[m_unitTop].bytecode.all.size();
			setRelativeJumpTarget( m_units[m_unitTop].bytecode, defaultCaseJumpTarget );
		}
		else
		{
			if ( swCase == 0 && defaultOffset == -1 )
			{
				m_err = WR_ERR_switch_case_or_default_expected;
				return false;
			}

			m_loadedToken = token;
			m_loadedValue = value;
			m_loadedQuoted = m_quoted;

			if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
			{
				return false;
			}
		}
	}

	if ( startingBytecodeMarker == m_units[m_unitTop].bytecode.all.size() )
	{
		// no code was added so this is one big null operation, go
		// ahead and null it
		m_units[m_unitTop].bytecode.all.shave(3);
		m_units[m_unitTop].bytecode.opcodes.clear();
			
		pushOpcode(m_units[m_unitTop].bytecode, O_PopOne); // pop off the selection criteria

		m_units[m_unitTop].bytecode.jumpOffsetTargets.pop();
		m_breakTargets.pop();
		return true;
	}
	
	// make sure the last instruction is a break (jump) so the
	// selection logic is skipped at the end of the last case/default
	if ( !m_units[m_unitTop].bytecode.opcodes.size() 
		 || (m_units[m_unitTop].bytecode.opcodes.size() && m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_RelativeJump) )
	{
		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, *m_breakTargets.tail() );
	}

	// selection logic jumps HERE
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, selectionLogicPoint );

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

	if ( size < 254 ) // cases are labeled 0-254, just use a linear jump table
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_SwitchLinear );

		packbuf[0] = size;
		pushData( m_units[m_unitTop].bytecode, packbuf, 1 ); // size

		int currentPos = m_units[m_unitTop].bytecode.all.size();

		if ( defaultOffset == -1 )
		{
			defaultOffset = size*2 + 2;
		}
		else
		{
			defaultOffset -= currentPos;
		}

		table = (WRSwitchCase *)g_malloc(size * sizeof(WRSwitchCase));
		memset( table, 0, size*sizeof(WRSwitchCase) );

		for( unsigned int i = 0; i<size; ++i ) // for each of the possible entries..
		{
			for( unsigned int hash = 0; hash<cases.count(); ++hash ) // if a hash matches it, populate that table entry
			{
				if ( cases[hash].occupied && !cases[hash].defaultCase && (cases[hash].hash == i) )
				{
					table[cases[hash].hash].jumpOffset = cases[hash].jumpOffset - currentPos;
					table[cases[hash].hash].occupied = true;
					break;
				}
			}
		}

		pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );

		for( unsigned int i=0; i<size; ++i )
		{
			if ( table[i].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(table[i].jumpOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );
			}
		}
	}
	else
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_Switch ); // add switch command
		unsigned char packbuf[4];

		int currentPos = m_units[m_unitTop].bytecode.all.size();

		// find a suitable mod
		uint16_t mod = 1;
		for( ; mod<0x7FFE; ++mod )
		{
			table = (WRSwitchCase *)g_malloc(mod * sizeof(WRSwitchCase));
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
				table[cases[c].hash % mod].jumpOffset = cases[c].jumpOffset - currentPos;
				table[cases[c].hash % mod].occupied = true;
			}

			if ( c >= cases.count() )
			{
				break;
			}
			else
			{
				g_free( table );
				table = 0;
			} 
		}

		if ( mod >= 0x7FFE )
		{
			m_err = WR_ERR_switch_construction_error;
			return false;
		}

		if ( defaultOffset == -1 )
		{
			defaultOffset = mod*6 + 4;
		}
		else
		{
			defaultOffset -= currentPos;
		}

		pushData( m_units[m_unitTop].bytecode, wr_pack16(mod, packbuf), 2 ); // mod value
		pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 ); // default offset

		for( uint16_t m = 0; m<mod; ++m )
		{
			pushData( m_units[m_unitTop].bytecode, wr_pack32(table[m].hash, packbuf), 4 );

			if ( !table[m].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(table[m].jumpOffset, packbuf), 2 );
			}
		}
	}

	g_free( table );

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	m_breakTargets.pop();

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseIf( WROpcode opcodeToReturn )
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
	m_loadedQuoted = m_quoted;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	int conditionFalseMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, conditionFalseMarker );

	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
	{
		return false;
	}

 	if ( !getToken(ex) )
	{
		setRelativeJumpTarget(m_units[m_unitTop].bytecode, conditionFalseMarker);
	}
	else if ( !m_quoted && token == "else" )
	{
		int conditionTrueMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode ); // when it hits here it will jump OVER this section

		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionTrueMarker );
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );

		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
		{
			return false;
		}
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionTrueMarker );
	}
	else
	{
		m_loadedToken = token;
		m_loadedValue = value;
		m_loadedQuoted = m_quoted;
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );
	}

	resolveRelativeJumps( m_units[m_unitTop].bytecode ); // at least do the ones we added

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseStatement( int unitIndex, char end, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	bool varSeen = false;
	m_exportNextUnit = false;

	for(;;)
	{
		WRstr& token = ex.token;
//		WRValue& value = ex.value;

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

		if ( m_exportNextUnit
			 && token != "function"
			 && token != "struct"
			 && token != "unit" )
		{
			m_err = WR_ERR_unexpected_export_keyword;
			break;
		}
		
		if ( !m_quoted && token == "{" )
		{
			return parseStatement( unitIndex, '}', opcodeToReturn );
		}

		if ( !m_quoted && token == "return" )
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( !m_quoted && token == ";" ) // special case of a null return, add the null
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
				m_loadedQuoted = m_quoted;

				if ( parseExpression( nex ) != ';')
				{
					m_err = WR_ERR_unexpected_token;
					return false;
				}

				appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			}

			pushDebug( WRD_LineNumber, m_units[m_unitTop].bytecode, getSourcePosition() );
			pushOpcode( m_units[unitIndex].bytecode, opcodeToReturn );
		}
		else if ( !m_quoted && token == "struct" )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}

			if ( !parseUnit(true, unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && (token == "function" || token == "unit") )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}
			
			if ( !parseUnit(false, unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "if" )
		{
			if ( !parseIf(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "while" )
		{
			if ( !parseWhile(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "for" )
		{
			if ( !parseForLoop(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "enum" )
		{
			if ( !parseEnum(unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "export" )
		{
			m_exportNextUnit = true;
			continue;
		}
		else if ( !m_quoted && token == "switch" )
		{
			if ( !parseSwitch(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "do" )
		{
			if ( !parseDoWhile(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "break" )
		{
			if ( !m_breakTargets.count() )
			{
				m_err = WR_ERR_break_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_breakTargets.tail() );
		}
		else if ( !m_quoted && token == "continue" )
		{
			if ( !m_continueTargets.count() )
			{
				m_err = WR_ERR_continue_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_continueTargets.tail() );
		}
		else if ( !m_quoted && token == "goto" )
		{
			if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			bool isGlobal;
			WRstr prefix;
			bool isLibConstant;
			if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
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
/*
		else if ( !m_quoted && token == "var" )
		{
			if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			bool isGlobal;
			WRstr prefix;
			bool isLibConstant;
			if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isLibConstant )
			{
				m_err = WR_ERR_bad_goto_label;
				return false;
			}
			
			varSeen = true;
			goto parseAsVar;
		}
		else
		{
parseAsVar:
*/
		else
		{
			WRExpression nex( m_units[unitIndex].bytecode.localSpace, m_units[unitIndex].bytecode.isStructSpace );
			nex.context[0].varSeen = varSeen;
			nex.context[0].token = token;
			nex.context[0].value = ex.value;
			nex.lValue = true;
			m_loadedToken = token;
			m_loadedValue = ex.value;
			m_loadedQuoted = m_quoted;
			if ( parseExpression(nex) != ';' )
			{
				if ( !m_err )
				{
					m_err = WR_ERR_unexpected_token;
				}
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

	return m_err ? false : true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::checkAsComment( char lead )
{
	char c;
	if ( lead == '/' )
	{
		if ( !getChar(c) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( c == '/' )
		{
			while( getChar(c) && c != '\n' );
			return true;
		}
		else if ( c == '*' )
		{
			for(;;)
			{
				if ( !getChar(c) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return false;
				}

				if ( c == '*' )
				{
					if ( !getChar(c) )
					{
						m_err = WR_ERR_unexpected_EOF;
						return false;
					}

					if ( c == '/' )
					{
						break;
					}
				}
			}

			return true;
		}

		// else not a comment
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::readCurlyBlock( WRstr& block )
{
	char c;
	int closesNeeded = 0;

	for(;;)
	{
		if ( !getChar(c) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		// read past comments
		if ( checkAsComment(c) )
		{
			continue;
		}

		if ( m_err )
		{
			return false;
		}

		block += c;
		
		if ( c == '{' )
		{
			++closesNeeded;
		}
		
		if ( c == '}' )
		{
			if ( closesNeeded == 0 )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
			
			if ( !--closesNeeded )
			{
				break;
			}
		}
	}

	return true;
}


#else // WRENCH_WITHOUT_COMPILER

WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					char* errMsg,
					const uint8_t compilerOptionFlags )
{
	return WR_ERR_compiler_not_loaded;
}
	
#endif
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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

#define WR_DUMP_LINK_OUTPUT(D) //D
#define WR_DUMP_UNIT_OUTPUT(D) //D
#define WR_DUMP_BYTECODE(D) //D
//WRstr str;

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

	*buf = (unsigned char *)g_malloc( (offsets.m_mod * 5) + 4 );

	(*buf)[0] = (unsigned char)(unit.bytecode.localSpace.count() - unit.arguments);
	(*buf)[1] = unit.arguments;

	wr_pack16( offsets.m_mod, *buf + 2 );

	*size = 4;

	for( int i=0; i<offsets.m_mod; ++i )
	{
		wr_pack32( offsets.m_list[i].hash, *buf + *size );
		*size += 4;
		(*buf)[(*size)++] = offsets.m_list[i].hash ? offsets.m_list[i].value : (unsigned char)-1;
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::link( unsigned char** out, int* outLen, const uint8_t compilerOptionFlags )
{
	WROpcodeStream code;

	uint8_t data[4];

	NamespacePush *namespaceLookups = 0;

	unsigned int globals = m_units[0].bytecode.localSpace.count();
	assert( globals < 256 );

	code += (uint8_t)globals; // globals count (for VM allocation)
	code += (uint8_t)(m_units.count() - 1); // function count (for VM allocation)
	code += (uint8_t)compilerOptionFlags;

	// push function signatures
	for( unsigned int u=1; u<m_units.count(); ++u )
	{
		m_units[u].offsetInBytecode = code.size(); // mark these two spots for later

		// WRFunction.namespaceOffset
		data[0] = 0xAA;
		data[1] = 0xBB;
		code.append( data, 2 ); // placeholder

		// WRFunction.functionOffset
		data[0] = 0xCC;
		data[1] = 0xDD;
		code.append( data, 2 ); // placeholder

		// WRFunction.hash
		code.append( wr_pack32(m_units[u].hash, data), 4 );

		// WRFunction.arguments;
		data[0] = m_units[u].arguments;
		code.append( data, 1 );

		// WRFunction.frameSpaceNeeded;
		data[1] = m_units[u].bytecode.localSpace.count() - m_units[u].arguments;
		code.append( data + 1, 1 );

		// WRFunction.frameBaseAdjustment;
		data[2] = 1 + data[0] + data[1];
		code.append( data + 2, 1 );
	}

	m_units[0].name = "::global";

	if ( compilerOptionFlags & WR_INCLUDE_GLOBALS )
	{
		for( unsigned int i=0; i<globals; ++i )
		{
			code.append( wr_pack32(m_units[0].bytecode.localSpace[i].hash, data), 4 );
		}
	}

	if ( (compilerOptionFlags & WR_EMBED_DEBUG_CODE)
		 || (compilerOptionFlags & WR_EMBED_SOURCE_CODE) )
	{
		// hash of source compiled for this
		code.append( wr_pack32(wr_hash(m_source, m_sourceLen), data), 4 ); 

		WRstr symbols;

		uint16_t functions = m_units.count();
		symbols.append( (char*)wr_pack16(functions, data), 2 );

		data[0] = 0; // terminator
		for( unsigned int u=0; u<m_units.count(); ++u ) // all the local labels by unit
		{
			data[1] = m_units[u].bytecode.localSpace.count();
			data[2] = m_units[u].arguments;
			symbols.append( (char *)(data + 1), 2 );

			symbols.append( m_units[u].name );
			symbols.append( (char *)data, 1 ); 

			for( unsigned int s=0; s<m_units[u].bytecode.localSpace.count(); ++s )
			{
				symbols.append( m_units[u].bytecode.localSpace[s].label );
				symbols.append( (char *)data, 1 ); 
			}
		}

		uint16_t symbolSize = symbols.size();
		code.append( wr_pack16(symbolSize, data), 2 );
		code.append( (uint8_t*)symbols.p_str(), symbols.size() );
	}

	if ( compilerOptionFlags & WR_EMBED_SOURCE_CODE )
	{
		code.append( wr_pack32(m_sourceLen, data), 4 );
		code.append( (uint8_t*)m_source, m_sourceLen );
	}

	// export any explicitly marked for export or are referred to by a 'new'
	for( unsigned int ux=0; ux<m_units.count(); ++ux )
	{
		for( unsigned int f=0; f<m_units[ux].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[ux].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						m_units[u2].exportNamespace = true;
						break;
					}
				}
			}
		}
	}

	WR_DUMP_LINK_OUTPUT(printf("header funcs[%d] locals[%d] flags[0x%02X]:\n%s\n",
							   (unsigned char)(m_units.count() - 1),
							   (unsigned char)(m_units[0].bytecode.localSpace.count()),
							   (unsigned char)compilerOptionFlags, wr_asciiDump( code.p_str(), code.size(), str )));

	// append all the unit code
	for( unsigned int u=0; u<m_units.count(); ++u )
	{
		uint16_t base;

		if ( u > 0 ) // for the non-zero unit fill locations into the jump table
		{
			if ( m_units[u].exportNamespace )
			{
				base = code.size();

				int size = 0;
				uint8_t* map = 0;
				createLocalHashMap( m_units[u], &map, &size );
				
				if ( size == 0 )
				{
					base = 0;
				}
				m_units[u].offsetOfLocalHashMap = base;

				wr_pack16( base, code.p_str(m_units[u].offsetInBytecode)); // WRFunction.namespaceOffset

				WR_DUMP_LINK_OUTPUT(printf("<new> namespace\n%s\n", wr_asciiDump(map, size, str)));

				code.append( map, size );
				g_free( map );
			}
			else
			{
				base = 0;
				wr_pack16( base, code.p_str(m_units[u].offsetInBytecode) ); // WRFunction.namespaceOffset
			}

			base = code.size();
			wr_pack16( base, code.p_str(m_units[u].offsetInBytecode + 2) ); // WRFunction.functionOffset
		}

		WR_DUMP_UNIT_OUTPUT(printf("unit %d:\n%s\n", u, wr_asciiDump(code, code.size(), str)));

		base = code.size();

		// fill relative jumps in for the goto's
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
						wr_pack16( diff, m_units[u].bytecode.all.p_str(m_units[u].bytecode.gotoSource[g].offset + 1) );
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

		WR_DUMP_LINK_OUTPUT(printf("Adding Unit %d [%d]\n%s", u, m_units[u].bytecode.all.size(), wr_asciiDump(m_units[u].bytecode.all, m_units[u].bytecode.all.size(), str)));

		code.append( m_units[u].bytecode.all, m_units[u].bytecode.all.size() );

		WR_DUMP_LINK_OUTPUT(printf("->\n%s\n", wr_asciiDump( code.p_str(), code.size(), str )));

		// populate 'new' vectors
		for( unsigned int f=0; f<m_units[u].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						NamespacePush *n = (NamespacePush *)g_malloc(sizeof(NamespacePush));
						n->next = namespaceLookups;
						namespaceLookups = n;
						n->unit = u2;
						n->location = base + N.references[r];

						break;
					}
				}
			}
		}

		// load function table
		for( unsigned int x=0; x<m_units[u].bytecode.functionSpace.count(); ++x )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.functionSpace[x];

			WR_DUMP_LINK_OUTPUT(printf("function[%d] fixup before:\n%s\n", x, wr_asciiDump(code, code.size(), str)));

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				unsigned int u2 = 1;
				int index = base + N.references[r];

				for( ; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash ) // local function! call it, manage preserving retval
					{
						if ( m_addDebugSymbols )
						{
							// fill in the codeword with internal function number
							uint16_t codeword = (uint16_t)WRD_FunctionCall | (uint16_t)u2;
							wr_pack16( codeword, (unsigned char *)code.p_str(index - 2) );
						}

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

				if ( u2 >= m_units.count() ) // no local function found, rely on it being found run-time
				{
					if ( N.hash == wr_hashStr("yield") )
					{
						code[index] = O_Yield;
					}
					else
					{
						if ( code[index+5] == O_PopOne )
						{
							code[index] = O_CallFunctionByHashAndPop;
						}
						else
						{
							code[index] = O_CallFunctionByHash;
						}

						wr_pack32( N.hash, code.p_str(index+2) );
					}
				}
			}

			WR_DUMP_LINK_OUTPUT(printf("function[%d] fixup after:\n%s\n", x, wr_asciiDump(code, code.size(), str)));
		}
	}

	// plug in namespace lookups
	while( namespaceLookups )
	{
		wr_pack16( m_units[namespaceLookups->unit].offsetOfLocalHashMap, code.p_str(namespaceLookups->location) );
		NamespacePush* next = namespaceLookups->next;
		g_free( namespaceLookups );
		namespaceLookups = next;
	}

	// append a CRC
	uint32_t salted = wr_hash( code, code.size() );
	salted += WRENCH_VERSION_MAJOR;
	code.append( wr_pack32(salted, data), 4 );

	WR_DUMP_BYTECODE(printf("bytecode [%d]:\n%s\n", code.size(), wr_asciiDump(code, code.size(), str)));

	if ( !m_err )
	{
		*outLen = code.size();
		code.release( out );
	}
}

#endif
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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

//------------------------------------------------------------------------------
char WRCompilationContext::parseExpression( WRExpression& expression )
{
	int depth = 0;
	char end = 0;

	m_newHashValue = 0;

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
			if ( (uint8_t)value.type == WR_COMPILER_LITERAL_STRING )
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

		pushDebug( WRD_LineNumber, expression.bytecode, getSourcePosition() );
		
		if ( token == "var" )
		{
			expression.context[depth].varSeen = true;
			continue;
		}
		else if ( token == "{" )
		{
			depth = parseInitializer( expression, depth ) + 1;
			if ( depth == 0 )
			{
				return false;
			}

			continue;
		}
		
		if ( operatorFound(token, expression.context, depth) )
		{
			++depth;
			continue;
		}
		
		if ( !m_quoted && token == "new" )
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
			bool isLibConstant;

			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !isValidLabel(token2, isGlobal, prefix, isLibConstant) || isLibConstant )
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

			if ( !m_quoted && token2 == ";" )
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}

				m_loadedToken = ";";
				m_loadedValue.p2 = INIT_AS_REF;
				m_loadedQuoted = m_quoted;
			}
			else if ( !m_quoted && token2 == "(" )
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
					m_loadedQuoted = m_quoted;
				}
			}
			else if (!m_quoted && token2 == "{")
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}
				token2 = "{";
			}
			else if ( !m_quoted && token2 == "[" )
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

				if ( !getToken(expression.context[depth], "{") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				uint16_t initializer = 0;

				if ( !m_quoted && token3 == "{" )
				{
					for(;;)
					{
						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( !m_quoted && token3 == "}" )
						{
							break;
						}

						if ( !parseCallFunction(expression, functionName, depth, false) )
						{
							return 0;
						}

						if (!m_quoted && token3 != "{")
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
						wr_pack16( initializer, data );
						++initializer;

						pushData( expression.context[depth].bytecode, data, 2 );

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( !m_quoted && token3 == "," )
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
					m_loadedQuoted = m_quoted;
				}
				else if ( token2 != ";" )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

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

		if ( !m_quoted && token == "(" )
		{
			// might be cast, call or sub-expression
			
			if ( (depth > 0) &&
				 (expression.context[depth - 1].type == EXTYPE_LABEL
				 || expression.context[depth - 1].type == EXTYPE_LIB_CONSTANT) )
			{
				// always only a call
				expression.context[depth - 1].type = EXTYPE_LABEL;
				
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

				operatorFound( WRstr("@i"), expression.context, depth );
			}
			else if ( token == "float" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				operatorFound( WRstr("@f"), expression.context, depth );
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
				m_loadedQuoted = m_quoted;
				if ( parseExpression(nex) != ')' )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}


				if ( depth > 0 && expression.context[depth - 1].operation
					 && expression.context[depth - 1].operation->opcode == O_Remove )
				{
					--depth;
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( WRstr("._remove"), expression.context, depth );
				}
				else if ( depth > 0 && expression.context[depth - 1].operation
						  && expression.context[depth - 1].operation->opcode == O_HashEntryExists )
				{
					--depth;
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( WRstr("._exists"), expression.context, depth );
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

		if ( !m_quoted && token == "[" )
		{
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
				operatorFound( WRstr("@[]"), expression.context, depth );
				expression.context[depth].bytecode.all.clear();
				expression.context[depth].bytecode.opcodes.clear();
				pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
				pushOpcode( expression.context[depth].bytecode, O_InitArray );
			}
			else 
			{
				expression.context[depth].bytecode = nex.bytecode;
				operatorFound( WRstr("@[]"), expression.context, depth );
			}

			++depth;
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		bool isLibConstant;

		if ( isValidLabel(token, isGlobal, prefix, isLibConstant) )
		{
			if ( (depth > 0) 
				&& ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					|| (expression.context[depth - 1].type == EXTYPE_LITERAL)
					|| (expression.context[depth - 1].type == EXTYPE_LIB_CONSTANT)) )
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

				if ( m_parsingNew )
				{
					if ( token == "=" )
					{
						m_newHashValue = wr_hashStr( label );

						continue;
//						getToken(expression.context[depth]) );
//						--depth;
						
					}
				}
				
				if ( !m_quoted && token == ":" )
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
					m_loadedQuoted = m_quoted;
				}
			}
			

			expression.context[depth].type = isLibConstant ? EXTYPE_LIB_CONSTANT : EXTYPE_LABEL;
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

	if ( depth == 2
		 && expression.lValue
		 && expression.context[0].type == EXTYPE_LABEL
		 && expression.context[1].type == EXTYPE_OPERATION
		 && expression.context[1].operation->opcode == O_Index
		 && expression.context[1].bytecode.opcodes.size() > 0 )
	{
		unsigned int i = expression.context[1].bytecode.opcodes.size() - 1;
		unsigned int a = expression.context[1].bytecode.all.size() - 1;
		WROpcode o = (WROpcode)(expression.context[1].bytecode.opcodes[ i ]);
		
		switch( o )
		{
			case O_Index:
			{
				expression.context[1].bytecode.all[a] = O_InitArray;
				break;
			}

			case O_IndexLiteral16:
			{
				if (a > 1)
				{
					expression.context[1].bytecode.all[a - 2] = O_LiteralInt16;
					pushOpcode(expression.context[1].bytecode, O_InitArray);
				}
				break;
			}
			
			case O_IndexLiteral8:
			{
				if (a > 0)
				{
					expression.context[1].bytecode.all[a - 1] = O_LiteralInt8;
					pushOpcode(expression.context[1].bytecode, O_InitArray);
				}
				break;
			}

			default: break;
		}
	}

	resolveExpression( expression );

	return end;
}

#endif
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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER

#define KEYHOLE_OPTIMIZER

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
		bytecode.opcodes += opcode;
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
#ifdef KEYHOLE_OPTIMIZER
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
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareLE;
			return;
		}
		else if ( opcode == O_CompareEQ && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareEQ;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
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
			if ( o>1 )
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
					bytecode.opcodes.clear();
					return;
				}
				else if ( bytecode.opcodes[o] == O_LoadFromLocal
						  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
				{
					bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
					bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
					bytecode.all[ a - 2 ] = bytecode.all[ a ];
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
					return;
				}
				else if ( bytecode.opcodes[o] == O_LoadFromGlobal
						  && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
					bytecode.all[ a - 1 ] = bytecode.all[ a ];
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
					return;
				}
				else if ( bytecode.opcodes[o] == O_LoadFromLocal
						  && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 3 ] = O_LLBinaryMultiplication;
					bytecode.all[ a - 1 ] = bytecode.all[ a ];
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
					return;
				}
			}
			
			bytecode.all += opcode;
			bytecode.opcodes += opcode;
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
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
				bytecode.all[ a - 2 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
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
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
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
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareEQBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareEQBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareNEBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareNEBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
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
				bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_IncLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
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
					bytecode.opcodes.clear();
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_DecLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[ a - 3 ] = O_BinaryDivisionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[ a - 3 ] = O_BinaryAdditionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[ a - 3 ] = O_BinaryMultiplicationAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[ a - 3 ] = O_BinarySubtractionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_FUNCTION_CALL_PLACEHOLDER )
						{
							bytecode.all[a-2] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes[o] = O_AssignToGlobalAndPop;
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
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
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[ a - 3 ] = O_BinaryDivisionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[ a - 3 ] = O_BinaryAdditionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[ a - 3 ] = O_BinaryMultiplicationAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[ a - 3 ] = O_BinarySubtractionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToLocalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
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
				bytecode.opcodes.clear();
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
#endif
	bytecode.all += opcode;
	bytecode.opcodes += opcode;
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
		bytecode.opcodes.clear();
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
		bytecode.opcodes.clear();
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
		bytecode.opcodes.clear();
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
		bytecode.opcodes.clear();
		bytecode.opcodes += O_IndexGlobalLiteral8;
		return;
	}
	else if ( bytecode.all.size() > 0
			  && bytecode.opcodes.size() > 0
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

				bytecode.opcodes.clear();
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

				bytecode.opcodes.clear();
				bytecode.all.shave(1);
			}
			else if (bytecode.opcodes.size() > 1
					 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
					 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_StackSwap )
			{
				int a = bytecode.all.size() - 7;

				bytecode.all[a] = O_StackIndexHash;

				bytecode.opcodes.clear();
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


#endif
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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
int wr_strnlen( const char* s, int len )
{
	const char* found = (const char *)memchr( s, '\0', len );
	return found ? (found - s) : len;
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
		m_quoted = m_loadedQuoted;
	}
	else
	{
		m_quoted = false;
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
			int len = wr_strnlen( c_operations[t].token, 20 );
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
				m_quoted = true;
				
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
				else if (token[0] == '0' && isdigit(m_source[m_pos]) ) // octal
				{
					token.clear();

					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( !isdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					value.p2 = INIT_AS_INT;
					value.i = strtol( token, 0, 8 );
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
	m_loadedQuoted = m_quoted;
	m_loadedValue.p2 = INIT_AS_REF;

	if ( expect && (token != expect) )
	{
		return false;
	}

	return true;
}

#endif
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

#include "wrench.h"

//------------------------------------------------------------------------------
int WRGCObject::init( const unsigned int size, const WRGCObjectType type, bool clear )
{
	int ret = (m_size = size);

	if ( (m_type = type) == SV_VALUE )
	{
		ret *= sizeof(WRValue);
		m_Vdata = (WRValue*)g_malloc( ret );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_Vdata )
		{
			m_size = 0;
			g_mallocFailed = true;
			return 0;
		}
#endif
		
		if ( clear )
		{
			memset( m_SCdata, 0, ret );
		}
	}
	else if ( m_type == SV_CHAR )
	{
		m_Cdata = (unsigned char*)g_malloc( size );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_Cdata )
		{
			m_size = 0;
			g_mallocFailed = true;
			return 0;
		}
#endif
		if ( clear )
		{
			memset( m_SCdata, 0, size );
		}
	}
	else
	{
		growHash( WRENCH_NULL_HASH, size, &ret );
	}

	return ret;
}

//------------------------------------------------------------------------------
WRValue* WRGCObject::getAsRawValueHashTable( const uint32_t hash, int* index )
{
#ifdef WRENCH_COMPACT
	int i = getIndexOfHit( hash, false );
#else
	int i = hash % m_mod;
	if ( m_hashTable[i] != hash )
	{
		if ( m_hashTable[(i = (i + 1) % m_mod)] != hash )
		{
			if ( m_hashTable[(i = (i + 1) % m_mod)] != hash )
			{
				if ( m_hashTable[(i = (i + 1) % m_mod)] != hash )
				{
					i = getIndexOfHit(hash, true);
				}
			}
		}
	}
#endif

	if ( index ) { *index = i; }
	return m_Vdata + i;
}

//------------------------------------------------------------------------------
WRValue* WRGCObject::exists( const uint32_t hash, bool removeIfPresent )
{
	uint32_t index = hash % m_mod;

	if ( m_hashTable[index] != hash )
	{
		int tries = 3;
		do
		{
			index = (index + 1) % m_mod;
			if ( m_hashTable[index] == hash )
			{
				goto foundExists;
			}

		} while( tries-- );

		return 0;
	}

foundExists:

	if ( removeIfPresent )
	{
		--m_size;
		m_hashTable[index] = WRENCH_NULL_HASH;
	}

	return m_Vdata + index;
}

//------------------------------------------------------------------------------
void* WRGCObject::get( const uint32_t l, int* index )
{
	int s = l < m_size ? l : m_size - 1;
	void* ret = m_Vdata + s;

	if ( m_type == SV_CHAR )
	{
		ret = m_Cdata + s;
	}
	else if ( m_type == SV_HASH_TABLE )
	{
		s = getIndexOfHit(l, false) << 1;
		ret = m_Vdata + s;
	}
	else if ( m_type == SV_VOID_HASH_TABLE )
	{
		ret = getAsRawValueHashTable( l, index );
	}
	// else it must be SV_VALUE

	if ( index ) { *index = s; }
	return ret;
}

//------------------------------------------------------------------------------
uint32_t WRGCObject::getIndexOfHit( const uint32_t hash, const bool inserting )
{
	uint32_t index = hash % m_mod;
	if ( m_hashTable[index] == hash )
	{
		return index; // immediate hits should be cheap
	}

	int tries = 3;
	do
	{
		if ( inserting && m_hashTable[index] == WRENCH_NULL_HASH )
		{
			++m_size;
			m_hashTable[index] = hash;
			return index;
		}

		index = (index + 1) % m_mod;

		if ( m_hashTable[index] == hash )
		{
			return index;
		}

	} while( tries-- );

	return inserting ? growHash(hash) : getIndexOfHit( hash, true );
}

//------------------------------------------------------------------------------
uint32_t WRGCObject::growHash( const uint32_t hash, const uint16_t sizeHint, int* sizeAllocated )
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

		newSize *= sizeof(WRValue);
		newSize += sizeof(WRGCBase);
	
		int total = newMod*sizeof(uint32_t) + newSize;
		if ( sizeAllocated )
		{
			*sizeAllocated = total;
		}

		WRGCBase* base = (WRGCBase*)g_malloc( total );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !base )
		{
			g_mallocFailed = true;
			return 0; // congradulations, clobber this one. we're dying it doesn't matter.
		}
#endif

		memset( (unsigned char *)base, 0, total );

		uint32_t* proposed = (uint32_t *)((char*)base + newSize);

		for( int n = 0; n<newMod; ++n )
		{
			proposed[n] = WRENCH_NULL_HASH;
		}

		proposed[ hash % newMod ] = hash;

		for( unsigned int h=0; h<m_mod; ++h )
		{
			int tries = 3;
			int newEntry = m_hashTable[h] % newMod;
			for(;;)
			{
				if ( proposed[newEntry] == WRENCH_NULL_HASH )
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
					g_free( base );
					++t;

					assert( (unsigned int)newMod != 49157 );

					goto tryAgain;
				}
			}
		}

		// accepted, link it into the existing table
		base->m_type = SV_HASH_INTERNAL;

		base->m_nextGC = m_nextGC;
		m_nextGC = base;
		
		WRValue* newValues = (WRValue*)(base + 1);

		uint32_t* oldHashTable = m_hashTable;
		m_hashTable = proposed;
		int oldMod = m_mod;
		m_mod = newMod;

		for( int v=0; v<oldMod; ++v )
		{
			if ( oldHashTable[v] == WRENCH_NULL_HASH )
			{
				continue;
			}

			unsigned int newPos = getIndexOfHit( oldHashTable[v], true );

			if ( m_type == SV_VOID_HASH_TABLE )
			{
				newValues[newPos] = m_Vdata[v];
				m_Vdata[v].r = newValues + newPos;
				m_Vdata[v].p2 = INIT_AS_REF;
			}
			else
			{
				// copy all the new hashes to their new locations
				WRValue* to = newValues + (newPos<<1);
				WRValue* from = m_Vdata + (v<<1);

				// value
				to->p2 = from->p2;
				to->p = from->p;
				from->p2 = INIT_AS_REF;
				from->r = to;

				// key
				++to;
				++from;
				to->p2 = from->p2;
				to->p = from->p;
				from->p2 = INIT_AS_REF;
				from->r = to;
			}
		}

		m_Vdata = newValues;

		return getIndexOfHit( hash, true );
	}
}
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

#include "wrench.h"

//------------------------------------------------------------------------------
void WRContext::mark( WRValue* s )
{
	if ( IS_CONTAINER_MEMBER(s->xtype) && IS_EXARRAY_TYPE(s->r->xtype) )
	{
		// we don't mark this type, but we might mark it's target
		mark( s->r );
		return;
	}

	if ( !IS_EXARRAY_TYPE(s->xtype) || (s->va->m_flags & GCFlag_Marked) )
	{
		return;
	}

	assert( !IS_RAW_ARRAY(s->xtype) );

	WRGCBase* svb = s->vb;

	if ( svb->m_type == SV_VALUE )
	{
		WRValue* top = ((WRGCObject*)svb)->m_Vdata + ((WRGCObject*)svb)->m_size;

		for( WRValue* V = ((WRGCObject*)svb)->m_Vdata; V<top; ++V )
		{
			mark( V );
		}
	}
	else if ( svb->m_type == SV_HASH_TABLE )
	{
		// hash table points one WRGCBase size PAST the actual pointer,
		// recover it and mark it
		((WRGCBase*)(((WRGCObject*)svb)->m_Vdata) - 1)->m_flags |= GCFlag_Marked;

		for( uint32_t i=0; i<((WRGCObject*)svb)->m_mod; ++i )
		{
			if ( ((WRGCObject*)svb)->m_hashTable[i] != WRENCH_NULL_HASH )
			{
				uint32_t item = i<<1;
				mark( ((WRGCObject*)svb)->m_Vdata + item++ );
				mark( ((WRGCObject*)svb)->m_Vdata + item );
			}
		}
	}
	else if ( svb->m_type == SV_HASH_ENTRY )
	{
		// mark the referenced table so it is not collected
		((WRGCHashReference*)svb)->m_referencedTable->m_flags |= GCFlag_Marked;
	}

	svb->m_flags |= GCFlag_Marked;
}

//------------------------------------------------------------------------------
void WRContext::gc( WRValue* stackTop )
{
	if ( allocatedMemoryHint < allocatedMemoryLimit )
	{
		return;
	}

	allocatedMemoryHint = 0;

	// mark stack
	for( WRValue* s=stack; s<stackTop; ++s)
	{
		// an array in the chain?
		mark( s );
	}

	// mark context's globals
	WRValue* globalSpace = (WRValue *)(this + 1); // globals are allocated directly after this context

	for( unsigned int i=0; i<globals; ++i, ++globalSpace )
	{
		mark( globalSpace );
	}

	// sweep
	WRGCBase* current = svAllocated;
	WRGCBase* prev = 0;
	while( current )
	{
		// if set, clear it
		if ( current->m_flags & GCFlag_Marked )
		{
			current->m_flags &= ~GCFlag_Marked;
			prev = current;
			current = current->m_nextGC;
		}
		// otherwise free it as unreferenced
		else
		{
			current->clear();

			if ( prev == 0 )
			{
				svAllocated = current->m_nextGC;
				g_free( current );
				current = svAllocated;
			}
			else
			{
				prev->m_nextGC = current->m_nextGC;
				g_free( current );
				current = prev->m_nextGC;
			}
		}
	}
}

//------------------------------------------------------------------------------
WRGCObject* WRContext::getSVA( int size, WRGCObjectType type, bool init )
{
	WRGCObject* ret = (WRGCObject*)g_malloc( sizeof(WRGCObject) );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !ret )
	{
		g_mallocFailed = true;
		return 0;
	}
#endif

	memset( (unsigned char*)ret, 0, sizeof(WRGCObject) );
	ret->m_nextGC = svAllocated;
	svAllocated = ret;

	allocatedMemoryHint += ret->init( size, type, init );

	if ( (int)type >= SV_VALUE )
	{
		ret->m_creatorContext = this;
	}

	return ret;
}

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

#include "wrench.h"

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
			if ( iterator->va->m_hashTable[element] != WRENCH_NULL_HASH )
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

		if ( iterator->va->m_type == SV_VALUE )
		{
			value->p2 = INIT_AS_REF;
			value->r = iterator->va->m_Vdata + element;
		}
		else if ( iterator->va->m_type == SV_CHAR )
		{
			value->p2 = INIT_AS_INT;
			value->i = iterator->va->m_Cdata[element];
		}
		else
		{
			return false;
		}

		iterator->p2 = INIT_AS_ITERATOR | ENCODE_ARRAY_ELEMENT_TO_P2(++element);
	}

	return true;
}


/*
static void dumpStack( const WRValue* bottom, const WRValue* top )
{
	WRstr out;
	wr_stackDump( bottom, top, out );
	printf( "stack:\n%s===================================\n", out.c_str() );
}
*/
//#define DEBUG_PER_INSTRUCTION { printf( "%s\n", c_opcodeName[(int)*pc] ); }

#define DEBUG_PER_INSTRUCTION

//------------------------------------------------------------------------------
#ifdef WRENCH_PROTECT_STACK_FROM_OVERFLOW
#define CHECK_STACK { if ( stackTop >= stackLimit ) { w->err = WR_ERR_stack_overflow; return 0; } }
#else
#define CHECK_STACK
#endif

//------------------------------------------------------------------------------
#ifdef WRENCH_HANDLE_MALLOC_FAIL
  bool g_mallocFailed = false;
  #define MALLOC_FAIL_CHECK {if( g_mallocFailed ) { w->err = WR_ERR_malloc_failed; g_mallocFailed = false; return 0; } }
#else
  #define MALLOC_FAIL_CHECK
#endif

//------------------------------------------------------------------------------
#ifdef WRENCH_TIME_SLICES

//------------------------------------------------------------------------------
void wr_setInstructionsPerSlice( WRState* w, const int instructions )
{
	w->instructionsPerSlice = instructions;
}

//------------------------------------------------------------------------------
void wr_forceYield( WRState* w )
{
	w->yieldEnabled = true; // prevent a race in case the count is decremented before this flag is checked
	w->sliceInstructionCount = 1;
}

#define CHECK_FORCE_YIELD { if ( !--w->sliceInstructionCount && w->yieldEnabled ) { context->yieldArgs = 0; context->flags |= (uint8_t)WRC_ForceYielded; goto doYield; } }
#else
#define CHECK_FORCE_YIELD
#endif

//------------------------------------------------------------------------------
#define WRENCH_JUMPTABLE_INTERPRETER
#ifdef WRENCH_REALLY_COMPACT
#undef WRENCH_JUMPTABLE_INTERPRETER
#elif !defined(__clang__)
#if _MSC_VER
#undef WRENCH_JUMPTABLE_INTERPRETER
#endif
#endif

//------------------------------------------------------------------------------
// FASTCONTINUE for when malloc could not have been called so no need to check
#ifdef WRENCH_JUMPTABLE_INTERPRETER
 #define CONTINUE { DEBUG_PER_INSTRUCTION; MALLOC_FAIL_CHECK; goto *opcodeJumptable[READ_8_FROM_PC(pc++)];  }
 #define FASTCONTINUE { DEBUG_PER_INSTRUCTION; goto *opcodeJumptable[READ_8_FROM_PC(pc++)];  }
 #define CASE(LABEL) LABEL
#else
 #define CONTINUE { DEBUG_PER_INSTRUCTION; MALLOC_FAIL_CHECK; continue; }
 #define FASTCONTINUE { DEBUG_PER_INSTRUCTION; continue; }
 #define CASE(LABEL) case O_##LABEL
#endif

#ifdef WRENCH_COMPACT

static float divisionF( float a, float b )
{
	return b ? a / b : 0.0f;
}
static float addF( float a, float b ) { return a + b; }
static float subtractionF( float a, float b ) { return a - b; }
static float multiplicationF( float a, float b ) { return a * b; }

static int divisionI( int a, int b )
{
	return b ? a / b : 0;
}
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
WRValue* wr_continue( WRContext* context )
{
	if ( !context->yield_pc )
	{
		context->w->err = WR_ERR_context_not_yielded;
		return 0;
	}

	return wr_callFunction( context, (WRFunction*)0, context->yield_argv, context->yield_argn );
}

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, WRFunction* function, const WRValue* argv, const int argn )
{
#ifdef WRENCH_JUMPTABLE_INTERPRETER
	const void* opcodeJumptable[] =
	{
		&&Yield,

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
		&&AssignToObjectTableByHash,

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
		&&InitVar,

		&&DebugInfo,
	};
#endif

	const uint8_t* pc;

	union
	{
		unsigned char findex;
		WRValue* register0;
		WRGCObject* va;
		const unsigned char *hashLoc;
		uint32_t hashLocInt;
	};

	union
	{
		WRValue* register1;
		WRContext* import;
	};
	register1 = 0;
	
	WRValue* frameBase = 0;
	WRState* w = context->w;
	const unsigned char* table = 0;
	
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

#ifdef WRENCH_TIME_SLICES
	if (w->instructionsPerSlice == 0)
	{
		w->yieldEnabled = false;
	}
	else
	{
		w->yieldEnabled = true;
		w->sliceInstructionCount = w->instructionsPerSlice;
	}
#endif

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

	WRValue* stackBase = context->stack + context->stackOffset;
	WRValue* stackTop;
#ifdef WRENCH_PROTECT_STACK_FROM_OVERFLOW
	WRValue* stackLimit = stackBase + (w->stackSize - 2); // one entry buffer, we SHOULD detect on every frame inc
#endif

	if ( context->yield_pc )
	{
		pc = context->yield_pc;
		context->yield_pc = 0;

		stackTop = context->yield_stackTop;
		
		if ( context->yieldArgs )
		{
			register0 = stackTop;
			*(stackTop -= context->yieldArgs) = *register0;
		}

		if ( context->flags & (uint8_t)WRC_ForceYielded )
		{
			context->flags &= ~(uint8_t)WRC_ForceYielded;
		}
		else
		{
			++stackTop; // otherwise we expect a return value
		}
	
		frameBase = context->yield_frameBase;
		
		goto yieldContinue;
	}
	else
	{
		stackTop = stackBase;
	}

	if ( function )
	{
		context->gc( stackTop );

		stackTop->p = 0;
		(stackTop++)->p2 = INIT_AS_INT;

		for( args = 0; args < argn; ++args )
		{
			*stackTop++ = argv[args];
		}
		
		pc = context->stopLocation;
		
		goto callFunction;
	}

	pc = context->codeStart;

yieldContinue:

	context->gc( stackTop );

#ifdef WRENCH_JUMPTABLE_INTERPRETER

	FASTCONTINUE;
	
#else

	for(;;)
	{
		switch( READ_8_FROM_PC(pc++) )
		{
#endif
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
				CHECK_STACK;
				FASTCONTINUE;
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
				
				stackTop->va = context->getSVA( hash, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
				if ( !stackTop->va )
				{
					CONTINUE; // don't need to do anything else, this will immediately return
				}
#endif
				stackTop->p2 = INIT_AS_ARRAY;

				for ( char* to = (char *)(stackTop++)->va->m_data ; hash ; --hash )
				{
					*to++ = READ_8_FROM_PC(pc++);
				}

				context->gc( stackTop );

				CHECK_STACK;
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
				CHECK_STACK;
				CONTINUE;
			}

			CASE(InitArray):
			{
				register0 = &(--stackTop)->singleValue();
				register1 = &(stackTop - 1)->deref();
				register1->p2 = INIT_AS_ARRAY;
				register1->va = context->getSVA( register0->ui, SV_VALUE, true );
				context->gc( stackTop );
				CONTINUE;
			}

			CASE(InitVar):
			{
				(stackTop - 1)->deref().init();
				FASTCONTINUE;
			}
			
			CASE(Yield):
			{
				// preserve the calling and stack context so the code
				// can pick up where it left off on continue
				context->yieldArgs = READ_8_FROM_PC(pc++);
#if defined(WRENCH_TIME_SLICES) || defined(WRENCH_INCLUDE_DEBUG_CODE)
doYield:
#endif
				context->yield_pc = pc;
				context->yield_stackTop = stackTop->init();
				context->yield_frameBase = frameBase;
				context->yield_argv = argv;
				context->yield_argn = argn;
				return 0;
			}

			CASE(DebugInfo):
			{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
				if ( context->debugInterface
					 && context->debugInterface->I->codewordEncountered(pc, READ_16_FROM_PC(pc), stackTop) )
				{
debugReturn:
					context->yieldArgs = 0;
					context->flags |= (uint8_t)WRC_ForceYielded;
					pc += 2;
					goto doYield;
				}
#endif
				pc += 2; // no debug code compiled in, just skip the directive
				CONTINUE;
			}

			CASE(CallFunctionByHash):
			{
				args = READ_8_FROM_PC(pc++);

				// initialize a return value of 'zero'
				register0 = stackTop->init();

				uint32_t fhash = READ_32_FROM_PC(pc);
				pc += 4;
				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(fhash))->ccb) )
				{
					if ( (import = context->imported) ) // check imported code
					{
						while( import != context )
						{
							WRValue* I;
							if ( (I = import->registry.exists(fhash, false)) )
							{
								// import shares our stack, tell it where to find it's args
								import->stackOffset = (stackTop - stackBase);
								wr_callFunction( import, I->wrf, stackTop - args, args );
								register0 = stackTop;

								if ( *pc == O_NewObjectTable )
								{
									if ( !I->wrf->namespaceOffset )
									{
										w->err = WR_ERR_struct_not_exported;
										return 0;
									}
									
									table = import->bottom + I->wrf->namespaceOffset;
									// so this was in service of a new
									// ObjectTable. no problemo, find
									// the actual table location and
									// use it

									if ( args ) // fix up the return value
									{
										stackTop -= (args - 1);
										*(stackTop - 1) = *register0;
									}
									else
									{
										++stackTop;
									}
									
									pc += 3;
									goto NewObjectTablePastLoad;
								}
								
								goto CallFunctionByHash_continue;
							}

							import = import->imported;
						}
					}

					w->err = WR_ERR_function_not_found;
					return 0;
				}
				else
				{
					register1->ccb( context, stackTop - args, args, *stackTop, register1->usr );
				}

				// DO care about return value, which will be at the top
				// of the stack
CallFunctionByHash_continue:
				
				if ( args )
				{
					stackTop -= (args - 1);
					*(stackTop - 1) = *register0;
				}
				else
				{
					++stackTop;
				}
				CHECK_STACK;
				CONTINUE;
			}

			CASE(CallFunctionByHashAndPop):
			{
				args = READ_8_FROM_PC(pc++);

				uint32_t fhash = READ_32_FROM_PC(pc);
				pc += 4;
				if ( !((register1 = w->globalRegistry.getAsRawValueHashTable(fhash))->ccb) )
				{
					// is in an imported context
					if ( (import = context->imported) )
					{
						while( import != context )
						{
							if ( (register0 = import->registry.exists(fhash, false)) )
							{
								// import shares our stack, tell it where to find it's args
								import->stackOffset = (stackTop - stackBase);
								wr_callFunction( import, register0->wrf, stackTop - args, args );
								goto CallFunctionByHashAndPop_continue;
							}

							import = import->imported;
						}
					}

					w->err = WR_ERR_function_not_found;
					return 0;
				}
				else
				{
					register1->ccb( context, stackTop - args, args, *stackTop, register1->usr );
				}

CallFunctionByHashAndPop_continue:
				stackTop -= args;
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
				
				// rectify arg count?
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
							CHECK_STACK;
						}
					}
				}

#ifdef WRENCH_PROTECT_STACK_FROM_OVERFLOW
				if ( stackTop + function->frameSpaceNeeded + 2 >= stackLimit )
				{
					w->err = WR_ERR_stack_overflow;
					return 0;
				}
#endif
				
				// initialize locals to int zero
				for( int l=0; l<function->frameSpaceNeeded; ++l )
				{
					(stackTop++)->p2 = INIT_AS_INT;
					stackTop->p = 0;
				}
			
				// temp value contains return vector/frame base
				register0 = stackTop++; // return vector
				register0->frame = frameBase;
				register0->returnOffset = pc - context->bottom; // can't use "pc" because it might set the "gc me" bit, this is guaranteed to be small enough

				pc = function->functionOffset + context->bottom;

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - function->frameBaseAdjustment;

				CHECK_STACK;
				CONTINUE;
			}

			CASE(PushIndexFunctionReturnValue):
			{
				*(stackTop++) = (++register0)->deref();
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(CallLibFunction):
			{
				stackTop->p2 = INIT_AS_INT;
				stackTop->p = 0;

				args = READ_8_FROM_PC(pc++); // which have already been pushed

				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb) )
				{
					w->err = WR_ERR_lib_function_not_found;
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
				if ( w->err )
				{
					return 0;
				}

				CHECK_STACK;
				CONTINUE;
			}

			CASE(CallLibFunctionAndPop):
			{
				args = READ_8_FROM_PC(pc++); // which have already been pushed

				if ( ! ((register1 = w->globalRegistry.getAsRawValueHashTable(READ_32_FROM_PC(pc)))->lcb) )
				{
					w->err = WR_ERR_lib_function_not_found;
					return 0;
				}

				register1->lcb( stackTop, args, context );
				pc += 4;

				stackTop -= args;

				if ( w->err )
				{
					return 0;
				}

				CONTINUE;
			}

			CASE(NewObjectTable):
			{
				table = context->bottom + READ_16_FROM_PC(pc);
				pc += 2;

				if ( table > context->bottom )
				{
NewObjectTablePastLoad:
					// if unit was called with no arguments from global
					// level there are no "free" stack entries to
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
					
#ifdef WRENCH_HANDLE_MALLOC_FAIL
					if ( !stackTop->va )
					{
						CONTINUE;
					}
#endif
					stackTop->va->m_ROMHashTable = table + 3;
					
					stackTop->va->m_mod = READ_16_FROM_PC(table+1);

					register0 = stackTop->va->m_Vdata;
					register0->r = register1;
					(register0++)->r2 = register2;

					if ( --count > 0 )
					{
						memcpy( (char*)register0, stackTop + READ_8_FROM_PC(table) + 1, count*sizeof(WRValue) );
					}

					context->gc(++stackTop); // take care of any memory the 'new' allocated
				}
				else
				{
					goto literalZero;
				}

				FASTCONTINUE;
			}

			CASE(AssignToObjectTableByHash):
			{
				hash = READ_32_FROM_PC(pc);
				pc += 4;

				register1 = --stackTop;
				register0 = stackTop - 1;

				const unsigned char* table = register0->va->m_ROMHashTable + ((hash % register0->va->m_mod) * 5);
				
				if ( (uint32_t)READ_32_FROM_PC(table) == hash )
				{
					register2 = (((WRValue*)(register0->va->m_data)) + READ_8_FROM_PC(table + 4));
					wr_assign[register2->type<<2|register1->type]( register2, register1 );
				}
				else
				{
					w->err = WR_ERR_hash_not_found;
					return 0;
				}
				
				CONTINUE;
			}
			
			CASE(AssignToObjectTableByOffset):
			{
				register1 = --stackTop;
				register0 = (stackTop - 1);
				hash = READ_8_FROM_PC(pc++);
				if ( IS_EXARRAY_TYPE(register0->xtype) && (hash < register0->va->m_size) )
				{
					register0 = register0->va->m_Vdata + hash;
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
				context->gc( stackTop );
				CONTINUE;
			}
			
			CASE(Remove):
			{
				hash = (--stackTop)->getHash();
				register0 = &((stackTop - 1)->deref());
				if ( register0->xtype == WR_EX_HASH_TABLE )
				{
					register0->va->exists( hash, true );
				}
				else if ( register0->xtype == WR_EX_ARRAY )
				{
					va = register0->va;
					if (va->m_type == SV_VALUE )
					{
						for( uint32_t move = hash; move < va->m_size; ++move )
						{
							va->m_Vdata[move] = va->m_Vdata[move+1];
						}
					}
					else if ( register0->va->m_type == SV_CHAR )
					{
						for( uint32_t move = hash; move < va->m_size; ++move )
						{
							va->m_Cdata[move] = va->m_Cdata[move+1];
						}
					}

					--va->m_size;
				}
				FASTCONTINUE;	
			}

			CASE(HashEntryExists):
			{
				register0 = --stackTop;
				register1 = (stackTop - 1);
				register2 = &(register1->deref());
				register1->p2 = INIT_AS_INT;
				register1->i = ((register2->xtype == WR_EX_HASH_TABLE) && register2->va->exists(register0->getHash(), false)) ? 1 : 0;
				
				FASTCONTINUE;	
			}

			CASE(PopOne):
			{
				--stackTop;
				FASTCONTINUE;
			}

			CASE(ReturnZero):
			{
				(stackTop++)->init();
				CHECK_STACK;
			}
			CASE(Return):
			{
				register0 = stackTop - 2;

				pc = context->bottom + register0->returnOffset; // grab return PC

				stackTop = frameBase;
				frameBase = register0->frame;

#ifdef WRENCH_INCLUDE_DEBUG_CODE
				if ( context->debugInterface
					 && context->debugInterface->I->codewordEncountered(pc, WRD_Return, stackTop) )
				{
					goto debugReturn;
				}
#endif
				FASTCONTINUE;
			}

			CASE(ToInt):
			{
				register0 = stackTop - 1;
				register0->i = register0->asInt();
				register0->p2 = INIT_AS_INT;
				FASTCONTINUE;
			}

			CASE(ToFloat):
			{
				register0 = stackTop - 1;
				register0->f = register0->asFloat();
				register0->p2 = INIT_AS_FLOAT;
				FASTCONTINUE;
			}
			
			CASE(GlobalStop):
			{
				register0 = stackBase - 1;
				++pc;
			}			
			CASE(Stop):
			{
				*stackBase = *(register0 + 1);
				context->stopLocation = pc - 1;
#ifdef WRENCH_INCLUDE_DEBUG_CODE
				if ( context->debugInterface )
				{
					context->debugInterface->I->codewordEncountered( pc, WRD_FunctionCall | WRD_GlobalStopFunction, stackTop );
				}
#endif
				return &(stackBase)->deref();
			}

			CASE(Dereference):
			{
				register0 = (stackTop - 1);
				*register0 = register0->deref();
				FASTCONTINUE;
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
				FASTCONTINUE;
			}

			CASE(HashOf):
			{
				register0 = stackTop - 1;
				register0->ui = register0->getHash();
				register0->p2 = INIT_AS_INT;
				FASTCONTINUE;
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
#ifdef WRENCH_HANDLE_MALLOC_FAIL
					if ( !register0->va )
					{
						CONTINUE;
					}
#endif
				}

				context->gc( stackTop );

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

				FASTCONTINUE;
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

				FASTCONTINUE;
			}

			CASE(LoadFromLocal):
			{
				stackTop->p = frameBase + READ_8_FROM_PC(pc++);
				(stackTop++)->p2 = INIT_AS_REF;
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LoadFromGlobal):
			{
				stackTop->p = globalSpace + READ_8_FROM_PC(pc++);
 				(stackTop++)->p2 = INIT_AS_REF;
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LLValues): { register0 = frameBase + READ_8_FROM_PC(pc++); register1 = frameBase + READ_8_FROM_PC(pc++); FASTCONTINUE; }
			CASE(LGValues): { register0 = frameBase + READ_8_FROM_PC(pc++); register1 = globalSpace + READ_8_FROM_PC(pc++); FASTCONTINUE; }
			CASE(GLValues): { register0 = globalSpace + READ_8_FROM_PC(pc++); register1 = frameBase + READ_8_FROM_PC(pc++); FASTCONTINUE; }
			CASE(GGValues):	{ register0 = globalSpace + READ_8_FROM_PC(pc++); register1 = globalSpace + READ_8_FROM_PC(pc++); FASTCONTINUE; }

			CASE(BitwiseNOT):
			{
				register0 = stackTop - 1;
				register0->ui = wr_bitwiseNot[ register0->type ]( register0 );
				register0->p2 = INIT_AS_INT;
				FASTCONTINUE;
			}

			CASE(RelativeJump):
			{
				pc += READ_16_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(RelativeJump8):
			{
				pc += (int8_t)READ_8_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(BZ):
			{
				register0 = --stackTop;
				pc += wr_LogicalNot[register0->type](register0) ? READ_16_FROM_PC(pc) : 2;
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(BZ8):
			{
				register0 = --stackTop;
				pc += wr_LogicalNot[register0->type](register0) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(LogicalNot):
			{
				register0 = stackTop - 1;
				register0->i = wr_LogicalNot[ register0->type ]( register0 );
				register0->p2 = INIT_AS_INT;
				FASTCONTINUE;
			}

			CASE(Negate):
			{
				register0 = stackTop - 1;
				wr_negate[ register0->type ]( register0, register0 );
				FASTCONTINUE;
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
				CHECK_STACK;
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
				CHECK_STACK;
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
				CHECK_STACK;
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
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LiteralInt16):
			{
				stackTop->i = READ_16_FROM_PC(pc);
				pc += 2;
				(stackTop++)->p2 = INIT_AS_INT;
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LiteralInt8ToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->i = (int32_t)(int8_t)READ_8_FROM_PC(pc++);
				register0->p2 = INIT_AS_INT;
				FASTCONTINUE;
			}

			CASE(LiteralInt16ToGlobal):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				register0->i = READ_16_FROM_PC(pc);
				register0->p2 = INIT_AS_INT;
				pc += 2;
				FASTCONTINUE;
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
				FASTCONTINUE;
			}

			CASE(LiteralInt16ToLocal):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				register0->i = READ_16_FROM_PC(pc);
				register0->p2 = INIT_AS_INT;
				pc += 2;
				FASTCONTINUE;
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
			
			CASE(GPushIterator):
			{
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_pushIterator[register0->type]( register0, globalSpace + READ_8_FROM_PC(pc++) );
				FASTCONTINUE;
			}

			CASE(LPushIterator):
			{
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_pushIterator[register0->type]( register0, globalSpace + READ_8_FROM_PC(pc++) );
				FASTCONTINUE;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				CHECK_STACK;
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
			CASE(AddAssignAndPop): { floatCall = addF; intCall = wr_addI; goto binaryTableOpAndPop;	}
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
				CHECK_FORCE_YIELD;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(BLA8):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalAND[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(BLO):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : READ_16_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			CASE(BLO8):
			{
				register0 = --stackTop;
				register1 = --stackTop;
				pc += wr_LogicalOR[(register0->type<<2)|register1->type]( register0, register1 ) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}

			
			CASE(GGBinaryMultiplication):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(GLBinaryMultiplication):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LLBinaryMultiplication):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_MultiplyBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			
			CASE(GGBinaryAddition):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(GLBinaryAddition):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++); 
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LLBinaryAddition):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_AdditionBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			
			CASE(GGBinarySubtraction):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(GLBinarySubtraction):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LGBinarySubtraction):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LLBinarySubtraction):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
				wr_SubtractBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
				CHECK_STACK;
				FASTCONTINUE;
			}


			CASE(GGBinaryDivision):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop );
				if ( IS_INVALID(stackTop++->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
#endif
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(GLBinaryDivision):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = globalSpace + READ_8_FROM_PC(pc++);
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop );
				if ( IS_INVALID(stackTop++->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
#endif
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LGBinaryDivision):
			{
				register1 = globalSpace + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop );
				if ( IS_INVALID(stackTop++->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
#endif
				CHECK_STACK;
				FASTCONTINUE;
			}

			CASE(LLBinaryDivision):
			{
				register1 = frameBase + READ_8_FROM_PC(pc++);
				register0 = frameBase + READ_8_FROM_PC(pc++);
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop );
				if ( IS_INVALID(stackTop++->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				wr_DivideBinary[(register0->type<<2)|register1->type]( register0, register1, stackTop++ );
#endif
				CHECK_STACK;
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}
			
			CASE(CompareBNE):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? READ_16_FROM_PC(pc) : 2;
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				pc += returnFunc[(register0->type << 2) | register1->type](register0, register1) ? 2 : (int8_t)READ_8_FROM_PC(pc);
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
			}
			
			CASE(CompareBNE8):
			{
				returnFunc = wr_CompareEQ;
returnFuncBInverted8:
				register0 = --stackTop;
				register1 = --stackTop;
				pc += returnFunc[(register0->type<<2)|register1->type]( register0, register1 ) ? (int8_t)READ_8_FROM_PC(pc) : 2;
				CHECK_FORCE_YIELD;
				FASTCONTINUE;
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
				CHECK_STACK;
				FASTCONTINUE;
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
				CHECK_STACK;
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				FASTCONTINUE;
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
				CHECK_STACK;
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
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				if ( IS_INVALID(register0->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#endif
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
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				if ( IS_INVALID(register0->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#endif
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
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				if ( IS_INVALID(register0->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#endif
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreGlobal) : { targetFunc = wr_AdditionBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinarySubtractionAndStoreGlobal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryMultiplicationAndStoreGlobal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreGlobalOp; }
			CASE(BinaryDivisionAndStoreGlobal):
			{
				targetFunc = wr_DivideBinary;
				
targetFuncStoreGlobalOp:
				register1 = --stackTop;
				register0 = --stackTop;
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				WRValue* T = globalSpace + READ_8_FROM_PC(pc++);
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, T );
				if ( IS_INVALID(T->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, globalSpace + READ_8_FROM_PC(pc++) );
#endif
				CONTINUE;
			}
			
			CASE(BinaryAdditionAndStoreLocal): { targetFunc = wr_AdditionBinary; goto targetFuncStoreLocalOp; }
			CASE(BinarySubtractionAndStoreLocal): { targetFunc = wr_SubtractBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryMultiplicationAndStoreLocal): { targetFunc = wr_MultiplyBinary; goto targetFuncStoreLocalOp; }
			CASE(BinaryDivisionAndStoreLocal):
			{
				targetFunc = wr_DivideBinary;
				
targetFuncStoreLocalOp:
				register1 = --stackTop;
				register0 = --stackTop;
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
				WRValue* T = frameBase + READ_8_FROM_PC(pc++);
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, T );
				if ( IS_INVALID(T->p2) )
				{
					w->err = WR_ERR_division_by_zero;
					return 0;
				}
#else
				targetFunc[(register1->type<<2)|register0->type]( register1, register0, frameBase + READ_8_FROM_PC(pc++) );
#endif
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

#include "wrench.h"

WR_ALLOC g_malloc = &malloc;
WR_FREE g_free = &free;
//------------------------------------------------------------------------------
void wr_setGlobalAllocator( WR_ALLOC wralloc, WR_FREE wrfree )
{
	g_malloc = wralloc;
	g_free = wrfree;
}

//------------------------------------------------------------------------------
void* wr_malloc( size_t size )
{
	return g_malloc( size );
}

//------------------------------------------------------------------------------
void wr_free( void* ptr )
{
	g_free( ptr );
}

//------------------------------------------------------------------------------
void wr_freeGCChain( WRGCBase* chain )
{
	while( chain )
	{
		WRGCBase* next = chain->m_nextGC;
		chain->clear();
		g_free( chain );
		chain = next;
	}
}

//------------------------------------------------------------------------------
unsigned char* wr_pack16( int16_t i, unsigned char* buf )
{
	*buf = i & 0xFF;
	*(buf + 1) = (i>>8) & 0xFF;
	return buf;
}

//------------------------------------------------------------------------------
unsigned char* wr_pack32( int32_t l, unsigned char* buf )
{
	*buf = l & 0xFF;
	*(buf + 1) = (l>>8) & 0xFF;
	*(buf + 2) = (l>>16) & 0xFF;
	*(buf + 3) = (l>>24) & 0xFF;
	return buf;
}

#ifdef WRENCH_BIG_ENDIAN
//------------------------------------------------------------------------------
int32_t wr_x32( const int32_t val )
{
	int32_t v = READ_32_FROM_PC( (const unsigned char *)&val );
	return v;
}

//------------------------------------------------------------------------------
int16_t wr_x16( const int16_t val )
{
	int16_t v = READ_16_FROM_PC( (const unsigned char *)&val );
	return v;
}
#endif


// convert the value in-place and return a pointer to it
int32_t* makeInt();
float* makeFloat();

//------------------------------------------------------------------------------
int32_t* WrenchValue::Int()
{
	if ( m_value->type != WR_INT )
	{
		m_value->i = m_value->asInt();
		m_value->p2 = INIT_AS_INT;
	}
	return &(m_value->i);
}

//------------------------------------------------------------------------------
float* WrenchValue::Float()
{
	if ( m_value->type != WR_FLOAT )
	{
		m_value->f = m_value->asFloat();
		m_value->p2 = INIT_AS_FLOAT;
	}
	return &(m_value->f);
}

//------------------------------------------------------------------------------
WRValue* WrenchValue::asArrayMember( const int index )
{
	if ( !IS_ARRAY(m_value->xtype) ) 
	{
		// then make it one!
		m_value->va = m_context->getSVA( index + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_value->va )
		{
			m_value->p2 = INIT_AS_INT;
			return m_value;
		}
#endif
		m_value->p2 = INIT_AS_ARRAY;
	}
	else if ( index >= (int)m_value->va->m_size )
	{
		wr_growValueArray( m_value->va, index );
		m_context->allocatedMemoryHint += index * ((m_value->va->m_type == SV_CHAR) ? 1 : sizeof(WRValue));
	}

	return (WRValue*)m_value->va->get( index );
}

//------------------------------------------------------------------------------
WRState* wr_newState( int stackSize )
{
	WRState* w = (WRState *)g_malloc( sizeof(WRState) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !w ) { return 0; }
#endif
																		   
	memset( (unsigned char*)w, 0, sizeof(WRState) );
	w->globalRegistry.growHash( WRENCH_NULL_HASH, 0 );

	w->stackSize = stackSize;

	return w;
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	while( w->libCleanupFunctions )
	{
		w->libCleanupFunctions->cleanupFunction( w, w->libCleanupFunctions->param );
		WRLibraryCleanup *next = w->libCleanupFunctions->next;
		g_free( w->libCleanupFunctions );
		w->libCleanupFunctions = next;
	}

	while( w->contextList )
	{
		wr_destroyContext( w->contextList );
	}

	wr_freeGCChain( w->globalRegistry.m_nextGC );

	w->globalRegistry.clear();

	g_free( w );
}

//------------------------------------------------------------------------------
bool wr_getYieldInfo( WRContext* context, int* args, WRValue** firstArg, WRValue** returnValue )
{
	if ( !context || !context->yield_pc )
	{
		return false;
	}

	if ( args )
	{
		*args = context->yieldArgs;
	}

	if ( firstArg )
	{
		*firstArg = context->yield_stackTop - context->yieldArgs;
	}

	if ( returnValue )
	{
		*returnValue = (context->flags & (uint8_t)WRC_ForceYielded) ? 0 : context->yield_stackTop;
	}

	return true;
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return (WRError)w->err;
}

//------------------------------------------------------------------------------
bool wr_executeFunctionZero( WRContext* context )
{
	return wr_executeContext(context) ? true : false;
}

//------------------------------------------------------------------------------
WRContext* wr_createContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership, WRValue* stack )
{
	// CRC the code block, at least is it what the compiler intended?
	uint32_t hash = READ_32_FROM_PC(block + (blockSize - 4));
	if ( hash != wr_hash_read8(block, (blockSize - 4)) + WRENCH_VERSION_MAJOR )
	{
		w->err = WR_ERR_bad_bytecode_CRC;
		return 0;
	}

	int globals = READ_8_FROM_PC( block );
	int localFuncs = READ_8_FROM_PC(block  + 1); // how many?
	
	int needed = sizeof(WRContext) // class
				 + (globals * sizeof(WRValue))  // globals
				 + (localFuncs * sizeof(WRFunction)) // functions
				 + (stack ? 0 : (w->stackSize * sizeof(WRValue))); // stack

	WRContext* C = (WRContext *)g_malloc( needed );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !C )
	{
		w->err = WR_ERR_malloc_failed;
		return 0;
	}
#endif
	
	memset((char*)C, 0, needed);
	C->registry.growHash( WRENCH_NULL_HASH, 0 );

	C->numLocalFunctions = localFuncs;
	C->localFunctions = (WRFunction *)((uint8_t *)(C + 1) + (globals * sizeof(WRValue)));

	C->globals = globals;
	
	C->stack = stack ? stack : (WRValue *)(C->localFunctions + localFuncs);

	C->allocatedMemoryLimit = WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT;

	C->flags |= takeOwnership ? WRC_OwnsMemory : 0;
	
	C->w = w;

	C->bottom = block;
	C->bottomSize = blockSize;

	C->codeStart = block + 3 + (C->numLocalFunctions * WR_FUNCTION_CORE_SIZE);

	uint8_t compilerFlags = READ_8_FROM_PC( block + 2 );

	if ( compilerFlags & WR_INCLUDE_GLOBALS )
	{
		C->codeStart += C->globals * sizeof(uint32_t); // these are lazily loaded, just skip over them for now
	}

	if ( (compilerFlags & WR_EMBED_DEBUG_CODE) || (compilerFlags & WR_EMBED_SOURCE_CODE) )
	{
		uint16_t symbolsSize = READ_16_FROM_PC( C->codeStart + 4 );
		C->codeStart += 6 + symbolsSize; // 6 = 4 for code hash + 2 for size
	}

	if ( compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		C->codeStart += 4 + READ_32_FROM_PC( C->codeStart );		
	}


	int pos = 3;
	for( int i=0; i<C->numLocalFunctions; ++i )
	{
		C->localFunctions[i].namespaceOffset = READ_16_FROM_PC( block + pos );
		pos += 2;
		C->localFunctions[i].functionOffset = READ_16_FROM_PC( block + pos );
		pos += 2;
		C->localFunctions[i].hash = READ_32_FROM_PC( block + pos );
		pos += 3;
		C->localFunctions[i].arguments = READ_8_FROM_PC( block + ++pos );
		C->localFunctions[i].frameSpaceNeeded = READ_8_FROM_PC( block + ++pos );
		C->localFunctions[i].frameBaseAdjustment = READ_8_FROM_PC( block + ++pos );
		
		++pos;

		C->registry.getAsRawValueHashTable(C->localFunctions[i].hash)->wrf = C->localFunctions + i;
	}

	return C;
}

//------------------------------------------------------------------------------
WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* import = wr_createContext( context->w, block, blockSize, takeOwnership, context->stack );
	if ( !import )
	{
		return 0;
	}

	if ( !wr_callFunction(import, (WRFunction*)0) )
	{ 
		// imported context may not yield
		wr_destroyContext( context );
		return 0;
	}

	import->imported = context->imported;
	context->imported = import;

	// make this a circular linked list
	if ( import->imported == 0 )
	{
		import->imported = context;
	}

	return import;
}

//------------------------------------------------------------------------------
WRContext* wr_newContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* C = wr_createContext( w, block, blockSize, takeOwnership );
	if ( C )
	{
		C->nextStateContextLink = w->contextList;
		w->contextList = C;
	}
	return C;
}

//------------------------------------------------------------------------------
WRValue* wr_executeContext( WRContext* context )
{
	WRState* S = context->w;
	if ( context->stopLocation )
	{
		S->err = WR_ERR_execute_function_zero_called_more_than_once;
		return 0;
	}
	
	return wr_callFunction( context, (WRFunction*)0 );
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* context = wr_newContext( w, block, blockSize, takeOwnership );

	if ( !context )
	{
		return 0;
	}

	if ( !wr_callFunction(context, (WRFunction*)0) )
	{
		if ( !context->yield_pc )
		{
			wr_destroyContext( context );
			context = 0;
		}
	}
		
	return context;
}

//------------------------------------------------------------------------------
void wr_destroyContextEx( WRContext* context )
{
	// g_free all memory allocations by forcing the gc to collect everything
	context->globals = 0;
	context->allocatedMemoryHint = context->allocatedMemoryLimit;
	context->gc( 0 );

	wr_freeGCChain( context->registry.m_nextGC );

	context->registry.clear();

	if ( context->flags & WRC_OwnsMemory )
	{
		g_free( (void*)(context->bottom) );
	}

	g_free( context );
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
	for( WRContext* c = context->w->contextList; c; c = (WRContext*)c->nextStateContextLink )
	{
		if ( c == context )
		{
			if ( prev )
			{
				prev->nextStateContextLink = c->nextStateContextLink;
			}
			else
			{
				context->w->contextList = (WRContext*)context->w->contextList->nextStateContextLink;
			}

			while( context->imported )
			{
				WRContext* next = context->imported->imported;
				wr_destroyContextEx( context->imported );

				context->imported = next == context ? 0 : next; // the list is circular, stop here!
			}

			wr_destroyContextEx( context );

			break;
		}

		prev = c;
	}
}

//------------------------------------------------------------------------------
bool wr_runCommand( WRState* w, const char* sourceCode, const int size )
{
	int len = size == -1 ? strlen(sourceCode) : size;

	unsigned char* outBytes;
	int outLen;
	if ( !wr_compile(sourceCode, len, &outBytes, &outLen) )
	{
		wr_runOnce( w, outBytes, outLen );
		g_free( outBytes );
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
void wr_setAllocatedMemoryGCHint( WRContext* context, const uint16_t bytes )
{
	context->allocatedMemoryLimit = bytes;
}

//------------------------------------------------------------------------------
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRValue* V = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(name) );
	if ( V != (void*)WRENCH_NULL_HASH )
	{
		V->usr = usr;
		V->ccb = function;
	}
}

//------------------------------------------------------------------------------
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function )
{
	w->globalRegistry.getAsRawValueHashTable(wr_hashStr(signature))->lcb = function;
}

//------------------------------------------------------------------------------
void wr_registerLibraryConstant( WRState* w, const char* signature, const int32_t i )
{
	WRValue* C = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(signature) );
	C->p2 = INIT_AS_INT | INIT_AS_LIB_CONST;
	C->i = i;
}

//------------------------------------------------------------------------------
void wr_registerLibraryConstant( WRState* w, const char* signature, const float f )
{
	WRValue* C = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(signature) );
	C->p2 = INIT_AS_FLOAT | INIT_AS_LIB_CONST;
	C->f = f;
}

//------------------------------------------------------------------------------
int WRValue::asInt() const
{
	if ( type == WR_INT )
	{
		return i;
	}
	else if ( type == WR_FLOAT )
	{
		return (int)f;
	}

	return singleValue().asInt();
}

//------------------------------------------------------------------------------
float WRValue::asFloat() const
{
	if ( type == WR_FLOAT )
	{
		return f;
	}
	else if ( type == WR_INT )
	{
		return (float)i;
	}

	return singleValue().asFloat();
}

//------------------------------------------------------------------------------
void WRValue::setInt( const int val )
{
	if ( type == WR_REF )
	{
		r->setInt( val );
	}
	else
	{
		p2 = INIT_AS_INT;
		i = val;
	}
}

//------------------------------------------------------------------------------
void WRValue::setFloat( const float val )
{
	if ( type == WR_REF )
	{
		r->setFloat( val );
	}
	else
	{
		p2 = INIT_AS_FLOAT;
		f = val;
	}
}

//------------------------------------------------------------------------------
bool WRValue::isString( int* len ) const
{
	WRValue& V = deref();
	if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_CHAR )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isWrenchArray( int* len ) const
{
	WRValue& V = deref();
	if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_VALUE )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isRawArray( int* len ) const
{
	WRValue& V = deref();
	if ( IS_RAW_ARRAY(V.xtype) )
	{
		if ( len )
		{
			*len = EX_RAW_ARRAY_SIZE_FROM_P2(V.p2);
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isHashTable( int* len ) const
{
	WRValue& V = deref();
	if ( IS_HASH_TABLE(V.xtype) )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
WRValue* WRValue::indexArray( WRContext* context, const uint32_t index, const bool create )
{
	WRValue& V = deref();
	
	if ( !IS_ARRAY(V.xtype) || V.va->m_type != SV_VALUE )
	{
		if ( !create )
		{
			return 0;
		}

		V.va = context->getSVA( index + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !V.va )
		{
			V.p2 = INIT_AS_INT;
			return &V;
		}
#endif

		V.p2 = INIT_AS_ARRAY;

	}

	if ( index >= V.va->m_size )
	{
		if ( !create )
		{
			return 0;
		}
		
		wr_growValueArray( V.va, index );
		context->allocatedMemoryHint += index * ((V.va->m_type == SV_CHAR) ? 1 : sizeof(WRValue));
	}

	return V.va->m_Vdata + index;
}

//------------------------------------------------------------------------------
WRValue* WRValue::indexHash( WRContext* context, const uint32_t hash, const bool create )
{
	WRValue& V = deref();
	
	if ( !IS_HASH_TABLE(V.xtype) )
	{
		if ( !create )
		{
			return 0;
		}

		V.va = context->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !V.va )
		{
			V.p2 = INIT_AS_INT;
			return &V;
		}
#endif
		
		V.p2 = INIT_AS_HASH_TABLE;
	}

	return create ? (WRValue*)V.va->get(hash) : V.va->exists(hash, false);
}

//------------------------------------------------------------------------------
void* WRValue::array( unsigned int* len, char arrayType ) const
{
	WRValue& V = deref();
	
	if ( (V.xtype != WR_EX_ARRAY) || (V.va->m_type != arrayType) )
	{
		return 0;
	}

	if ( len )
	{
		*len = V.va->m_size;
	}

	return V.va->m_data;
}

//------------------------------------------------------------------------------
int WRValue::arraySize() const
{
	WRValue& V = deref();
	return V.xtype == WR_EX_ARRAY ? V.va->m_size : -1;
}

//------------------------------------------------------------------------------
int wr_technicalAsStringEx( char* string, const WRValue* value, size_t pos, size_t maxLen, bool valuesInHex )
{
	if ( pos >= maxLen )
	{
		return maxLen - 1;
	}
	
	if ( value->type == WR_REF )
	{
		strncpy( (string + pos), "ref: ", maxLen - pos );
		pos += 5;
		return wr_technicalAsStringEx( string, value->r, pos, maxLen, valuesInHex );
	}
	else if ( value->type == WR_FLOAT )
	{
		return pos + snprintf( string + pos, maxLen - pos, "%g", value->f );
	}
	else if ( value->type == WR_INT )
	{
		return pos + (valuesInHex ? snprintf(string + pos, maxLen - pos, "0x%08lX", (unsigned long int)value->ui )
			: snprintf(string + pos, maxLen - pos, "%ld", (long int)value->i));
				
	}
	else if ( value->xtype == WR_EX_ARRAY )
	{
		if ( value->va->m_type == SV_VALUE )
		{
			pos += snprintf( string + pos, maxLen - pos, "[ " );

			bool first = true;
			for( uint32_t i=0; pos<maxLen && i<value->va->m_size; ++i )
			{
				if ( !first )
				{
					pos += snprintf( string + pos, maxLen - pos, ", " );
				}
				first = false;
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + i, pos, maxLen, valuesInHex );
			}

			if ( pos >= maxLen )
			{
				return maxLen;
			}

			pos += snprintf( string + pos, maxLen - pos, " ]" );
		}
		else if ( value->va->m_type == SV_CHAR )
		{
			pos += snprintf( string + pos, maxLen - pos, "\"" );

			for( uint32_t i=0; pos<maxLen && i<value->va->m_size; ++i )
			{
				string[pos++] = value->va->m_SCdata[i];
			}

			if ( pos >= maxLen )
			{
				return maxLen;
			}

			pos += snprintf( string + pos, maxLen - pos, "\"" );
		}
		else
		{
			pos += snprintf( string + pos, maxLen - pos, "<raw array>" );
		}
	}
	else if ( value->xtype == WR_EX_HASH_TABLE )
	{
		pos += snprintf( string + pos, maxLen - pos, "{ " );
		
		bool first = true;
		for( int32_t element=0; element<value->va->m_mod; ++element )
		{
			if ( value->va->m_hashTable[element] != WRENCH_NULL_HASH )
			{
				if ( !first )
				{
					pos += snprintf( string + pos, maxLen - pos, ", " );
				}
				first = false;
				int32_t index = element << 1;
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + index + 1, pos, maxLen, valuesInHex );
				pos += snprintf( string + pos, maxLen - pos, ":" );
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + index, pos, maxLen, valuesInHex );
			}
		}

		pos += snprintf( string + pos, maxLen - pos, " }" );
	}

	return (pos >= maxLen) ? maxLen : pos;
}

//------------------------------------------------------------------------------
char* WRValue::technicalAsString( char* string, unsigned int maxLen, bool valuesInHex, unsigned int* strLen ) const
{
	unsigned int len = wr_technicalAsStringEx( string, this, 0, maxLen, valuesInHex );
	string[ len ] = 0;
	if ( strLen )
	{
		*strLen = len;
	}
	return string;
}

//------------------------------------------------------------------------------
char* WRValue::asMallocString( unsigned int* strLen ) const
{
	if ( type == WR_REF )
	{
		return r->asMallocString( strLen );
	}

	unsigned int len = 0;
	char* ret = 0;

	if ( type == WR_FLOAT )
	{
		ret = (char*)g_malloc( 12 );
		len = wr_ftoa( f, ret, 11 );
	}
	else if ( type == WR_INT )
	{
		ret = (char*)g_malloc( 13 );
		len = wr_itoa( i, ret, 12 );
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		ret = (char*)g_malloc( va->m_size + 1 );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !ret )
		{
			g_mallocFailed = true;
			return 0;
		}
#endif
		memcpy( ret, va->m_Cdata, va->m_size );
		ret[ va->m_size ] = 0;
	}
	else
	{
		return singleValue().asMallocString( strLen );
	}

	if ( strLen )
	{
		*strLen = len;
	}

	return ret;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, unsigned int maxLen, unsigned int* strLen ) const
{
	if ( type == WR_REF )
	{
		return r->asString( string, maxLen, strLen );
	}

	unsigned int len = 0;
	
	if ( type == WR_FLOAT )
	{
		len = wr_ftoa( f, string, maxLen );
	}
	else if ( type == WR_INT )
	{
		len = wr_itoa( i, string, maxLen );
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		len = maxLen ? (maxLen > va->m_size ? va->m_size : maxLen) : va->m_size;
		memcpy( string, va->m_Cdata, len );
		string[len] = 0;
	}
	else if ( IS_CONTAINER_MEMBER(xtype) )
	{
		return deref().asString(string, maxLen, strLen);
	}
	else
	{
		return singleValue().asString( string, maxLen, strLen ); // never give up, never surrender	
	}

	if ( strLen )
	{
		*strLen = len;
	}
	
	return string;
}

//------------------------------------------------------------------------------
WRValue::Iterator::Iterator( WRValue const& V ) : m_va(V.va), m_element(0)
{
	memset((char*)&m_current, 0, sizeof(WRIteratorEntry) );

	if ( (V.xtype != WR_EX_ARRAY && V.xtype != WR_EX_HASH_TABLE) )
		 //|| (m_va->m_type == SV_VOID_HASH_TABLE) )
	{
		m_va = 0;
		m_current.type = 0;
	}
	else
	{
		m_current.type = m_va->m_type;
		++*this;
	}
}

//------------------------------------------------------------------------------
const WRValue::Iterator WRValue::Iterator::operator++()
{
	if ( m_va )
	{
		if ( m_current.type == SV_HASH_TABLE || m_current.type == SV_VOID_HASH_TABLE )
		{
			int temp = m_element;
			for( ; temp < m_va->m_mod; ++temp )
			{
				if ( m_va->m_hashTable[temp] != WRENCH_NULL_HASH )
				{
					m_element = temp + 1;
					temp <<= 1;

					m_current.value = m_va->m_Vdata + temp++;
					if ( m_current.value->type == WR_REF )
					{
						m_current.value = m_current.value->r;
					}

					m_current.key = m_va->m_Vdata + temp;

					return *this;
				}
			}
		}
		else if ( m_element < m_va->m_size )
		{
			m_current.index = m_element;
			if ( m_current.type == SV_VALUE )
			{
				m_current.value = m_va->m_Vdata + m_element;
			}
			else
			{
				m_current.character = m_va->m_Cdata[m_element];
			}

			++m_element;

			return *this;
		}
	}

	memset( (char*)&m_current, 0, sizeof(WRIteratorEntry) );
	m_va = 0;
	return *this;
}

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( context, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, const int32_t hash, const WRValue* argv, const int argn )
{
	WRValue* cF = 0;

	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			context->w->err = WR_ERR_run_must_be_called_by_itself_first;
			return 0;
		}

		cF = context->registry.getAsRawValueHashTable( hash );
		if ( !cF->wrf )
		{
			context->w->err = WR_ERR_wrench_function_not_found;
			return 0;
		}
	}

	return wr_callFunction( context, cF ? cF->wrf : 0, argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_returnValueFromLastCall( WRContext* context )
{
	return context->stack; // this is where it ends up
}

//------------------------------------------------------------------------------
WRFunction* wr_getFunction( WRContext* context, const char* functionName )
{
	WRValue* f = context->registry.exists(wr_hashStr(functionName), false);
	return f ? f->wrf : 0;
}

//------------------------------------------------------------------------------
WRValue* wr_getGlobalRef( WRContext* context, const char* label )
{
	char globalLabel[64] = "::";
	if ( !label || !(READ_8_FROM_PC(context->bottom + 2) & WR_INCLUDE_GLOBALS) )
	{
		return 0;
	}
	size_t len = strlen(label);
	uint32_t match;
	if ( len < 3 || (label[0] == ':' && label[1] == ':') )
	{
		match = wr_hashStr( label );
	}
	else
	{
		strncpy( globalLabel + 2, label, 61 );
		match = wr_hashStr( globalLabel );
	}

	const unsigned char* symbolsBlock = context->bottom + 3 + (context->numLocalFunctions * WR_FUNCTION_CORE_SIZE);
	for( unsigned int i=0; i<context->globals; ++i, symbolsBlock += 4 )
	{
		uint32_t symbolHash = READ_32_FROM_PC( symbolsBlock );
		if ( match == symbolHash )
		{
			return ((WRValue *)(context + 1)) + i; // global space lives immediately past the context
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
WRValue& wr_makeInt( WRValue* val, int i )
{
	val->p2 = INIT_AS_INT;
	val->i = i;
	return *val;
}

//------------------------------------------------------------------------------
WRValue& wr_makeFloat( WRValue* val, float f )
{
	val->p2 = INIT_AS_FLOAT;
	val->f = f;
	return *val;
}

//------------------------------------------------------------------------------
WRValue& wr_makeString( WRContext* context, WRValue* val, const char* data, const int len )
{
	const int slen = len ? len : strlen(data);
	val->va = context->getSVA( slen, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !val->va )
	{
		val->p2 = INIT_AS_INT;
		return *val;
	}
#endif
	val->p2 = INIT_AS_ARRAY;
	memcpy( (unsigned char *)val->va->m_data, data, slen );
	return *val;
}

//------------------------------------------------------------------------------
void wr_makeContainer( WRValue* val, const uint16_t sizeHint )
{
	val->va = (WRGCObject*)g_malloc( sizeof(WRGCObject) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !val->va )
	{
		val->init();
		return;
	}
#endif
	memset( (unsigned char*)val->va, 0, sizeof(WRGCObject) );

	val->va->init( sizeHint, SV_HASH_TABLE, false);
	
	val->va->m_flags |= GCFlag_SkipGC;
	val->p2 = INIT_AS_HASH_TABLE;
}

//------------------------------------------------------------------------------
void wr_destroyContainer( WRValue* val )
{
	// clear off the keys
	wr_freeGCChain( val->vb->m_nextGC );

	// clear the container
	val->vb->clear();
	g_free( val->vb );
}

//------------------------------------------------------------------------------
WRValue* wr_addToContainerEx( const char* name, WRValue* container )
{
	if (container->xtype != WR_EX_HASH_TABLE)
	{
		return 0;
	}

	const unsigned int len = (const unsigned int)strlen(name);

	// create a 'key' object
	WRValue key;
	key.va = (WRGCObject*)g_malloc( sizeof(WRGCObject) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !key.va )
	{
		return 0;
	}
#endif
	memset( (unsigned char*)key.va, 0, sizeof(WRGCObject) );

	key.va->m_nextGC = container->va->m_nextGC;
	container->va->m_nextGC = key.va;
	
	key.va->init( len, SV_CHAR, false);

	key.va->m_flags |= GCFlag_SkipGC;
	key.p2 = INIT_AS_ARRAY;
	memcpy(key.va->m_Cdata, name, len);

	WRValue* entry = (WRValue*)container->va->get(key.getHash());
	*(entry + 1) = key;

	return entry;
}

//------------------------------------------------------------------------------
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value )
{
	WRValue* entry = wr_addToContainerEx( name, container );
	if ( !entry )
	{
		return;
	}

	entry->r = value;
	entry->p2 = INIT_AS_REF;
}

//------------------------------------------------------------------------------
void wr_addFloatToContainer( WRValue* container, const char* name, const float f )
{
	WRValue* entry = wr_addToContainerEx( name, container );
	if ( !entry )
	{
		return;
	}

	entry->p2 = INIT_AS_FLOAT;
	entry->f = f;
}

//------------------------------------------------------------------------------
void wr_addIntToContainer( WRValue* container, const char* name, const int i )
{
	WRValue* entry = wr_addToContainerEx( name, container );
	if ( !entry )
	{
		return;
	}

	entry->p2 = INIT_AS_INT;
	entry->i = i;
}

//------------------------------------------------------------------------------
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size )
{
	WRValue* entry = wr_addToContainerEx( name, container );
	if ( !entry )
	{
		return;
	}

	entry->c = array;
	entry->p2 = INIT_AS_RAW_ARRAY | (size<<8);
}

//------------------------------------------------------------------------------
WRValue* wr_getValueFromContainer( WRValue const& container, const char* name )
{
	if (container.xtype != WR_EX_HASH_TABLE)
	{
		return 0;
	}

	return (WRValue*)container.va->get( wr_hash(name, (const unsigned int)strlen(name)) );
}

#ifndef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
const char* c_opcodeName[] = 
{
	"Yield",

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
	"AssignToObjectTableByHash",

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
	"InitVar",

	"DebugInfo",
};

//------------------------------------------------------------------------------
const char* c_errStrings[]=
{
	"WR_ERR_None",

	"WR_ERR_compiler_not_loaded",
	"WR_ERR_function_not_found",
	"WR_ERR_lib_function_not_found",
	"WR_ERR_hash_not_found",
	"WR_ERR_library_constant_not_loaded",
	"WR_ERR_unknown_opcode",
	"WR_ERR_unexpected_EOF",
	"WR_ERR_unexpected_token",
	"WR_ERR_bad_expression",
	"WR_ERR_bad_label",
	"WR_ERR_statement_expected",
	"WR_ERR_unterminated_string_literal",
	"WR_ERR_newline_in_string_literal",
	"WR_ERR_bad_string_escape_sequence",
	"WR_ERR_tried_to_load_non_resolvable",
	"WR_ERR_break_keyword_not_in_looping_structure",
	"WR_ERR_continue_keyword_not_in_looping_structure",
	"WR_ERR_expected_while",
	"WR_ERR_compiler_panic",
	"WR_ERR_constant_redefined",
	"WR_ERR_struct_in_struct",
	"WR_ERR_var_not_seen_before_label",
	"WR_ERR_unexpected_export_keyword",
	"WR_ERR_new_assign_by_label_or_offset_not_both",
	"WR_ERR_struct_not_exported",

	"WR_ERR_run_must_be_called_by_itself_first",
	"WR_ERR_hash_table_size_exceeded",
	"WR_ERR_hash_table_invalid_key",
	"WR_ERR_wrench_function_not_found",
	"WR_ERR_array_must_be_indexed",
	"WR_ERR_context_not_found",
	"WR_ERR_context_not_yielded",
	"WR_ERR_cannot_call_function_context_yielded",

	"WR_ERR_hash_declaration_in_array",
	"WR_ERR_array_declaration_in_hash",
	"WR_ERR_stack_overflow",

	"WR_ERR_bad_goto_label",
	"WR_ERR_bad_goto_location",
	"WR_ERR_goto_target_not_found",

	"WR_ERR_switch_with_no_cases",
	"WR_ERR_switch_case_or_default_expected",
	"WR_ERR_switch_construction_error",
	"WR_ERR_switch_bad_case_hash",
	"WR_ERR_switch_duplicate_case",

	"WR_ERR_bad_bytecode_CRC",

	"WR_ERR_execute_function_zero_called_more_than_once",

	"WR_ERR_malloc_failed",

	"WR_ERR_USER_err_out_of_range",

	"WR_ERR_division_by_zero",
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

#include "wrench.h"

//------------------------------------------------------------------------------
bool wr_serializeEx( WRValueSerializer& serializer, const WRValue& val )
{
	char temp;
	uint16_t temp16;
	uint32_t temp32;
	
	const WRValue& value = val.deref();

	serializer.write( &(temp = value.type), 1 );

	switch( (uint8_t)value.type )
	{
		case WR_INT:
		case WR_FLOAT:
		{
			temp32 = wr_x32( value.ui );
			serializer.write( (char *)&temp32, 4 );
			return true;
		}

		case WR_EX:
		{
			serializer.write( &(temp = value.xtype), 1 );

			switch( (uint8_t)value.xtype )
			{
				case WR_EX_ARRAY:
				{
					serializer.write( &(temp = value.va->m_type), 1 );
					temp16 = wr_x16( value.va->m_size );
					serializer.write( (char*)&temp16, 2 );

					if ( value.va->m_type == SV_CHAR )
					{
						serializer.write( value.va->m_SCdata, value.va->m_size );
						return true;
					}
					else if ( value.va->m_type == SV_VALUE )
					{
						for( uint32_t i=0; i<value.va->m_size; ++i )
						{
							if ( !wr_serializeEx(serializer, value.va->m_Vdata[i]) )
							{
								return false;
							}
						}
					}

					return true;
				}

				case WR_EX_HASH_TABLE:
				{
					temp16 = wr_x16( value.va->m_mod );
					serializer.write( (char *)&temp16, 2 );

					for( uint32_t i=0; i<value.va->m_mod; ++i )
					{
						if ( value.va->m_hashTable[i] == WRENCH_NULL_HASH )
						{
							serializer.write( &(temp = 0), 1 );
						}
						else
						{
							serializer.write( &(temp = 1), 1 );

							wr_serializeEx( serializer, value.va->m_Vdata[i<<1] );
							wr_serializeEx( serializer, value.va->m_Vdata[(i<<1) + 1] );
						}
					}

					return true;
				}
				
				default: break;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool wr_deserializeEx( WRValue& value, WRValueSerializer& serializer, WRContext* context )
{
	char temp;
	uint16_t temp16;

	if ( !serializer.read(&temp , 1) )
	{
		return false;
	}

	value.p2 = (int)temp;

	switch( (uint8_t)temp )
	{
		case WR_INT:
		case WR_FLOAT:
		{
			bool ret = serializer.read( (char *)&value.ui, 4 );
			value.ui = wr_x32( value.ui );
			return ret;
		}

		case WR_EX:
		{
			if ( !serializer.read(&temp, 1) )
			{
				return false;
			}

			switch( (uint8_t)temp )
			{
				case WR_EX_ARRAY:
				{
					if ( !serializer.read(&temp, 1)  // type
						 || !serializer.read((char *)&temp16, 2) ) // size
					{
						return false;
					}

					temp16 = wr_x16( temp16 );
					value.p2 = INIT_AS_ARRAY;
					
					switch( (uint8_t)temp )
					{
						case SV_CHAR:
						{
							value.va = context->getSVA( temp16, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
							if ( !value.va )
							{
								value.p2 = INIT_AS_INT;
								return false;
							}
#endif
							if ( !serializer.read(value.va->m_SCdata, temp16) )
							{
								return false;
							}
							return true;
						}

						case SV_VALUE:
						{
							value.va = context->getSVA( temp16, SV_VALUE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
							if ( !value.va )
							{
								value.p2 = INIT_AS_INT;
								return false;
							}
#endif
							for( uint16_t i=0; i<temp16; ++i )
							{
								if ( !wr_deserializeEx(value.va->m_Vdata[i], serializer, context) )
								{
									return false;
								}
							}

							return true;
						}
					}

					break;
				}

				case WR_EX_HASH_TABLE:
				{
					if ( !serializer.read((char *)&temp16, 2) ) // size
					{
						return false;
					}

					temp16 = wr_x16( temp16 );
					
					value.va = context->getSVA( temp16 - 1, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
					if ( !value.va )
					{
						value.p2 = INIT_AS_INT;
						return false;
					}
#endif
					value.p2 = INIT_AS_HASH_TABLE;


					for( uint16_t i=0; i<temp16; ++i )
					{
						if ( !serializer.read(&temp, 1) )
						{
							return false;
						}

						if ( !temp )
						{
							value.va->m_Vdata[i<<1].init();
							value.va->m_Vdata[(i<<1) + 1].init();
							value.va->m_hashTable[i] = WRENCH_NULL_HASH;
						}
						else
						{
							if ( !wr_deserializeEx(value.va->m_Vdata[i<<1], serializer, context)
								 || !wr_deserializeEx(value.va->m_Vdata[(i<<1)+1], serializer, context) )
							{
								return false;
							}
							value.va->m_hashTable[i] = value.va->m_Vdata[(i<<1)+1].getHash();
						}
					}
					
					return true;
				}

				default: break;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool wr_serialize( char** buf, int* len, const WRValue& value )
{
	WRValueSerializer S;
	if ( !wr_serializeEx(S, value) )
	{
		return false;
	}

	S.getOwnership( buf, len );
	return true;
}

//------------------------------------------------------------------------------
bool wr_deserialize( WRContext* context, WRValue& value, const char* buf, const int len )
{
	WRValueSerializer S( buf, len );
	return wr_deserializeEx( value, S, context );
}
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

#include "wrench.h"

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
bool WrenchDebugCommInterface::send( WrenchPacket* packet )
{
	uint32_t size = packet->xlate();
	bool ret = sendEx( (uint8_t*)packet, size );
	packet->xlate();
	return ret;
}

//------------------------------------------------------------------------------
WrenchPacket* WrenchDebugCommInterface::receive( const int timeoutMilliseconds )
{
	if ( peekEx(timeoutMilliseconds) < sizeof(WrenchPacket) )
	{
		return 0;
	}

	WrenchPacket* packet = (WrenchPacket *)g_malloc( sizeof(WrenchPacket) );
	if ( !packet )
	{
		return 0;
	}

	if ( !recvEx( (uint8_t*)packet, sizeof(WrenchPacket), timeoutMilliseconds) )
	{
		g_free( packet );
		return 0;
	}

	packet->xlate();

	if ( packet->payloadSize() )
	{
		WrenchPacket* full = (WrenchPacket *)g_malloc( sizeof(WrenchPacket) + packet->payloadSize() );
		if ( !full )
		{
			return 0;
		}
		memcpy( full, packet, sizeof(WrenchPacket) );
		g_free( packet );
		packet = full;

		if ( !recvEx( (uint8_t*)packet->payload(), packet->payloadSize(), timeoutMilliseconds) )
		{
			g_free( packet );
			return 0;
		}
	}

	return packet;
}

//------------------------------------------------------------------------------
void wr_formatGCObject( WRGCObject const& obj, WRstr& out )
{
	switch( obj.m_type )
	{
		case SV_VALUE:
		{
			out.appendFormat( "SV_VALUE : size[%d]", obj.m_size ); 
			break;
		}
		
		case SV_CHAR:
		{
			out.appendFormat( "SV_CHAR : size[%d]", obj.m_size ); 
			break;
		}
		
		case SV_HASH_TABLE:
		{
			out.appendFormat( "SV_HASH_TABLE : mod[%d] size[%d] ", obj.m_mod, obj.m_size ); 
			break;
		}
		
		case SV_VOID_HASH_TABLE:
		{
			out.appendFormat( "SV_VOID_HASH_TABLE : @[%p] mod[%d] ", obj.m_ROMHashTable, obj.m_mod ); 
			break;
		}
	}
}

//------------------------------------------------------------------------------
void wr_formatStackEntry( const WRValue* v, WRstr& out )
{
	out.appendFormat( "%p: ", v );
	
	switch( v->type )
	{
		default:
		{
			out.appendFormat( "frame [0x%08X]:[0x%08X]\n", v->p, v->p2 );
			break;
		}
		
		case WR_INT:
		{
			out.appendFormat( "Int [%d]\n", v->i );
			break;
		}

		case WR_FLOAT:
		{
			out.appendFormat( "Float [%g]\n", v->f );
			break;
		}

		case WR_REF:
		{
			out.appendFormat( "r [%p] ->\n", v->r );
			wr_formatStackEntry( v->r, out );
			break;
		}

		case WR_EX:
		{
			switch( v->xtype )
			{
				case WR_EX_RAW_ARRAY:
				{
					out.appendFormat( "EX:RAW_ARRAY [%p] size[%d]\n", v->r->c, EX_RAW_ARRAY_SIZE_FROM_P2(v->r->p2));
					break;
				}

				case WR_EX_DEBUG_BREAK:
				{
					out.appendFormat( "EX:DEBUG_BREAK\n" );
					break;
				}

				case WR_EX_ITERATOR:
				{
					// todo- break out VA
					out.appendFormat( "EX:ITERATOR of[%p] @[%d]\n", v->va, DECODE_ARRAY_ELEMENT_FROM_P2(v->p2) );
					break;
				}

				case WR_EX_CONTAINER_MEMBER:
				{
					// todo- decode r
//					out.appendFormat( "EX:CONTAINER_MEMBER element[%d] of[%p]\n", DECODE_ARRAY_ELEMENT_FROM_P2(v->p2), v->r );
					break;
				}
				
				case WR_EX_ARRAY:
				{
					// todo- decode va
					out.appendFormat( "EX_ARRAY :" );
					wr_formatGCObject( *v->va, out );
					out += "\n";
					break;
				}

				case WR_EX_STRUCT:
				{
					out.appendFormat( "EX:STRUCT\n" );
					break;
				}
				
				case WR_EX_HASH_TABLE:
				{
					out.appendFormat( "EX:HASH_TABLE :" );
					wr_formatGCObject( *v->va, out );
					out += "\n";

					break;
				}

				default:
					break;
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_stackDump( const WRValue* bottom, const WRValue* top, WRstr& out )
{
	out.clear();
	
	for( const WRValue* v=bottom; v<top; ++v)
	{
		out.appendFormat( "[%d] ", (int)(top - v) );
		wr_formatStackEntry( v, out );
	}
}

#endif
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
// hide this from the wrench.h header to keep it cleaner
void WRDebugClientInterface::init()
{
	I->m_scratchState = wr_newState();
	uint8_t* out;
	int outSize;
	wr_compile("", 0, &out, &outSize);
	I->m_scratchContext = wr_newContext( I->m_scratchState, out, outSize, true );
	I->m_scratchContext->allocatedMemoryHint = 0;
}

//------------------------------------------------------------------------------
WRDebugClientInterface::WRDebugClientInterface( WrenchDebugCommInterface* comm )
{
	I = new WRDebugClientInterfacePrivate;
	I->m_parent = this;
	I->m_comm = comm;

	init();
}

//------------------------------------------------------------------------------
WRDebugClientInterface::~WRDebugClientInterface()
{
	wr_destroyState( I->m_scratchState );
	delete I;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::load( const uint8_t* byteCode, const int size )
{
	I->m_scratchContext->gc(0);

	I->m_comm->send( WrenchPacketScoped(WRD_Load, size, byteCode) );

	// invalidate source block;
	g_free( I->m_sourceBlock );
	I->m_sourceBlock = 0;
	I->m_functions->clear();
	I->m_callstackDirty = true;
	I->m_symbolsLoaded = false;
	I->populateSymbols();
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::run( const int toLine ) 
{
	I->m_scratchContext->gc(0);
	I->m_callstackDirty = true;

	WrenchPacket packet( WRD_Run );
	packet.param1 = toLine;
	I->m_comm->send( &packet );

	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getSourceCode( const char** data, int* len )
{
	if ( !I->m_sourceBlock )
	{
		I->m_comm->send( WrenchPacket(WRD_RequestSourceBlock) );

		WrenchPacketScoped r( I->getPacket() );
		
		if ( !r || r.packet->_type != WRD_ReplySource )
		{
			return false;
		}
		
		I->m_sourceBlockLen = r.packet->payloadSize();
		I->m_sourceBlock = (char*)g_malloc( r.packet->payloadSize() + 1 );
		
		memcpy( I->m_sourceBlock, r.packet->payload(), r.packet->payloadSize() );
		I->m_sourceBlock[ r.packet->payloadSize() ] = 0;
	}
	
	*data = I->m_sourceBlock;
	*len = I->m_sourceBlockLen;
	
	return true;
}

//------------------------------------------------------------------------------
uint32_t WRDebugClientInterface::getSourceCodeHash()
{
	I->m_comm->send( WrenchPacket(WRD_RequestSourceHash) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || r.packet->_type != WRD_ReplySourceHash )
	{
		return 0;
	}

	return r.packet->param1;
}

//------------------------------------------------------------------------------
SimpleLL<WrenchFunction>& WRDebugClientInterface::getFunctions()
{
	I->populateSymbols();
	return *I->m_functions;
}

//------------------------------------------------------------------------------
const char* WRDebugClientInterface::getFunctionLabel( const int index )
{
	SimpleLL<WrenchFunction>& funcs = getFunctions();
	WrenchFunction *f = funcs[index];
	return f ? f->name : 0;
}

//------------------------------------------------------------------------------
WRValue* WRDebugClientInterface::getValue( WRValue& value, const int index, const int depth )
{
	value.init();

	WrenchPacket p( WRD_RequestValue );
	p.param1 = index;
	p.param2 = depth;

	I->m_comm->send( &p );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || r.packet->_type != WRD_ReplyValue )
	{
		return 0;
	}

	WRValueSerializer serializer( (char *)r.packet->payload(), r.packet->payloadSize() );

	wr_deserializeEx( value, serializer, I->m_scratchContext );

	return &value;
}

//------------------------------------------------------------------------------
const char* WRDebugClientInterface::getValueLabel( const int index, const int depth )
{
	WrenchFunction* func = (*I->m_functions)[depth];
	if (!func)
	{
		return 0;
	}
		
	int i = 0;
	for( WrenchSymbol* sym = func->vars->first(); sym; sym = func->vars->next() )
	{
		if ( i++ == index )
		{
			return sym->label;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
bool WRDebugClientInterface::getStackDump( char* out, const unsigned int maxOut )
{
	I->m_comm->send( WrenchPacket(WRD_RequestStackDump) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || (r.packet->_type != WRD_ReplyStackDump) )
	{
		return false;
	}

	strncpy( out, (const char*)r.packet->payload(), maxOut < r.packet->payloadSize() ? maxOut : r.packet->payloadSize() );
	
	return true;
}

//------------------------------------------------------------------------------
SimpleLL<WrenchCallStackEntry>* WRDebugClientInterface::getCallstack()
{
	if ( !I->m_callstackDirty )
	{
		return I->m_callStack;
	}

	I->m_callStack->clear();

	I->m_comm->send( WrenchPacket(WRD_RequestCallstack) );
	WrenchPacketScoped r( I->getPacket() );

	if ( !r || (r.packet->_type != WRD_ReplyCallstack) )
	{
		return 0;
	}

	WrenchCallStackEntry* entries = (WrenchCallStackEntry *)r.packet->payload();

	I->m_procState = (WRP_ProcState)r.packet->param2;

	uint32_t count = r.packet->param1;
	for( uint32_t i=0; i<count; ++i )
	{
		entries[i].onLine = wr_x32( entries[i].onLine );
		entries[i].locals = wr_x16( entries[i].locals );
		*I->m_callStack->addTail() = entries[i];
	}

	I->m_callstackDirty = false;
	return I->m_callStack;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::stepInto( const int steps )
{
	I->m_callstackDirty = true;
	WrenchPacket packet( WRD_RequestStepInto );
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::stepOver( const int steps )
{
	I->m_callstackDirty = true;
	WrenchPacket packet( WRD_RequestStepOver );
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
WRP_ProcState WRDebugClientInterface::getProcState()
{
	return I->m_procState;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::setDebugEmitCallback( void (*print)( const char* debugText ) )
{
	I->outputDebug = print;
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::setBreakpoint( const int lineNumber )
{
	WrenchPacket packet( WRD_RequestSetBreakpoint );
	packet.param1 = lineNumber;
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

//------------------------------------------------------------------------------
void WRDebugClientInterface::clearBreakpoint( const int lineNumber )
{
	WrenchPacket packet( WRD_RequestClearBreakpoint );
	packet.param1 = lineNumber;
	I->m_comm->send( &packet );
	WrenchPacketScoped r( I->getPacket() );
}

#endif
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
WRDebugServerInterface::WRDebugServerInterface( WRState* w,
												WrenchDebugCommInterface* CommInterface )
{
	memset( this, 0, sizeof(*this) );
	I = (WRDebugServerInterfacePrivate *)g_malloc( sizeof(WRDebugServerInterfacePrivate) );
	new (I) WRDebugServerInterfacePrivate( this );

	I->m_w = w;
	I->m_comm = CommInterface;
}

//------------------------------------------------------------------------------
WRDebugServerInterface::~WRDebugServerInterface()
{
	I->~WRDebugServerInterfacePrivate();
	g_free( I );
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

WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership );

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen )
{
	WRstr listing;
	
	WRState* w = wr_newState();
	
	WRContext* context = wr_createContext( w, bytecode, len, false );
	if ( !context )
	{
		listing = "err: context failed\n";
	}

	listing.format( "%d globals\n", context->globals );
	listing.appendFormat( "%d units\n", context->numLocalFunctions );
	for( int i=0; i<context->numLocalFunctions; ++i )
	{
		listing.appendFormat( "unit %d[0x%08X] offset[0x%04X] arguments[%d]\n",
							  i,
							  context->localFunctions[i].hash,
							  context->localFunctions[i].functionOffset,
							  context->localFunctions[i].arguments );
	}

	listing.release( out, outLen );
}
/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

MIT License

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

/*
//------------------------------------------------------------------------------
WrenchPacket::WrenchPacket( const WrenchDebugComm type, const uint32_t payloadSize )
{
	memset( (char*)this, 0, sizeof(WrenchPacket) );
	_type = type;
	setPayloadSize( payloadSize );
}
*/
//------------------------------------------------------------------------------
WrenchPacket::WrenchPacket( const int32_t type )
{
	memset( (char*)this, 0, sizeof(WrenchPacket) );
	t = type;
}

//------------------------------------------------------------------------------
WrenchPacket* WrenchPacket::alloc( WrenchPacket const& base )
{
	WrenchPacket* packet = (WrenchPacket*)g_malloc( base.size );

	memcpy( (char*)packet, (char*)&base, sizeof(WrenchPacket) );
	return packet;
}

//------------------------------------------------------------------------------
WrenchPacket* WrenchPacket::alloc( const uint32_t type, const uint32_t payloadSize, const uint8_t* payload )
{
	WrenchPacket* packet = (WrenchPacket*)g_malloc( sizeof(WrenchPacket) + payloadSize );
	if ( packet )
	{
		memset( (char*)packet, 0, sizeof(WrenchPacket) );

		packet->t = type;
		packet->setPayloadSize( payloadSize );
		if ( payload )
		{
			memcpy( packet->payload(), payload, payloadSize );
		}
	}

	return packet;
}

//------------------------------------------------------------------------------
uint32_t WrenchPacket::xlate()
{
	uint32_t s = size;

	size = wr_x32( size );
	t = wr_x32( t );
	param1 = wr_x32( param1 );
	param2 = wr_x32( param2 );

	return s;
}

#endif
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

MIT License

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
WRDebugServerInterfacePrivate::WRDebugServerInterfacePrivate( WRDebugServerInterface* parent )
{
	memset( (char*)this, 0, sizeof(*this) );
	m_parent = parent;


	m_lineBreaks = (SimpleLL<int>*)g_malloc(sizeof(SimpleLL<int>));
	new (m_lineBreaks)  SimpleLL<int>();

	m_callStack = (SimpleLL<WrenchCallStackEntry>*)g_malloc(sizeof(SimpleLL<WrenchCallStackEntry>));
	new (m_callStack)  SimpleLL<WrenchCallStackEntry>();
}

//------------------------------------------------------------------------------
WRDebugServerInterfacePrivate::~WRDebugServerInterfacePrivate()
{
	g_free( m_externalCodeBlock ); // MIGHT have been allocated

	m_lineBreaks->~SimpleLL<int>();
	g_free( m_lineBreaks );

	m_callStack->~SimpleLL<WrenchCallStackEntry>();
	g_free( m_callStack );
}

//------------------------------------------------------------------------------
void WRDebugServerInterface::tick()
{
	for(;;)
	{
		WrenchPacketScoped p(I->m_comm->receive());
		if ( !p )
		{
			return;
		}

		WrenchPacketScoped r( I->processPacket(p) );
		if ( r )
		{
			I->m_comm->send( r );
		}
	}
}

//------------------------------------------------------------------------------
WRContext* WRDebugServerInterface::loadBytes( const uint8_t* bytes, const int len )
{
	if ( I->m_context )
	{
		wr_destroyContext( I->m_context );
		I->m_context = 0;
	}

	if ( !bytes || !len || !I->m_w )
	{
		return 0;
	}

	WRContext* ret = wr_newContext( I->m_w, bytes, len );
	I->m_context = ret;

	if ( !ret )
	{
		return 0;
	}

	ret->debugInterface = this;
	
	I->m_steppingOverReturnVector = 0;
	I->m_lineSteps = 0;
	I->m_stepOverDepth = -1;

	I->m_lineBreaks->clear();

	I->m_stopOnLine = 0;
	I->m_firstCall = true;
	I->m_stepOut = false;
	I->m_halted = false;
 	
	const uint8_t* code = bytes + 2;
	
	I->m_compilerFlags = READ_8_FROM_PC( code++ );

	// skip over function signatures
	code += I->m_context->numLocalFunctions * WR_FUNCTION_CORE_SIZE;

	if ( I->m_compilerFlags & WR_INCLUDE_GLOBALS )
	{
		code += ret->globals * sizeof(uint32_t); // these are lazily loaded, just skip over them for now
	}

	if ( I->m_compilerFlags & WR_EMBED_DEBUG_CODE || I->m_compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		I->m_embeddedSourceHash = READ_32_FROM_PC( code );
		code += 4;

		I->m_symbolBlockLen = READ_16_FROM_PC( code );
		code += 2;

		I->m_symbolBlock = code;
		code += I->m_symbolBlockLen;
	}

	if ( I->m_compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		I->m_embeddedSourceLen = READ_32_FROM_PC( code );
		I->m_embeddedSource = code + 4;
	}

	return ret;
}

//------------------------------------------------------------------------------
bool WRDebugServerInterfacePrivate::codewordEncountered( const uint8_t* pc, uint16_t codeword, WRValue* stackTop )
{
	unsigned int type = codeword & WRD_TypeMask;

	if ( type == WRD_LineNumber )
	{
		int line = codeword & WRD_PayloadMask;
		m_callStack->tail()->onLine = line;

		for( int* L = m_lineBreaks->first() ; L ; L = m_lineBreaks->next() )
		{
			if ( *L == line )
			{
				return true;
			}
		}

		if ( m_stepOverDepth <= 0
			 && m_lineSteps
			 && !--m_lineSteps )
		{
			m_stopOnLine = 0;
			m_stepOverDepth = -1;
			return true;
		}

		if ( line == m_stopOnLine )
		{
			m_stopOnLine = 0;
			return true;
		}
	}
	else if ( type == WRD_FunctionCall )
	{
		if ( (codeword & WRD_PayloadMask) == WRD_GlobalStopFunction )
		{
			m_halted = true;
			return false;
		}
		
		if ( (codeword & WRD_PayloadMask) == WRD_ExternalFunction )
		{
			// no need for bookkeeping, just skip it
			return false;
		}
		
		if ( m_stepOverDepth >= 0 ) // track stepping over/into functions
		{
			++m_stepOverDepth;
		}

		WrenchCallStackEntry* from = m_callStack->tail();
		WrenchCallStackEntry* entry = m_callStack->addTail();
		entry->onLine = -1; // don't know yet!
		entry->fromUnitIndex = from->thisUnitIndex;
		entry->thisUnitIndex = codeword & WRD_PayloadMask;
//		entry->stackOffset = stackTop - m_w->stack;

		WRFunction* func = m_context->localFunctions + (entry->thisUnitIndex - 1); // unit '0' is the global unit
		entry->arguments = func->arguments;
		entry->locals = func->frameSpaceNeeded;
		
		//printf( "Calling[%d] @ line[%d] stack[%d]\n", m_status.onFunction, m_status.onLine, (int)(stackTop - m_context->w->stack) );
	}
	else if ( type == WRD_Return )
	{
		m_callStack->popHead();

		if ( m_stepOverDepth >= 0 )
		{
			--m_stepOverDepth;
		}
	}
	
	return false;
}

//------------------------------------------------------------------------------
void WRDebugServerInterfacePrivate::clearBreakpoint( const int bp )
{
	for( int* L = m_lineBreaks->first(); L ; L = m_lineBreaks->next() )
	{
		if ( *L == bp )
		{
			m_lineBreaks->popItem( L );
			return;
		}
	}
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugServerInterfacePrivate::processPacket( WrenchPacket* packet )
{
	WrenchPacket* reply = 0;
	
	switch( packet->_type )
	{
		case WRD_RequestStepOver:
		{
			m_lineSteps = 1;
			m_stepOverDepth = 0;
			goto WRDRun;
		}

		case WRD_RequestStepInto:
		{
			m_lineSteps = 1;
			m_stepOverDepth = -1;
			goto WRDRun;
		}
		
		case WRD_Run:
		{
WRDRun:
			if ( !m_context || m_halted )
			{
				reply = WrenchPacket::alloc( WRD_Err );
				break;
			}

			m_stopOnLine = packet->param1;

			if ( m_firstCall )
			{ 
				m_firstCall = false;
				
				m_callStack->clear();
				
				WrenchCallStackEntry* stack = m_callStack->addTail();
				memset( (char*)stack, 0, sizeof(WrenchCallStackEntry) );
				stack->locals = m_context->globals;
				stack->thisUnitIndex = 0;

				wr_executeContext( m_context );
			}
			else
			{
				wr_continue( m_context );
			}

			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_Load:
		{
			uint32_t size = packet->payloadSize();
			if ( size )
			{
				if ( m_externalCodeBlockSize < size )
				{
					g_free( m_externalCodeBlock );
					m_externalCodeBlock = (uint8_t*)g_malloc( size );
					m_externalCodeBlockSize = size;
				}
				
				memcpy( m_externalCodeBlock, packet->payload(), size );
				
				m_parent->loadBytes( m_externalCodeBlock, m_externalCodeBlockSize );
			}
			else
			{
				m_parent->loadBytes( m_context->bottom, m_context->bottomSize );
			}

			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_RequestSourceBlock:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else if ( !m_embeddedSourceLen )
			{
				reply = WrenchPacket::alloc( WRD_ReplyUnavailable );
			}
			else
			{
				uint32_t size = sizeof(WrenchPacket) + m_embeddedSourceLen + 1;
				reply = WrenchPacket::alloc( WRD_ReplySource, size );
				uint32_t i=0;
				for( ; i< m_embeddedSourceLen; ++i )
				{
					*reply->payload( i ) = (uint8_t)READ_8_FROM_PC( m_embeddedSource + i );
				}
				*reply->payload( i ) = 0;
			}
			
			break;
		}

		case WRD_RequestSymbolBlock:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else if ( !m_symbolBlockLen )
			{
				reply = WrenchPacket::alloc( WRD_ReplyUnavailable );
			}
			else
			{
				uint32_t size = sizeof(WrenchPacket) + m_symbolBlockLen;
				reply = WrenchPacket::alloc( WRD_ReplySymbolBlock, size );
				uint32_t i=0;
				for( ; i < m_symbolBlockLen; ++i )
				{
					*reply->payload( i ) = (uint8_t)READ_8_FROM_PC( m_symbolBlock + i );
				}
			}				
			
			break;
		}

		case WRD_RequestSourceHash:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else
			{
				reply = WrenchPacket::alloc( WRD_ReplySourceHash );
				reply->param1 = m_embeddedSourceHash;
			}
			break;
		}

		case WRD_RequestValue:
		{
			if ( !m_context )
			{
				reply = WrenchPacket::alloc( WRD_Err );
			}
			else
			{
				int index = packet->param1;
				int depth = packet->param2;

				WRValueSerializer serializer;

				if (depth == 0) // globals are stored separately
				{
					wr_serializeEx( serializer, ((WRValue *)(m_context + 1))[index] );
				}
				else
				{
					WrenchCallStackEntry* frame = (*m_callStack)[depth];
					
					if ( !frame )
					{
						reply = WrenchPacket::alloc( WRD_Err );
					}
					else
					{
						WRValue* stackFrame = m_context->stack + (frame->stackOffset - frame->arguments);
						
						wr_serializeEx( serializer, *(stackFrame + index) );
					}
				}
				
				if ( !reply )
				{
					reply = WrenchPacket::alloc( WRD_ReplyValue, serializer.size() );
					memcpy( reply->payload(), serializer.data(), serializer.size() );
				}
			}
				
			break;
		}

		case WRD_RequestCallstack:
		{
			if ( !m_context || m_halted )
			{
				reply = WrenchPacket::alloc( WRD_ReplyCallstack );
				reply->param1 = 0;
				reply->param2 = (int32_t)(m_context ? WRP_Complete : (m_firstCall ? WRP_Loaded : WRP_Unloaded) );
			}
			else
			{
				uint32_t count = m_callStack->count();

				reply = WrenchPacket::alloc( WRD_ReplyCallstack, sizeof(WrenchCallStackEntry) * count );
				reply->param1 = count;

				reply->param2 = (int32_t)( m_firstCall ? WRP_Loaded : WRP_Running );
				
				WrenchCallStackEntry* pack = (WrenchCallStackEntry *)reply->payload();

				for( WrenchCallStackEntry* E = m_callStack->first(); E; E = m_callStack->next() )
				{
					*pack = *E;
					pack->onLine = wr_x32( pack->onLine );
					pack->locals = wr_x16( pack->locals );
					++pack;
				}
			}
			
			break;
		}

		case WRD_RequestStackDump:
		{
			WRstr dump( "none\n" );
			if ( m_context )
			{
				wr_stackDump( m_context->stack, m_context->yield_stackTop, dump );
			}
				
			reply = WrenchPacket::alloc( WRD_ReplyStackDump, dump.size() );
			memcpy( reply->payload(), dump.c_str(), dump.size() );

			break;
		}

		case WRD_RequestSetBreakpoint:
		{
			clearBreakpoint( packet->param1 );
			*m_lineBreaks->addTail() = packet->param1;
			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		case WRD_RequestClearBreakpoint:
		{
			clearBreakpoint( packet->param1 );
			reply = WrenchPacket::alloc( WRD_Ok );
			break;
		}

		default:
		{
			reply = WrenchPacket::alloc( WRD_Err );
			break;
		}
	}

	return reply;
}

#endif
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::WRDebugClientInterfacePrivate()
{
	init();
}

//------------------------------------------------------------------------------
void wr_FunctionClearFunc( WrenchFunction& F )
{
	delete F.vars;
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::init()
{
	memset( this, 0, sizeof(*this) );
	m_functions = new SimpleLL<WrenchFunction>( wr_FunctionClearFunc );
	m_callStack = new SimpleLL<WrenchCallStackEntry>();
}

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::~WRDebugClientInterfacePrivate()
{
	g_free( m_sourceBlock );

	delete m_functions;
	delete m_callStack;
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::getValues( SimpleLL<WrenchDebugValue>& values, const int depth )
{
	values.clear();

	WrenchFunction* g = (*m_functions)[depth];


	for( unsigned int i = 0; i<g->vars->count(); ++i )
	{
		WrenchDebugValue* v = values.addTail();

		strncpy( v->name, (*g->vars)[i]->label, 63 );
		m_parent->getValue( v->value, i, depth );
	}
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugClientInterfacePrivate::getPacket( const int timeoutMilliseconds )
{
	for(;;)
	{
		WrenchPacket* packet = m_comm->receive( timeoutMilliseconds );

		if ( !packet || (packet->_type != WRD_DebugOut) )
		{
			return packet;
		}

		if ( outputDebug )
		{
			outputDebug( (const char*)packet->payload() );
		}

		g_free( packet );
	}
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::populateSymbols()
{
	if ( m_symbolsLoaded )
	{
		return;
	}

	m_functions->clear();

	m_comm->send( WrenchPacket(WRD_RequestSymbolBlock) );

	WrenchPacketScoped r( m_comm->receive() );

	if ( !r || r.packet->_type != WRD_ReplySymbolBlock )
	{
		return;
	}

	m_symbolsLoaded = true;

	const unsigned char *block = (const unsigned char *)r.packet->payload();

	int functionCount = READ_16_FROM_PC( block );
	int pos = 2;

	for( int f=0; f<functionCount; ++f )
	{
		uint8_t count = READ_8_FROM_PC( block + pos );
		++pos;

		WrenchFunction* func = m_functions->addTail(); // so first is always "global"

		func->arguments = READ_8_FROM_PC( block + pos++ );
		sscanf( (const char *)(block + pos), "%63s", func->name );

		while( block[pos++] ); // skip to next null

		func->vars = new SimpleLL<WrenchSymbol>();
		for( uint8_t v=0; v<count; ++v )
		{
			WrenchSymbol* s = func->vars->addTail();
			sscanf( (const char*)(block + pos), "%63s", s->label );
			while( block[pos++] );
		}
	}
}


#endif
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

#include "wrench.h"

#if (defined(WRENCH_WIN32_TCP) || defined(WRENCH_LINUX_TCP)) && defined(WRENCH_INCLUDE_DEBUG_CODE)

//------------------------------------------------------------------------------
WrenchDebugTcpInterface::WrenchDebugTcpInterface()
{
	m_socket = -1;

#ifdef WRENCH_WIN32_TCP
	WSADATA wsaData;
	WSAStartup( MAKEWORD(2, 2), &wsaData );
#endif	
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::serve( const int port )
{
	wr_closeTCPEx( m_socket );
	m_socket = wr_bindTCPEx( port );
	return m_socket != -1;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::accept()
{
	sockaddr info;
	int socket = wr_acceptTCPEx( m_socket, &info );
	if ( socket == -1 )
	{
		return false;
	}

	wr_closeTCPEx( m_socket );
	m_socket = socket;
	
	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::connect( const char* address, const int port )
{
	wr_closeTCPEx( m_socket );
	return (m_socket = wr_connectTCPEx( address, port )) != -1;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::sendEx( const uint8_t* data, const int bytes )
{
	int soFar = 0;
	while( soFar < bytes )
	{
		int ret = wr_sendTCPEx( m_socket, (const char*)data + soFar, bytes - soFar );
		if ( ret == -1 )
		{
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugTcpInterface::recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds )
{
	int soFar = 0;

	while( soFar < bytes )
	{
		int ret = wr_receiveTCPEx( m_socket, (char*)(data + soFar), bytes - soFar, timeoutMilliseconds );
		if ( ret == -1 )
		{
			return false;
		}
		
		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WrenchDebugTcpInterface::peekEx( const int timeoutMilliseconds )
{
	int ret = wr_peekTCPEx( m_socket, timeoutMilliseconds );
	if ( ret < 0 )
	{
		wr_closeTCPEx( m_socket );
		m_socket = -1;
		return 0;
	}
			
	return ret;
}

#endif
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

#include "wrench.h"

#if (defined(WRENCH_WIN32_SERIAL) || defined(WRENCH_LINUX_SERIAL) || defined(WRENCH_ARDUINO_SERIAL)) && defined(WRENCH_INCLUDE_DEBUG_CODE)

//------------------------------------------------------------------------------
WrenchDebugSerialInterface::WrenchDebugSerialInterface()
{
#ifndef WRENCH_ARDUINO_SERIAL
	m_interface = WR_BAD_SOCKET;
#endif
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::open( const char* name )
{
	m_interface = wr_serialOpen( name );
	return m_interface != WR_BAD_SOCKET;
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::sendEx( const uint8_t* data, const int bytes )
{
	int soFar = 0;
	while( soFar < bytes )
	{
		int ret = wr_serialSend( m_interface, (const char*)data + soFar, bytes - soFar );
		if ( ret == -1 )
		{
			wr_serialClose( m_interface );
			m_interface = WR_BAD_SOCKET;
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
bool WrenchDebugSerialInterface::recvEx( uint8_t* data, const int bytes, const int timeoutMilliseconds )
{
	int soFar = 0;

	while( soFar < bytes )
	{
		int ret = wr_serialReceive( m_interface, (char*)(data + soFar), bytes - soFar );
		if ( ret == -1 )
		{
			wr_serialClose( m_interface );
			m_interface = WR_BAD_SOCKET;
			return false;
		}

		soFar += ret;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WrenchDebugSerialInterface::peekEx( const int timeoutMilliseconds )
{
	int ret = wr_serialPeek( m_interface );
	if ( ret < 0 )
	{
		wr_serialClose( m_interface );
		m_interface = WR_BAD_SOCKET;
		return 0;
	}

	return (uint32_t)ret;
}

#endif
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

#include "wrench.h"

#ifdef WRENCH_TIME_SLICES

//------------------------------------------------------------------------------
struct WrenchScheduledTask
{
	WRContext* context;
	WrenchScheduledTask* next;
	int id;
};

//------------------------------------------------------------------------------
WrenchScheduler::WrenchScheduler( const int stackSizePerThread )
{
	m_w = wr_newState( stackSizePerThread );
	m_tasks = 0;
}

//------------------------------------------------------------------------------
WrenchScheduler::~WrenchScheduler()
{
	while( m_tasks )
	{
		WrenchScheduledTask* next = m_tasks->next;
		g_free( m_tasks );
		m_tasks = next;
	}

	wr_destroyState( m_w );
}

//------------------------------------------------------------------------------
void WrenchScheduler::tick( int instructionsPerSlice )
{
	wr_setInstructionsPerSlice( m_w, instructionsPerSlice );
	WrenchScheduledTask* task = m_tasks;

	while( task )
	{
		if ( !task->context->yield_pc )
		{
			const int id = task->id;
			task = task->next;
			removeTask( id );
		}
		else
		{
			wr_callFunction( task->context, (WRFunction*)0, task->context->yield_argv, task->context->yield_argn );
			task = task->next;
		}
	}
}

static int wr_idGenerator = 0;
//------------------------------------------------------------------------------
int WrenchScheduler::addThread( const uint8_t* byteCode, const int size, const int instructionsThisSlice, const bool takeOwnership )
{
	wr_setInstructionsPerSlice( m_w, instructionsThisSlice );

	WRContext* context = wr_run( m_w, byteCode, size, takeOwnership );
	if ( !context )
	{
		return -1; // error running task
	}
	else if ( !wr_getYieldInfo(context) )
	{
		return 0; // task ran to completion there was no need to yield
	}

	WrenchScheduledTask* task = (WrenchScheduledTask*)g_malloc( sizeof(WrenchScheduledTask) );
	task->context = context;
	task->next = m_tasks;
	task->id = ++wr_idGenerator;
	m_tasks = task;
			  	
	return task->id;
}

//------------------------------------------------------------------------------
bool WrenchScheduler::removeTask( const int taskId )
{
	WrenchScheduledTask* last = 0;
	WrenchScheduledTask* task = m_tasks;
	while( task )
	{
		if ( task->id == taskId )
		{
			wr_destroyContext( m_tasks->context );

			if ( last )
			{
				last->next = task->next;
			}
			else
			{
				m_tasks = task->next;
			}
			
			g_free( task );
			return true;
		}

		last = task;
		task = task->next;
	}
	
	return false;
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
void wr_growValueArray( WRGCObject* va, int newMinIndex )
{
	int size_of = (va->m_type == SV_CHAR) ? 1 : sizeof(WRValue);

	// increase size to accomodate new element
	int size_el = va->m_size * size_of;

	// create new array to hold the data, and g_free the existing one
	uint8_t* old = va->m_Cdata;

	va->m_Cdata = (uint8_t *)g_malloc( (newMinIndex + 1) * size_of );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !va->m_Cdata )
	{
		va->m_Cdata = old;
		g_mallocFailed = true;
		return;
	}
#endif
	
	memcpy( va->m_Cdata, old, size_el );
	g_free( old );
	
	va->m_size = newMinIndex + 1;

	// clear new entries
	memset( va->m_Cdata + size_el, 0, (va->m_size * size_of) - size_el );
}

static WRValue s_temp1;

//------------------------------------------------------------------------------
WRValue& WRValue::singleValue() const
{
	if ( (s_temp1 = deref()).type > WR_FLOAT )
	{
		s_temp1.ui = s_temp1.getHash();
		s_temp1.p2 = INIT_AS_INT;
	}

	return s_temp1;
}

static WRValue s_temp2;

//------------------------------------------------------------------------------
WRValue& WRValue::deref() const
{
	if ( type == WR_REF )
	{
		return r->deref();
	}

	if ( !IS_CONTAINER_MEMBER(xtype) )
	{
		return const_cast<WRValue&>(*this);
	}

	s_temp2.p2 = INIT_AS_INT;
	unsigned int s = DECODE_ARRAY_ELEMENT_FROM_P2(p2);

	if ( IS_RAW_ARRAY(r->xtype) )
	{
		s_temp2.ui = (s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(r->p2))) ? (uint32_t)(unsigned char)(r->c[s]) : 0;
	}
	else if ( vb->m_type == SV_HASH_INTERNAL )
	{
		return ((WRValue*)(vb + 1))[s].deref();
	}
	else if ( s < r->va->m_size )
	{
		if ( r->va->m_type == SV_VALUE )
		{
			return r->va->m_Vdata[s];
		}
		else
		{
			s_temp2.ui = (uint32_t)(unsigned char)r->va->m_Cdata[s];
		}
	}

	return s_temp2;
}

//------------------------------------------------------------------------------
uint32_t WRValue::getHashEx() const
{
	// QUICKLY return the easy answers, that's why this code looks a bit convoluted
	if ( type == WR_REF )
	{
		return r->getHash();
	}

	if ( IS_CONTAINER_MEMBER(xtype) )
	{
		return deref().getHash();
	}
	else if ( xtype == WR_EX_ARRAY )
	{
		if (va->m_type == SV_CHAR)
		{
			return wr_hash( va->m_Cdata, va->m_size );
		}
		else if (va->m_type == SV_VALUE)
		{
			return wr_hash(va->m_Vdata, va->m_size * sizeof(WRValue));
		}
	}
	else if ( xtype == WR_EX_HASH_TABLE )
	{
		// start with a hash of the key hashes
		uint32_t hash = wr_hash(va->m_hashTable, va->m_mod * sizeof(uint32_t));

		// hash each element, positionally dependant
		for( uint32_t i=0; i<va->m_mod; ++i)
		{
			if ( va->m_hashTable[i] != WRENCH_NULL_HASH )
			{
				uint32_t h = i<<16 | va->m_Vdata[i<<1].getHash();
				hash = wr_hash( &h, 4, hash );
			}
		}
		return hash;
	}
	else if ( xtype == WR_EX_STRUCT )
	{
		uint32_t hash = wr_hash( (char *)&va->m_hashTable, sizeof(void*) ); // this is in ROM and must be identical for the struct to be the same (same namespace)

		for( uint32_t i=0; i<va->m_mod; ++i )
		{
			const uint8_t* offset = va->m_ROMHashTable + (i * 5);
			if ( (uint32_t)READ_32_FROM_PC(offset) != WRENCH_NULL_HASH)
			{
				uint32_t h = va->m_Vdata[READ_8_FROM_PC(offset + 4)].getHash();
				hash = wr_hash( &h, 4, hash );
			}
		}

		return hash;
	}

	return 0;
}

//------------------------------------------------------------------------------
void wr_valueToContainer( const WRValue* ex, WRValue* value )
{
	uint32_t s = DECODE_ARRAY_ELEMENT_FROM_P2(ex->p2);

	if ( ex->vb->m_type == SV_HASH_INTERNAL )
	{
		((WRValue *)(ex->vb + 1))[s].deref() = *value;
	}
	else if ( IS_RAW_ARRAY(ex->r->xtype ) )
	{
		if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(ex->r->p2)) )
		{
			ex->r->c[s] = value->ui;
		}
	}
	else if ( !IS_HASH_TABLE(ex->xtype) )
	{
		if ( s >= ex->r->va->m_size )
		{
			wr_growValueArray( ex->r->va, s );
			ex->r->va->m_creatorContext->allocatedMemoryHint += s * ((ex->r->va->m_type == SV_CHAR) ? 1 : sizeof(WRValue));
		}
		
		if ( ex->r->va->m_type == SV_CHAR )
		{
			ex->r->va->m_Cdata[s] = value->ui;
		}
		else 
		{
			WRValue* V = ex->r->va->m_Vdata + s;
			wr_assign[(V->type<<2)+value->type](V, value);
		}
	}
}

//------------------------------------------------------------------------------
void wr_addLibraryCleanupFunction( WRState* w, void (*function)(WRState* w, void* param), void* param )
{
	WRLibraryCleanup* entry = (WRLibraryCleanup *)g_malloc(sizeof(WRLibraryCleanup));
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !entry )
	{
		g_mallocFailed = true;
		w->err = WR_ERR_malloc_failed;
		return;
	}
#endif
	
	entry->cleanupFunction = function;
	entry->param = param;
	entry->next = w->libCleanupFunctions;
	w->libCleanupFunctions = entry;
}

//------------------------------------------------------------------------------
void wr_countOfArrayElement( WRValue* array, WRValue* target )
{
	array = &array->deref();
				
	if ( IS_EXARRAY_TYPE(array->xtype) )
	{
		target->i = array->va->m_size;
	}
	else if ( IS_EX_RAW_ARRAY_TYPE(array->xtype) )
	{
		target->i = EX_RAW_ARRAY_SIZE_FROM_P2(array->p2);
	}
	else
	{
		target->i = 0;
	}

	target->p2 = INIT_AS_INT;
}

//------------------------------------------------------------------------------
// put from into TO as a hash table, if to is not a hash table, make it
// one, leave the result in target
void wr_assignToHashTable( WRContext* c, WRValue* index, WRValue* value, WRValue* table )
{
	if ( value->type == WR_REF )
	{
		wr_assignToHashTable( c, index, value->r, table );
		return;
	}
	
	if ( table->type == WR_REF )
	{
		wr_assignToHashTable( c, index, value, table->r );
		return;
	}

	if ( index->type == WR_REF )
	{
		wr_assignToHashTable( c, index->r, value, table );
		return;
	}

	if ( table->xtype != WR_EX_HASH_TABLE )
	{
		table->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !table->va )
		{
			table->p2 = INIT_AS_INT;
			return;
		}
#endif
		table->p2 = INIT_AS_HASH_TABLE;
	}

	WRValue *entry = (WRValue *)table->va->get( index->getHash() );

	*entry++ = *value;
	*entry = *index;
}

//------------------------------------------------------------------------------
bool doLogicalNot_X( WRValue* value ) { return value->i == 0; } // float also uses 0x0000000 as 'zero'
bool doLogicalNot_R( WRValue* value ) { return wr_LogicalNot[ value->r->type ]( value->r ); }
bool doLogicalNot_E( WRValue* value )
{
	WRValue& V = value->singleValue();
	return wr_LogicalNot[ V.type ]( &V );
}
WRReturnSingleFunc wr_LogicalNot[4] = 
{
	doLogicalNot_X,  doLogicalNot_X,  doLogicalNot_R,  doLogicalNot_E
};


//------------------------------------------------------------------------------
void doNegate_I( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = -value->i; }
void doNegate_F( WRValue* value, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = -value->f; }
void doNegate_E( WRValue* value, WRValue* target )
{
	WRValue& V = value->singleValue();
	return wr_negate[ V.type ]( &V, target );
}
void doNegate_R( WRValue* value, WRValue* target )
{
	wr_negate[ value->r->type ]( value->r, target );
}

WRSingleTargetFunc wr_negate[4] = 
{
	doNegate_I,  doNegate_F,  doNegate_R,  doNegate_E
};

//------------------------------------------------------------------------------
uint32_t doBitwiseNot_I( WRValue* value ) { return ~value->ui; }
uint32_t doBitwiseNot_F( WRValue* value ) { return 0; }
uint32_t doBitwiseNot_R( WRValue* value ) { return wr_bitwiseNot[ value->r->type ]( value->r ); }
uint32_t doBitwiseNot_E( WRValue* value )
{
	WRValue& V = value->singleValue();
	return wr_bitwiseNot[ V.type ]( &V );
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
void doAssign_IF_E( WRValue* to, WRValue* from )
{
	*to = from->deref();
}

void doAssign_E_IF( WRValue* to, WRValue* from )
{
	if ( IS_RAW_ARRAY(to->r->xtype) )
	{
		uint32_t s = DECODE_ARRAY_ELEMENT_FROM_P2(to->p2);

		if ( s < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(to->r->p2)) )
		{
			to->r->c[s] = from->ui;
		}
	}
	else
	{
		to->deref() = *from;
	}
}

void doAssign_E_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->deref();
	if ( IS_CONTAINER_MEMBER(to->xtype) )
	{
		wr_valueToContainer( to, &V );
	}
	else
	{
		*to = V;
	}
}


void doAssign_R_E( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_EX](to->r, from); }
void doAssign_R_R( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|from->r->type](to->r, from->r); }
void doAssign_E_R( WRValue* to, WRValue* from ) { wr_assign[(WR_EX<<2)|from->r->type](to, from->r); }
void doAssign_R_I( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_INT](to->r, from); }
void doAssign_R_F( WRValue* to, WRValue* from ) { wr_assign[(to->r->type<<2)|WR_FLOAT](to->r, from); }
void doAssign_I_R( WRValue* to, WRValue* from ) { wr_assign[(WR_INT<<2)|from->r->type](to, from->r); }
void doAssign_F_R( WRValue* to, WRValue* from ) { wr_assign[(WR_FLOAT<<2)|from->r->type](to, from->r); }
void doAssign_X_X( WRValue* to, WRValue* from ) { *to = *from; }
WRVoidFunc wr_assign[16] = 
{
	doAssign_X_X,  doAssign_X_X,  doAssign_I_R,  doAssign_IF_E,
	doAssign_X_X,  doAssign_X_X,  doAssign_F_R,  doAssign_IF_E,
	doAssign_R_I,  doAssign_R_F,  doAssign_R_R,  doAssign_R_E,
	doAssign_E_IF, doAssign_E_IF, doAssign_E_R,  doAssign_E_E,
};

//==================================================================================
//==================================================================================
//==================================================================================
//==================================================================================

#ifdef WRENCH_COMPACT

extern bool CompareEQI( int a, int b );
extern bool CompareEQF( float a, float b );

//------------------------------------------------------------------------------
void unaryPost_E( WRValue* value, WRValue* stack, int add )
{
	WRValue& V = value->singleValue();
	WRValue temp;
	m_unaryPost[ V.type ]( &V, &temp, add );
	wr_valueToContainer( value, &V );
	*stack = temp;
}
void unaryPost_I( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_INT; stack->i = value->i; value->i += add; }
void unaryPost_R( WRValue* value, WRValue* stack, int add ) { m_unaryPost[ value->r->type ]( value->r, stack, add ); }
void unaryPost_F( WRValue* value, WRValue* stack, int add ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f; value->f += add; }
WRVoidPlusFunc m_unaryPost[4] = 
{
	unaryPost_I,  unaryPost_F,  unaryPost_R, unaryPost_E
};

void FuncAssign_R_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	wr_FuncAssign[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall);
}
void FuncAssign_E_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue& V = to->singleValue();
	
	wr_FuncAssign[(V.type<<2)|from->type]( &V, from, intCall, floatCall );
	
	wr_valueToContainer( to, &V );
	*from = V;
}
void FuncAssign_E_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall ) 
{
	if ( IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();

		wr_FuncAssign[(WR_EX<<2)|V.type]( to, &V, intCall, floatCall );
		*from = to->deref();
	}
	else if ( intCall == wr_addI
			  && IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !(to->va->m_flags & GCFlag_SkipGC)
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		char* t = to->va->m_SCdata;
		to->va->m_SCdata = (char*)g_malloc( to->va->m_size + from->va->m_size );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !to->va->m_SCdata )
		{
			to->va->m_SCdata = t;
			g_mallocFailed = true;
			return;
		}
#endif
		memcpy( to->va->m_SCdata, t, to->va->m_size );
		memcpy( to->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size );
		to->va->m_size = to->va->m_size + from->va->m_size;
		g_free( t );
	}
}
void FuncAssign_X_E( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue& V = from->singleValue();
	wr_FuncAssign[(to->type<<2)|V.type](to, &V, intCall, floatCall);
	*from = *to;
}
void FuncAssign_E_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall);
}

void FuncAssign_R_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	WRValue temp = *from->r; 
	wr_FuncAssign[(to->r->type<<2)|temp.type](to->r, &temp, intCall, floatCall); 
	*from = *to->r;
}

void FuncAssign_R_X( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); *from = *to->r;
}

void FuncAssign_X_R( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_FuncAssign[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); *from = *to;
}

void FuncAssign_F_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->f = floatCall( to->f, from->f );
}

void FuncAssign_I_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->i = intCall( to->i, from->i );
}

void FuncAssign_I_F( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	to->p2 = INIT_AS_FLOAT;
	to->f = floatCall( (float)to->i, from->f );
}
void FuncAssign_F_I( WRValue* to, WRValue* from, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
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
void FuncBinary_E_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	WRValue& V = to->singleValue();
	wr_funcBinary[(V.type<<2)|from->type](&V, from, target, intCall, floatCall);
}
void FuncBinary_E_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	if ( IS_CONTAINER_MEMBER(to->xtype) && IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue V1 = to->deref();
		WRValue& V2 = from->deref();
		wr_funcBinary[(V1.type<<2)|V2.type](&V1, &V2, target, intCall, floatCall );
	}
	else if ( intCall == wr_addI
			  && IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !(to->va->m_flags & GCFlag_SkipGC)
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		target->va = from->va->m_creatorContext->getSVA( from->va->m_size + to->va->m_size, SV_CHAR, false );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !target->va )
		{
			target->p2 = INIT_AS_INT;
			return;
		}
#endif
		target->p2 = INIT_AS_ARRAY;

		memcpy( target->va->m_SCdata, to->va->m_SCdata, to->va->m_size );
		memcpy( target->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size );
	}
}
void FuncBinary_X_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	WRValue& V = from->singleValue();
	wr_funcBinary[(to->type<<2)|V.type](to, &V, target, intCall, floatCall);
}
void FuncBinary_E_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(WR_EX<<2)|from->r->type](to, from->r, target, intCall, floatCall );
}

void FuncBinary_R_E( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
{
	wr_funcBinary[(to->r->type<<2)|WR_EX](to->r, from, target, intCall, floatCall );
}

void FuncBinary_X_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->type<<2)+from->r->type](to, from->r, target, intCall, floatCall);
}

void FuncBinary_R_X( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->type](to->r, from, target, intCall, floatCall);
}

void FuncBinary_R_R( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	wr_funcBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target, intCall, floatCall);
}

void FuncBinary_I_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_INT; target->i = intCall( to->i, from->i );
}

void FuncBinary_I_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( (float)to->i, from->f );
}

void FuncBinary_F_I( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall )
{
	target->p2 = INIT_AS_FLOAT; target->f = floatCall( to->f, (float)from->i );
}

void FuncBinary_F_F( WRValue* to, WRValue* from, WRValue* target, WRFuncIntCall intCall, WRFuncFloatCall floatCall  )
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



bool Compare_E_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue V1 = to->singleValue();
	WRValue& V2 = from->singleValue();
	return wr_Compare[(V1.type<<2)|V2.type](&V1, &V2, intCall, floatCall);
}
bool Compare_E_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue& V = to->singleValue();
	return wr_Compare[(V.type<<2)|from->type](&V, from, intCall, floatCall);
}
bool Compare_X_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall )
{
	WRValue& V = from->singleValue();
	return wr_Compare[(to->type<<2)|V.type](to, &V, intCall, floatCall );
}
bool Compare_R_E( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|WR_EX](to->r, from, intCall, floatCall); }
bool Compare_E_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(WR_EX<<2)|from->r->type](to, from->r, intCall, floatCall); }
bool Compare_R_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->r->type](to->r, from->r, intCall, floatCall); }
bool Compare_R_X( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->r->type<<2)|from->type](to->r, from, intCall, floatCall); }
bool Compare_X_R( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return wr_Compare[(to->type<<2)+from->r->type](to, from->r, intCall, floatCall); }
bool Compare_I_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return intCall( to->i, from->i ); }
bool Compare_I_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( (float)to->i, from->f); }
bool Compare_F_I( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, (float)from->i); }
bool Compare_F_F( WRValue* to, WRValue* from, WRCompareFuncIntCall intCall, WRCompareFuncFloatCall floatCall ) { return floatCall( to->f, from->f); }
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

void doVoidFuncBlank( WRValue* to, WRValue* from ) {}

#define X_INT_ASSIGN( NAME, OPERATION ) \
void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	NAME##Assign[(WR_EX<<2)|from->r->type]( to, from->r );\
}\
void NAME##Assign_R_E( WRValue* to, WRValue* from ) \
{\
	NAME##Assign[(to->r->type<<2)|WR_EX](to->r, from);\
}\
void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_INT]( &V, from );\
	\
	wr_valueToContainer( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	WRValue& V = from->singleValue();\
	\
	NAME##Assign[(WR_EX<<2)+V.type]( to, &V );\
	*from = to->deref();\
}\
void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_INT<<2)+V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
void NAME##Assign_I_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(WR_INT<<2)+temp.type](to, &temp); *from = *to; }\
void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
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
void NAME##Assign_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_INT]( &V, from );\
	\
	wr_valueToContainer( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_F( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	\
	NAME##Assign[(V.type<<2)|WR_FLOAT]( &V, from );\
	\
	wr_valueToContainer( to, &V );\
	*from = V;\
}\
void NAME##Assign_E_E( WRValue* to, WRValue* from ) \
{\
	WRValue& V = from->singleValue();\
	\
	NAME##Assign[(WR_EX<<2)|V.type]( to, &V );\
	*from = to->deref();\
}\
void NAME##Assign_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_INT<<2)|V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_F_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	NAME##Assign[(WR_FLOAT<<2)|V.type](to, &V);\
	*from = *to;\
}\
void NAME##Assign_E_R( WRValue* to, WRValue* from )\
{\
	NAME##Assign[(WR_EX<<2)|from->r->type]( to, from->r );\
}\
void NAME##Assign_R_E( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_EX](to->r, from); *from = *to->r; } \
void NAME##Assign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; NAME##Assign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }\
void NAME##Assign_R_I( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }\
void NAME##Assign_R_F( WRValue* to, WRValue* from ) { NAME##Assign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }\
void NAME##Assign_I_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }\
void NAME##Assign_F_R( WRValue* to, WRValue* from ) { NAME##Assign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }\
void NAME##Assign_F_F( WRValue* to, WRValue* from ) { to->f OPERATION##= from->f; }\
void NAME##Assign_I_I( WRValue* to, WRValue* from ) { to->i OPERATION##= from->i; }\
void NAME##Assign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i OPERATION from->f; }\
void NAME##Assign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f OPERATION##= (float)from->i; }\
WRVoidFunc NAME##Assign[16] = \
{\
	NAME##Assign_I_I,  NAME##Assign_I_F,  NAME##Assign_I_R,  NAME##Assign_I_E,\
	NAME##Assign_F_I,  NAME##Assign_F_F,  NAME##Assign_F_R,  NAME##Assign_F_E,\
	NAME##Assign_R_I,  NAME##Assign_R_F,  NAME##Assign_R_R,  NAME##Assign_R_E,\
	NAME##Assign_E_I,  NAME##Assign_E_F,  NAME##Assign_E_R,  NAME##Assign_E_E,\
};\


X_ASSIGN( wr_Subtract, - );
//X_ASSIGN( wr_Add, + ); -- broken out so strings work
X_ASSIGN( wr_Multiply, * );
//X_ASSIGN( wr_Divide, / ); -- broken out for divide by zero


void wr_DivideAssign_E_I( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();

	wr_DivideAssign[(V.type<<2)|WR_INT]( &V, from );

	wr_valueToContainer( to, &V );
	*from = V;
}
void wr_DivideAssign_E_F( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();

	wr_DivideAssign[(V.type<<2)|WR_FLOAT]( &V, from );

	wr_valueToContainer( to, &V );
	*from = V;
}
void wr_DivideAssign_E_E( WRValue* to, WRValue* from ) 
{
	WRValue& V = from->singleValue();

	wr_DivideAssign[(WR_EX<<2)|V.type]( to, &V );
	*from = to->deref();
}
void wr_DivideAssign_I_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_DivideAssign[(WR_INT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_DivideAssign_F_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_DivideAssign[(WR_FLOAT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_DivideAssign_E_R( WRValue* to, WRValue* from )
{
	wr_DivideAssign[(WR_EX<<2)|from->r->type]( to, from->r );
}
void wr_DivideAssign_R_E( WRValue* to, WRValue* from ) { wr_DivideAssign[(to->r->type<<2)|WR_EX](to->r, from); *from = *to->r; } 
void wr_DivideAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_DivideAssign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }
void wr_DivideAssign_R_I( WRValue* to, WRValue* from ) { wr_DivideAssign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }
void wr_DivideAssign_R_F( WRValue* to, WRValue* from ) { wr_DivideAssign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }
void wr_DivideAssign_I_R( WRValue* to, WRValue* from ) { wr_DivideAssign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }
void wr_DivideAssign_F_R( WRValue* to, WRValue* from ) { wr_DivideAssign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }

void wr_DivideAssign_F_F( WRValue* to, WRValue* from )
{
	if ( from->f )
	{
		to->f = to->f / from->f;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		to->p2 = INIT_AS_INVALID;
#else
		to->i = 0;
#endif
	}

}
void wr_DivideAssign_I_I( WRValue* to, WRValue* from )
{
	if ( from->i )
	{
		to->i = to->i / from->i;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		to->p2 = INIT_AS_INVALID;
#else
		to->i = 0;
#endif
	}
}
void wr_DivideAssign_I_F( WRValue* to, WRValue* from )
{
	to->p2 = INIT_AS_FLOAT;
	if ( from->f )
	{
		to->f = (float)to->i / from->f;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		to->p2 = INIT_AS_INVALID;
#else
		to->i = 0;
#endif
	}
}
void wr_DivideAssign_F_I( WRValue* to, WRValue* from )
{
	from->p2 = INIT_AS_FLOAT;
	if ( from->i )
	{
		to->f = to->f / (float)from->i;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		to->p2 = INIT_AS_INVALID;
#else
		to->i = 0;
#endif
	}
}

WRVoidFunc wr_DivideAssign[16] = 
{
	wr_DivideAssign_I_I,  wr_DivideAssign_I_F,  wr_DivideAssign_I_R,  wr_DivideAssign_I_E,
	wr_DivideAssign_F_I,  wr_DivideAssign_F_F,  wr_DivideAssign_F_R,  wr_DivideAssign_F_E,
	wr_DivideAssign_R_I,  wr_DivideAssign_R_F,  wr_DivideAssign_R_R,  wr_DivideAssign_R_E,
	wr_DivideAssign_E_I,  wr_DivideAssign_E_F,  wr_DivideAssign_E_R,  wr_DivideAssign_E_E,
};


void wr_AddAssign_E_I( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	
	wr_AddAssign[(V.type<<2)|WR_INT]( &V, from );
	
	wr_valueToContainer( to, &V );
	*from = V;
}

void wr_AddAssign_E_F( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();

	wr_AddAssign[(V.type<<2)|WR_FLOAT]( &V, from );

	wr_valueToContainer( to, &V );
	*from = V;
}
void wr_AddAssign_E_E( WRValue* to, WRValue* from ) 
{
	if ( IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue& V = from->deref();

		wr_AddAssign[(WR_EX<<2)|V.type]( to, &V );
		*from = to->deref();
	}
	else if ( IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !(to->va->m_flags & GCFlag_SkipGC)
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		char* t = to->va->m_SCdata;
		to->va->m_SCdata = (char*)g_malloc( to->va->m_size + from->va->m_size );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !to->va->m_SCdata )
		{
			to->va->m_SCdata = t;
			g_mallocFailed = true;
			return;
		}
#endif
		memcpy( to->va->m_SCdata, t, to->va->m_size );
		memcpy( to->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size );
		to->va->m_size = to->va->m_size + from->va->m_size;
		g_free( t );
	}
}
void wr_AddAssign_I_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_AddAssign[(WR_INT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_AddAssign_F_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	wr_AddAssign[(WR_FLOAT<<2)|V.type](to, &V);
	*from = *to;
}
void wr_AddAssign_E_R( WRValue* to, WRValue* from )
{
	wr_AddAssign[(WR_EX<<2)|from->r->type]( to, from->r );
}
void wr_AddAssign_R_E( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_EX](to->r, from); *from = *to->r; }
void wr_AddAssign_R_R( WRValue* to, WRValue* from ) { WRValue temp = *from->r; wr_AddAssign[(to->r->type<<2)|temp.type](to->r, &temp); *from = *to->r; }
void wr_AddAssign_R_I( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_INT](to->r, from); *from = *to->r; }
void wr_AddAssign_R_F( WRValue* to, WRValue* from ) { wr_AddAssign[(to->r->type<<2)|WR_FLOAT](to->r, from); *from = *to->r; }
void wr_AddAssign_I_R( WRValue* to, WRValue* from ) { wr_AddAssign[(WR_INT<<2)+from->r->type](to, from->r); *from = *to; }
void wr_AddAssign_F_R( WRValue* to, WRValue* from ) { wr_AddAssign[(WR_FLOAT<<2)+from->r->type](to, from->r); *from = *to; }
void wr_AddAssign_F_F( WRValue* to, WRValue* from ) { to->f += from->f; }
void wr_AddAssign_I_I( WRValue* to, WRValue* from ) { to->i += from->i; }
void wr_AddAssign_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; to->f = (float)to->i + from->f; }
void wr_AddAssign_F_I( WRValue* to, WRValue* from ) { from->p2 = INIT_AS_FLOAT; to->f += (float)from->i; }
WRVoidFunc wr_AddAssign[16] = 
{
	wr_AddAssign_I_I,  wr_AddAssign_I_F,  wr_AddAssign_I_R,  wr_AddAssign_I_E,
	wr_AddAssign_F_I,  wr_AddAssign_F_F,  wr_AddAssign_F_R,  wr_AddAssign_F_E,
	wr_AddAssign_R_I,  wr_AddAssign_R_F,  wr_AddAssign_R_R,  wr_AddAssign_R_E,
	wr_AddAssign_E_I,  wr_AddAssign_E_F,  wr_AddAssign_E_R,  wr_AddAssign_E_E,
};


//------------------------------------------------------------------------------
#define X_BINARY( NAME, OPERATION ) \
void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_INT](&V, from, target);\
}\
void NAME##Binary_E_F( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_FLOAT](&V, from, target);\
}\
void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	NAME##Binary[(V1.type<<2)|V2.type](&V1, &V2, target);\
}\
void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_INT<<2)|V.type](to, &V, target);\
}\
void NAME##Binary_F_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_FLOAT<<2)|V.type](to, &V, target);\
}\
void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_EX]( to->r, from, target); }\
void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_EX<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_R_F( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }\
void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
void NAME##Binary_F_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
void NAME##Binary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i OPERATION from->f; }\
void NAME##Binary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION (float)from->i; }\
void NAME##Binary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f OPERATION from->f; }\
WRTargetFunc NAME##Binary[16] = \
{\
	NAME##Binary_I_I,  NAME##Binary_I_F,  NAME##Binary_I_R,  NAME##Binary_I_E,\
	NAME##Binary_F_I,  NAME##Binary_F_F,  NAME##Binary_F_R,  NAME##Binary_F_E,\
	NAME##Binary_R_I,  NAME##Binary_R_F,  NAME##Binary_R_R,  NAME##Binary_R_E,\
	NAME##Binary_E_I,  NAME##Binary_E_F,  NAME##Binary_E_R,  NAME##Binary_E_E,\
};\

//X_BINARY( wr_Addition, + );  -- broken out so strings work
X_BINARY( wr_Multiply, * );
X_BINARY( wr_Subtract, - );
//X_BINARY( wr_Divide, / );

void wr_DivideBinary_E_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_DivideBinary[(V.type<<2)|WR_INT](&V, from, target);
}
void wr_DivideBinary_E_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_DivideBinary[(V.type<<2)|WR_FLOAT](&V, from, target);
}
void wr_DivideBinary_E_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue V1 = to->singleValue();
	WRValue& V2 = from->singleValue();
	wr_DivideBinary[(V1.type<<2)|V2.type](&V1, &V2, target);
}
void wr_DivideBinary_I_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_DivideBinary[(WR_INT<<2)|V.type](to, &V, target);
}
void wr_DivideBinary_F_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_DivideBinary[(WR_FLOAT<<2)|V.type](to, &V, target);
}
void wr_DivideBinary_R_E( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(to->r->type<<2)|WR_EX]( to->r, from, target); }
void wr_DivideBinary_E_R( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(WR_EX<<2)+from->r->type](to, from->r, target); }
void wr_DivideBinary_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(WR_INT<<2)+from->r->type](to, from->r, target); }
void wr_DivideBinary_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }
void wr_DivideBinary_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }
void wr_DivideBinary_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(to->r->type<<2)|WR_INT](to->r, from, target); }
void wr_DivideBinary_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_DivideBinary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }
void wr_DivideBinary_I_I( WRValue* to, WRValue* from, WRValue* target )
{
	target->p2 = INIT_AS_INT;
	if ( from->i )
	{
		target->i = to->i / from->i;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		target->p2 = INIT_AS_INVALID;
#else
		target->i = 0;
#endif
	}
}
void wr_DivideBinary_I_F( WRValue* to, WRValue* from, WRValue* target )
{
	target->p2 = INIT_AS_FLOAT;
	if ( from->f )
	{
		target->f = (float)to->i / from->f;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		target->p2 = INIT_AS_INVALID;
#else
		target->i = 0;
#endif
	}
}
void wr_DivideBinary_F_I( WRValue* to, WRValue* from, WRValue* target )
{
	target->p2 = INIT_AS_FLOAT;
	if ( from->i )
	{
		target->f = to->f / (float)from->i;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		target->p2 = INIT_AS_INVALID;
#else
		target->i = 0;
#endif
	}
}
void wr_DivideBinary_F_F( WRValue* to, WRValue* from, WRValue* target )
{
	target->p2 = INIT_AS_FLOAT;
	if ( from->f )
	{
		target->f = to->f / from->f;
	}
	else
	{
#ifdef WRENCH_TRAP_DIVISION_BY_ZERO
		target->p2 = INIT_AS_INVALID;
#else
		target->i = 0;
#endif
	}
}

WRTargetFunc wr_DivideBinary[16] = 
{
	wr_DivideBinary_I_I,  wr_DivideBinary_I_F,  wr_DivideBinary_I_R,  wr_DivideBinary_I_E,
	wr_DivideBinary_F_I,  wr_DivideBinary_F_F,  wr_DivideBinary_F_R,  wr_DivideBinary_F_E,
	wr_DivideBinary_R_I,  wr_DivideBinary_R_F,  wr_DivideBinary_R_R,  wr_DivideBinary_R_E,
	wr_DivideBinary_E_I,  wr_DivideBinary_E_F,  wr_DivideBinary_E_R,  wr_DivideBinary_E_E,
};


void wr_AdditionBinary_E_I( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_AdditionBinary[(V.type<<2)|WR_INT](&V, from, target);
}
void wr_AdditionBinary_E_F( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = to->singleValue();
	wr_AdditionBinary[(V.type<<2)|WR_FLOAT](&V, from, target);
}
void wr_AdditionBinary_E_E( WRValue* to, WRValue* from, WRValue* target )
{
	if ( IS_CONTAINER_MEMBER(to->xtype) && IS_CONTAINER_MEMBER(from->xtype) )
	{
		WRValue V1 = to->deref();
		WRValue& V2 = from->deref();
		wr_AdditionBinary[(V1.type<<2)|V2.type](&V1, &V2, target);
	}
	else if ( IS_ARRAY(to->xtype)
			  && to->va->m_type == SV_CHAR
			  && !(to->va->m_flags & GCFlag_SkipGC)
			  && IS_ARRAY(from->xtype)
			  && from->va->m_type == SV_CHAR )
	{
		target->va = to->va->m_creatorContext->getSVA( to->va->m_size + from->va->m_size, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !target->va )
		{
			target->p2 = INIT_AS_INT;
			return;
		}
#endif
		target->p2 = INIT_AS_ARRAY;

		memcpy( target->va->m_SCdata, to->va->m_SCdata, to->va->m_size );
		memcpy( target->va->m_SCdata + to->va->m_size, from->va->m_SCdata, from->va->m_size );
	}
}
void wr_AdditionBinary_I_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_AdditionBinary[(WR_INT<<2)|V.type](to, &V, target);
}
void wr_AdditionBinary_F_E( WRValue* to, WRValue* from, WRValue* target )
{
	WRValue& V = from->singleValue();
	wr_AdditionBinary[(WR_FLOAT<<2)|V.type](to, &V, target);
}
void wr_AdditionBinary_R_E( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_EX]( to->r, from, target); }
void wr_AdditionBinary_E_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_EX<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_I_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_INT<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_R_F( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_FLOAT](to->r, from, target); }
void wr_AdditionBinary_R_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }
void wr_AdditionBinary_R_I( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(to->r->type<<2)|WR_INT](to->r, from, target); }
void wr_AdditionBinary_F_R( WRValue* to, WRValue* from, WRValue* target ) { wr_AdditionBinary[(WR_FLOAT<<2)+from->r->type](to, from->r, target); }
void wr_AdditionBinary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i + from->i; }
void wr_AdditionBinary_I_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = (float)to->i + from->f; }
void wr_AdditionBinary_F_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f + (float)from->i; }
void wr_AdditionBinary_F_F( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_FLOAT; target->f = to->f + from->f; }
WRTargetFunc wr_AdditionBinary[16] = 
{
	wr_AdditionBinary_I_I,  wr_AdditionBinary_I_F,  wr_AdditionBinary_I_R,  wr_AdditionBinary_I_E,
	wr_AdditionBinary_F_I,  wr_AdditionBinary_F_F,  wr_AdditionBinary_F_R,  wr_AdditionBinary_F_E,
	wr_AdditionBinary_R_I,  wr_AdditionBinary_R_F,  wr_AdditionBinary_R_R,  wr_AdditionBinary_R_E,
	wr_AdditionBinary_E_I,  wr_AdditionBinary_E_F,  wr_AdditionBinary_E_R,  wr_AdditionBinary_E_E,
};


void doTargetFuncBlank( WRValue* to, WRValue* from, WRValue* target ) {}

//------------------------------------------------------------------------------
#define X_INT_BINARY( NAME, OPERATION ) \
void NAME##Binary_E_I( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = to->singleValue();\
	NAME##Binary[(V.type<<2)|WR_INT](&V, from, target);\
}\
void NAME##Binary_E_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	NAME##Binary[(V1.type<<2)|V2.type](&V1, &V2, target);\
}\
void NAME##Binary_I_E( WRValue* to, WRValue* from, WRValue* target )\
{\
	WRValue& V = from->singleValue();\
	NAME##Binary[(WR_INT<<2)+V.type](to, &V, target);\
}\
void NAME##Binary_E_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_EX)|from->r->type](to, from->r, target); }\
void NAME##Binary_R_E( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_EX]( to->r, from, target); }\
void NAME##Binary_I_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(WR_INT<<2)+from->r->type](to, from->r, target); }\
void NAME##Binary_R_R( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|from->r->type](to->r, from->r, target); }\
void NAME##Binary_R_I( WRValue* to, WRValue* from, WRValue* target ) { NAME##Binary[(to->r->type<<2)|WR_INT](to->r, from, target); }\
void NAME##Binary_I_I( WRValue* to, WRValue* from, WRValue* target ) { target->p2 = INIT_AS_INT; target->i = to->i OPERATION from->i; }\
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
bool NAME##_E_E( WRValue* to, WRValue* from )\
{\
	WRValue V1 = to->singleValue();\
	WRValue& V2 = from->singleValue();\
	return NAME[(V1.type<<2)|V2.type](&V1, &V2);\
}\
bool NAME##_E_I( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	return NAME[(V.type<<2)|WR_INT](&V, from);\
}\
bool NAME##_E_F( WRValue* to, WRValue* from )\
{\
	WRValue& V = to->singleValue();\
	return NAME[(V.type<<2)|WR_FLOAT](&V, from);\
}\
bool NAME##_I_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	return NAME[(WR_INT<<2)|V.type](to, &V);\
}\
bool NAME##_F_E( WRValue* to, WRValue* from )\
{\
	WRValue& V = from->singleValue();\
	return NAME[(WR_FLOAT<<2)|V.type](to, &V);\
}\
bool NAME##_R_E( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_EX](to->r, from); }\
bool NAME##_E_R( WRValue* to, WRValue* from ) { return NAME[(WR_EX<<2)|from->r->type](to, from->r); }\
bool NAME##_R_R( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|from->r->type](to->r, from->r); }\
bool NAME##_R_I( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_INT](to->r, from); }\
bool NAME##_R_F( WRValue* to, WRValue* from ) { return NAME[(to->r->type<<2)|WR_FLOAT](to->r, from); }\
bool NAME##_I_R( WRValue* to, WRValue* from ) { return NAME[(WR_INT<<2)+from->r->type](to, from->r); }\
bool NAME##_F_R( WRValue* to, WRValue* from ) { return NAME[(WR_FLOAT<<2)+from->r->type](to, from->r); }\
bool NAME##_I_I( WRValue* to, WRValue* from ) { return to->i OPERATION from->i; }\
bool NAME##_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->i OPERATION from->f; }\
bool NAME##_F_I( WRValue* to, WRValue* from ) { return to->f OPERATION (float)from->i; }\
bool NAME##_F_F( WRValue* to, WRValue* from ) { return to->f OPERATION from->f; }\
WRReturnFunc NAME[16] = \
{\
	NAME##_I_I, NAME##_I_F, NAME##_I_R, NAME##_I_E,\
	NAME##_F_I, NAME##_F_F, NAME##_F_R, NAME##_F_E,\
	NAME##_R_I, NAME##_R_F, NAME##_R_R, NAME##_R_E,\
	NAME##_E_I, NAME##_E_F, NAME##_E_R, NAME##_E_E,\
};\

X_COMPARE( wr_CompareGT, > );
X_COMPARE( wr_LogicalAND, && );
X_COMPARE( wr_LogicalOR, || );











//X_COMPARE( wr_CompareLT, < );

bool wr_CompareLT_E_E( WRValue* to, WRValue* from )
{
	WRValue V1 = to->singleValue();
	WRValue& V2 = from->singleValue();
	return wr_CompareLT[(V1.type<<2)|V2.type](&V1, &V2);
}
bool wr_CompareLT_E_I( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	return wr_CompareLT[(V.type<<2)|WR_INT](&V, from);
}
bool wr_CompareLT_E_F( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	return wr_CompareLT[(V.type<<2)|WR_FLOAT](&V, from);
}
bool wr_CompareLT_I_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	return wr_CompareLT[(WR_INT<<2)|V.type](to, &V);
}
bool wr_CompareLT_F_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	return wr_CompareLT[(WR_FLOAT<<2)|V.type](to, &V);
}
bool wr_CompareLT_R_E( WRValue* to, WRValue* from ) { return wr_CompareLT[(to->r->type<<2)|WR_EX](to->r, from); }
bool wr_CompareLT_E_R( WRValue* to, WRValue* from ) { return wr_CompareLT[(WR_EX<<2)|from->r->type](to, from->r); }
bool wr_CompareLT_R_R( WRValue* to, WRValue* from ) { return wr_CompareLT[(to->r->type<<2)|from->r->type](to->r, from->r); }
bool wr_CompareLT_R_I( WRValue* to, WRValue* from ) { return wr_CompareLT[(to->r->type<<2)|WR_INT](to->r, from); }
bool wr_CompareLT_R_F( WRValue* to, WRValue* from ) { return wr_CompareLT[(to->r->type<<2)|WR_FLOAT](to->r, from); }
bool wr_CompareLT_I_R( WRValue* to, WRValue* from ) { return wr_CompareLT[(WR_INT<<2)+from->r->type](to, from->r); }
bool wr_CompareLT_F_R( WRValue* to, WRValue* from ) { return wr_CompareLT[(WR_FLOAT<<2)+from->r->type](to, from->r); }
bool wr_CompareLT_I_I( WRValue* to, WRValue* from ) { return to->i < from->i; }
bool wr_CompareLT_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->i < from->f; }
bool wr_CompareLT_F_I( WRValue* to, WRValue* from ) { return to->f < (float)from->i; }
bool wr_CompareLT_F_F( WRValue* to, WRValue* from ) { return to->f < from->f; }
WRReturnFunc wr_CompareLT[16] = 
{
	wr_CompareLT_I_I, wr_CompareLT_I_F, wr_CompareLT_I_R, wr_CompareLT_I_E,
	wr_CompareLT_F_I, wr_CompareLT_F_F, wr_CompareLT_F_R, wr_CompareLT_F_E,
	wr_CompareLT_R_I, wr_CompareLT_R_F, wr_CompareLT_R_R, wr_CompareLT_R_E,
	wr_CompareLT_E_I, wr_CompareLT_E_F, wr_CompareLT_E_R, wr_CompareLT_E_E,
};











//X_COMPARE( wr_CompareEQ, == );

bool wr_CompareEQ_E_E( WRValue* to, WRValue* from )
{
	WRValue V1 = to->singleValue();
	WRValue& V2 = from->singleValue();
	return wr_CompareEQ[(V1.type<<2)|V2.type](&V1, &V2);
}
bool wr_CompareEQ_E_I( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	return wr_CompareEQ[(V.type<<2)|WR_INT](&V, from);
}
bool wr_CompareEQ_E_F( WRValue* to, WRValue* from )
{
	WRValue& V = to->singleValue();
	return wr_CompareEQ[(V.type<<2)|WR_FLOAT](&V, from);
}
bool wr_CompareEQ_I_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	return wr_CompareEQ[(WR_INT<<2)|V.type](to, &V);
}
bool wr_CompareEQ_F_E( WRValue* to, WRValue* from )
{
	WRValue& V = from->singleValue();
	return wr_CompareEQ[(WR_FLOAT<<2)|V.type](to, &V);
}
bool wr_CompareEQ_R_E( WRValue* to, WRValue* from ) { return wr_CompareEQ[(to->r->type<<2)|WR_EX](to->r, from); }
bool wr_CompareEQ_E_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[(WR_EX<<2)|from->r->type](to, from->r); }
bool wr_CompareEQ_R_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[(to->r->type<<2)|from->r->type](to->r, from->r); }
bool wr_CompareEQ_R_I( WRValue* to, WRValue* from ) { return wr_CompareEQ[(to->r->type<<2)|WR_INT](to->r, from); }
bool wr_CompareEQ_R_F( WRValue* to, WRValue* from ) { return wr_CompareEQ[(to->r->type<<2)|WR_FLOAT](to->r, from); }
bool wr_CompareEQ_I_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[(WR_INT<<2)+from->r->type](to, from->r); }
bool wr_CompareEQ_F_R( WRValue* to, WRValue* from ) { return wr_CompareEQ[(WR_FLOAT<<2)+from->r->type](to, from->r); }
bool wr_CompareEQ_I_I( WRValue* to, WRValue* from ) { return to->i == from->i; }
bool wr_CompareEQ_I_F( WRValue* to, WRValue* from ) { to->p2 = INIT_AS_FLOAT; return to->i == from->f; }
bool wr_CompareEQ_F_I( WRValue* to, WRValue* from ) { return to->f == (float)from->i; }
bool wr_CompareEQ_F_F( WRValue* to, WRValue* from ) { return to->f == from->f; }
WRReturnFunc wr_CompareEQ[16] = 
{
	wr_CompareEQ_I_I, wr_CompareEQ_I_F, wr_CompareEQ_I_R, wr_CompareEQ_I_E,
	wr_CompareEQ_F_I, wr_CompareEQ_F_F, wr_CompareEQ_F_R, wr_CompareEQ_F_E,
	wr_CompareEQ_R_I, wr_CompareEQ_R_F, wr_CompareEQ_R_R, wr_CompareEQ_R_E,
	wr_CompareEQ_E_I, wr_CompareEQ_E_F, wr_CompareEQ_E_R, wr_CompareEQ_E_E,
};















//------------------------------------------------------------------------------
#define X_UNARY_PRE( NAME, OPERATION ) \
void NAME##_E( WRValue* value )\
{\
	WRValue& V = value->singleValue();\
	NAME [ V.type ]( &V );\
	wr_valueToContainer( value, &V );\
}\
void NAME##_I( WRValue* value ) { OPERATION value->i; }\
void NAME##_F( WRValue* value ) { OPERATION value->f; }\
void NAME##_R( WRValue* value ) { NAME [ value->r->type ]( value->r ); *value = *value->r; }\
WRUnaryFunc NAME[4] = \
{\
	NAME##_I, NAME##_F, NAME##_R, NAME##_E\
};\

X_UNARY_PRE( wr_preinc, ++ );
X_UNARY_PRE( wr_predec, -- );

//------------------------------------------------------------------------------
#define X_UNARY_POST( NAME, OPERATION ) \
void NAME##_E( WRValue* value, WRValue* stack )\
{\
	WRValue& V = value->singleValue();\
	WRValue temp; \
	NAME [ V.type ]( &V, &temp );\
	wr_valueToContainer( value, &V );\
	*stack = temp; \
}\
void NAME##_I( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_INT; stack->i = value->i OPERATION; }\
void NAME##_R( WRValue* value, WRValue* stack ) { NAME[ value->r->type ]( value->r, stack ); }\
void NAME##_F( WRValue* value, WRValue* stack ) { stack->p2 = INIT_AS_FLOAT; stack->f = value->f OPERATION; }\
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

//------------------------------------------------------------------------------
void elementToTarget( const uint32_t index, WRValue* target, WRValue* value )
{
	if ( target == value )
	{
		// this happens when a return value is used directly instead of
		// assigned to something. So it should be safe to just
		// dereference the value and use it directly rather than
		// preserve the whole array

		if ( value->va->m_type == SV_VALUE )
		{
			*target = value->va->m_Vdata[ index ];
		}
		else if ( value->va->m_type == SV_CHAR )
		{
			target->p2 = INIT_AS_INT;
			target->ui = value->va->m_Cdata[ index ];
		}
		else // SV_HASH_TABLE, right?
		{
			*target = *(WRValue *)value->va->get( index );
		}
	}
	else
	{
		target->r = value;
		target->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( index );
	}
}

//------------------------------------------------------------------------------
void doIndexHash( WRValue* value, WRValue* target, WRValue* index )
{
	uint32_t hash = index->getHash();
	
	if ( value->xtype == WR_EX_HASH_TABLE ) 
	{
		int element;
		WRValue* entry = (WRValue*)(value->va->get(hash, &element));

		if ( IS_EX_SINGLE_CHAR_RAW_P2( (target->r = entry)->p2 ) )
		{
			target->p2 = INIT_AS_CONTAINER_MEMBER;
		}
		else
		{
			// need to create a hash ref
			*(entry + 1) = *index; // might be the first time it was registered
		
			target->p2 = INIT_AS_CONTAINER_MEMBER | ENCODE_ARRAY_ELEMENT_TO_P2( element );
			target->vb = (WRGCBase*)(value->va->m_Vdata) - 1;
		}
	}
	else // naming an element of a struct "S.element"
	{
		const unsigned char* table = value->va->m_ROMHashTable + ((hash % value->va->m_mod) * 5);

		if ( (uint32_t)READ_32_FROM_PC(table) == hash )
		{
			target->p2 = INIT_AS_REF;
			target->p = ((WRValue*)(value->va->m_data)) + READ_8_FROM_PC(table + 4);
		}
		else
		{
			target->init();
		}
	}
}

//------------------------------------------------------------------------------
void doIndex_I_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	// all we know is the value is not an array, so make it one
	value->va = c->getSVA( index->ui + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !value->va )
	{
		value->p2 = INIT_AS_INT;
		return;
	}
#endif
	value->p2 = INIT_AS_ARRAY;
	
	elementToTarget( index->ui, target, value );
}

//------------------------------------------------------------------------------
void doIndex_I_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	value = &value->deref();
	
	if (EXPECTS_HASH_INDEX(value->xtype))
	{
		doIndexHash( value, target, index );
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
		// nope, make it one of this size and return a ref
		WRValue* I = &index->deref();

		if ( I->type == WR_INT )
		{
			value->va = c->getSVA( index->ui+1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !value->va )
			{
				value->p2 = INIT_AS_INT;
				return;
			}
#endif
			value->p2 = INIT_AS_ARRAY;

		}
		else
		{
			value->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !value->va )
			{
				value->p2 = INIT_AS_INT;
				return;
			}
#endif
			value->p2 = INIT_AS_HASH_TABLE;

			doIndexHash( value, target, I );
			return;
		}
	}
	else if ( index->ui >= value->va->m_size )
	{
		if ( value->va->m_flags & GCFlag_SkipGC )
		{
boundsFailed:
			target->init();
			return;
		}

		wr_growValueArray( value->va, index->ui );
	}

	elementToTarget( index->ui, target, value );
}

//------------------------------------------------------------------------------
void doIndex_E_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	WRValue* V = &value->deref();

	WRValue* I = &index->deref();
	if ( I->type == WR_INT )
	{
		wr_index[WR_INT<<2 | V->type](c, I, V, target);
	}
	else
	{
		if ( !IS_HASH_TABLE(V->xtype) )
		{
			V->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !V->va )
			{
				V->p2 = INIT_AS_INT;
				return;
			}
#endif
			V->p2 = INIT_AS_HASH_TABLE;
		}
		doIndexHash( V, target, I );
	}
}

//------------------------------------------------------------------------------
void doIndex_R_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	wr_index[(index->r->type<<2)|value->r->type](c, index->r, value->r, target);
}

//------------------------------------------------------------------------------
void doIndex_E_FI( WRContext* c, WRValue* index, WRValue* value, WRValue* target )
{
	WRValue* V = &value->deref();

	V->va = c->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !V->va )
	{
		V->p2 = INIT_AS_INT;
		return;
	}
#endif
	V->p2 = INIT_AS_HASH_TABLE;
	doIndexHash( V, target, index );
}


void doVoidIndexFunc( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) {}


#ifdef WRENCH_REALLY_COMPACT

void doIndex_X_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->type<<2)|value->r->type](c, index, value->r, target); }
void doIndex_R_X( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|value->type](c, index->r, value, target); }

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_X_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_X,     doIndex_R_X,     doIndex_R_R,     doIndex_R_X, 
	doIndex_E_FI,    doIndex_E_FI,    doIndex_X_R,     doIndex_E_E,
};


#else

void doIndex_I_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_INT<<2)|value->r->type](c, index, value->r, target); }
void doIndex_R_I( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_INT](c, index->r, value, target); }
void doIndex_R_F( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_FLOAT](c, index->r, value, target); }
void doIndex_R_E( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(index->r->type<<2)|WR_EX](c, index->r, value, target); }
void doIndex_E_R( WRContext* c, WRValue* index, WRValue* value, WRValue* target ) { wr_index[(WR_EX<<2)|value->r->type](c, index, value->r, target); }

WRStateFunc wr_index[16] = 
{
	doIndex_I_X,     doIndex_I_X,     doIndex_I_R,     doIndex_I_E,
	doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc, doVoidIndexFunc,
	doIndex_R_I,     doIndex_R_F,     doIndex_R_R,     doIndex_R_E, 
	doIndex_E_FI,    doIndex_E_FI,    doIndex_E_R,     doIndex_E_E,
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

#include "wrench.h"

// standard functions that sort of come up a lot

int32_t wr_Seed = 0xA5EED;

//------------------------------------------------------------------------------
uint32_t wr_hash_read8( const void *dat, const int len )
{
	// fnv-1
	uint32_t hash = 0x811C9DC5;
	const unsigned char* data = (const unsigned char *)dat;

	for( int i=0; i<len; ++i )
	{
		hash ^= (uint32_t)READ_8_FROM_PC(data++);
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hash( const void *dat, const int len, uint32_t serial )
{
	// fnv-1
	uint32_t hash = serial ? serial : 0x811C9DC5;
	const unsigned char* data = (const unsigned char *)dat;

	for( int i=0; i<len; ++i )
	{
		hash ^= (uint32_t)data[i];
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hashStr_read8( const char* dat )
{
	uint32_t hash = 0x811C9DC5;
	const char* data = dat;
	while ( *data )
	{
		hash ^= (uint32_t)READ_8_FROM_PC(data++);
		hash *= 0x1000193;
	}

	return hash;
}

//------------------------------------------------------------------------------
uint32_t wr_hashStr( const char* dat, uint32_t serial )
{
	uint32_t hash = serial ? serial : 0x811C9DC5;
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
	stackTop->init();

	if ( argn > 0 )
	{
		WRValue* args = stackTop - argn;
		
		int32_t a = args[0].asInt();
		int32_t b; 
		if ( argn > 1 )
		{
			a = args[0].asInt();
			b = args[1].asInt() - a;
		}
		else
		{
			a = 0;
			b = args[0].asInt();
		}

		if ( b > 0 )
		{
			int32_t k = wr_Seed / 127773;
			wr_Seed = 16807 * (wr_Seed - k * 127773) - 2836 * k;
			stackTop->i = a + ((uint32_t)wr_Seed % (uint32_t)b);
		}
	}
}

//------------------------------------------------------------------------------
void wr_std_srand( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		wr_Seed = (uint32_t)((stackTop - 1)->asInt());
	}
}

#if __arm__ || WIN32 || _WIN32 || __linux__ || __MINGW32__ || __APPLE__ || __MINGW64__ || __clang__
#include <time.h>
//------------------------------------------------------------------------------
void wr_std_time( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_INT;
	stackTop->i = (int32_t)time(0);
}
#else
//------------------------------------------------------------------------------
void wr_std_time( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
}
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/time.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

//------------------------------------------------------------------------------
void wr_milliseconds(WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_INT;

#ifdef _WIN32
	stackTop->ui = (uint32_t)GetTickCount();
#endif
#ifdef __linux__
	struct timeval tv;
	gettimeofday( &tv, NULL );
	stackTop->ui = (uint32_t)((tv.tv_usec/1000) + (tv.tv_sec * 1000));
#endif
}


//------------------------------------------------------------------------------
void wr_loadStdLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::rand", wr_std_rand );
	wr_registerLibraryFunction( w, "std::srand", wr_std_srand );
	wr_registerLibraryFunction( w, "std::time", wr_std_time );

	wr_registerLibraryFunction( w, "time::ms", wr_milliseconds ); // return ms count that always increases
}

//------------------------------------------------------------------------------
void wr_loadAllLibs( WRState* w )
{
	wr_loadMathLib( w );
	wr_loadStdLib( w );
	wr_loadIOLib( w );
	wr_loadStringLib( w );
	wr_loadMessageLib( w );
	wr_loadSysLib( w );
	wr_loadSerializeLib( w );
	wr_loadDebugLib( w );
	wr_loadTCPLib( w );
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

#if defined(WRENCH_WIN32_FILE_IO) \
	|| defined(WRENCH_LINUX_FILE_IO) \
	|| defined(WRENCH_SPIFFS_FILE_IO) \
	|| defined(WRENCH_LITTLEFS_FILE_IO)

#if defined(WRENCH_WIN32_FILE_IO) || defined(WRENCH_LINUX_FILE_IO)

//------------------------------------------------------------------------------
void wr_stdout( const char* data, const int size )
{
#if defined(WRENCH_WIN32_FILE_IO)
	if ( _write( _fileno(stdout), data, size ) > 0 )
#endif
#if defined(WRENCH_LINUX_FILE_IO)
	if ( write( STDOUT_FILENO, data, size ) > 0 )
#endif
	{
		fflush( stdout );
	}
}

//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		WRValue* arg = stackTop - 1;
		WRValue::MallocStrScoped fileName( *arg );

#if defined(WRENCH_WIN32_FILE_IO)
		struct _stat sbuf;
		int ret = _stat( fileName, &sbuf );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		struct stat sbuf;
		int ret = stat( fileName, &sbuf );
#endif
		
		if ( ret == 0 )
		{
			FILE *infil = fopen( fileName, "rb" );
			if ( infil )
			{
				stackTop->va = c->getSVA( (int)sbuf.st_size, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
				if ( !stackTop->va )
				{
					return;
				}
#endif
				stackTop->p2 = INIT_AS_ARRAY;
				if ( fread(stackTop->va->m_Cdata, sbuf.st_size, 1, infil) != 1 )
				{
					stackTop->init();
					return;
				}
			}

			fclose( infil );
		}
	}
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 2 )
	{
		WRValue* arg1 = stackTop - 2;
		unsigned int len;
		const char* data = (char*)((stackTop - 1)->array(&len));
		if ( !data )
		{
			return;
		}

		WRValue::MallocStrScoped fileName( *arg1 );

		FILE *outfil = fopen( fileName, "wb" );
		if ( !outfil )
		{
			return;
		}

		stackTop->i = (int)fwrite( data, len, 1, outfil );
		fclose( outfil );
	}
}

//------------------------------------------------------------------------------
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		const char* filename = WRValue::MallocStrScoped(*(stackTop - 1));
		if ( filename )
		{
#if defined(WRENCH_WIN32_FILE_IO)
			_unlink( filename );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			unlink( filename );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
	char buf[256];
	int pos = 0;
	for (;;)
	{
		int in = fgetc( stdin );

		if ( in == EOF || in == '\n' || in == '\r' || pos >= 256 )
		{ 
			stackTop->va = c->getSVA( pos, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !stackTop->va )
			{
				return;
			}
#endif
			stackTop->p2 = INIT_AS_ARRAY;
			memcpy( stackTop->va->m_Cdata, buf, pos );
			break;
		}

		buf[pos++] = in;
	}
}

//------------------------------------------------------------------------------
void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->i = -1;

	if ( argn )
	{
		WRValue* args = stackTop - argn;

		WRValue::MallocStrScoped fileName( *args );

		if ( fileName )
		{
			int mode = (argn > 1) ? args[1].asInt() : O_RDWR | O_CREAT;
			
#if defined(WRENCH_WIN32_FILE_IO)
			stackTop->i = _open( fileName, mode | O_BINARY, _S_IREAD | _S_IWRITE /*0600*/ );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			stackTop->i = open( fileName, mode, 0666 );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->i = _close( (stackTop - argn)->asInt() );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->i = close( (stackTop - argn)->asInt() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn != 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	int toRead = args[1].asInt();
	if ( toRead <= 0 )
	{
		return;
	}

	stackTop->va = c->getSVA( toRead, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;


#if defined(WRENCH_WIN32_FILE_IO)
	int result = _read( args[0].asInt(), stackTop->va->m_Cdata, toRead );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
	int result = read( args[0].asInt(), stackTop->va->m_Cdata, toRead );
#endif

	stackTop->va->m_size = (result > 0) ? result : 0;
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		int fd = args[0].asInt();
		WRValue& data = args[1].deref();

		if ( IS_ARRAY(data.xtype) )
		{
			uint32_t size = data.va->m_size;
			if ( argn > 2 )
			{
				size = args[2].asInt();
			}

			if ( data.va->m_type == SV_CHAR )
			{
#if defined(WRENCH_WIN32_FILE_IO)
				stackTop->i = _write( fd, data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
				stackTop->i = write( fd, data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
#endif
			}
			else if ( data.va->m_type == SV_VALUE )
			{
				// .. does this even make sense?
			}
		}
		else if ( IS_RAW_ARRAY(data.xtype) )
		{
			uint32_t size = EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2);
			if ( argn > 2 )
			{
				size = args[2].asInt();
			}

#if defined(WRENCH_WIN32_FILE_IO)
			stackTop->i = _write( fd, data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
			stackTop->i = write( fd, data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
#endif
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		int fd = args[0].asInt();
		int offset = args[1].asInt();
		int whence = argn > 2 ? args[2].asInt() : SEEK_SET;

#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->ui = _lseek( fd, offset, whence );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->ui = lseek( fd, offset, whence );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioFSync(WRValue* stackTop, const int argn, WRContext* c)
{
	if (argn)
	{
#if defined(WRENCH_WIN32_FILE_IO)
		stackTop->i = _commit( (stackTop - argn)->asInt() );
#endif
#if defined(WRENCH_LINUX_FILE_IO)
		stackTop->i = fsync( (stackTop - argn)->asInt() );
#endif
	}
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	wr_registerLibraryConstant( w, "io::O_RDONLY", (int32_t)O_RDONLY );
	wr_registerLibraryConstant( w, "io::O_RDWR", (int32_t)O_RDWR );
	wr_registerLibraryConstant( w, "io::O_APPEND", (int32_t)O_APPEND );
	wr_registerLibraryConstant( w, "io::O_CREAT", (int32_t)O_CREAT );
	wr_registerLibraryConstant( w, "io::O_TRUNC", (int32_t)O_TRUNC );
	wr_registerLibraryConstant( w, "io::O_EXCL", (int32_t)O_EXCL );

	wr_registerLibraryConstant( w, "io::SEEK_SET", (int32_t)SEEK_SET );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", (int32_t)SEEK_CUR );
	wr_registerLibraryConstant( w, "io::SEEK_END", (int32_t)SEEK_END );
}

#endif

//------------------------------------------------------------------------------
void wr_loadIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::readFile", wr_read_file ); // (name) returns array which is file
	wr_registerLibraryFunction( w, "io::writeFile", wr_write_file ); // (name, array) writes array
	wr_registerLibraryFunction( w, "io::deleteFile", wr_delete_file ); // (name)
	wr_registerLibraryFunction( w, "io::getline", wr_getline ); // get a line of text from input
	wr_registerLibraryFunction( w, "io::open", wr_ioOpen ); //( name, flags, mode ); // returning a file handle 'fd' ; 'mode' specifies unix file access permissions.
	wr_registerLibraryFunction( w, "io::close", wr_ioClose ); //( fd );
	wr_registerLibraryFunction( w, "io::read", wr_ioRead );  //( fd, data, max_count );
	wr_registerLibraryFunction( w, "io::write", wr_ioWrite ); //( fd, data, count );
	wr_registerLibraryFunction( w, "io::seek", wr_ioSeek );  //( fd, offset, whence );
	wr_registerLibraryFunction( w, "io::fsync", wr_ioFSync ); //( fd );

	wr_ioPushConstants( w );
}

#else

void wr_loadIOLib( WRState* w )
{
}

#endif
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

#include "wrench.h"

#if defined(WRENCH_WIN32_TCP) \
	|| defined(WRENCH_LINUX_TCP)

//------------------------------------------------------------------------------
const unsigned int c_legalMasks[33] = 
{
	0x00000000, // 0
	0x80000000,
	0xC0000000,
	0xE0000000,
	0xF0000000,
	0xF8000000,
	0xFC000000,
	0xFE000000,
	0xFF000000, // /8
	0xFF800000, // /9
	0xFFC00000,
	0xFFE00000,
	0xFFF00000,
	0xFFF80000,
	0xFFFC0000,
	0xFFFE0000,
	0xFFFF0000, // /16
	0xFFFF8000, // /17
	0xFFFFC000,
	0xFFFFE000,
	0xFFFFF000,
	0xFFFFF800,
	0xFFFFFC00,
	0xFFFFFE00,   
	0xFFFFFF00, // 24
	0xFFFFFF80, // 25
	0xFFFFFFC0,
	0xFFFFFFE0,
	0xFFFFFFF0,
	0xFFFFFFF8,
	0xFFFFFFFC,
	0xFFFFFFFE,
	0xFFFFFFFF, // 32
};

// 'isdigit' has crazy amounts of bloat, this is substsantial gain
// (even though we chuck single-eval)
#define COMM_isdigit(c) ( (c)>='0' && (c)<='9' )

//------------------------------------------------------------------------------
// very quickly determine if the input string is an ip address, and if
// so, what it is. NOTE: Intolerant of whitespace
static int isIp( const char *data, unsigned int *mask =0 )
{
	if ( !data )
	{
		return 0;
	}

	int pos = 0;
	unsigned int running = 0;
	int address = 0;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running;
	running = 0;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running << 8;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( (data[pos++] != '.') || (running > 255) )
	{
		return 0;
	}

	address |= running << 16;

	if( !COMM_isdigit(data[pos]) )
	{
		return 0;
	}

	running = data[pos++] - '0';

	if ( COMM_isdigit(data[pos]) )
	{
		running *= 10;
		running += data[pos++] - '0';

		if ( COMM_isdigit(data[pos]) )
		{
			running *= 10;
			running += data[pos++] - '0';
		}
	}

	if ( running > 255 )
	{
		return 0;
	}

	address |= running << 24;

	if ( mask )
	{
		*mask = 0;
		if ( !data[pos] )
		{
			return address;
		}

		if ( data[pos++] != '/' )
		{
			return 0;
		}

		if ( !data[pos] )
		{
			return address;
		}

		if ( !COMM_isdigit(data[pos]) )
		{
			return 0;
		}

		*mask = data[pos++] - '0';
		if ( !data[pos] )
		{
			*mask = c_legalMasks[*mask];
			*mask = htonl(*mask);
			return address;
		}

		if ( !COMM_isdigit(data[pos]) )
		{
			return 0;
		}

		*mask *= 10;
		*mask += data[pos++] - '0';

		if ( *mask > 32 || data[pos] )
		{
			return 0;
		}

		*mask = htonl(c_legalMasks[*mask]);
	}
	else if ( data[pos] ) // more characters? must be exactly an IP address, no go
	{
		return 0;
	}

	return address;
}

//------------------------------------------------------------------------------
static char* ipFromAddress( const char* hostname, sockaddr_in* location, char* s )
{
	memset( location, 0, sizeof(sockaddr_in) );

	*((int *)&location->sin_addr) = isIp( hostname );
	if ( !*((int *)&location->sin_addr) )
	{
		struct addrinfo *result;

		if ( getaddrinfo(hostname, NULL, NULL, &result) )
		{
			return 0;
		}

		location->sin_addr = ((sockaddr_in*)(result->ai_addr))->sin_addr;

		freeaddrinfo(result);
	}

	return (char*)inet_ntop(AF_INET, &(((struct sockaddr_in *)location)->sin_addr), s, 20 );
}

//------------------------------------------------------------------------------
int wr_bindTCPEx( const int port )
{
	sockaddr_in location;
	memset( &location, 0, sizeof(location) );

	location.sin_family = AF_INET;
	location.sin_addr.s_addr = htonl( INADDR_ANY );
	location.sin_port = htons( (short)port );

	int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( s == -1 )
	{
		return -1;
	}

	if ( (::bind(s, reinterpret_cast<sockaddr *>(&location), sizeof(location)) == -1)
		 || (::listen(s, 5) == -1) )
	{
		wr_closeTCPEx( s );
		return -1;
	}

	return s;
}

//------------------------------------------------------------------------------
void wr_bindTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
		stackTop->i = wr_bindTCPEx( (stackTop - argn)->asInt() );
	}
}

//------------------------------------------------------------------------------
int wr_acceptTCPEx( const int socket, sockaddr* info )
{
	socklen_t length = sizeof( sockaddr );
	memset( info, 0, length );

	return accept( socket, info, &length );
}

//------------------------------------------------------------------------------
void wr_acceptTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn < 1 )
	{
		return;
	}

	sockaddr info;
	stackTop->i = wr_acceptTCPEx( (stackTop - argn)->asInt(), &info );
}

//------------------------------------------------------------------------------
int wr_connectTCPEx( const char* address, const int port )
{
	sockaddr_in location;
	memset( &location, 0, sizeof(location) );

	char ret[21];
	ipFromAddress( address, &location, ret );
	
	location.sin_family = AF_INET;
	location.sin_port = htons( port );

	int sock;
	if ( (sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 )
	{
		return -1;
	}

	if ( ::connect( sock, reinterpret_cast<sockaddr *>(&location), sizeof(location)) == -1 )
	{
		wr_closeTCPEx( sock );
		return -1;
	}

	return sock;
}

//------------------------------------------------------------------------------
void wr_connectTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	char* address = args[0].asMallocString();
	if ( !address || !address[0] )
	{
		return;
	}

	stackTop->i = wr_connectTCPEx( address, args[1].asInt() );
	g_free( address );
}

//------------------------------------------------------------------------------
void wr_closeTCPEx( const int socket )
{
#ifdef WRENCH_WIN32_TCP
	_close( socket );
#endif
#ifdef WRENCH_LINUX_TCP
	close( socket );
#endif
}

//------------------------------------------------------------------------------
void wr_closeTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( !argn )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	wr_closeTCPEx( args[0].asInt() );
}


//------------------------------------------------------------------------------
int wr_sendTCPEx( const int socket, const char* data, const int toSend )
{
	return send( socket, data, toSend, 0 );
}

//------------------------------------------------------------------------------
void wr_sendTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	unsigned int len;
	char* data = args[0].asMallocString( &len );
	if ( data )
	{
		stackTop->i = wr_sendTCPEx( args[0].asInt(), data, len );
	}

	g_free( data );
}

//------------------------------------------------------------------------------
int wr_receiveTCPEx( const int socket, char* data, const int toRead, const int timeoutMilliseconds )
{
#ifdef WRENCH_WIN32_TCP
	DWORD timeout = timeoutMilliseconds;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) );
	return _read( socket, data, toRead );
#endif
#ifdef WRENCH_LINUX_TCP
	struct timeval timeout;
	timeout.tv_sec = timeoutMilliseconds;
	timeout.tv_usec = 0;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	return read( socket, data, toRead );
#endif
}

//------------------------------------------------------------------------------
void wr_receiveTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	int toRead = 8192;
	if ( argn > 1 )
	{
		toRead = args[1].asInt();
	}

	stackTop->va = c->getSVA( toRead, SV_CHAR, false );
	stackTop->p2 = INIT_AS_ARRAY;

	int ret = wr_receiveTCPEx( args[0].asInt(), (char*)stackTop->va->m_Cdata, toRead );

	if ( !ret )
	{
		stackTop->init();
	}
	else
	{
		stackTop->va->m_size = ret;
	}
}

//------------------------------------------------------------------------------
int wr_peekTCPEx( const int socket, const int timeoutMilliseconds )
{
#ifdef WRENCH_WIN32_TCP

	DWORD timeout = timeoutMilliseconds;
	setsockopt( socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout) );
	int retval = recv( socket, 0, 65000, MSG_PEEK );
	
	return retval == SOCKET_ERROR ? -1 : retval;

#endif
#ifdef WRENCH_LINUX_TCP

	int retval = 0;

	short poll_events = POLLIN;

	pollfd readfd;
	readfd.fd = socket;
	readfd.events = poll_events;
	readfd.revents = 0;
	retval = ::poll( &readfd, 1, timeoutMilliseconds );

	return retval;

#endif
}

//------------------------------------------------------------------------------
void wr_peekTCP( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 1 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	stackTop->i = wr_peekTCPEx( args[0].asInt(), argn > 1 ? args[1].asInt() : 0 );
}

//------------------------------------------------------------------------------
void wr_loadTCPLib( WRState* w )
{
	wr_registerLibraryFunction( w, "tcp::bind", wr_bindTCP ); // (port) ret: socket
	wr_registerLibraryFunction( w, "tcp::accept", wr_acceptTCP ); // (socket) ret: socket
	wr_registerLibraryFunction( w, "tcp::connect", wr_connectTCP ); // (address, port) ret: socket
	wr_registerLibraryFunction( w, "tcp::close", wr_closeTCP ); // (socket)
	wr_registerLibraryFunction( w, "tcp::send", wr_sendTCP ); // (socket, data) ret: data sent
	wr_registerLibraryFunction( w, "tcp::receive", wr_receiveTCP ); // (socket) ret: data received
	wr_registerLibraryFunction( w, "tcp::peek", wr_peekTCP ); // (socket) ret: data available

#ifdef WRENCH_WIN32_TCP
	WSADATA wsaData;
	WSAStartup( MAKEWORD(2, 2), &wsaData );
#endif
}


#else

//------------------------------------------------------------------------------
void wr_loadTCPLib( WRState* w )
{
}

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

#include "wrench.h"

#if defined(WRENCH_LITTLEFS_FILE_IO) || defined(WRENCH_SPIFFS_FILE_IO)
#include <FS.h>
#ifdef WRENCH_LITTLEFS_FILE_IO
#include <LittleFS.h>
#define FILE_OBJ LittleFS
#endif

#ifdef WRENCH_SPIFFS_FILE_IO
#include <Spiffs.h>
#define FILE_OBJ Spiffs
#endif

//------------------------------------------------------------------------------
enum WRFSOpenModes
{
	LFS_READ   = 0x1,
	LFS_RDWR   = 0x2,
	LFS_APPEND = 0x4,
};

//------------------------------------------------------------------------------
enum WRFSSeek
{
	LFS_SET = 0,
	LFS_CUR,
	LFS_END,
};

//------------------------------------------------------------------------------
struct WRFSFile
{
	File file;
	WRFSFile* next;
};

WRFSFile* g_OpenFiles =0;

//------------------------------------------------------------------------------
void wr_stdout( const char* data, const int size )
{
}

//------------------------------------------------------------------------------
WRFSFile* wr_safeGetFile( const void* p )
{
	for( WRFSFile* safe = g_OpenFiles; safe; safe = safe->next )
	{
		if( safe == p )
		{
			return safe;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
void wr_read_file( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn != 1 )
	{
		return;
	}
	
	WRValue* arg = stackTop - 1;
	WRValue::MallocStrScoped fileName( *arg );

	File file = FILE_OBJ.open( fileName );
	if ( file && !file.isDirectory() )
	{
		stackTop->va = c->getSVA( file.size(), SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !stackTop->va )
		{
			return;
		}
#endif
		stackTop->p2 = INIT_AS_ARRAY;

		if ( file.readBytes( stackTop->va->m_SCdata, file.size() ) != file.size() )
		{
			stackTop->init();
		}

		file.close();
	}
}

//------------------------------------------------------------------------------
void wr_write_file( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn != 2 )
	{
		return;
	}

	WRValue::MallocStrScoped fileName( *arg );
	
	unsigned int len;
	const char* data = (char*)((stackTop - 1)->array(&len));
	if ( !data )
	{
		return;
	}

	File file = FILE_OBJ.open( fileName, FILE_WRITE );
	if ( !file )
	{
		return;
	}

	stackTop->i = file.write( (uint8_t *)data, len );

	file.close();
}

//------------------------------------------------------------------------------
void wr_delete_file( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		FILE_OBJ.remove( WRValue::MallocStrScoped fileName(*arg) );
	} 
}

//------------------------------------------------------------------------------
void wr_getline( WRValue* stackTop, const int argn, WRContext* c )
{
}

//------------------------------------------------------------------------------
void wr_ioOpen( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( !argn )
	{
		return;
	}
	
	WRValue* args = stackTop - argn;

	int mode = LFS_RDWR;
	if ( argn > 1 )
	{
		mode = args[1].asInt();
	}

	const char* fileName = (const char*)args->array();
	if ( fileName )
	{
		WRFSFile* entry = (WRFSFile *)g_malloc( sizeof(WRFSFile) );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if(!entry) { g_mallocFailed = true; return; }
#endif
		
		if ( mode & LFS_READ )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_READ );
		}
		else if ( (mode & LFS_RDWR) || (mode & LFS_APPEND) )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_WRITE );
		}
		else if ( mode & LFS_APPEND )
		{
			entry->file = FILE_OBJ.open( fileName, FILE_APPEND );
		}
		else
		{
			g_free( entry );
			return;
		}

		if ( !entry->file )
		{
			g_free( entry );
			return;
		}

		entry->next = g_OpenFiles;
		g_OpenFiles = entry;
		stackTop->p = entry;
	}
}

//------------------------------------------------------------------------------
void wr_ioClose( WRValue* stackTop, const int argn, WRContext* c )
{
	WRValue* args = stackTop - argn;
	if ( !argn || !args->p )
	{
		return;
	}

	stackTop->init();

	WRFSFile* prev = 0;
	WRFSFile* cur = g_OpenFiles;
	while( cur )
	{
		if ( cur == args->p )
		{
			cur->file.close();
			
			if ( !prev )
			{
				g_OpenFiles = g_OpenFiles->next;
			}
			else
			{
				prev->next = cur->next;
			}
			
			g_free( cur );
			break;
		}

		prev = cur;
		cur = cur->next;
	}
}

//------------------------------------------------------------------------------
void wr_ioRead( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn != 2 )
	{
		return;
	}
	
	WRValue* args = stackTop - argn;

	int toRead = args[1].asInt();
	if ( toRead <= 0 )
	{
		return;
	}

	WRFSFile* fd = wr_safeGetFile( args[0].p );
	if ( !fd )
	{
		return;
	}
		
	stackTop->va = c->getSVA( toRead, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	int result = fd->file.readBytes( stackTop->va->m_SCdata, toRead );
	
	stackTop->va->m_size = (result > 0) ? result : 0;
}

//------------------------------------------------------------------------------
void wr_ioWrite( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn != 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;

	WRFSFile* fd = wr_safeGetFile( args[0].p );
	if ( !fd )
	{
		return;
	}

	WRValue& data = args[1].deref();
	if ( IS_ARRAY(data.xtype) )
	{
		uint32_t size = data.va->m_size;
		if ( argn > 2 )
		{
			size = args[2].asInt();
		}

		if ( data.va->m_type == SV_CHAR )
		{
			stackTop->ui = fd->file.write( data.va->m_Cdata, (size > data.va->m_size) ? data.va->m_size : size );
		}
		else if ( data.va->m_type == SV_VALUE )
		{
			// .. does this even make sense?
		}
	}
	else if ( IS_RAW_ARRAY(data.xtype) )
	{
		uint32_t size = EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2);
		if ( argn > 2 )
		{
			size = args[2].asInt();
		}

		stackTop->ui = fd->file.write( (const uint8_t *)data.r->c, (size > EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2)) ? EX_RAW_ARRAY_SIZE_FROM_P2(data.r->p2) : size );
	}
}

//------------------------------------------------------------------------------
void wr_ioSeek( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 1 )
	{
		WRValue* args = stackTop - argn;

		WRFSFile* fd = wr_safeGetFile( args[0].p );
		if ( !fd )
		{
			return;
		}
		
		int offset = args[1].asInt();
		int whence = argn > 2 ? args[2].asInt() : LFS_SET;

		if ( whence == LFS_END )
		{
			offset += (int)fd->file.size();
		}
		else if ( whence == LFS_CUR )
		{
			offset += (int)fd->file.position();
		}
		
		fd->file.seek( (unsigned long)offset );
	}
}

//------------------------------------------------------------------------------
void wr_ioFSync(WRValue* stackTop, const int argn, WRContext* c)
{
	if ( argn == 1 )
	{
		WRFSFile* fd = wr_safeGetFile( (stackTop - 1)->p );
		if ( fd )
		{
			fd->file.flush();
		}
	}
}

//------------------------------------------------------------------------------
void wr_ioCleanupFunction( WRState* w, void* param )
{
	WRFSFile* g_OpenFiles =0;
	while( g_OpenFiles )
	{
		WRFSFile* next = g_OpenFiles->next;
		g_OpenFiles->file.close();
		g_free( g_OpenFiles );
		g_OpenFiles = next;
	}
}

//------------------------------------------------------------------------------
void wr_ioPushConstants( WRState* w )
{
	WRValue C;
	
	wr_registerLibraryConstant( w, "io::O_RDONLY", (int32_t)LFS_READ );
	wr_registerLibraryConstant( w, "io::O_RDWR", (int32_t)LFS_RDWR );
	wr_registerLibraryConstant( w, "io::O_APPEND", (int32_t)LFS_APPEND );
	wr_registerLibraryConstant( w, "io::O_CREAT", (int32_t)LFS_RDWR );

	wr_registerLibraryConstant( w, "io::SEEK_SET", (int32_t)LFS_SET );
	wr_registerLibraryConstant( w, "io::SEEK_CUR", (int32_t)LFS_CUR );
	wr_registerLibraryConstant( w, "io::SEEK_END", (int32_t)LFS_END );

	FILE_OBJ.begin();

	wr_addLibraryCleanupFunction( w, wr_ioCleanupFunction, 0 );

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
inline void addChar( char* out, char c, unsigned int& pos, const unsigned int max )
{
	if ( pos < max )
	{
		out[pos] = c;
	}
	
	++pos;
}

//------------------------------------------------------------------------------
int wr_sprintfEx( char* outbuf,
				  const unsigned int outsize,
				  const char* fmt,
				  const unsigned int fmtsize,
				  WRValue* args,
				  const int argn )
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

	unsigned int pos = 0;
	const char* fmtend = fmt + fmtsize;

	int listPtr = 0;

resetState:

	char padChar = ' ';
	unsigned int columns = 0;
	char flags = 0;

	for(;;)
	{
		if ( fmt >= fmtend )
		{
			goto sprintf_end;
		}
		
		char c = *fmt++;

		if ( !(secondPass & flags) )
		{
			if ( c != '%' ) // literal
			{
				addChar( outbuf, c, pos, outsize );
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
#ifdef WRENCH_FLOAT_SPRINTF
		else if (c == '.') // ignore, we might be reading a floating point decimal position
		{ }
#endif
		else if ( c == 'c' ) // character
		{
			if ( listPtr < argn )
			{
				addChar( outbuf, (char)(args[listPtr++].asInt()), pos, outsize );
			}
			goto resetState;
		}
		else if ( c == '%' ) // literal %
		{
			addChar( outbuf, c, pos, outsize );
			goto resetState;
		}
		else // string or integer
		{
			const int c_bufSize = 20;
			char buf[c_bufSize]; // buffer for integer

			const char *ptr; // pointer to first char of integer
			unsigned int len;

			if ( c == 's' ) // string
			{
				buf[0] = 0;
				if ( listPtr < argn )
				{
					ptr = (char *)args[listPtr].array(&len);
					if ( !ptr )
					{
						return 0;
					}
					++listPtr;
				}
				else
				{
					ptr = buf;
					len = 0;
				}

				padChar = ' '; // in case some joker uses a 0 in their column spec

copyToString:

				// get the string length so it can be formatted, don't
				// copy it, just count it
//				unsigned int len = 0;
//				for ( ; *ptr; ptr++ )
//				{
//					len++;
//				}
//
//				ptr -= len;

				// Right-justify
				if ( !(flags & negativeJustify) )
				{
					for ( ; columns > len; columns-- )
					{
						addChar( outbuf, padChar, pos, outsize );
					}
				}

				if ( flags & negativeSign )
				{
					addChar( outbuf, '-', pos, outsize );
				}

				if ( flags & altRep || c == 'p' )
				{
					addChar( outbuf, '0', pos, outsize );
					addChar( outbuf, 'x', pos, outsize );
				}

				for (unsigned int l = 0; l < len; ++l )
				{
					addChar( outbuf, *ptr++, pos, outsize );
				}

				// Left-justify
				for ( ; columns > len; columns-- )
				{
					addChar( outbuf, ' ', pos, outsize );
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
#ifdef WRENCH_FLOAT_SPRINTF
				else if ( c == 'f' || c == 'g' )
				{
					char floatBuf[32];
					int i = 30;
					floatBuf[31] = 0;
					const char *f = fmt;
					for( ; *f != '%'; --f, --i )
					{
						floatBuf[i] = *f;
					}
					floatBuf[i] = '%';

					// suck in whatever lib we need for this
					const int chars = snprintf( buf, 31, floatBuf + i, args[listPtr++].asFloat());

					// your system not have snprintf? the unsafe version is:
					//					const int chars = sprintf( buf, floatBuf + i, args[listPtr++].asFloat());

					for( int j=0; j<chars; ++j )
					{
						addChar( outbuf, buf[j], pos, outsize );
					}
					goto resetState;
				}
#endif
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

					--ptr; // was one past char we want
					len = (buf + width) - ptr;
					goto copyToString;
				}
				else // invalid format specifier
				{
					goto resetState;
				}
			}
		}
	}
	
sprintf_end:
	
	return pos;
}


#ifndef WR_FORMAT_TRY_SIZE
#define WR_FORMAT_TRY_SIZE 80
#endif

//------------------------------------------------------------------------------
unsigned int wr_doSprintf( WRValue& to, WRValue& fmt, WRValue* args, const int argn, WRContext* c )
{
	unsigned int fmtBufferSize = 0;
	char* fmtbuffer = (char*)fmt.array( &fmtBufferSize, SV_CHAR );
	if ( !fmtbuffer )
	{
		return 0;
	}

	unsigned int outbufSize = 0;
	char* outbuf = (char*)to.array( &outbufSize, SV_CHAR );

	unsigned int size;

	if ( !outbuf ) // going to have to make one then
	{
		char tmpbuf[WR_FORMAT_TRY_SIZE + 1]; // start with a temp buf
		
		size = wr_sprintfEx( tmpbuf, WR_FORMAT_TRY_SIZE, fmtbuffer, fmtBufferSize, args, argn );

		if ( size > WR_FORMAT_TRY_SIZE )
		{
			goto createNewBuffer;
		}

		to.va = c->getSVA( size, SV_CHAR, false );

		#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !to.va )
		{
			to.p2 = WR_INT;
			return 0;
		}
		#endif

		memcpy( to.va->m_Cdata, tmpbuf, size );
	}
	else
	{
		size = wr_sprintfEx( outbuf, outbufSize, fmtbuffer, fmtBufferSize, args, argn );

		if ( size > outbufSize )
		{
createNewBuffer:

			to.va = c->getSVA( size, SV_CHAR, false );

			#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !to.va )
			{
				to.p2 = WR_INT;
				return 0;
			}
			#endif
			wr_sprintfEx( (char *)to.va->m_Cdata, size, fmtbuffer, fmtBufferSize, args, argn );
		}
		else
		{
			to.va->m_size = size; // just is the new size, TODO- recover memory? seems like not worth the squeeze
		}
	}

	to.p2 = INIT_AS_ARRAY;

	return size;

}


//------------------------------------------------------------------------------
void wr_format( WRValue* stackTop, const int argn, WRContext* c )
{
	if( argn > 0 )
	{
		WRValue* args = stackTop - argn;
		wr_doSprintf( *stackTop, args->deref(), args + 1, argn - 1, c );
	}
	else
	{
		stackTop->init();
	}
}

//------------------------------------------------------------------------------
void wr_sprintf( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if( argn > 1 )
	{
		WRValue* args = stackTop - argn;
		stackTop->i = wr_doSprintf( args->deref(), args[1].deref(), args + 2, argn - 2, c );
	}
}

//------------------------------------------------------------------------------
void wr_printf( WRValue* stackTop, const int argn, WRContext* c )
{
	unsigned int chars = 0;
	if( argn > 0 )
	{
		WRValue* args = stackTop - argn;
		if ( (chars = wr_doSprintf( *stackTop, args->deref(), args + 1, argn - 1, c )) )
		{
			wr_stdout( (const char*)stackTop->va->m_Cdata, stackTop->va->m_size );
		}
	}

	stackTop->p2 = WR_INT;
	stackTop->i = chars;
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
	const char* data = (char *)(args[0].array(&len));
	
	if( !data || len <= 0 )
	{
		return;
	}
	
	unsigned int start = args[1].asInt();

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

	#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
	#endif
	
	stackTop->p2 = INIT_AS_ARRAY;
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

	unsigned int len;
	const char* str = (const char*)(args[0].array(&len));
	if ( !str )
	{
		return;
	}
	
	char ch = (char)args[1].asInt();
	for( unsigned int i=0; i<len; ++i )
	{
		if ( str[i] == ch )
		{
			stackTop->i = i;
			break;
		}
	}
}

//------------------------------------------------------------------------------
void wr_tolower( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		WRValue& value = (stackTop - 1)->deref();
		if ( value.xtype == WR_EX_ARRAY && value.va->m_type == SV_CHAR )
		{
			stackTop->va = c->getSVA( value.va->m_size, SV_CHAR, false );

#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !stackTop->va )
			{
				return;
			}
#endif
			stackTop->p2 = INIT_AS_ARRAY;

			for( uint32_t i=0; i<value.va->m_size; ++i )
			{
				stackTop->va->m_SCdata[i] = tolower(value.va->m_SCdata[i]);
			}
		}
		else if ( value.type == WR_INT )
		{
			stackTop->i = (int)tolower( (stackTop - 1)->asInt() );
		}
	}
}

//------------------------------------------------------------------------------
void wr_toupper( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn == 1 )
	{
		WRValue& value = (stackTop - 1)->deref();
		if ( value.xtype == WR_EX_ARRAY && value.va->m_type == SV_CHAR )
		{
			stackTop->va = c->getSVA( value.va->m_size, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !stackTop->va )
			{
				return;
			}
#endif
			stackTop->p2 = INIT_AS_ARRAY;

			for( uint32_t i=0; i<value.va->m_size; ++i )
			{
				stackTop->va->m_SCdata[i] = toupper(value.va->m_SCdata[i]);
			}
		}
		else if ( value.type == WR_INT )
		{
			stackTop->i = (int)toupper( (stackTop - 1)->asInt() );
		}

	}
}

//------------------------------------------------------------------------------
void wr_tol( WRValue* stackTop, const int argn, WRContext* c )
{
	char buf[21];
	stackTop->init();
	if ( argn == 2 )
	{
		stackTop->i = (int)strtol( stackTop[-2].asString(buf, 20), 0, stackTop[-1].asInt() );
	}
	else if ( argn == 1 )
	{
		stackTop->i = (int)strtol( stackTop[-1].asString(buf, 20), 0, 10 );
	}
}

//------------------------------------------------------------------------------
void wr_concat( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	unsigned int len1 = 0;
	unsigned int len2 = 0;
	const char* data1 = (const char*)args[0].array( &len1 );
	const char* data2 = (const char*)args[1].array( &len2 );

	if( !data1 || !data2 )
	{
		return;
	}

	stackTop->va = c->getSVA( len1 + len2, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;
	memcpy( stackTop->va->m_Cdata, data1, len1 );
	memcpy( stackTop->va->m_Cdata + len1, data2, len2 );
}

//------------------------------------------------------------------------------
void wr_left( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	unsigned int len = 0;
	const char* data = (const char*)args[0].array(&len );
	if ( !data )
	{
		return;
	}
	unsigned int chars = args[1].asInt();

	if ( chars > len )
	{
		chars = len;
	}
	
	stackTop->va = c->getSVA( chars, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	memcpy( stackTop->va->m_Cdata, data, chars );
}

//------------------------------------------------------------------------------
void wr_right( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	unsigned int len = 0;
	const char* data = (const char*)args[0].array(&len );
	if ( !data )
	{
		return;
	}
	unsigned int chars = args[1].asInt();

	if ( chars > len )
	{
		chars = len;
	}

	stackTop->va = c->getSVA( chars, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;
	memcpy( stackTop->va->m_Cdata, data + (len - chars), chars );
}

//------------------------------------------------------------------------------
void wr_trimright( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	WRValue* args = stackTop - argn;
	const char* data;
	int len = 0;
	
	if ( argn < 1 || ((data = (const char*)args->array((unsigned int *)&len)) == 0) )
	{
		return;
	}

	while( --len >= 0 && isspace(data[len]) );

	++len;

	stackTop->va = c->getSVA( len, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	memcpy( stackTop->va->m_Cdata, data, len );
}

//------------------------------------------------------------------------------
void wr_trimleft( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	WRValue* args = stackTop - argn;
	const char* data;
	unsigned int len = 0;
	
	if ( argn < 1 || ((data = (const char*)args->array(&len)) == 0) )
	{
		return;
	}

	unsigned int marker = 0;
	while( marker < len && isspace(data[marker]) ) { ++marker; }

	stackTop->va = c->getSVA( len - marker, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	memcpy( stackTop->va->m_Cdata, data + marker, len - marker );
}

//------------------------------------------------------------------------------
void wr_trim( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	WRValue* args = stackTop - argn;
	const char* data;
	int len = 0;
	if ( argn < 1 || ((data = (const char*)args->array((unsigned int*)&len)) == 0) )
	{
		return;
	}

	int marker = 0;
	while( marker < len && isspace(data[marker]) ) { ++marker; }

	while( --len >= marker && isspace(data[len]) );

	++len;

	stackTop->va = c->getSVA( len - marker, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	memcpy( stackTop->va->m_Cdata, data + marker, len - marker );
}

//------------------------------------------------------------------------------
void wr_insert( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn < 3 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	unsigned int len1 = 0;
	unsigned int len2 = 0;
	const char* data1 = (const char*)args[0].array( &len1 );
	const char* data2 = (const char*)args[1].array( &len2 );

	if( !data1 || !data2 )
	{
		return;
	}
	
	unsigned int pos = args[2].asInt();
	if ( pos >= len1 )
	{
		pos = len1;
	}

	unsigned int newlen = len1 + len2;
	stackTop->va = c->getSVA( newlen, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !stackTop->va )
	{
		return;
	}
#endif
	stackTop->p2 = INIT_AS_ARRAY;

	memcpy( stackTop->va->m_Cdata, data1, pos );
	memcpy( stackTop->va->m_Cdata + pos, data2, len2 );
	memcpy( stackTop->va->m_Cdata + pos + len2, data1 + pos, len1  - pos );
}

//------------------------------------------------------------------------------
void wr_tprint( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn )
	{
		char buf[4096];
		(stackTop - argn)->technicalAsString( buf, 4096, false );
		printf( "\n%s\n",  buf );
		(stackTop - argn)->technicalAsString( buf, 4096, true );
		printf( "%s\n",  buf );
	}
}


//------------------------------------------------------------------------------
void wr_loadStringLib( WRState* w )
{
	wr_registerLibraryFunction( w, "str::strlen", wr_strlen );

	wr_registerLibraryFunction( w, "str::sprintf", wr_sprintf );
	wr_registerLibraryFunction( w, "str::format", wr_format );
	
	wr_registerLibraryFunction( w, "str::printf", wr_printf );
	wr_registerLibraryFunction( w, "str::isspace", wr_isspace );
	wr_registerLibraryFunction( w, "str::isdigit", wr_isdigit );
	wr_registerLibraryFunction( w, "str::isalpha", wr_isalpha );
	wr_registerLibraryFunction( w, "str::mid", wr_mid );
	wr_registerLibraryFunction( w, "str::chr", wr_strchr );
	wr_registerLibraryFunction( w, "str::tolower", wr_tolower );
	wr_registerLibraryFunction( w, "str::toupper", wr_toupper );
	wr_registerLibraryFunction( w, "str::tol", wr_tol );
	wr_registerLibraryFunction( w, "str::concat", wr_concat );
	wr_registerLibraryFunction( w, "str::left", wr_left );
	wr_registerLibraryFunction( w, "str::trunc", wr_left );
	wr_registerLibraryFunction( w, "str::right", wr_right );
	wr_registerLibraryFunction( w, "str::substr", wr_mid );
	wr_registerLibraryFunction( w, "str::trimright", wr_trimright );
	wr_registerLibraryFunction( w, "str::trimleft", wr_trimleft );
	wr_registerLibraryFunction( w, "str::trim", wr_trim );
	wr_registerLibraryFunction( w, "str::insert", wr_insert );

	wr_registerLibraryFunction( w, "str::tprint", wr_tprint );

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
void wr_intScale(WRValue* stackTop, const int argn, WRContext* c)
{
	stackTop->init();
	int32_t res = 0;

	if (argn > 2)
	{
		WRValue* args = stackTop - argn;
		int32_t dvsr = args[2].asInt();

		if (dvsr != 0)
		{
			int32_t fac1 = args[0].asInt();
			int32_t fac2 = args[1].asInt();

			res = (int32_t) (((int64_t)fac1 * fac2) / dvsr);
		}
	}

	stackTop->i = res;
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

	wr_registerLibraryFunction( w, "math::iscale", wr_intScale);
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

		WRValue* msg = c->w->globalRegistry.exists( args[0].getHash(), clear );

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

		WRValue* msg = c->w->globalRegistry.getAsRawValueHashTable( args[0].getHash() );
		if ( msg != (void*)WRENCH_NULL_HASH )
		{
			msg->ui = args[1].getHash();
			msg->p2 = INIT_AS_INT;
		}
	}
}

//------------------------------------------------------------------------------
// key  : anything hashable
void wr_mboxClear( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		c->w->globalRegistry.exists((stackTop - argn)->getHash(), true );
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
		 && c->w->globalRegistry.exists((stackTop - argn)->getHash(), false) )
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
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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
// returns : 0 - function  does not exist
//         : 1 - function is native (callback)
//         : 2 - function is in wrench (was in source code)
void wr_isFunction( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0 )
	{
		WRValue* arg = stackTop - argn;
		uint32_t hash = arg->getHash();
		if ( c->w->globalRegistry.exists(hash, false) )
		{
			stackTop->i = 1;
		}
		else if ( c->registry.exists(hash, false) )
		{
			stackTop->i = 2;
		}
	}
}

//------------------------------------------------------------------------------
void wr_importByteCode( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0 )
	{
		unsigned int len;
		char* data = (char *)((stackTop - argn)->array( &len, SV_CHAR ));
		if ( data )
		{
			uint8_t* import = (uint8_t*)g_malloc( len );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if(!import) { g_mallocFailed = true; return; }
#endif
			memcpy( (char*)import, data, len );
			wr_import( c, import, len, true );
		}
		else
		{
			stackTop->i = -1;
		}
	}
}

//------------------------------------------------------------------------------
void wr_importCompile( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

	if ( argn > 0 )
	{
		unsigned int len;
		char* data = (char *)((stackTop - argn)->array( &len, SV_CHAR ));
		if ( data )
		{
			unsigned char* out;
			int outlen;
			if ( (stackTop->i = wr_compile(data, len, &out, &outlen)) == WR_ERR_None )
			{
				wr_import( c, out, outlen, true );
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_halt( WRValue* stackTop, const int argn, WRContext* c )
{
	if ( argn > 0 )
	{
		unsigned int e = (stackTop - argn)->asInt();
		c->w->err = (e <= (unsigned int)WR_USER || e > (unsigned int)WR_ERR_LAST)
					? (unsigned int)WR_ERR_USER_err_out_of_range : e;
	}
}

//------------------------------------------------------------------------------
void wr_loadSysLib( WRState* w )
{
	wr_registerLibraryFunction( w, "sys::isFunction", wr_isFunction );
	wr_registerLibraryFunction( w, "sys::importByteCode", wr_importByteCode );
	wr_registerLibraryFunction( w, "sys::importCompile", wr_importCompile );
	wr_registerLibraryFunction( w, "sys::halt", wr_halt ); // halts execution and sets w->err to whatever was passed                 
														   // NOTE: value must be between WR_USER and WR_ERR_LAST

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
void wr_stdSerialize( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn )
	{
		char* buf;
		int len;
		if ( wr_serialize(&buf, &len, *(stackTop - argn) ) )
		{
			stackTop->va = c->getSVA( 0, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !stackTop->va )
			{
				return;
			}
#endif
			stackTop->p2 = INIT_AS_ARRAY;
			free( stackTop->va->m_Cdata );
			stackTop->va->m_data = buf;
			stackTop->va->m_size = len;
		}
	}
}

//------------------------------------------------------------------------------
void wr_stdDeserialize( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn )
	{
		WRValue& V = (stackTop - argn)->deref();
		if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_CHAR )
		{
			if ( !wr_deserialize( c, *stackTop, V.va->m_SCdata, V.va->m_size ) )
			{
				stackTop->init();
			}

			c->gc( stackTop + 1 );
		}
	}
}

//------------------------------------------------------------------------------
void wr_loadSerializeLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::serialize", wr_stdSerialize );
	wr_registerLibraryFunction( w, "std::deserialize", wr_stdDeserialize );
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

int wr_sprintfEx( char* outbuf,
				  const unsigned int outsize,
				  const char* fmt,
				  const unsigned int fmtsize,
				  WRValue* args,
				  const int argn );

//------------------------------------------------------------------------------
void wr_debugPrintEx( WRValue* stackTop, const int argn, WRContext* c, const char* append )
{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	if( argn >= 1 && c->debugInterface )
	{
		WRValue* args = stackTop - argn;
		unsigned int inlen;
		char* inbuf = args[0].asMallocString( &inlen );
		if ( !inbuf )
		{
			return;
		}

		char outbuf[200];
		int size = wr_sprintfEx( outbuf, 199, inbuf, inlen, args + 1, argn - 1);
			
		WrenchPacket* P = WrenchPacket::alloc( WRD_DebugOut, size, (uint8_t*)outbuf );
			
		g_free( inbuf );

		c->debugInterface->I->m_comm->send( P );

		g_free( P );
	}
#endif
}

//------------------------------------------------------------------------------
void wr_debugPrint( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_debugPrintEx( stackTop, argn, c, 0 );
}

//------------------------------------------------------------------------------
void wr_debugPrintln( WRValue* stackTop, const int argn, WRContext* c )
{
	wr_debugPrintEx( stackTop, argn, c, "\r\n" );
}

//------------------------------------------------------------------------------
void wr_loadDebugLib( WRState* w )
{
#ifdef WRENCH_INCLUDE_DEBUG_CODE
	wr_registerLibraryFunction( w, "debug::print", wr_debugPrint );
	wr_registerLibraryFunction( w, "debug::println", wr_debugPrintln );
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

#ifdef ARDUINO

#include "wrench.h"

#include <Arduino.h>
#include <Wire.h>

//------------------------------------------------------------------------------
void wr_loadEsp32Lib( WRState* w )
{
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

#ifdef ARDUINO

#include <Arduino.h>
#include <Wire.h>

//------------------------------------------------------------------------------
void wr_std_delay( WRValue* stackTop, const int argn, WRContext* c)
{
	// std::delay( milliseconds )
	if( argn == 1)
	{
		delay( (unsigned long)stackTop[-1].asInt() ); // a signed int will work up to 25 days
	}
}

//------------------------------------------------------------------------------
void wr_io_pinMode( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::pinMode( pin, <0 input, 1 output> )
	if( argn == 2)
	{
		pinMode( stackTop[-2].asInt(), (stackTop[-1].asInt() == 0) ? INPUT : OUTPUT );
	}
}

//------------------------------------------------------------------------------
void wr_io_digitalWrite( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::digitalWrite( pin, value )
	if( argn == 2)
	{
		digitalWrite(stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_io_analogRead( WRValue* stackTop, const int argn, WRContext* c)
{
	// io::analogRead( pin )
	if ( argn == 1)
	{
		stackTop->i = analogRead( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_lcd_begin( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::begin( columns, rows )
	// returns status
	if( argn == 2)
	{
//		stackTop->i = lcd.begin( stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_lcd_setCursor( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::setCursor( column, row )
	if( argn == 2 )
	{
//		lcd.setCursor( stackTop[-2].asInt(), stackTop[-1].asInt());
	}
}

//------------------------------------------------------------------------------
void wr_lcd_print( WRValue* stackTop, const int argn, WRContext* c)
{
	// lcd::print( arg )
	// returns number of chars printed
	if( argn == 1 )
	{
//		char buf[61];
//		stackTop->i = lcd.print( stackTop[-1].asString(buf, sizeof(buf)-1));
	}
}

//------------------------------------------------------------------------------
void wr_wire_begin( WRValue* stackTop, const int argn, WRContext* c)
{
	Wire.begin();
}

//------------------------------------------------------------------------------
void wr_wire_beginTransmission( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 1)
	{
		Wire.beginTransmission( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_write( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 1)
	{
		Wire.write( stackTop[-1].asInt() );
	}
	else if( argn == 2)
	{
		// To be added !
	}
}

//------------------------------------------------------------------------------
void wr_wire_endTransmission( WRValue* stackTop, const int argn, WRContext* c)
{
	if( argn == 0)
	{
		stackTop->i = Wire.endTransmission();
	}
	else if( argn == 1)
	{
		stackTop->i = Wire.endTransmission( stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_requestFrom( WRValue* stackTop, const int argn, WRContext* c)
{
	// wire::requestFrom( address, bytes )
	if( argn == 2)
	{
		stackTop->i = Wire.requestFrom( stackTop[-2].asInt(), stackTop[-1].asInt() );
	}
}

//------------------------------------------------------------------------------
void wr_wire_available( WRValue* stackTop, const int argn, WRContext* c)
{
	stackTop->i = Wire.available();
}

//------------------------------------------------------------------------------
void wr_wire_read( WRValue* stackTop, const int argn, WRContext* c)
{
	stackTop->i = Wire.read();
}

//------------------------------------------------------------------------------
void wr_loadArduinoWireLib( WRState* w )
{
	wr_registerLibraryFunction( w, "wire::begin", wr_wire_begin);
	wr_registerLibraryFunction( w, "wire::beginTransmission", wr_wire_beginTransmission);
	wr_registerLibraryFunction( w, "wire::write", wr_wire_write);
	wr_registerLibraryFunction( w, "wire::endTransmission", wr_wire_endTransmission);
	wr_registerLibraryFunction( w, "wire::requestFrom", wr_wire_requestFrom);
	wr_registerLibraryFunction( w, "wire::available", wr_wire_available);
	wr_registerLibraryFunction( w, "wire::read", wr_wire_read);
}

//------------------------------------------------------------------------------
void wr_loadArduinoSTDLib( WRState* w )
{
	wr_registerLibraryFunction( w, "std::delay", wr_std_delay );
}

//------------------------------------------------------------------------------
void wr_loadArduinoIOLib( WRState* w )
{
	wr_registerLibraryFunction( w, "io::pinMode", wr_io_pinMode );
	wr_registerLibraryFunction( w, "io::digitalWrite", wr_io_digitalWrite );
	wr_registerLibraryFunction( w, "io::analogRead", wr_io_analogRead );
}

//------------------------------------------------------------------------------
void wr_loadArduinoLCDLib( WRState* w )
{
	wr_registerLibraryFunction( w, "lcd::begin", wr_lcd_begin );
	wr_registerLibraryFunction( w, "lcd::setCursor", wr_lcd_setCursor );
	wr_registerLibraryFunction( w, "lcd::print", wr_lcd_print );
}

//------------------------------------------------------------------------------
void wr_loadArduinoLib( WRState* w )
{
	wr_loadArduinoLCDLib( w );
	wr_loadArduinoIOLib( w );
	wr_loadArduinoSTDLib( w );
	wr_loadArduinoWireLib( w );
}

#endif
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

#ifdef WRENCH_ARDUINO_SERIAL

#include "wrench.h"

//------------------------------------------------------------------------------
HANDLE wr_serialOpen( const char* name )
{
	return Serial;
}

//------------------------------------------------------------------------------
int wr_serialSend( HANDLE comm, const char* data, const int size )
{
	return comm.write( data, size );
}

//------------------------------------------------------------------------------
int wr_serialReceive( HANDLE comm, char* data, const int expected )
{
	return comm.readBytes( data, expected );
}

//------------------------------------------------------------------------------
int wr_serialPeek( HANDLE comm )
{
	return comm.available();
}

#endif
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_LINUX_SERIAL

#include "wrench.h"

//------------------------------------------------------------------------------
HANDLE wr_serialOpen( const char* name )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialSend( HANDLE comm, const char* data, const int size )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialReceive( HANDLE comm, char* data, const int expected )
{
	return -1;
}

//------------------------------------------------------------------------------
int wr_serialPeek( HANDLE comm )
{
	return 0;
}

//------------------------------------------------------------------------------
void wr_serialClose( HANDLE comm )
{
	close( comm );
}

#endif
