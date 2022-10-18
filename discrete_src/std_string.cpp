#include "wrench.h"

#include <string.h>

//------------------------------------------------------------------------------
int wr_strlenEx( WRValue* val )
{
	if ( val->type == WR_REF )
	{
		return wr_strlenEx( val->r );
	}

	return (val->xtype == WR_EX_ARRAY && val->va->m_type == SV_CHAR) ? val->va->m_size :  0;
}

//------------------------------------------------------------------------------
void wr_strlen( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->p2 = INIT_AS_INT;
	stackTop->i = argn == 1 ? wr_strlenEx( stackTop - 1 ) : 0;
}

//------------------------------------------------------------------------------
void wr_substr( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	unsigned int size;
	const char* str;
	WRValue* args = stackTop - argn;

	if ( argn < 1 || !(str = args[0].c_str(&size)) )
	{
		return;
	}

	unsigned int start = 0;
	unsigned int count = size;
	if ( argn > 1 )
	{
		start = args[1].asInt();
		start = (start > size) ? size : start;
		count = size - start;
	}

	if ( argn > 2 )
	{
		count = args[2].asInt();
		if ( (start + count) > size )
		{
			count = size - start;
		}
	}

	stackTop->p2 = INIT_AS_ARRAY;
	stackTop->va = c->getSVA( count, SV_CHAR, false );
	memcpy( stackTop->va->m_Cdata, str + start, count );
}

//------------------------------------------------------------------------------
int wr_sprintfEx( char* outbuf, const char* fmt, WRValue* args, const int argn )
{
	enum
	{
		zeroPad         = 1<<0,
		negativeJustify = 1<<1,
		secondPass      = 1<<2,
		negativeSign    = 1<<3,
		parsingSigned   = 1<<4,
		altRep			= 1<<5,
	};

	char* out = outbuf;
	int listPtr = 0;

resetState:

	char padChar = ' ';
	unsigned char columns = 0;
	char flags = 0;

	for(;;)
	{
		char c = *fmt;
		fmt++;

		if ( !c )
		{
			*out = 0;
			break;
		}

		if ( !(secondPass & flags) )
		{
			if ( c != '%' ) // literal
			{
				*out++ = c;
			}
			else // possibly % format specifier
			{
				flags |= secondPass;
			}
		}
		else if ( c >= '0' && c <= '9' ) // width
		{
			columns *= 10;
			columns += c - '0';
			if ( !columns ) // leading zero
			{
				flags |= zeroPad;
				padChar = '0';
			}
		}
		else if ( c == '#' )
		{
			flags |= altRep;
		}
		else if ( c == '-' ) // left-justify
		{
			flags |= negativeJustify;
		}
		else if ( c == 'c' ) // character
		{
			if ( listPtr < argn )
			{
				*out++ = (char)(args[listPtr++].i);
			}
			goto resetState;
		}
		else if ( c == '%' ) // literal %
		{
			*out++ = c;
			goto resetState;
		}
		else // string or integer
		{
			char buf[20]; // buffer for integer

			const char *ptr; // pointer to first char of integer

			if ( c == 's' ) // string
			{
				buf[0] = 0;
				if ( listPtr < argn )
				{
					ptr = args[listPtr].c_str();
					if ( !ptr )
					{
						return 0;
					}
					++listPtr;
				}
				else
				{
					ptr = buf;
				}

				padChar = ' '; // in case some joker uses a 0 in their column spec

copyToString:

				// get the string length so it can be formatted, don't
				// copy it, just count it
				unsigned char len = 0;
				for ( ; *ptr; ptr++ )
				{
					len++;
				}
				
				ptr -= len;
				
				// Right-justify
				if ( !(flags & negativeJustify) )
				{
					for ( ; columns > len; columns-- )
					{
						*out++ = padChar;
					}
				}
				
				if ( flags & negativeSign )
				{
					*out++ = '-';
				}

				if ( flags & altRep || c == 'p' )
				{
					*out++ = '0';
					*out++ = 'x';
				}

				for (unsigned char l = 0; l < len; ++l )
				{
					*out++ = *ptr++;
				}

				// Left-justify
				for ( ; columns > len; columns-- )
				{
					*out++ = ' ';
				}

				goto resetState;
			}
			else
			{
				unsigned char base;
				unsigned char width;
				unsigned int val;

				if ( c == 'd' || c == 'i' )
				{
					flags |= parsingSigned;
					goto parseDecimal;
				}
				else if ( c == 'u' ) // decimal
				{
parseDecimal:
					base  = 10;
					width = 10;
					goto convertBase;
				}
				else if ( c == 'b' ) // binary
				{
					base  = 2;
					width = 16;
					goto convertBase;
				}
				else if ( c == 'o' ) // octal
				{
					base  = 8;
					width = 5;
					goto convertBase;
				}
				else if ( c == 'x' || c == 'X' || c == 'p' ) // hexadecimal or pointer (pointer is treated as 'X')
				{
					base = 16;
					width = 4;
convertBase:
					if ( listPtr < argn )
					{
						val = args[listPtr++].i;
					}
					else
					{
						val = 0;
					}

					if ( (flags & parsingSigned) && (val & 0x80000000) )
					{
						flags |= negativeSign;
						val = -(int)val;
					}

					// Convert to given base, filling buffer backwards from least to most significant
					char* p = buf + width;
					*p = 0;
					ptr = p; // keep track of one past left-most non-zero digit
					do
					{
						char d = val % base;
						val /= base;

						if ( d )
						{
							ptr = p;
						}

						d += '0';
						if ( d > '9' ) // handle bases higher than 10
						{
							d += 'A' - ('9' + 1);
							if ( c == 'x' ) // lowercase
							{
								d += 'a' - 'A';
							}
						}

						*--p = d;

					} while ( p != buf );

					ptr--; // was one past char we want

					goto copyToString;
				}
				else // invalid format specifier
				{
					goto resetState;
				}
			}
		}
	}

	return out - outbuf;
}

//------------------------------------------------------------------------------
void wr_printf( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();

#ifdef WRENCH_STD_FILE
	if( argn < 1 )
	{
		return;
	}
	WRValue* args = stackTop - argn;

	const char *fmt = (const char*)args[0].va->m_Cdata;
	if ( !fmt )
	{
		return;
	}
	
	char outbuf[512];
	stackTop->i = wr_sprintfEx( outbuf, fmt, args + 1, argn - 1 );

	printf( "%s", outbuf );
#endif
}

//------------------------------------------------------------------------------
void wr_sprintf( WRValue* stackTop, const int argn, WRContext* c )
{
	stackTop->init();
	if ( argn < 2 )
	{
		return;
	}

	WRValue* args = stackTop - argn;
	if ( args[1].xtype != WR_EX_ARRAY || args[1].va->m_type != SV_CHAR
		 || args[0].type != WR_REF )
	{
		return;
	}

	const char *fmt = (const char*)args[1].va->m_Cdata;
	if ( !fmt )
	{
		return;
	}

	char outbuf[512];
	stackTop->i = wr_sprintfEx( outbuf, fmt, args + 2, argn - 2 );

	args[0].r->p2 = INIT_AS_ARRAY;
	args[0].r->va = c->getSVA( stackTop->i, SV_CHAR, false );
	memcpy( args[0].r->va->m_Cdata, outbuf, stackTop->i );
}


//------------------------------------------------------------------------------
void wr_loadStringLib( WRState* w )
{
	wr_registerLibraryFunction( w, "str::strlen", wr_strlen );
	wr_registerLibraryFunction( w, "str::substr", wr_substr );
	wr_registerLibraryFunction( w, "str::sprintf", wr_sprintf );
	wr_registerLibraryFunction( w, "str::printf", wr_printf );
}

