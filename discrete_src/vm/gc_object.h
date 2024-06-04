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

// ever solve a rubik's cube? You have if you define your current
// arrangement as "solved" since it is exactly as unique as the
// traditional solid-color "solve". That's what this is, so int/floats
// can be their own hash (0x0 needing to be valid)
#define WRENCH_NULL_HASH 0xABABABAB  // -1414812757 / -1.2197928214371934e-12


//------------------------------------------------------------------------------
class WRGCObject
{
public:

	// the order here matters for data alignment
	
#if (__cplusplus <= 199711L)
	int8_t m_type; // carries the type
#else
	WRGCObjectType m_type;
#endif

	int8_t m_skipGC;
	uint16_t m_mod;
	uint32_t m_size;

	union
	{
		uint32_t* m_hashTable;
		const unsigned char* m_ROMHashTable;
		WRContext* m_creatorContext;
	};

	union
	{
		void* m_data;
		char* m_SCdata;
		unsigned char* m_Cdata;
		WRValue* m_Vdata;
	};

	union
	{
		WRGCObject* m_next; // for gc
		void* m_vNext;
	};

	//------------------------------------------------------------------------------
	void init( const unsigned int size, const WRGCObjectType type, bool clear )
	{
		memset( (unsigned char*)this, 0, sizeof(WRGCObject) );

		if ( (m_type = type) == SV_VALUE )
		{
			m_size = size;
			m_Vdata = (WRValue*)g_malloc( m_size * sizeof(WRValue) );
			if ( clear )
			{
				memset( m_SCdata, 0, m_size * sizeof(WRValue) );
			}

		}
		else if ( m_type == SV_CHAR )
		{
			m_size = size;
			m_Cdata = (unsigned char*)g_malloc( m_size + 1 );
			if ( clear )
			{
				memset( m_SCdata, 0, m_size + 1 );
			}
			else
			{
				m_Cdata[size] = 0;
			}
		}
		else
		{
			growHash( WRENCH_NULL_HASH, size );
		}
	}

	//------------------------------------------------------------------------------
	void clear()
	{
		if ( m_type == SV_VALUE || m_type == SV_CHAR )
		{
			g_free( m_Cdata );
		}
		else
		{
			g_free( m_hashTable );
		}
	}
	
	//------------------------------------------------------------------------------
	WRValue* getAsRawValueHashTable( const uint32_t hash )
	{
#ifdef WRENCH_COMPACT
		uint32_t index = getIndexOfHit( hash, false );
#else
		uint32_t index = hash % m_mod;
		if ( m_hashTable[index] != hash )
		{
			if ( m_hashTable[(index = (index + 1) % m_mod)] != hash )
			{
				if ( m_hashTable[(index = (index + 1) % m_mod)] != hash )
				{
					if ( m_hashTable[(index = (index + 1) % m_mod)] != hash )
					{
						index = getIndexOfHit(hash, true);
					}
				}
			}
		}
#endif
		
		return m_Vdata + index;
	}

	//------------------------------------------------------------------------------
	WRValue* exists( const uint32_t hash, bool removeIfPresent )
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
			m_hashTable[index] = WRENCH_NULL_HASH;
		}
		
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
			s = getIndexOfHit(l, false) << 1;
		}
		else if ( m_type == SV_VOID_HASH_TABLE )
		{
			return getAsRawValueHashTable( l );
		}
		// else it must be SV_VALUE

		return m_Vdata + s;
	}

private:

	//------------------------------------------------------------------------------
	uint32_t getIndexOfHit( const uint32_t hash, const bool insert )
	{
		uint32_t index = hash % m_mod;
		if ( m_hashTable[index] == hash )
		{
			return index; // immediate hits should be cheap
		}

		int tries = 3;
		do
		{
			if ( insert && m_hashTable[index] == WRENCH_NULL_HASH )
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
			uint32_t* proposed = (uint32_t*)g_malloc( total );

			memset( (unsigned char *)proposed, 0, total );
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
						g_free( proposed );
						++t;

						assert( (unsigned int)newMod != 49157 );

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
				if ( oldHashTable[v] == WRENCH_NULL_HASH )
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

			g_free( oldHashTable );

			m_Vdata = newValues;

			return getIndexOfHit( hash, true );
		}
	}

	WRGCObject& operator= ( WRGCObject& A );
	WRGCObject(WRGCObject& A);
};

#endif
