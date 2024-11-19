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
		m_Vdata = (WRValue*)g_malloc( size * sizeof(WRValue) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_Vdata )
		{
			m_size = 0;
			g_mallocFailed = true;
			return 0;
		}
#endif
		ret *= sizeof(WRValue);
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
		growHash( WRENCH_NULL_HASH, size );
		ret = sizeof(WRGCBase) + (m_size * sizeof(WRValue));
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
	else if ( m_type == SV_HASH_ENTRY )
	{

		
		printf("WAIT");
		assert(0);
		// todo


		
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
uint32_t WRGCObject::growHash( const uint32_t hash, const uint16_t sizeHint )
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
