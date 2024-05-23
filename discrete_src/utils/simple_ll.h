/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

MIT Licence

Permission is hereby granted, g_free of charge, to any person obtaining a copy
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

#ifndef SIMPLE_LL_H
#define SIMPLE_LL_H
/*------------------------------------------------------------------------------*/

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

		const Iterator operator++() { if ( m_current ) m_current = m_current->next; return *this; }
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
