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

//------------------------------------------------------------------------------
class WRGCObject
{
public:

	// the order here matters for data alignment
	
	char m_type;
	char m_preAllocated;
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

	WRGCObject* m_next; // for gc
	
	~WRGCObject() { clear(); }

	//------------------------------------------------------------------------------
	void growHash( const uint32_t hash )
	{
		// there was a collision with the passed hash, grow until the
		// collision disappears
		
		int t = 0;
		for( ; c_primeTable[t] <= m_mod ; ++t );

		for(;;)
		{
tryAgain:
			int newMod;
			if ( !(newMod = c_primeTable[t]) )
			{
				return;
			}

			uint32_t* proposed = new uint32_t[newMod];
			memset( proposed, 0, newMod*sizeof(uint32_t) );

			proposed[ hash % newMod ] = hash;

			int h = 0;
			for( ; h<m_mod; ++h )
			{
				if ( m_hashTable[h] == 0 )
				{
					continue;
				}

				int newEntry = m_hashTable[h] % newMod;
				if ( proposed[newEntry] == 0 )
				{
					proposed[newEntry] = m_hashTable[h];
				}
				else
				{
					delete[] proposed;
					++t;
					goto tryAgain;
				}
			}

			int newSize;
			if ( m_type == SV_VOID_HASH_TABLE )
			{
				newSize = newMod;
			}
			else
			{
				newSize = newMod << 1;
			}
			
			WRValue* newValues;

			newValues = new WRValue[newSize];
			memset( (char*)newValues, 0, newSize*sizeof(WRValue) );

			for( int v=0; v<m_mod; ++v )
			{
				if ( !m_hashTable[v] )
				{
					continue;
				}

				unsigned int m1 = m_hashTable[v] % m_mod;
				unsigned int m2 = m_hashTable[v] % newMod;
				
				if ( m_type == SV_VOID_HASH_TABLE )
				{
					newValues[m2] = m_Vdata[m1];
				}
				else
				{
					WRValue* to = newValues + (m2<<1);
					WRValue* from = m_Vdata + (m1<<1);
					
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

			m_mod = newMod;
			m_size = newSize;
			delete[] m_Vdata;
			delete[] m_hashTable;
			m_hashTable = proposed;
			m_Vdata = newValues;
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------
	WRGCObject( const unsigned int size,
				const WRGCObjectType type,
				const void* preAlloc =0 )
	{
		m_type = (char)type;
		m_next = 0;
		m_size = size;
		if ( !(m_preAllocated = (m_constData = preAlloc) ? 1 : 0) )
		{
			switch( m_type )
			{
				case SV_VALUE: { m_Vdata = new WRValue[size]; break; }
				case SV_CHAR: { m_Cdata = new unsigned char[size+1]; m_Cdata[size]=0; break; }
				case SV_VOID_HASH_TABLE:
				case SV_HASH_TABLE:
				{
					m_mod = 0;
					m_hashTable = 0;
					m_size = 0;
					growHash(0);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void clear()
	{
		if ( m_preAllocated )
		{
			return;
		}

		switch( m_type )
		{
			case SV_VOID_HASH_TABLE:
			case SV_HASH_TABLE: delete[] m_hashTable; m_hashTable = 0; // intentionally fall through:
			case SV_VALUE: delete[] m_Vdata; break; 

			case SV_CHAR: { delete[] m_Cdata; break; }
		}

		m_data = 0;
	}

	//------------------------------------------------------------------------------
	WRGCObject& operator= ( WRGCObject& A )
	{
		clear();

		m_preAllocated = 0;

		m_mod = A.m_mod;
		m_size = A.m_size;
		m_ROMHashTable = A.m_ROMHashTable;

		if ( !m_next )
		{
			m_next = A.m_next;
			A.m_next = this;
		}

		switch( (m_type = A.m_type) )
		{
			case SV_HASH_TABLE:
			case SV_VALUE:
			{
				m_data = new WRValue[m_size];
				m_hashTable = new uint32_t[m_mod];
				memcpy( m_hashTable, A.m_hashTable, m_mod*sizeof(uint32_t) );
				for( unsigned int i=0; i<m_size; ++i )
				{
					((WRValue*)m_data)[i] = ((WRValue*)A.m_data)[i];
				}
				break;
			}

			case SV_CHAR:
			{
				m_Cdata = new unsigned char[m_size+1];
				m_Cdata[m_size] = 0;
				memcpy( (char*)m_data, (char*)A.m_data, m_size );
				break;
			}
		}

		return *this;
	}

	void* operator[]( const unsigned int l ) { return get(l); }

	//------------------------------------------------------------------------------
	WRValue* getAsRawValueHashTable( const uint32_t hash )
	{
		uint32_t index = hash % m_mod;
		if ( m_hashTable[index] != hash )
		{
			if (m_hashTable[index] == 0)
			{
				m_hashTable[index] = hash;
			}
			else
			{
				growHash(hash);
				index = hash % m_mod;
			}
		}
		return m_Vdata + index;
	}
	
	//------------------------------------------------------------------------------
	void* get( const uint32_t l )
	{
		int s = l < m_size ? l : m_size - 1;

		switch( m_type )
		{
			case SV_VALUE: { return (void*)(m_Vdata + s); }
			case SV_CHAR: { return (void*)(m_Cdata + s); }

			case SV_HASH_TABLE:
			{

				// zero remains "null hash" by scrambling. Which means
				// a hash of "0x55555555" will break the system. If you
				// are reading this comment then I'm sorry, the only
				// way out is to re-architect so this hash is not
				// generated by your program
				assert( l != 0x55555555 );


				uint32_t hash = l ^ 0x55555555; 
				uint32_t index = hash % m_mod;

				if (m_hashTable[index] != hash) // its us
				{
					if ( m_hashTable[index] == 0 ) // empty? claim it
					{
						m_hashTable[index] = hash;
					}
					else
					{
						growHash( hash ); // not empty and not us, there was a collision
						index = hash % m_mod;
					}
				}

				return m_Vdata + (index<<1);
			}

			default: return 0;
		}
	}

private:
	WRGCObject(WRGCObject& A);
};

#endif
