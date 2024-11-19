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

#endif