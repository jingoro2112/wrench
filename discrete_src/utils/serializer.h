/*******************************************************************************
Copyright (c) 2026 Curt Hartung -- curt.hartung@gmail.com

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
	WRValueSerializer( const char* data, const unsigned int size ) : m_pos(0), m_size(size)
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
				if ( size > 0 )
				{
					memcpy( m_buf, data, size );
				}
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
		if ( size <= 0 )
		{
			return true;
		}

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
		if ( size <= 0 )
		{
			return;
		}

		if ( m_pos + size >= m_size )
		{
			m_size += (size*2) + 8;
			char* newBuf = (char*)g_malloc( m_size );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
			if ( !newBuf )
			{
				g_mallocFailed = true;
				return;
			}
#endif
			if ( m_pos > 0 )
			{
				memcpy( newBuf, m_buf, m_pos );
			}
			g_free( m_buf );
			m_buf = newBuf;
		}

		memcpy( m_buf + m_pos, data, size );
		m_pos += size;
	}

private:

	unsigned int m_pos;
	unsigned int m_size;
	char* m_buf;
};

#endif
