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

	WRGCObject* m_next; // for gc

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
	
	void* operator[]( const unsigned int l ) { return get(l); }

	//------------------------------------------------------------------------------
	uint32_t getIndexOfHit( const uint32_t hash, const bool insert )
	{
		uint32_t index = hash % m_mod;
		int tries = m_mod >> 3;
		do
		{
			if ( m_hashTable[index] == hash )
			{
				return index; // hits are very cheap
			}
			else if ( insert && m_hashTable[index] == 0 )
			{
				m_hashTable[index] = hash;
				return index;
			}

			index = (index + 1) % m_mod;
			
		} while( tries-- );

		return insert ? growHash(hash) : getIndexOfHit( hash, true );
	}

	//------------------------------------------------------------------------------
	WRValue* getAsRawValueHashTable( const uint32_t hash )
	{
		uint32_t index = getIndexOfHit( hash, false );
		return m_Vdata + index;
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
			// a hash of "0x55555555" will break the system. If you
			// are reading this comment then I'm sorry, the only
			// way out is to re-architect so this hash is not
			// generated by your program
			assert( l != 0x55555555 );

			uint32_t hash = l ^ 0x55555555; 

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
