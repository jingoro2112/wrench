#ifndef SIMPLE_LL_H
#define SIMPLE_LL_H
/*------------------------------------------------------------------------------*/

// for queue's and stacks implemented as a LL

//-----------------------------------------------------------------------------
template<class L> class SimpleLL
{
public:

	SimpleLL() : m_iter(0), m_head(0), m_tail(0) {}
	~SimpleLL() { clear(); }

	//------------------------------------------------------------------------------
	L* first() { return m_head ? &(m_iter = m_head)->item : 0; }
	L* next() { return m_iter ? &((m_iter = m_iter->next)->item) : 0; }

	//------------------------------------------------------------------------------
	L* operator[]( const int i )
	{
		int cur = 0;
		for( Node* N = m_head; N ; N = N->next, ++cur )
		{
			if( cur == i )
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
			m_head = new Node;
			m_tail = m_head;
		}
		else
		{
			Node* N = new Node( m_head );
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
			Node* N = new Node;
			m_tail->next = N;
			m_tail = N;
			return &(m_tail->item);
		}
	}

	//------------------------------------------------------------------------------
	L* head() { return m_head ? &(m_head->item) : 0; }
	L* tail() { return m_tail ? &(m_tail->item) : 0; }

	//------------------------------------------------------------------------------
	void popHead()
	{
		if ( m_head )
		{
			if ( m_tail == m_head )
			{
				delete m_head;
				m_head = 0;
				m_tail = 0;
			}
			else
			{
				Node* N = m_head;
				m_head = m_head->next;
				delete N;
			}
		}
	}

	//------------------------------------------------------------------------------
	void popTail()
	{
		if ( m_head == m_tail )
		{
			clear();
		}
		else
		{
			for( Node* N = m_head; N; N = N->next )
			{
				if ( N->next == m_tail )
				{
					delete N->next;
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
		Node( Node* n=0 ) : next(n) {}
		L item;
		Node *next;
	};

	Node* m_iter;
	Node* m_head;
	Node* m_tail;
};

#endif
