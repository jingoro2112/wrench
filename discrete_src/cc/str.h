#ifndef _STR_H
#define _STR_H
/* ------------------------------------------------------------------------- */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef STR_FILE_OPERATIONS
#include <sys/stat.h>
#endif

// same as str.h but for char only so no template overhead, also no
// new/delete just malloc/free

const unsigned int c_sizeofBaseString = 15; // this lib tries not to use dynamic RAM unless it has to
const int c_formatBaseTrySize = 80;

//-----------------------------------------------------------------------------
class WRstr
{
public:
	WRstr() { m_len = 0; m_smallbuf[0] = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; }
	WRstr( const WRstr& str) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(str, str.size()); } 
	WRstr( const WRstr* str ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; if ( str ) { set(*str, str->size()); } } 
	WRstr( const char* s, const unsigned int len ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, len); }
	WRstr( const char* s ) { m_len = 0; m_str = m_smallbuf; m_buflen = c_sizeofBaseString; set(s, (unsigned int)strlen(s)); }
	WRstr( const char c) { m_len = 1; m_str = m_smallbuf; m_smallbuf[0] = c; m_smallbuf[1] = 0; m_buflen = c_sizeofBaseString; } 

#ifdef STR_FILE_OPERATIONS
	inline bool fileToBuffer( const char* fileName, const bool appendToBuffer =false );
	inline bool bufferToFile( const char* fileName, const bool append =false ) const;
#else
	bool fileToBuffer( const char* fileName, const bool appendToBuffer =false ) { return false; }
	bool bufferToFile( const char* fileName, const bool append =false ) const { return false; }
#endif

	WRstr& clear() { m_len = 0; m_str[0] = 0; return *this; }

#if __cplusplus > 199711L
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

#if __cplusplus > 199711L

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
		char* alloc = (char*)g_malloc( len + 1 );

		va_list vacopy;
		va_copy( vacopy, arg );
		len = vsnprintf( alloc, len, format, arg );
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

	if ( len < c_formatBaseTrySize )
	{
		set( buf, len );
	}
	else
	{
		char* alloc = (char*)g_malloc(len + 1);

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

	if ( len < c_formatBaseTrySize )
	{
		insert( buf, len, m_len );
	}
	else
	{
		char* alloc = (char*)g_malloc(len + 1);

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
