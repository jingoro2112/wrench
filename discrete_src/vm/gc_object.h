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
	GCFlag_NoContext = 1<<0,
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
