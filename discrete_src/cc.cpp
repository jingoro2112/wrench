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
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

#define WR_COMPILER_LITERAL_STRING 0x10 
#define KEYHOLE_OPTIMIZER

//------------------------------------------------------------------------------
const char* c_reserved[] =
{
	"enum",
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
	"false",
	"float",
	"for",
	"if",
	"int",
	"return",
	"switch",
	"true",
	"function",
	"while",
	"new",
	"null",
	"struct",
	"goto",
	"gc_pause",
	""
};

//#define _DUMP
#ifdef _DUMP
#define D_STREAM(a) streamDump(a)
//------------------------------------------------------------------------------
const char* wr_asciiDump( const void* d, unsigned int len, WRstr& str )
{
	const unsigned char* data = (char unsigned *)d;
	str.clear();
	for( unsigned int i=0; i<len; i++ )
	{
		str.appendFormat( "0x%08X: ", i );
		char dump[24];
		unsigned int j;
		for( j=0; j<16 && i<len; j++, i++ )
		{
			dump[j] = isgraph((unsigned char)data[i]) ? data[i] : '.';
			dump[j+1] = 0;
			str.appendFormat( "%02X ", (unsigned char)data[i] );
		}

		for( ; j<16; j++ )
		{
			str.appendFormat( "   " );
		}
		i--;
		str += ": ";
		str += dump;
		str += "\n";
	}

	return str;
}
void streamDump( WROpcodeStream const& stream )
{
	WRstr str;
	wr_asciiDump( stream, stream.size(), str );
	printf( "%d:\n%s\n", stream.size(), str.c_str() );
}

#else
#define D_STREAM(a)
#endif

//------------------------------------------------------------------------------
bool WRCompilationContext::isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix )
{
	prefix.clear();

	if ( !token.size() || (!isalpha(token[0]) && token[0] != '_' && token[0] != ':') ) // non-zero size and start with alpha or '_' ?
	{
		return false;
	}

	isGlobal = false;

	if ( token[0] == ':' )
	{
		if ( (token.size() > 2) && (token[1] == ':') )
		{
			isGlobal = true;
		}
		else
		{
			return false;
		}
	}

	for( unsigned int i=0; c_reserved[i][0]; ++i )
	{
		if ( token == c_reserved[i] )
		{
			return false;
		}
	}

	for( unsigned int i=0; i<token.size(); i++ ) // entire token alphanumeric or '_'?
	{
		if ( token[i] == ':' )
		{
			if ( token[++i] != ':' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			if ( i != 1 )
			{
				for( unsigned int p=0; p<i - 1 && p<token.size(); ++p )
				{
					prefix += token[p];
				}

				for( unsigned int t=2; (t+prefix.size())<token.size(); ++t)
				{
					token[t-2] = token[t+prefix.size()];
				}
				token.shave( prefix.size() + 2 );
				i -= (prefix.size() + 1);
			}
			
			isGlobal = true;
			continue;
		}
		
		if ( !isalnum(token[i]) && token[i] != '_' )
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::getToken( WRExpressionContext& ex, const char* expect )
{
	WRValue& value = ex.value;
	WRstr& token = ex.token;
	
	if ( m_loadedToken.size() || (m_loadedValue.type != WR_REF) )
	{
		token = m_loadedToken;
		value = m_loadedValue;
	}
	else
	{
		value.p2 = INIT_AS_REF;

		ex.spaceBefore = (m_pos < m_sourceLen) && isspace(m_source[m_pos]);
	
		do
		{
			if ( m_pos >= m_sourceLen )
			{
				m_EOF = true;
				return false;
			}
			
		} while( isspace(m_source[m_pos++]) );

		token = m_source[m_pos - 1];

		unsigned int t=0;
		for( ; c_operations[t].token && strncmp( c_operations[t].token, "@macroBegin", 11); ++t );

		int offset = m_pos - 1;

		for( ; c_operations[t].token; ++t )
		{
			int len = (int)strnlen( c_operations[t].token, 20 );
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, c_operations[t].token, len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}
				
				m_pos += len - 1;
				token = c_operations[t].token;
				goto foundMacroToken;
			}
		}

		for( t = 0; t<m_units[0].constantValues.count(); ++t )
		{
			int len = m_units[0].constantValues[t].label.size();
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, m_units[0].constantValues[t].label.c_str(), len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}

				m_pos += len - 1;
				token.clear();
				value = m_units[0].constantValues[t].value;
				goto foundMacroToken;
			}
		}

		for( t = 0; t<m_units[m_unitTop].constantValues.count(); ++t )
		{
			int len = m_units[m_unitTop].constantValues[t].label.size();
			if ( ((offset + len) < m_sourceLen)
				 && !strncmp(m_source + offset, m_units[m_unitTop].constantValues[t].label.c_str(), len) )
			{
				if ( isalnum(m_source[offset+len]) )
				{
					continue;
				}

				m_pos += len - 1;
				token.clear();
				value = m_units[m_unitTop].constantValues[t].value;
				goto foundMacroToken;
			}
		}

		if ( token[0] == '-' )
		{
			if ( m_pos < m_sourceLen )
			{
				if ( (isdigit(m_source[m_pos]) && !m_LastParsedLabel) || m_source[m_pos] == '.' )
				{
					goto parseAsNumber;
				}
				else if ( m_source[m_pos] == '-' )
				{
					token += '-';
					++m_pos;
				}
				else if ( m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
		}
		else
		{
			m_LastParsedLabel = false;
		
			if ( token[0] == '=' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '!' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '*' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '%' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '<' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
				else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '<' )
				{
					token += '<';
					++m_pos;

					if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
					{
						token += '=';
						++m_pos;
					}
				}
			}
			else if ( token[0] == '>' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
				else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '>' )
				{
					token += '>';
					++m_pos;

					if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
					{
						token += '=';
						++m_pos;
					}
				}
			}
			else if ( token[0] == '&' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '&' )
				{
					token += '&';
					++m_pos;
				}
				else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '^' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '|' )
			{
				if ( (m_pos < m_sourceLen) && m_source[m_pos] == '|' )
				{
					token += '|';
					++m_pos;
				}
				else if ( (m_pos < m_sourceLen) && m_source[m_pos] == '=' )
				{
					token += '=';
					++m_pos;
				}
			}
			else if ( token[0] == '+' )
			{
				if ( m_pos < m_sourceLen )
				{
					if ( m_source[m_pos] == '+' )
					{
						token += '+';
						++m_pos;
					}
					else if ( m_source[m_pos] == '=' )
					{
						token += '=';
						++m_pos;
					}
				}
			}
			else if ( token[0] == '\"' || token[0] == '\'' )
			{
				bool single = token[0] == '\'';
				token.clear();

				do
				{
					if (m_pos >= m_sourceLen)
					{
						m_err = WR_ERR_unterminated_string_literal;
						m_EOF = true;
						return false;
					}

					char c = m_source[m_pos];
					if (c == '\"' && !single ) // terminating character
					{
						if ( single )
						{
							m_err = WR_ERR_bad_expression;
							return false;
						}
						++m_pos;
						break;
					}
					else if ( c == '\'' && single )
					{
						if ( !single || (token.size() > 1) )
						{
							m_err = WR_ERR_bad_expression;
							return false;
						}
						++m_pos;
						break;
					}
					else if (c == '\n')
					{
						m_err = WR_ERR_newline_in_string_literal;
						return false;
					}
					else if (c == '\\')
					{
						c = m_source[++m_pos];
						if (m_pos >= m_sourceLen)
						{
							m_err = WR_ERR_unterminated_string_literal;
							m_EOF = true;
							return false;
						}

						if (c == '\\') // escaped slash
						{
							token += '\\';
						}
						else if (c == '0')
						{
							token += atoi(m_source + m_pos);
							for (; (m_pos < m_sourceLen) && isdigit(m_source[m_pos]); ++m_pos);
							--m_pos;
						}
						else if (c == '\"')
						{
							token += '\"';
						}
						else if (c == '\'')
						{
							token += '\'';
						}
						else if (c == 'n')
						{
							token += '\n';
						}
						else if (c == 'r')
						{
							token += '\r';
						}
						else if (c == 't')
						{
							token += '\t';
						}
						else
						{
							m_err = WR_ERR_bad_string_escape_sequence;
							return false;
						}
					}
					else
					{
						token += c;
					}

				} while( ++m_pos < m_sourceLen );

				value.p2 = INIT_AS_INT;
				if ( single )
				{
					value.ui = token.size() == 1 ? token[0] : 0;
					token.clear();
				}
				else
				{
					value.p2 = INIT_AS_INT;
					value.type = (WRValueType)WR_COMPILER_LITERAL_STRING;
					value.p = &token;
				}
			}
			else if ( token[0] == '/' ) // might be a comment
			{
				if ( m_pos < m_sourceLen )
				{	
					if ( !isspace(m_source[m_pos]) )
					{
						if ( m_source[m_pos] == '/' )
						{
							for( ; m_pos<m_sourceLen && m_source[m_pos] != '\n'; ++m_pos ); // clear to end EOL

							return getToken( ex, expect );
						}
						else if ( m_source[m_pos] == '*' )
						{
							for( ; (m_pos+1)<m_sourceLen
								 && !(m_source[m_pos] == '*' && m_source[m_pos+1] == '/');
								 ++m_pos ); // find end of comment

							m_pos += 2;

							return getToken( ex, expect );

						}
						else if ( m_source[m_pos] == '=' )
						{
							token += '=';
							++m_pos;
						}
					}					
					//else // bare '/' 
				}
			}
			else if ( isdigit(token[0])
					  || (token[0] == '.' && isdigit(m_source[m_pos])) )
			{
				if ( m_pos >= m_sourceLen )
				{
					return false;
				}

parseAsNumber:

				m_LastParsedLabel = true;

				if ( token[0] == '0' && m_source[m_pos] == 'x' ) // interpret as hex
				{
					token.clear();
					m_pos++;

					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( !isxdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					value.p2 = INIT_AS_INT;
					value.ui = strtoul( token, 0, 16 );
				}
				else if (token[0] == '0' && m_source[m_pos] == 'b' )
				{
					token.clear();
					m_pos++;

					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( !isxdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					value.p2 = INIT_AS_INT;
					value.i = strtol( token, 0, 2 );
				}
				else
				{
					bool decimal = token[0] == '.';
					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( m_source[m_pos] == '.' )
						{
							if ( decimal )
							{
								return false;
							}

							decimal = true;
						}
						else if ( !isdigit(m_source[m_pos]) )
						{
							if ( m_source[m_pos] == 'f' || m_source[m_pos] == 'F')
							{
								decimal = true;
								m_pos++;
							}

							break;
						}

						token += m_source[m_pos++];
					}

					if ( decimal )
					{
						value.p2 = INIT_AS_FLOAT;
						value.f = (float)atof( token );
					}
					else
					{
						value.p2 = INIT_AS_INT;
						value.ui = (uint32_t)strtoul( token, 0, 10 );
					}
				}
			}
			else if ( token[0] == ':' && isspace(m_source[m_pos]) )
			{
				
			}
			else if ( isalpha(token[0]) || token[0] == '_' || token[0] == ':' ) // must be a label
			{
				if ( token[0] != ':' || m_source[m_pos] == ':' )
				{
					m_LastParsedLabel = true;
					if ( m_pos < m_sourceLen
						 && token[0] == ':'
						 && m_source[m_pos] == ':' )
					{
						token += ':';
						++m_pos;
					}

					for (; m_pos < m_sourceLen; ++m_pos)
					{
						if ( m_source[m_pos] == ':' && m_source[m_pos + 1] == ':' && token.size() > 0 )
						{
							token += "::";
							m_pos ++;
							continue;
						}
						
						if (!isalnum(m_source[m_pos]) && m_source[m_pos] != '_' )
						{
							break;
						}

						token += m_source[m_pos];
					}

					if (token == "true")
					{
						value.p2 = INIT_AS_INT;
						value.i = 1;
						token = "1";
					}
					else if (token == "false" || token == "null" )
					{
						value.p2 = INIT_AS_INT;
						value.i = 0;
						token = "0";
					}
				}
			}
		}

foundMacroToken:
	
		ex.spaceAfter = (m_pos < m_sourceLen) && isspace(m_source[m_pos]);
	}

	m_loadedToken.clear();
	m_loadedValue.p2 = INIT_AS_REF;

	if ( expect && (token != expect) )
	{
		return false;
	}
	
	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckSkipLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o )
{
	if ( bytecode.opcodes[o] == O_LoadFromLocal
		 && bytecode.opcodes[o-1] == O_LoadFromLocal )
	{
		bytecode.all[a-3] = O_LLValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += O_IndexSkipLoad;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromGlobal
			  && bytecode.opcodes[o-1] == O_LoadFromLocal )
	{
		bytecode.all[a-3] = O_LGValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromLocal
			  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
	{
		bytecode.all[a-3] = O_GLValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}
	else if ( bytecode.opcodes[o] == O_LoadFromGlobal
			  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
	{
		bytecode.all[a-3] = O_GGValues;
		bytecode.all[a-1] = bytecode.all[a];
		bytecode.all[a] = opcode;
		bytecode.opcodes.shave(2);
		bytecode.opcodes += opcode;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckFastLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o )
{
	if ( a <= 2 || o == 0 )
	{
		return false;
	}

	if ( (opcode == O_Index && CheckSkipLoad(O_IndexSkipLoad, bytecode, a, o))
		 || (opcode == O_BinaryMod && CheckSkipLoad(O_BinaryModSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryRightShift && CheckSkipLoad(O_BinaryRightShiftSkipLoad, bytecode, a, o))
		 || (opcode == O_BinaryLeftShift && CheckSkipLoad(O_BinaryLeftShiftSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryAnd && CheckSkipLoad(O_BinaryAndSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryOr && CheckSkipLoad(O_BinaryOrSkipLoad, bytecode, a, o)) 
		 || (opcode == O_BinaryXOR && CheckSkipLoad(O_BinaryXORSkipLoad, bytecode, a, o)) )
	{
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::IsLiteralLoadOpcode( unsigned char opcode )
{
	return opcode == O_LiteralInt32
			|| opcode == O_LiteralZero
			|| opcode == O_LiteralFloat
			|| opcode == O_LiteralInt8
			|| opcode == O_LiteralInt16;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::CheckCompareReplace( WROpcode LS, WROpcode GS, WROpcode ILS, WROpcode IGS, WRBytecode& bytecode, unsigned int a, unsigned int o )
{
	if ( IsLiteralLoadOpcode(bytecode.opcodes[o-1]) )
	{
		if ( bytecode.opcodes[o] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 1 ] = GS;
			bytecode.opcodes[o] = GS;
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LoadFromLocal )
		{
			bytecode.all[ a - 1 ] = LS;
			bytecode.opcodes[o] = LS;
			return true;
		}
	}
	else if ( IsLiteralLoadOpcode(bytecode.opcodes[o]) )
	{
		if ( (bytecode.opcodes[o] == O_LiteralInt32 || bytecode.opcodes[o] == O_LiteralFloat)
			 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 6 ] = bytecode.all[ a ]; // store i3
			bytecode.all[ a ] = bytecode.all[ a - 5 ]; // move index

			bytecode.all[ a - 5 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 4 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a - 6 ];
			bytecode.all[ a - 6 ] = bytecode.opcodes[o];
			bytecode.all[ a - 1 ] = IGS;

			bytecode.opcodes[o] = IGS; // reverse the logic

			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt8
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a ];
			bytecode.all[ a ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.all[ a - 3 ] = O_LiteralInt8;

			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt16
				  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 2 ] = bytecode.all[ a ];

			bytecode.all[ a - 4 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a ] = bytecode.all[ a - 4 ];
			bytecode.all[ a - 1 ] = IGS;
			bytecode.all[ a - 4 ] = O_LiteralInt16;

			bytecode.opcodes[o] = IGS; // reverse the logic
			return true;
		}
		if ( (bytecode.opcodes[o] == O_LiteralInt32 || bytecode.opcodes[o] == O_LiteralFloat)
			 && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 6 ] = bytecode.all[ a ]; // store i3
			bytecode.all[ a ] = bytecode.all[ a - 5 ]; // move index

			bytecode.all[ a - 5 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 4 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a - 6 ];
			bytecode.all[ a - 6 ] = bytecode.opcodes[o];
			bytecode.all[ a - 1 ] = ILS;

			bytecode.opcodes[o] = ILS; // reverse the logic

			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralZero
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = O_LiteralZero;
			bytecode.all[ a ] = bytecode.all[ a - 1];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt8
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
			bytecode.all[ a - 2 ] = bytecode.all[ a ];
			bytecode.all[ a ] = bytecode.all[ a - 1 ];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.all[ a - 3 ] = O_LiteralInt8;

			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
		else if ( bytecode.opcodes[o] == O_LiteralInt16
				  && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 2 ] = bytecode.all[ a ];

			bytecode.all[ a - 4 ] = bytecode.all[ a - 3 ];
			bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
			bytecode.all[ a ] = bytecode.all[ a - 4 ];
			bytecode.all[ a - 1 ] = ILS;
			bytecode.all[ a - 4 ] = O_LiteralInt16;

			bytecode.opcodes[o] = ILS; // reverse the logic
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushOpcode( WRBytecode& bytecode, WROpcode opcode )
{
#ifdef KEYHOLE_OPTIMIZER
	unsigned int o = bytecode.opcodes.size();
	if ( o )
	{
		// keyhole optimizations

		--o;
		unsigned int a = bytecode.all.size() - 1;

		if ( opcode == O_Return
			 && bytecode.opcodes[o] == O_LiteralZero )
		{
			bytecode.all[a] = O_ReturnZero;
			bytecode.opcodes[o] = O_ReturnZero;
			return;
		}
		else if ( opcode == O_CompareEQ && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareEQ;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromLocal && bytecode.opcodes[o-1] == O_LoadFromLocal )
		{
			bytecode.all[ a - 3 ] = O_LLCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_LLCompareLE;
			return;
		}
		else if ( opcode == O_CompareEQ && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareEQ;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareEQ;
			return;
		}
		else if ( opcode == O_CompareNE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareNE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareNE;
			return;
		}
		else if ( opcode == O_CompareGT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareGT;
			return;
		}
		else if ( opcode == O_CompareLT && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLT;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareLT;
			return;
		}
		else if ( opcode == O_CompareGE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareGE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareGE;
			return;
		}
		else if ( opcode == O_CompareLE && o>0 && bytecode.opcodes[o] == O_LoadFromGlobal && bytecode.opcodes[o-1] == O_LoadFromGlobal )
		{
			bytecode.all[ a - 3 ] = O_GGCompareLE;
			bytecode.all[ a - 1 ] = bytecode.all[ a ];
			bytecode.all.shave( 1 );
			bytecode.opcodes.clear();
			bytecode.opcodes += O_GGCompareLE;
			return;
		}
		else if ( (opcode == O_CompareEQ && (o>0))
				  && CheckCompareReplace(O_LSCompareEQ, O_GSCompareEQ, O_LSCompareEQ, O_GSCompareEQ, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareNE && (o>0))
				  && CheckCompareReplace(O_LSCompareNE, O_GSCompareNE, O_LSCompareNE, O_GSCompareNE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareGE && (o>0))
			 && CheckCompareReplace(O_LSCompareGE, O_GSCompareGE, O_LSCompareLE, O_GSCompareLE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareLE && (o>0))
				  && CheckCompareReplace(O_LSCompareLE, O_GSCompareLE, O_LSCompareGE, O_GSCompareGE, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareGT && (o>0))
				  && CheckCompareReplace(O_LSCompareGT, O_GSCompareGT, O_LSCompareLT, O_GSCompareLT, bytecode, a, o) )
		{
			return;
		}
		else if ( (opcode == O_CompareLT && (o>0))
				  && CheckCompareReplace(O_LSCompareLT, O_GSCompareLT, O_LSCompareGT, O_GSCompareGT, bytecode, a, o) )
		{
			return;
		}
		else if ( CheckFastLoad(opcode, bytecode, a, o) )
		{
			return;
		}
		else if ( opcode == O_BinaryMultiplication && (a>2) )
		{
			
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				// LoadFromGlobal   a - 3
				// [index]          a - 2
				// LoadFromGlobal   a - 1
				// [index]          a

				bytecode.all[ a - 3 ] = O_GGBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
				bytecode.all[ a - 2 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryMultiplication;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}
			
			return;
		}
		else if ( opcode == O_BinaryAddition && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a - 2 ];
				bytecode.all[ a - 2 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryAddition;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_BinarySubtraction && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];

				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinarySubtraction;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_BinaryDivision && (a>2) )
		{
			if ( bytecode.opcodes[o] == O_LoadFromGlobal
				 && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_GGBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromGlobal )
			{
				bytecode.all[ a - 3 ] = O_LGBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromGlobal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_GLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else if ( bytecode.opcodes[o] == O_LoadFromLocal
					  && bytecode.opcodes[o-1] == O_LoadFromLocal )
			{
				bytecode.all[ a - 3 ] = O_LLBinaryDivision;
				bytecode.all[ a - 1 ] = bytecode.all[ a ];
				bytecode.all.shave(1);
				bytecode.opcodes.clear();
			}
			else
			{
				bytecode.all += opcode;
				bytecode.opcodes += opcode;
			}

			return;
		}
		else if ( opcode == O_Index )
		{
			if ( bytecode.opcodes[o] == O_LiteralInt16 )
			{
				bytecode.all[a-2] = O_IndexLiteral16;
				bytecode.opcodes[o] = O_IndexLiteral16;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LiteralInt8 )
			{
				bytecode.all[a-1] = O_IndexLiteral8;
				bytecode.opcodes[o] = O_IndexLiteral8;
				return;
			}
		}
		else if ( opcode == O_BZ )
		{
			if ( bytecode.opcodes[o] == O_LLCompareLT )
			{
				bytecode.opcodes[o] = O_LLCompareLTBZ;
				bytecode.all[ a - 2 ] = O_LLCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareGT )
			{
				bytecode.opcodes[o] = O_LLCompareGTBZ;
				bytecode.all[ a - 2 ] = O_LLCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareGE )
			{
				bytecode.opcodes[o] = O_LLCompareGEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareLE )
			{
				bytecode.opcodes[o] = O_LLCompareLEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareEQ )
			{
				bytecode.opcodes[o] = O_LLCompareEQBZ;
				bytecode.all[ a - 2 ] = O_LLCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LLCompareNE )
			{
				bytecode.opcodes[o] = O_LLCompareNEBZ;
				bytecode.all[ a - 2 ] = O_LLCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareLT )
			{
				bytecode.opcodes[o] = O_GGCompareLTBZ;
				bytecode.all[ a - 2 ] = O_GGCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareGT )
			{
				bytecode.opcodes[o] = O_GGCompareGTBZ;
				bytecode.all[ a - 2 ] = O_GGCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareLE )
			{
				bytecode.opcodes[o] = O_GGCompareLEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareGE )
			{
				bytecode.opcodes[o] = O_GGCompareGEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareEQ )
			{
				bytecode.opcodes[o] = O_GGCompareEQBZ;
				bytecode.all[ a - 2 ] = O_GGCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GGCompareNE )
			{
				bytecode.opcodes[o] = O_GGCompareNEBZ;
				bytecode.all[ a - 2 ] = O_GGCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareEQ )
			{
				bytecode.opcodes[o] = O_GSCompareEQBZ;
				bytecode.all[ a - 1 ] = O_GSCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareEQ )
			{
				bytecode.opcodes[o] = O_LSCompareEQBZ;
				bytecode.all[ a - 1 ] = O_LSCompareEQBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareNE )
			{
				bytecode.opcodes[o] = O_GSCompareNEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareNE )
			{
				bytecode.opcodes[o] = O_LSCompareNEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareNEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareGE )
			{
				bytecode.opcodes[o] = O_GSCompareGEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareGE )
			{
				bytecode.opcodes[o] = O_LSCompareGEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareGEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareLE )
			{
				bytecode.opcodes[o] = O_GSCompareLEBZ;
				bytecode.all[ a - 1 ] = O_GSCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareLE )
			{
				bytecode.opcodes[o] = O_LSCompareLEBZ;
				bytecode.all[ a - 1 ] = O_LSCompareLEBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareGT )
			{
				bytecode.opcodes[o] = O_GSCompareGTBZ;
				bytecode.all[ a - 1 ] = O_GSCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareGT )
			{
				bytecode.opcodes[o] = O_LSCompareGTBZ;
				bytecode.all[ a - 1 ] = O_LSCompareGTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_GSCompareLT )
			{
				bytecode.opcodes[o] = O_GSCompareLTBZ;
				bytecode.all[ a - 1 ] = O_GSCompareLTBZ;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LSCompareLT )
			{
				bytecode.opcodes[o] = O_LSCompareLTBZ;
				bytecode.all[ a - 1 ] = O_LSCompareLTBZ;
				return;
			}			
			else if ( bytecode.opcodes[o] == O_CompareEQ ) // assign+pop is very common
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareEQBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareEQBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareEQBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareEQBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBEQ;
					bytecode.opcodes[o] = O_CompareBEQ;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLT )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareLTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareLTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBLT;
					bytecode.opcodes[o] = O_CompareBLT;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGT )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareGTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGTBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareGTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBGT;
					bytecode.opcodes[o] = O_CompareBGT;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareGE )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareGEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareLTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareGEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareLTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBGE;
					bytecode.opcodes[o] = O_CompareBGE;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareLE )
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareLEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareGTBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareLEBZ;
					bytecode.all[a-2] = bytecode.all[a-3];
					bytecode.all[a-3] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareGTBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBLE;
					bytecode.opcodes[o] = O_CompareBLE;
				}

				return;
			}
			else if ( bytecode.opcodes[o] == O_CompareNE ) // assign+pop is very common
			{
				if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromLocal) && (bytecode.opcodes[o-2] == O_LoadFromLocal) )
				{
					bytecode.all[a-4] = O_LLCompareNEBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_LLCompareNEBZ;
					bytecode.all.shave(2);
				}
				else if ( (o>1) && (bytecode.opcodes[o-1] == O_LoadFromGlobal) && (bytecode.opcodes[o-2] == O_LoadFromGlobal) )
				{
					bytecode.all[a-4] = O_GGCompareNEBZ;
					bytecode.all[a-2] = bytecode.all[a-1];
					bytecode.opcodes.clear();
					bytecode.opcodes[o-2] = O_GGCompareNEBZ;
					bytecode.all.shave(2);
				}
				else
				{
					bytecode.all[a] = O_CompareBNE;
					bytecode.opcodes[o] = O_CompareBNE;
				}
				return;
			}
			else if ( bytecode.opcodes[o] == O_LogicalOr ) // assign+pop is very common
			{
				bytecode.all[a] = O_BLO;
				bytecode.opcodes[o] = O_BLO;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LogicalAnd ) // assign+pop is very common
			{
				bytecode.all[a] = O_BLA;
				bytecode.opcodes[o] = O_BLA;
				return;
			}
		}
		else if ( opcode == O_PopOne )
		{
			if ( bytecode.opcodes[o] == O_LoadFromLocal || bytecode.opcodes[o] == O_LoadFromGlobal ) 
			{
				bytecode.all.shave(2);
				bytecode.opcodes.clear();
				return;
			}
			else if ( bytecode.opcodes[o] == O_CallLibFunction && (a > 4) )
			{
				bytecode.all[a-5] = O_CallLibFunctionAndPop;
				bytecode.opcodes[o] = O_CallLibFunctionAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_FUNCTION_CALL_PLACEHOLDER )
			{
				bytecode.all[ a ] = O_PopOne;
				return;
			}
			else if ( bytecode.opcodes[o] == O_PreIncrement || bytecode.opcodes[o] == O_PostIncrement )
			{	
				if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromGlobal )
				{
					bytecode.all[ a - 2 ] = O_IncGlobal;
										
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_IncLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
				}
				else
				{
					bytecode.all[a] = O_PreIncrementAndPop;
					bytecode.opcodes[o] = O_PreIncrementAndPop;
				}

				return;
			}
			else if ( bytecode.opcodes[o] == O_PreDecrement || bytecode.opcodes[o] == O_PostDecrement )
			{	
				if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromGlobal )
				{
					bytecode.all[ a - 2 ] = O_DecGlobal;
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
				}
				else if ( (o > 0) && bytecode.opcodes[o-1] == O_LoadFromLocal )
				{
					bytecode.all[ a - 2 ] = O_DecLocal;
					bytecode.all.shave(1);
					bytecode.opcodes.clear();
				}
				else
				{
					bytecode.all[a] = O_PreDecrementAndPop;
					bytecode.opcodes[o] = O_PreDecrementAndPop;
				}
				
				return;
			}
			else if ( bytecode.opcodes[o] == O_Assign ) // assign+pop is very common
			{
				if ( o > 0 )
				{
					if ( bytecode.opcodes[o-1] == O_LoadFromGlobal )
					{
						if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt8 )
						{
							// a - 4: O_literalInt8
							// a - 3: val
							// a - 2: load from global
							// a - 1: index
							// a -- assign

							bytecode.all[ a - 4 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							bytecode.all[ a - 7 ] = O_LiteralInt32ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							bytecode.all[ a - 7 ] = O_LiteralFloatToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[ a - 3 ] = O_BinaryDivisionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[ a - 3 ] = O_BinaryAdditionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[ a - 3 ] = O_BinaryMultiplicationAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[ a - 3 ] = O_BinarySubtractionAndStoreGlobal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( (o > 1) && bytecode.opcodes[o - 2] == O_FUNCTION_CALL_PLACEHOLDER )
						{
							//bytecode.all[a] = bytecode.all[a-1];
							bytecode.all[a-2] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes[o] = O_AssignToGlobalAndPop;
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToGlobalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}

						return;
					}
					else if ( bytecode.opcodes[ o - 1 ] == O_LoadFromLocal )
					{
						if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt8 )
						{
							bytecode.all[ a - 4 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt16 )
						{
							bytecode.all[ a - 5 ] = O_LiteralInt16ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralInt32 )
						{
							bytecode.all[ a - 7 ] = O_LiteralInt32ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralFloat )
						{
							bytecode.all[ a - 7 ] = O_LiteralFloatToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 3 ];
							bytecode.all[ a - 3 ] = bytecode.all[ a - 4 ];
							bytecode.all[ a - 4 ] = bytecode.all[ a - 5 ];
							bytecode.all[ a - 5 ] = bytecode.all[ a - 6 ];
							bytecode.all[ a - 6 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o-2] == O_LiteralZero )
						{
							bytecode.all[ a - 3 ] = O_LiteralInt8ToLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all[ a - 1 ] = 0;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryDivision )
						{
							bytecode.all[ a - 3 ] = O_BinaryDivisionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryAddition )
						{
							bytecode.all[ a - 3 ] = O_BinaryAdditionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinaryMultiplication )
						{
							bytecode.all[ a - 3 ] = O_BinaryMultiplicationAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else if ( o > 1 && bytecode.opcodes[o - 2] == O_BinarySubtraction )
						{
							bytecode.all[ a - 3 ] = O_BinarySubtractionAndStoreLocal;
							bytecode.all[ a - 2 ] = bytecode.all[ a - 1 ];
							bytecode.all.shave(2);
							bytecode.opcodes.clear();
						}
						else
						{
							bytecode.all[ a - 2 ] = O_AssignToLocalAndPop;
							bytecode.all.shave(1);
							bytecode.opcodes.clear();
						}
						return;
					}
				}
								
				bytecode.all[a] = O_AssignAndPop;
				bytecode.opcodes[o] = O_AssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LiteralZero ) // put a zero on just to pop it off..
			{
				bytecode.opcodes.clear();
				bytecode.all.shave(1);
				return;
			}
			else if ( bytecode.opcodes[o] == O_SubtractAssign )
			{
				bytecode.all[a] = O_SubtractAssignAndPop;
				bytecode.opcodes[o] = O_SubtractAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_AddAssign )
			{
				bytecode.all[a] = O_AddAssignAndPop;
				bytecode.opcodes[o] = O_AddAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ModAssign )
			{
				bytecode.all[a] = O_ModAssignAndPop;
				bytecode.opcodes[o] = O_ModAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_MultiplyAssign )
			{
				bytecode.all[a] = O_MultiplyAssignAndPop;
				bytecode.opcodes[o] = O_MultiplyAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_DivideAssign )
			{
				bytecode.all[a] = O_DivideAssignAndPop;
				bytecode.opcodes[o] = O_DivideAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ORAssign )
			{
				bytecode.all[a] = O_ORAssignAndPop;
				bytecode.opcodes[o] = O_ORAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_ANDAssign )
			{
				bytecode.all[a] = O_ANDAssignAndPop;
				bytecode.opcodes[o] = O_ANDAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_XORAssign )
			{
				bytecode.all[a] = O_XORAssignAndPop;
				bytecode.opcodes[o] = O_XORAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_RightShiftAssign )
			{
				bytecode.all[a] = O_RightShiftAssignAndPop;
				bytecode.opcodes[o] = O_RightShiftAssignAndPop;
				return;
			}
			else if ( bytecode.opcodes[o] == O_LeftShiftAssign )
			{
				bytecode.all[a] = O_LeftShiftAssignAndPop;
				bytecode.opcodes[o] = O_LeftShiftAssignAndPop;
				return;
			}
		}
	}
#endif
	bytecode.all += opcode;
	bytecode.opcodes += opcode;
}

//------------------------------------------------------------------------------
int WRCompilationContext::addRelativeJumpTarget( WRBytecode& bytecode )
{
	bytecode.jumpOffsetTargets.append().references.clear();
	return bytecode.jumpOffsetTargets.count() - 1;
}

//------------------------------------------------------------------------------
void WRCompilationContext::setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget )
{
	bytecode.jumpOffsetTargets[relativeJumpTarget].offset = bytecode.all.size();
}

//------------------------------------------------------------------------------
void WRCompilationContext::addRelativeJumpSourceEx( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget, const unsigned char* data, const int dataSize )
{
	pushOpcode( bytecode, opcode );

	int offset = bytecode.all.size();
	
	if ( dataSize ) // additional data
	{
		pushData( bytecode, data, dataSize );
	}

	bytecode.jumpOffsetTargets[relativeJumpTarget].references.append() = offset;

	pushData( bytecode, "\t\t", 2 ); // 16-bit relative vector
}


//------------------------------------------------------------------------------
// add a jump FROM with whatever opcode is supposed to do it
void WRCompilationContext::addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget )
{
	pushOpcode( bytecode, opcode );

	int offset = bytecode.all.size();
	switch( bytecode.opcodes[bytecode.opcodes.size() - 1] )
	{
		case O_GSCompareEQBZ:
		case O_LSCompareEQBZ:
		case O_GSCompareNEBZ:
		case O_LSCompareNEBZ:
		case O_GSCompareGEBZ:
		case O_LSCompareGEBZ:
		case O_GSCompareLEBZ:
		case O_LSCompareLEBZ:
		case O_GSCompareGTBZ:
		case O_LSCompareGTBZ:
		case O_GSCompareLTBZ:
		case O_LSCompareLTBZ:
		{
			--offset;
			break;
		}

		case O_LLCompareLTBZ:
		case O_LLCompareGTBZ:
		case O_LLCompareLEBZ:
		case O_LLCompareGEBZ:
		case O_LLCompareEQBZ:
		case O_LLCompareNEBZ:
		case O_GGCompareLTBZ:
		case O_GGCompareGTBZ:
		case O_GGCompareLEBZ:
		case O_GGCompareGEBZ:
		case O_GGCompareEQBZ:
		case O_GGCompareNEBZ:
		{
			offset -= 2;
			break;
		}

		default: break;
	}
	
	bytecode.jumpOffsetTargets[relativeJumpTarget].references.append() = offset;
	pushData( bytecode, "\t\t", 2 );
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveRelativeJumps( WRBytecode& bytecode )
{
	for( unsigned int j=0; j<bytecode.jumpOffsetTargets.count(); ++j )
	{
		for( unsigned int t=0; t<bytecode.jumpOffsetTargets[j].references.count(); ++t )
		{
			int16_t diff = bytecode.jumpOffsetTargets[j].offset - bytecode.jumpOffsetTargets[j].references[t];

			int offset = bytecode.jumpOffsetTargets[j].references[t];
			WROpcode o = (WROpcode)bytecode.all[offset - 1];
			bool no8version = false;

			switch( o )
			{
				case O_GSCompareEQBZ8:
				case O_LSCompareEQBZ8:
				case O_GSCompareNEBZ8:
				case O_LSCompareNEBZ8:
				case O_GSCompareGEBZ8:
				case O_LSCompareGEBZ8:
				case O_GSCompareLEBZ8:
				case O_LSCompareLEBZ8:
				case O_GSCompareGTBZ8:
				case O_LSCompareGTBZ8:
				case O_GSCompareLTBZ8:
				case O_LSCompareLTBZ8:
				case O_GSCompareEQBZ:
				case O_LSCompareEQBZ:
				case O_GSCompareNEBZ:
				case O_LSCompareNEBZ:
				case O_GSCompareGEBZ:
				case O_LSCompareGEBZ:
				case O_GSCompareLEBZ:
				case O_LSCompareLEBZ:
				case O_GSCompareGTBZ:
				case O_LSCompareGTBZ:
				case O_GSCompareLTBZ:
				case O_LSCompareLTBZ:
				{
					--diff; // these instructions are offset
					break;
				}

				case O_LLCompareLTBZ8:
				case O_LLCompareGTBZ8:
				case O_LLCompareLEBZ8:
				case O_LLCompareGEBZ8:
				case O_LLCompareEQBZ8:
				case O_LLCompareNEBZ8:
				case O_GGCompareLTBZ8:
				case O_GGCompareGTBZ8:
				case O_GGCompareLEBZ8:
				case O_GGCompareGEBZ8:
				case O_GGCompareEQBZ8:
				case O_GGCompareNEBZ8:
				case O_LLCompareLTBZ:
				case O_LLCompareGTBZ:
				case O_LLCompareLEBZ:
				case O_LLCompareGEBZ:
				case O_LLCompareEQBZ:
				case O_LLCompareNEBZ:
				case O_GGCompareLTBZ:
				case O_GGCompareGTBZ:
				case O_GGCompareLEBZ:
				case O_GGCompareGEBZ:
				case O_GGCompareEQBZ:
				case O_GGCompareNEBZ:
				{
					diff -= 2;
					break;
				}

				case O_GGNextKeyValueOrJump:
				case O_GLNextKeyValueOrJump:
				case O_LGNextKeyValueOrJump:
				case O_LLNextKeyValueOrJump:
				{
					no8version = true;
					diff -= 3;
					break;
				}

				case O_GNextValueOrJump:
				case O_LNextValueOrJump:
				{
					no8version = true;
					diff -= 2;
					break;
				}

				default:
					break;
			}

			
			if ( (diff < 128) && (diff > -129) && !no8version )
			{
				switch( o )
				{
					case O_RelativeJump: *bytecode.all.p_str(offset - 1) = O_RelativeJump8; break;
					case O_BZ: *bytecode.all.p_str(offset - 1) = O_BZ8; break;

					case O_CompareBEQ: *bytecode.all.p_str(offset - 1) = O_CompareBEQ8; break;
					case O_CompareBNE: *bytecode.all.p_str(offset - 1) = O_CompareBNE8; break;
					case O_CompareBGE: *bytecode.all.p_str(offset - 1) = O_CompareBGE8; break;
					case O_CompareBLE: *bytecode.all.p_str(offset - 1) = O_CompareBLE8; break;
					case O_CompareBGT: *bytecode.all.p_str(offset - 1) = O_CompareBGT8; break;
					case O_CompareBLT: *bytecode.all.p_str(offset - 1) = O_CompareBLT8; break;

					case O_GSCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareEQBZ8; ++offset; break;
					case O_LSCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareEQBZ8; ++offset; break;
					case O_GSCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareNEBZ8; ++offset; break;
					case O_LSCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareNEBZ8; ++offset; break;
					case O_GSCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareGEBZ8; ++offset; break;
					case O_LSCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareGEBZ8; ++offset; break;
					case O_GSCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareLEBZ8; ++offset; break;
					case O_LSCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareLEBZ8; ++offset; break;
					case O_GSCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareGTBZ8; ++offset; break;
					case O_LSCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareGTBZ8; ++offset; break;
					case O_GSCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_GSCompareLTBZ8; ++offset; break;
					case O_LSCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_LSCompareLTBZ8; ++offset; break;
										  
					case O_LLCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareLTBZ8; offset += 2; break;
					case O_LLCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareGTBZ8; offset += 2; break;
					case O_LLCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareLEBZ8; offset += 2; break;
					case O_LLCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareGEBZ8; offset += 2; break;
					case O_LLCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareEQBZ8; offset += 2; break;
					case O_LLCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_LLCompareNEBZ8; offset += 2; break;
					case O_GGCompareLTBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareLTBZ8; offset += 2; break;
					case O_GGCompareGTBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareGTBZ8; offset += 2; break;
					case O_GGCompareLEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareLEBZ8; offset += 2; break;
					case O_GGCompareGEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareGEBZ8; offset += 2; break;
					case O_GGCompareEQBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareEQBZ8; offset += 2; break;
					case O_GGCompareNEBZ: *bytecode.all.p_str(offset - 1) = O_GGCompareNEBZ8; offset += 2; break;

					case O_BLA: *bytecode.all.p_str(offset - 1) = O_BLA8; break;
					case O_BLO: *bytecode.all.p_str(offset - 1) = O_BLO8; break;

					// no work to be done
					case O_RelativeJump8:
					case O_BZ8:
					case O_BLA8:
					case O_BLO8:
					case O_CompareBLE8:
					case O_CompareBGE8:
					case O_CompareBGT8:
					case O_CompareBLT8:
					case O_CompareBEQ8:
					case O_CompareBNE8:
						break;
						
					case O_GSCompareEQBZ8:
					case O_LSCompareEQBZ8:
					case O_GSCompareNEBZ8:
					case O_LSCompareNEBZ8:
					case O_GSCompareGEBZ8:
					case O_LSCompareGEBZ8:
					case O_GSCompareLEBZ8:
					case O_LSCompareLEBZ8:
					case O_GSCompareGTBZ8:
					case O_LSCompareGTBZ8:
					case O_GSCompareLTBZ8:
					case O_LSCompareLTBZ8:
						++offset;
						break;

					case O_LLCompareLTBZ8:
					case O_LLCompareGTBZ8:
					case O_LLCompareLEBZ8:
					case O_LLCompareGEBZ8:
					case O_LLCompareEQBZ8:
					case O_LLCompareNEBZ8:
					case O_GGCompareLTBZ8:
					case O_GGCompareGTBZ8:
					case O_GGCompareLEBZ8:
					case O_GGCompareGEBZ8:
					case O_GGCompareEQBZ8:
					case O_GGCompareNEBZ8:
					{
						offset += 2;
						break;
					}

					default:
						m_err = WR_ERR_compiler_panic;
						return;
				}

				*bytecode.all.p_str(offset) = (int8_t)diff;
			}
			else
			{
				switch( o )
				{
					// check to see if any were pushed into 16-bit land
					// that were previously optimized
					case O_RelativeJump8: *bytecode.all.p_str(offset - 1) = O_RelativeJump; break;
					case O_BZ8: *bytecode.all.p_str(offset - 1) = O_BZ; break;
					case O_CompareBEQ8: *bytecode.all.p_str(offset - 1) = O_CompareBEQ; break;
					case O_CompareBNE8: *bytecode.all.p_str(offset - 1) = O_CompareBNE; break;
					case O_CompareBGE8: *bytecode.all.p_str(offset - 1) = O_CompareBGE; break;
					case O_CompareBLE8: *bytecode.all.p_str(offset - 1) = O_CompareBLE; break;
					case O_CompareBGT8: *bytecode.all.p_str(offset - 1) = O_CompareBGT; break;
					case O_CompareBLT8: *bytecode.all.p_str(offset - 1) = O_CompareBLT; break;
					case O_GSCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareEQBZ; ++offset; break;
					case O_LSCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareEQBZ; ++offset; break;
					case O_GSCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareNEBZ; ++offset; break;
					case O_LSCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareNEBZ; ++offset; break;
					case O_GSCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareGEBZ; ++offset; break;
					case O_LSCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareGEBZ; ++offset; break;
					case O_GSCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareLEBZ; ++offset; break;
					case O_LSCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareLEBZ; ++offset; break;
					case O_GSCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareGTBZ; ++offset; break;
					case O_LSCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareGTBZ; ++offset; break;
					case O_GSCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_GSCompareLTBZ; ++offset; break;
					case O_LSCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_LSCompareLTBZ; ++offset; break;

					case O_LLCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareLTBZ; offset += 2; break;
					case O_LLCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareGTBZ; offset += 2; break;
					case O_LLCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareLEBZ; offset += 2; break;
					case O_LLCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareGEBZ; offset += 2; break;
					case O_LLCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareEQBZ; offset += 2; break;
					case O_LLCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_LLCompareNEBZ; offset += 2; break;
					case O_GGCompareLTBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareLTBZ; offset += 2; break;
					case O_GGCompareGTBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareGTBZ; offset += 2; break;
					case O_GGCompareLEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareLEBZ; offset += 2; break;
					case O_GGCompareGEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareGEBZ; offset += 2; break;
					case O_GGCompareEQBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareEQBZ; offset += 2; break;
					case O_GGCompareNEBZ8: *bytecode.all.p_str(offset - 1) = O_GGCompareNEBZ; offset += 2; break;

					case O_GGNextKeyValueOrJump:
					case O_GLNextKeyValueOrJump:
					case O_LGNextKeyValueOrJump:
					case O_LLNextKeyValueOrJump:
					{
						offset += 3;
						break;
					}

					case O_GNextValueOrJump:
					case O_LNextValueOrJump:
					{
						offset += 2;
						break;
					}

					case O_BLA8: *bytecode.all.p_str(offset - 1) = O_BLA; break;
					case O_BLO8: *bytecode.all.p_str(offset - 1) = O_BLO; break;

					 // no work to be done, already visited
					case O_RelativeJump:
					case O_BZ:
					case O_BLA:
					case O_BLO:
					case O_CompareBLE:
					case O_CompareBGE:
					case O_CompareBGT:
					case O_CompareBLT:
					case O_CompareBEQ:
					case O_CompareBNE:
						break;

					case O_GSCompareEQBZ:
					case O_LSCompareEQBZ:
					case O_GSCompareNEBZ:
					case O_LSCompareNEBZ:
					case O_GSCompareGEBZ:
					case O_LSCompareGEBZ:
					case O_GSCompareLEBZ:
					case O_LSCompareLEBZ:
					case O_GSCompareGTBZ:
					case O_LSCompareGTBZ:
					case O_GSCompareLTBZ:
					case O_LSCompareLTBZ:
						++offset;
						break;
					
					case O_LLCompareLTBZ:
					case O_LLCompareGTBZ:
					case O_LLCompareLEBZ:
					case O_LLCompareGEBZ:
					case O_LLCompareEQBZ:
					case O_LLCompareNEBZ:
					case O_GGCompareLTBZ:
					case O_GGCompareGTBZ:
					case O_GGCompareLEBZ:
					case O_GGCompareGEBZ:
					case O_GGCompareEQBZ:
					case O_GGCompareNEBZ:
					{
						offset += 2;
						break;
					}
						
					default:
					{
						m_err = WR_ERR_compiler_panic;
						return;
					}
				}
				
				pack16( diff, bytecode.all.p_str(offset) );
			}
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::appendBytecode( WRBytecode& bytecode, WRBytecode& addMe )
{
	for (unsigned int n = 0; n < addMe.jumpOffsetTargets.count(); ++n)
	{
		if (addMe.jumpOffsetTargets[n].gotoHash)
		{
			int index = addRelativeJumpTarget(bytecode);
			bytecode.jumpOffsetTargets[index].gotoHash = addMe.jumpOffsetTargets[n].gotoHash;
			bytecode.jumpOffsetTargets[index].offset = addMe.jumpOffsetTargets[n].offset + bytecode.all.size();
		}
	}

	// add the namespace, making sure to offset it into the new block properly
	for (unsigned int n = 0; n < addMe.localSpace.count(); ++n)
	{
		unsigned int m = 0;
		for (m = 0; m < bytecode.localSpace.count(); ++m)
		{
			if (bytecode.localSpace[m].hash == addMe.localSpace[n].hash)
			{
				for (unsigned int s = 0; s < addMe.localSpace[n].references.count(); ++s)
				{
					bytecode.localSpace[m].references.append() = addMe.localSpace[n].references[s] + bytecode.all.size();
				}

				break;
			}
		}

		if (m >= bytecode.localSpace.count())
		{
			WRNamespaceLookup* space = &bytecode.localSpace.append();
			*space = addMe.localSpace[n];
			for (unsigned int s = 0; s < space->references.count(); ++s)
			{
				space->references[s] += bytecode.all.size();
			}
		}
	}

	// add the function space, making sure to offset it into the new block properly
	for (unsigned int n = 0; n < addMe.functionSpace.count(); ++n)
	{
		WRNamespaceLookup* space = &bytecode.functionSpace.append();
		*space = addMe.functionSpace[n];
		for (unsigned int s = 0; s < space->references.count(); ++s)
		{
			space->references[s] += bytecode.all.size();
		}
	}

	// add the function space, making sure to offset it into the new block properly
	for (unsigned int u = 0; u < addMe.unitObjectSpace.count(); ++u)
	{
		WRNamespaceLookup* space = &bytecode.unitObjectSpace.append();
		*space = addMe.unitObjectSpace[u];
		for (unsigned int s = 0; s < space->references.count(); ++s)
		{
			space->references[s] += bytecode.all.size();
		}
	}

	// add the goto targets, making sure to offset it into the new block properly
	for (unsigned int u = 0; u < addMe.gotoSource.count(); ++u)
	{
		GotoSource* G = &bytecode.gotoSource.append();
		G->hash = addMe.gotoSource[u].hash;
		G->offset = addMe.gotoSource[u].offset + bytecode.all.size();
	}

	if ( bytecode.all.size() > 1
		 && bytecode.opcodes.size() > 0
		 && addMe.opcodes.size() == 1
		 && addMe.all.size() > 2
		 && addMe.gotoSource.count() == 0
		 && addMe.opcodes[0] == O_IndexLiteral16
		 && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexLocalLiteral16;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.clear();
		bytecode.opcodes += O_IndexLocalLiteral16;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 2
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral16
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexGlobalLiteral16;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.clear();
		bytecode.opcodes += O_IndexGlobalLiteral16;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 1
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral8
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexLocalLiteral8;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.clear();
		bytecode.opcodes += O_IndexLocalLiteral8;
		return;
	}
	else if ( bytecode.all.size() > 1
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 1
			  && addMe.all.size() > 1
			  && addMe.gotoSource.count() == 0
			  && addMe.opcodes[0] == O_IndexLiteral8
			  && bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal )
	{
		bytecode.all[bytecode.all.size()-2] = O_IndexGlobalLiteral8;
		for( unsigned int i=1; i<addMe.all.size(); ++i )
		{
			bytecode.all += addMe.all[i];	
		}
		bytecode.opcodes.clear();
		bytecode.opcodes += O_IndexGlobalLiteral8;
		return;
	}
	else if ( bytecode.all.size() > 0
			  && bytecode.opcodes.size() > 0
			  && addMe.opcodes.size() == 2
			  && addMe.gotoSource.count() == 0
			  && addMe.all.size() == 3
			  && addMe.opcodes[1] == O_Index )
	{
		if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal
			 && addMe.opcodes[0] == O_LoadFromLocal )
		{
			int a = bytecode.all.size() - 1;
			
			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_LLValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;
			
			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal
			 && addMe.opcodes[0] == O_LoadFromGlobal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_GGValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromLocal
				  && addMe.opcodes[0] == O_LoadFromGlobal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_GLValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
		else if ( bytecode.opcodes[bytecode.opcodes.size() - 1] == O_LoadFromGlobal
				  && addMe.opcodes[0] == O_LoadFromLocal )
		{
			int a = bytecode.all.size() - 1;

			addMe.all[0] = bytecode.all[a];
			bytecode.all[a] = addMe.all[1];
			bytecode.all[a-1] = O_LGValues;
			bytecode.all += addMe.all[0];
			bytecode.all += O_IndexSkipLoad;

			bytecode.opcodes += O_LoadFromLocal;
			bytecode.opcodes += O_IndexSkipLoad;
			return;
		}
	}
	else if ( addMe.all.size() == 1 )
	{
		if ( addMe.opcodes[0] == O_HASH_PLACEHOLDER )
		{
			if ( bytecode.opcodes.size() > 1
				 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
				 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromLocal )
			{
				int o = bytecode.all.size() - 7;

				bytecode.all[o] = O_LocalIndexHash;
				bytecode.all[o + 5] = bytecode.all[o + 4];
				bytecode.all[o + 4] = bytecode.all[o + 3];
				bytecode.all[o + 3] = bytecode.all[o + 2];
				bytecode.all[o + 2] = bytecode.all[o + 1];
				bytecode.all[o + 1] = bytecode.all[o + 6];

				bytecode.opcodes.clear();
				bytecode.all.shave(1);
			}
			else if (bytecode.opcodes.size() > 1
					 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
					 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_LoadFromGlobal )
			{
				int o = bytecode.all.size() - 7;

				bytecode.all[o] = O_GlobalIndexHash;
				bytecode.all[o + 5] = bytecode.all[o + 4];
				bytecode.all[o + 4] = bytecode.all[o + 3];
				bytecode.all[o + 3] = bytecode.all[o + 2];
				bytecode.all[o + 2] = bytecode.all[o + 1];
				bytecode.all[o + 1] = bytecode.all[o + 6];

				bytecode.opcodes.clear();
				bytecode.all.shave(1);
			}
			else if (bytecode.opcodes.size() > 1
					 && bytecode.opcodes[ bytecode.opcodes.size() - 2 ] == O_LiteralInt32
					 && bytecode.opcodes[ bytecode.opcodes.size() - 1 ] == O_StackSwap )
			{
				int a = bytecode.all.size() - 7;

				bytecode.all[a] = O_StackIndexHash;

				bytecode.opcodes.clear();
				bytecode.all.shave(2);

			}
			else
			{
				m_err = WR_ERR_compiler_panic;
			}

			return;
		}

		pushOpcode( bytecode, (WROpcode)addMe.opcodes[0] );
		return;
	}

	resolveRelativeJumps( addMe );

	bytecode.all += addMe.all;
	bytecode.opcodes += addMe.opcodes;
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLiteral( WRBytecode& bytecode, WRExpressionContext& context )
{
	WRValue& value = context.value;
	if ( value.type == WR_INT && value.i == 0 )
	{
		pushOpcode( bytecode, O_LiteralZero );
	}
	else if ( value.type == WR_INT )
	{
		if ( (value.i <= 127) && (value.i >= -128) )
		{
			pushOpcode( bytecode, O_LiteralInt8 );
			unsigned char be = (char)value.i;
			pushData( bytecode, &be, 1 );
		}
		else if ( (value.i <= 32767) && (value.i >= -32768) )
		{
			pushOpcode( bytecode, O_LiteralInt16 );
			unsigned char be = (char)(value.i>>8);
			pushData( bytecode, &be, 1 );
			be = (char)value.i;
			pushData( bytecode, &be, 1 );
		}
		else
		{
			pushOpcode( bytecode, O_LiteralInt32 );
			unsigned char data[4];
			pushData( bytecode, pack32(value.i, data), 4 );
		}
	}
	else if ( value.type == WR_COMPILER_LITERAL_STRING )
	{
		pushOpcode( bytecode, O_LiteralString );
		unsigned char data[2];
		int16_t be = context.literalString.size();
		pushData( bytecode, pack16(be, data), 2 );
		for( unsigned int i=0; i<context.literalString.size(); ++i )
		{
			pushData( bytecode, context.literalString.c_str(i), 1 );
		}
	}
	else
	{
		pushOpcode( bytecode, O_LiteralFloat );
		unsigned char data[4];
		int32_t be = value.i;
		pushData( bytecode, pack32(be, data), 4 );
	}
}

//------------------------------------------------------------------------------
int WRCompilationContext::addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly )
{
	if ( m_unitTop == 0 )
	{
		return addGlobalSpaceLoad( bytecode, token, addOnly );
	}

	uint32_t hash = wr_hashStr(token);
	
	unsigned int i=0;
		
	for( ; i<bytecode.localSpace.count(); ++i )
	{
		if ( bytecode.localSpace[i].hash == hash )
		{
			break;
		}
	}

	if ( i >= bytecode.localSpace.count() && !addOnly )
	{
		WRstr t2;
		if (token[0] == ':' && token[1] == ':')
		{
			t2 = token;
		}
		else
		{
			t2.format("::%s", token.c_str());
		}

		uint32_t ghash = wr_hashStr(t2);

		// was NOT found locally which is possible for a "global" if
		// the argument list names it, now check global with the global
		// hash
		if ( !bytecode.isStructSpace ) // structs are immune to this
		{
			for (unsigned int j = 0; j < m_units[0].bytecode.localSpace.count(); ++j)
			{
				if (m_units[0].bytecode.localSpace[j].hash == ghash)
				{
					pushOpcode(bytecode, O_LoadFromGlobal);
					unsigned char c = j;
					pushData(bytecode, &c, 1);
					return j;
				}
			}
		}
	}

	bytecode.localSpace[i].hash = hash;

	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromLocal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}
	
	return i;
}

//------------------------------------------------------------------------------
int WRCompilationContext::addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly )
{
	uint32_t hash;
	WRstr t2;
	if ( token[0] == ':' && token[1] == ':' )
	{
		t2 = token;
	}
	else
	{
		t2.format( "::%s", token.c_str() );
	}
	hash = wr_hash( t2, t2.size() );

	unsigned int i=0;

	for( ; i<m_units[0].bytecode.localSpace.count(); ++i )
	{
		if ( m_units[0].bytecode.localSpace[i].hash == hash )
		{
			break;
		}
	}
	
	m_units[0].bytecode.localSpace[i].hash = hash;

	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromGlobal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}

	return i;
}

//------------------------------------------------------------------------------
void WRCompilationContext::loadExpressionContext( WRExpression& expression, int depth, int operation )
{
	if ( operation > 0
		 && expression.context[operation].operation
		 && expression.context[operation].operation->opcode == O_HASH_PLACEHOLDER
		 && depth > operation )
	{
		unsigned char buf[4];
		pack32( wr_hash( expression.context[depth].token,
						 expression.context[depth].token.size()),
				buf );
		pushOpcode( expression.bytecode, O_LiteralInt32 );
		pushData( expression.bytecode, buf, 4 );
	}
	else
	{
		switch( expression.context[depth].type )
		{
			case EXTYPE_LITERAL:
			{
				pushLiteral( expression.bytecode, expression.context[depth] );
				break;
			}

			case EXTYPE_LABEL:
			{
				if ( expression.context[depth].global )
				{
					addGlobalSpaceLoad( expression.bytecode,
										expression.context[depth].token );
				}
				else
				{
					addLocalSpaceLoad( expression.bytecode,
									   expression.context[depth].token );
				}
				break;
			}

			case EXTYPE_BYTECODE_RESULT:
			{
				appendBytecode( expression.bytecode,
								expression.context[depth].bytecode );
				break;
			}

			case EXTYPE_RESOLVED:
			case EXTYPE_OPERATION:
			default:
			{
				break;
			}
		}
	}

	expression.pushToStack( depth ); // make sure stack knows something got loaded on top of it
	
	expression.context[depth].type = EXTYPE_RESOLVED; // this slot is now a resolved value sitting on the stack
}

//------------------------------------------------------------------------------
void WRExpression::swapWithTop( int stackPosition, bool addOpcodes )
{
	if ( stackPosition == 0 )
	{
		return;
	}
	
	unsigned int currentTop = -1;
	unsigned int swapWith = -1;
	for( unsigned int i=0; i<context.count(); ++i )
	{
		if ( context[i].stackPosition == stackPosition )
		{
			swapWith = i;
		}
		
		if ( context[i].stackPosition == 0 )
		{
			currentTop = i;
		}
	}

	assert( (currentTop != (unsigned int)-1) && (swapWith != (unsigned int)-1) );

	if ( addOpcodes )
	{
		unsigned char pos = stackPosition + 1;
		WRCompilationContext::pushOpcode( bytecode, O_StackSwap );
		WRCompilationContext::pushData( bytecode, &pos, 1 );
	}
	context[currentTop].stackPosition = stackPosition;
	context[swapWith].stackPosition = 0;
}

//------------------------------------------------------------------------------
void WRCompilationContext::resolveExpression( WRExpression& expression )
{
	if ( expression.context.count() == 1 ) // single expression is trivial to resolve, load it!
	{
		loadExpressionContext( expression, 0, 0 );
		return;
	}

	unsigned int resolves = 1;
	unsigned int startedWith = expression.context.count();

	for( int p=0; p<c_highestPrecedence && expression.context.count() > 1; ++p )
	{
		// left to right operations
		for( int o=0; (unsigned int)o < expression.context.count(); ++o )
		{
			if ( (expression.context[o].type != EXTYPE_OPERATION)
				 || expression.context[o].operation->precedence > p
				 || !expression.context[o].operation->leftToRight )
			{
				continue;
			}

			resolves += resolveExpressionEx( expression, o, p );
			if (m_err)
			{
				return;
			}
			o = (unsigned int)-1;
		}

		// right to left operations
		for( int o = expression.context.count() - 1; o >= 0; --o )
		{
			if ( (expression.context[o].type != EXTYPE_OPERATION)
				 || expression.context[o].operation->precedence > p
				 || expression.context[o].operation->leftToRight )
			{
				continue;
			}

			resolves += resolveExpressionEx( expression, o, p );
			if (m_err)
			{
				return;
			}
			o = expression.context.count();
		}
	}

	if ( startedWith && (resolves != startedWith) )
	{
		m_err = WR_ERR_bad_expression;
		return;
	}
}

//------------------------------------------------------------------------------
unsigned int WRCompilationContext::resolveExpressionEx( WRExpression& expression, int o, int p )
{
	unsigned int ret = 0;
	switch( expression.context[o].operation->type )
	{
		case WR_OPER_PRE:
		{
			ret = 1;

			if ( expression.context[o + 1].stackPosition == -1 )
			{
				loadExpressionContext( expression, o + 1, 0 ); // load argument
			}
			else if ( expression.context[o + 1].stackPosition != 0 )
			{
				expression.swapWithTop( expression.context[o + 1].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode );  // apply operator
			expression.context.remove( o, 1 ); // knock off operator
			expression.pushToStack(o);
			break;
		}

		case WR_OPER_BINARY_COMMUTE:
		{
			// for operations like + and * the operands can be in any
			// order, so don't go to any great lengths to shift the stack
			// around for them

			ret = 2;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			int second = o + 1; // push first
			int first = o - 1;  // push second
			// so
			// 1 - first
			// [oper]
			// 0 - second

			if ( expression.context[second].stackPosition == -1 )
			{
				if ( expression.context[first].stackPosition == -1 )
				{
					loadExpressionContext( expression, first, o ); 
				}
				else if ( expression.context[first].stackPosition != 0 )
				{
					// otherwise swap first to the top and load the second
					expression.swapWithTop( expression.context[first].stackPosition );
				}

				loadExpressionContext( expression, second, o );
			}
			else if ( expression.context[first].stackPosition == -1 )
			{
				if ( expression.context[second].stackPosition != 0 )
				{
					expression.swapWithTop( expression.context[second].stackPosition );
				}

				// just load the second to top
				loadExpressionContext( expression, first, o );
			}
			else if ( expression.context[second].stackPosition == 1 )
			{
				expression.swapWithTop( expression.context[first].stackPosition );
			}
			else if ( expression.context[first].stackPosition == 1 )
			{
				expression.swapWithTop( expression.context[second].stackPosition );
			}
			else
			{
				// first and second are both loaded but neither
				// is in the correct position

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = expression.context[second].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( expression.context[second].stackPosition, false );
				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}

		case WR_OPER_BINARY:
		{
			ret = 2;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			int second = o + 1; // push first
			int first = o - 1;  // push second
			bool useAlt = false;
			// so
			// 1 - first
			// [oper]
			// 0 - second

			if ( expression.context[second].stackPosition == -1 )
			{
				if ( expression.context[first].stackPosition == -1 )
				{
					loadExpressionContext( expression, second, o ); // nope, grab 'em
					loadExpressionContext( expression, first, o ); 
				}
				else
				{
					// first is in the stack somewhere, we need
					// to swap it to the top, then load, then
					// swap top values
					expression.swapWithTop( expression.context[first].stackPosition );

					loadExpressionContext( expression, second, o );

					if ( expression.context[o].operation->alt == O_LAST )
					{
						expression.swapWithTop( 1 );
					}
					else // otherwise top shuffle is NOT required because we have an equal-but-opposite operation
					{
						useAlt = true;
					}
				}
			}
			else if ( expression.context[first].stackPosition == -1 )
			{
				// perfect, swap up the second, then load the
				// first

				expression.swapWithTop( expression.context[second].stackPosition );

				loadExpressionContext( expression, first, o );
			}
			else if ( expression.context[second].stackPosition == 1 )
			{
				// second is already where its supposed to be,
				// swap first to top if it's not there already
				if ( expression.context[first].stackPosition != 0 )
				{
					expression.swapWithTop( expression.context[first].stackPosition );
				}
			}
			else if ( expression.context[second].stackPosition == 0 )
			{
				// second is on top of the stack, swap with
				// next level down then swap first up

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = 2;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}
			else
			{
				// first and second are both loaded but neither is in the correct position

				WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
				unsigned char pos = expression.context[first].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
				pos = expression.context[second].stackPosition + 1;
				WRCompilationContext::pushData( expression.bytecode, &pos, 1 );

				expression.swapWithTop( expression.context[second].stackPosition, false );
				expression.swapWithTop( 1, false );
				expression.swapWithTop( expression.context[first].stackPosition, false );
			}

			if ( useAlt )
			{
				pushOpcode( expression.bytecode, expression.context[o].operation->alt );
			}
			else
			{
				appendBytecode( expression.bytecode, expression.context[o].bytecode ); // apply operator
			}

			expression.context.remove( o - 1, 2 ); // knock off operator and arg
			expression.pushToStack(o - 1);

			break;
		}

		case WR_OPER_POST:
		{
			ret = 1;

			if( o == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( expression.context[o - 1].stackPosition == -1 )
			{
				loadExpressionContext( expression, o - 1, o ); // load argument
			}
			else if ( expression.context[o - 1].stackPosition != 0 )
			{
				expression.swapWithTop( expression.context[o - 1].stackPosition );
			}

			appendBytecode( expression.bytecode, expression.context[o].bytecode );
			expression.context.remove( o, 1 ); // knock off operator
			expression.pushToStack( o - 1 );

			break;
		}
	}
	
	return ret;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::operatorFound( WRstr& token, WRarray<WRExpressionContext>& context, int depth )
{
	for( int i=0; c_operations[i].token; i++ )
	{
		if ( token == c_operations[i].token )
		{
			if ( c_operations[i].type == WR_OPER_PRE
				 && depth > 0
				 && (context[depth-1].type != EXTYPE_OPERATION 
					 || (context[depth - 1].type == EXTYPE_OPERATION &&
						 (context[depth - 1].operation->type != WR_OPER_BINARY && context[depth - 1].operation->type != WR_OPER_BINARY_COMMUTE))) )
			{
				continue;
			}
			
			context[depth].operation = c_operations + i;
			context[depth].type = EXTYPE_OPERATION;

			pushOpcode( context[depth].bytecode, c_operations[i].opcode );

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseCallFunction( WRExpression& expression, WRstr functionName, int depth, bool parseArguments )
{
	WRstr prefix = expression.context[depth].prefix;

	expression.context[depth].type = EXTYPE_BYTECODE_RESULT;

	unsigned char argsPushed = 0;

	if ( parseArguments )
	{
		WRstr& token2 = expression.context[depth].token;
		WRValue& value2 = expression.context[depth].value;

		for(;;)
		{
			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}

			if ( token2 == ")" )
			{
				break;
			}

			++argsPushed;

			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;
			m_loadedToken = token2;
			m_loadedValue = value2;

			char end = parseExpression( nex );

			appendBytecode( expression.context[depth].bytecode, nex.bytecode );

			if ( end == ')' )
			{
				break;
			}
			else if ( end != ',' )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
		}
	}

	if ( prefix.size() )
	{
		prefix += "::";
		prefix += functionName;

		unsigned char buf[4];
		pack32( wr_hashStr(prefix), buf );

		pushOpcode( expression.context[depth].bytecode, O_CallLibFunction );
		pushData( expression.context[depth].bytecode, &argsPushed, 1 ); // arg #
		pushData( expression.context[depth].bytecode, buf, 4 ); // hash id

		// normal lib will copy down the result, otherwise
		// ignore it, it will be AFTER args
	}
	else
	{
		// push the number of args
		uint32_t hash = wr_hashStr( functionName );

		unsigned int i=0;
		for( ; i<expression.context[depth].bytecode.functionSpace.count(); ++i )
		{
			if ( expression.context[depth].bytecode.functionSpace[i].hash == hash )
			{
				break;
			}
		}

		expression.context[depth].bytecode.functionSpace[i].references.append() = getBytecodePosition( expression.context[depth].bytecode );
		expression.context[depth].bytecode.functionSpace[i].hash = hash;

		pushOpcode( expression.context[depth].bytecode, O_FUNCTION_CALL_PLACEHOLDER );

		pushData( expression.context[depth].bytecode, &argsPushed, 1 );
		pushData( expression.context[depth].bytecode, "0123", 4 ); // TBD opcode plus index, OR hash if index was not found

		// hash will copydown result same as lib, unless
		// copy/pop which case does nothing

		// normal call does nothing special, to preserve
		// the return value a call to copy-down must be
		// inserted
	}

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::pushObjectTable( WRExpressionContext& context,
											WRarray<WRNamespaceLookup>& localSpace,
											uint32_t hash )
{
	WRstr& token2 = context.token;
	WRValue& value2 = context.value;

	unsigned int i=0;
	for( ; i<context.bytecode.unitObjectSpace.count(); ++i )
	{
		if ( context.bytecode.unitObjectSpace[i].hash == hash )
		{
			break;
		}
	}

	pushOpcode( context.bytecode, O_NewObjectTable );

	context.bytecode.unitObjectSpace[i].references.append() = getBytecodePosition( context.bytecode );
	context.bytecode.unitObjectSpace[i].hash = hash;

	pushData( context.bytecode, "\0\0", 2 );

	context.type = EXTYPE_BYTECODE_RESULT;

	if ( token2 == "{" )
	{
		unsigned char offset = 0;
		for(;;)
		{
			WRExpression nex( localSpace, context.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;
			char end = parseExpression( nex );

			if ( nex.bytecode.all.size() )
			{
				appendBytecode( context.bytecode, nex.bytecode );
				pushOpcode( context.bytecode, O_AssignToObjectTableByOffset );
				pushData( context.bytecode, &offset, 1 );
			}

			++offset;

			if ( end == '}' )
			{
				break;
			}
			else if ( end != ',' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------------
char WRCompilationContext::parseExpression( WRExpression& expression )
{
	int depth = 0;
	char end = 0;

	for(;;)
	{
		WRValue& value = expression.context[depth].value;
		WRstr& token = expression.context[depth].token;

		expression.context[depth].bytecode.clear();
		expression.context[depth].setLocalSpace( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
		if ( !getToken(expression.context[depth]) )
		{
			return 0;
		}

		if ( value.type != WR_REF )
		{
			if ( (depth > 0) 
				 && ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					 || (expression.context[depth - 1].type == EXTYPE_LITERAL)) )
			{
				// two labels/literals cannot follow each other
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			// it's a literal
			expression.context[depth].type = EXTYPE_LITERAL;
			if ( value.type == WR_COMPILER_LITERAL_STRING )
			{
				expression.context[depth].literalString = *(WRstr*)value.p;
			}
			
			++depth;
			continue;
		}

		if ( token == ";" || token == ")" || token == "}" || token == "," || token == "]" || token == ":" )
		{
			end = token[0];
			break;
		}

		if ( token == "{" )
		{
			if ( depth == 0 )
			{
				if ( parseExpression(expression) != '}' )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				return ';';
			}
			else
			{
				// it's an initializer

				if ( depth < 2 )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}
				if ( (expression.context[depth - 1].operation
					  && expression.context[ depth - 1 ].operation->opcode != O_Assign) )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				if ( (expression.context[depth - 2].operation
					  && expression.context[ depth - 2 ].operation->opcode != O_Index) )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				expression.context.remove( depth - 1, 1 ); // knock off the equate
				depth--;

				WRstr t("@init");
				for( int o=0; c_operations[o].token; o++ )
				{
					if ( t == c_operations[o].token )
					{
						expression.context[depth].operation = c_operations + o;
						expression.context[depth].type = EXTYPE_OPERATION;
						break;
					}
				}


				WRstr& token2 = expression.context[depth].token;
				WRValue& value2 = expression.context[depth].value;

				uint16_t initializer = 0;


				// TODO - more effiecient way to do this, it allocates
				// twice for the case of
				// a[500] = { 1 };
				
/*
				// make sure the array we are about to initialize is
				// actually empty (set it to a nonsense int value)
				unsigned char offset[3];
				offset[2] = 0xEE;

				int target = depth - 1;
				if ( (depth > 1)
					 && expression.context[target].operation
					 && !strncmp( expression.context[target].operation->token, "@[]", 3) )
				{
					--target;
				}
								
				if ( expression.context[target].global || (m_unitTop == 0) )
				{
					offset[0] = O_LiteralInt8ToGlobal;
					offset[1] = addGlobalSpaceLoad( expression.bytecode,
													expression.context[target].token,
													true );
				}
				else
				{
					offset[0] = O_LiteralInt8ToLocal;
					offset[1] = addLocalSpaceLoad( expression.bytecode,
												   expression.context[target].token,
												   true );
				}

				pushData( expression.context[depth].bytecode, offset, 3 );
*/






				for(;;)
				{
					if ( !getToken(expression.context[depth]) )
					{
						m_err = WR_ERR_unexpected_EOF;
						return 0;
					}

					if ( token2 == "}" )
					{
						break;
					}

					WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
					nex.context[0].token = token2;
					nex.context[0].value = value2;
					m_loadedToken = token2;
					m_loadedValue = value2;

					char end = parseExpression( nex );

					unsigned char data[2];
					pack16( initializer, data );

					if ( end == '}' )
					{
						appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
						pushData( expression.context[depth].bytecode, data, 2 );
						break;
					}
					else if ( end == ',' )
					{
						appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
						pushData( expression.context[depth].bytecode, data, 2 );
					}
					else if ( end == ':' )
					{
						if ( nex.bytecode.all.size() == 0 )
						{
							pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						}
						else
						{
							appendBytecode( expression.context[depth].bytecode, nex.bytecode );
						}

						WRExpression nex2( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
						nex2.context[0].token = token2;
						nex2.context[0].value = value2;
						char end = parseExpression( nex2 );

						if ( nex2.bytecode.all.size() == 0 )
						{
							pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						}
						else
						{
							appendBytecode( expression.context[depth].bytecode, nex2.bytecode );
						}

						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );

						if ( end == '}' )
						{
							break;
						}
						else if ( end != ',' )
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}
					}
					else
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					++initializer;
				}

				if ( !getToken(expression.context[depth]) || token2 != ";" )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}

				m_loadedToken = token2;
				m_loadedValue = value2;

				++depth;

				continue;
			}
		}
		
		if ( operatorFound(token, expression.context, depth) )
		{
			++depth;
			continue;
		}
		
		if ( token == "new" )
		{
			if ( (depth < 2) || 
				 (expression.context[depth - 1].operation
				  && expression.context[ depth - 1 ].operation->opcode != O_Assign) )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}

			if ( (depth < 2) 
				 || (expression.context[depth - 2].operation
					 && expression.context[ depth - 2 ].operation->opcode != O_Index) )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
			
			WRstr& token2 = expression.context[depth].token;
			WRValue& value2 = expression.context[depth].value;

			bool isGlobal;
			WRstr prefix;

			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !isValidLabel(token2, isGlobal, prefix) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			WRstr functionName = token2;
			uint32_t hash = wr_hashStr( functionName );
			
			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( token2 == ";" )
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}

				m_loadedToken = ";";
				m_loadedValue.p2 = INIT_AS_REF;
			}
			else if ( token2 == "(" )
			{
				if ( !parseCallFunction(expression, functionName, depth, true) )
				{
					return 0;
				}

				if ( !getToken(expression.context[depth]) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}

				if ( token2 != "{" )
				{
					m_loadedToken = token2;
					m_loadedValue = value2;
				}
			}
			else if (token2 == "{")
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}
				token2 = "{";
			}
			else if ( token2 == "[" )
			{
				// must be "array" directive
				if ( !getToken(expression.context[depth], "]") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				expression.context.remove( depth - 1, 1 ); // knock off the equate
				depth--;

				WRstr& token3 = expression.context[depth].token;
				WRValue& value3 = expression.context[depth].value;

				// we know we're about to create at least X new
				// commands in a row, unless the function calls
				// allocate memory (a distinct possibility) no GC will
				// be required, so pause it by the number of
				// allocations (TBD) 
				pushOpcode( expression.context[depth].bytecode, O_GC_Command );
				unsigned int gcOffset = expression.context[depth].bytecode.all.size();
				pushData( expression.context[depth].bytecode, "\0\0", 2 );

				if ( !getToken(expression.context[depth], "{") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				uint16_t initializer = 0;

				if ( token3 == "{" )
				{
					for(;;)
					{
						if ( !parseCallFunction(expression, functionName, depth, false) )
						{
							return 0;
						}

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( token3 == "}" )
						{
							break;
						}
						else if (token3 != "{")
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}

						if ( !pushObjectTable(expression.context[depth], expression.bytecode.localSpace, hash) )
						{
							m_err = WR_ERR_bad_expression;
							return 0;
						}

						pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );

						unsigned char data[2];
						pack16( initializer, data );
						++initializer;

						pushData( expression.context[depth].bytecode, data, 2 );

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( token3 == "," )
						{
							continue;
						}
						else if ( token3 != "}" )
						{
							m_err = WR_ERR_unexpected_token;
							return 0;
						}
						else
						{
							break;
						}
					}
					
					if ( !getToken(expression.context[depth], ";") )
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					WRstr t("@init");
					for( int o=0; c_operations[o].token; o++ )
					{
						if ( t == c_operations[o].token )
						{
							expression.context[depth].type = EXTYPE_OPERATION;
							expression.context[depth].operation = c_operations + o;
							break;
						}
					}

					m_loadedToken = token3;
					m_loadedValue = value3;

				}
				else if ( token2 != ";" )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				pack16( initializer, expression.context[depth].bytecode.all.p_str(gcOffset) );

				++depth;
				continue;
			}
			else if ( token2 != "{" )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !pushObjectTable(expression.context[depth], expression.bytecode.localSpace, hash) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			++depth;
			continue;
		}

		if ( token == "(" )
		{
			// might be cast, call or sub-expression
			
			if ( (depth > 0) && expression.context[depth - 1].type == EXTYPE_LABEL )
			{
				// always only a call

				--depth;
				if ( !parseCallFunction(expression, expression.context[depth].token, depth, true) )
				{
					return 0;
				}
			}
			else if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}
			else if ( token == "int" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				WRstr t( "@i" );
				operatorFound( t, expression.context, depth );
			}
			else if ( token == "float" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				WRstr t( "@f" );
				operatorFound( t, expression.context, depth );
			}
			else if (depth < 0)
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}
			else
			{
				WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
				nex.context[0].token = token;
				nex.context[0].value = value;
				m_loadedToken = token;
				m_loadedValue = value;
				if ( parseExpression(nex) != ')' )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}


				if ( depth > 0 && expression.context[depth - 1].operation
					 && expression.context[depth - 1].operation->opcode == O_RemoveFromHashTable )
				{
					--depth;
					WRstr t( "._remove" );
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( t, expression.context, depth );
				}
				else if ( depth > 0 && expression.context[depth - 1].operation
						  && expression.context[depth - 1].operation->opcode == O_HashEntryExists )
				{
					--depth;
					WRstr t( "._exists" );
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( t, expression.context, depth );
				}
				else
				{
					expression.context[depth].type = EXTYPE_BYTECODE_RESULT;
					expression.context[depth].bytecode = nex.bytecode;
				}
			}

			++depth;
			continue;
		}

		if ( token == "[" )
		{
			if ( depth == 0
				 || (expression.context[depth - 1].type != EXTYPE_LABEL
					 && expression.context[depth - 1].type != EXTYPE_BYTECODE_RESULT) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}


			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;

			if ( parseExpression(nex) != ']' )
			{
				m_err = WR_ERR_unexpected_EOF;
				return 0;
			}
			
			if ( nex.bytecode.all.size() == 0 )
			{
				WRstr t( "@[]" );
				operatorFound( t, expression.context, depth );
				expression.context[depth].bytecode.all.shave(1);
				expression.context[depth].bytecode.opcodes.shave(1);
				pushOpcode( expression.context[depth].bytecode, O_IndexLiteral8 );
				unsigned char c = 0;
				pushData( expression.context[depth].bytecode, &c, 1 );
			}
			else 
			{
				WRstr t( "@[]" );
				expression.context[depth].bytecode = nex.bytecode;
				operatorFound( t, expression.context, depth );
			}


			++depth;
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		if ( isValidLabel(token, isGlobal, prefix) )
		{
			if ( (depth > 0) 
				&& ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					|| (expression.context[depth - 1].type == EXTYPE_LITERAL)) )
			{
				// two labels/literals cannot follow each other
				m_err = WR_ERR_bad_expression;
				return 0;
			}


			WRstr label = token;
			if ( depth == 0 && !m_parsingFor )
			{
				if ( !getToken(expression.context[depth]) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return 0;
				}
				
				if ( token == ":" )
				{
					uint32_t hash = wr_hashStr( label );
					for( unsigned int i=0; i<expression.bytecode.jumpOffsetTargets.count(); ++i )
					{
						if ( expression.bytecode.jumpOffsetTargets[i].gotoHash == hash )
						{
							m_err = WR_ERR_bad_goto_label;
							return false;
						}
					}
					
					int index = addRelativeJumpTarget( expression.bytecode );
					expression.bytecode.jumpOffsetTargets[index].gotoHash = hash;
					expression.bytecode.jumpOffsetTargets[index].offset = expression.bytecode.all.size() + 1;

					// this always return a value
					pushOpcode( expression.bytecode, O_LiteralZero );

					
					return ';';
				}
				else
				{
					m_loadedToken = token;
					m_loadedValue = value;
				}
			}
			
			
			expression.context[depth].type = EXTYPE_LABEL;
			expression.context[depth].token = label;
			expression.context[depth].global = isGlobal;
			expression.context[depth].prefix = prefix;
			++depth;

			continue;
		}

		m_err = WR_ERR_bad_expression;
		return 0;
	}

	expression.context.setCount( expression.context.count() - 1 );

	resolveExpression( expression );

	return end;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseUnit( bool isStruct )
{
	int previousIndex = m_unitTop;
	m_unitTop = m_units.count();
	
	bool isGlobal;

	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRstr prefix;
	
	// get the function name
	if ( !getToken(ex)
		 || !isValidLabel(token, isGlobal, prefix)
		 || isGlobal )
	{
		m_err = WR_ERR_bad_label;
		return false;
	}

	m_units[m_unitTop].hash = wr_hash( token, token.size() );
	m_units[m_unitTop].bytecode.isStructSpace = isStruct;
	
	// get the function name
	if ( getToken(ex, "(") )
	{
		for(;;)
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}
			
			if ( token == ")" )
			{
				break;
			}
			
			if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			++m_units[m_unitTop].arguments;

			// register the argument on the hash stack
			addLocalSpaceLoad( m_units[m_unitTop].bytecode, token, true );

			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( token == ")" )
			{
				break;
			}

			if ( token != "," )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
		}

		if ( !getToken(ex, "{") )
		{
			m_err = WR_ERR_unexpected_token;
			return false;
		}
	}
	else if ( token != "{" )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
		
	bool returnCalled;
	parseStatement( m_unitTop, '}', returnCalled, O_Return );

	if ( !returnCalled )
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_LiteralZero );
		pushOpcode( m_units[m_unitTop].bytecode, O_Return );
	}

	m_unitTop = previousIndex;

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseWhile( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	m_loadedToken = token;
	m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );

	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, *m_continueTargets.tail() );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();
	
	resolveRelativeJumps( m_units[m_unitTop].bytecode );
	
	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn )
{
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int jumpToTop = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, jumpToTop );
	
	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}

	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	if ( token != "while" )
	{
		m_err = WR_ERR_expected_while;
		return false;
	}

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	//m_loadedToken = token;
	//m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if (!getToken(ex, ";"))
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
	setRelativeJumpTarget(m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, jumpToTop );
	
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseForLoop( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	// create the continue and break reference points
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );


/*
	push iterator
B:
	get next or jump A
	.
	.
	.
	goto B
A:
*/					


/*


[setup]
<- condition point
[condition]
-> false jump break
[code]
<- continue point
[post code]
-> always jump condition
<- break point

*/

	bool foreachPossible = true;
	bool foreachKV = false;
	bool foreachV = false;
	int foreachLoadI = 0;
	unsigned char foreachLoad[4];
	unsigned char g;

	m_parsingFor = true;

	// [setup]
	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == ";" )
		{
			break;
		}

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;

		char end = parseExpression( nex );

		if ( foreachPossible )
		{
			if ( foreachLoadI < 3
				 && nex.bytecode.opcodes.size() == 1
				 && nex.bytecode.all.size() == 2
				 && ((nex.bytecode.all[0] == O_LoadFromLocal) || (nex.bytecode.all[0] == O_LoadFromGlobal) ) )
			{
				foreachLoad[foreachLoadI++] = nex.bytecode.all[0];
				foreachLoad[foreachLoadI++] = nex.bytecode.all[1];
			}
			else
			{
				foreachPossible = false;
			}
		}

		pushOpcode( nex.bytecode, O_PopOne );

		appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );
		
		if ( end == ';' )
		{
			foreachPossible = false;
			break;
		}
		else if ( end == ':' )
		{
			if ( foreachPossible )
			{
				WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
				nex.context[0].token = token;
				nex.context[0].value = value;
				end = parseExpression( nex );
				if ( end == ')'
					 && nex.bytecode.opcodes.size() == 1
					 && nex.bytecode.all.size() == 2 )
				{

					WRstr T;
					T.format( "foreach:%d", m_foreachHash++ );
					g = (unsigned char)(addGlobalSpaceLoad( m_units[0].bytecode, T, true ));

					if ( nex.bytecode.opcodes[0] == O_LoadFromLocal )
					{
						m_units[m_unitTop].bytecode.all += O_LPushIterator;
					}
					else
					{
						m_units[m_unitTop].bytecode.all += O_GPushIterator;
					}

					m_units[m_unitTop].bytecode.all += nex.bytecode.all[1];
					pushData( m_units[m_unitTop].bytecode, &g, 1 );

					if ( foreachLoadI == 4 )
					{
						foreachKV = true;
					}
					else if ( foreachLoadI == 2 )
					{
						foreachV = true;
					}
					else
					{
						m_err = WR_ERR_unexpected_token;
						return 0;
					}

					break;
				}
				else
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}
			}
			else
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
		}
		else if ( end != ',' )
		{
			m_err = WR_ERR_unexpected_token;
			return 0;
		}
	}

	// <- condition point
	int conditionPoint = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionPoint );
	
	if ( foreachV || foreachKV )
	{
		if ( foreachV )
		{
			unsigned char load[2] = { foreachLoad[1], g };

			if ( foreachLoad[0] == O_LoadFromLocal )
			{
				addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LNextValueOrJump, *m_breakTargets.tail(), load, 2 );
			}
			else
			{
				addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GNextValueOrJump, *m_breakTargets.tail(), load, 2 );
			}
		}
		else
		{
			unsigned char load[3] = { foreachLoad[1], foreachLoad[3], g };
			
			if ( foreachLoad[0] == O_LoadFromLocal )
			{
				if ( foreachLoad[2] == O_LoadFromLocal )
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LLNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
				else
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_LGNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
			}
			else
			{
				if ( foreachLoad[2] == O_LoadFromLocal )
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GLNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
				else
				{
					addRelativeJumpSourceEx( m_units[m_unitTop].bytecode, O_GGNextKeyValueOrJump, *m_breakTargets.tail(), load, 3 );
				}
			}
		}
		
		m_parsingFor = false;

		// [ code ]
		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}
	}
	else
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		// [ condition ]
		if ( token != ";" )
		{
			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;
			m_loadedToken = token;
			m_loadedValue = value;

			if ( parseExpression( nex ) != ';' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

			// -> false jump break
			addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, *m_breakTargets.tail() );
		}


		WRExpression post( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );

		// [ post code ]
		for(;;)
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( token == ")" )
			{
				break;
			}

			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;
			m_loadedToken = token;
			m_loadedValue = value;

			char end = parseExpression( nex );
			pushOpcode( nex.bytecode, O_PopOne );

			appendBytecode( post.bytecode, nex.bytecode );

			if ( end == ')' )
			{
				break;
			}
			else if ( end != ',' )
			{
				m_err = WR_ERR_unexpected_token;
				return 0;
			}
		}

		m_parsingFor = false;

		// [ code ]
		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}

		// <- continue point
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_continueTargets.tail() );

		// [post code]
		appendBytecode( m_units[m_unitTop].bytecode, post.bytecode );
	}

	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionPoint );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	m_continueTargets.pop();
	m_breakTargets.pop();

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::lookupConstantValue( WRstr& prefix, WRValue* value )
{
	for( unsigned int v=0; v<m_units[m_unitTop].constantValues.count(); ++v )
	{
		if ( m_units[m_unitTop].constantValues[v].label == prefix )
		{
			if ( value )
			{
				*value = m_units[m_unitTop].constantValues[v].value;
			}
			return true;
		}
	}

	for( unsigned int v=0; v<m_units[0].constantValues.count(); ++v )
	{
		if ( m_units[0].constantValues[v].label == prefix )
		{
			if ( value )
			{
				*value = m_units[0].constantValues[v].value;
			}
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseEnum( int unitIndex )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	unsigned int index = 0;

	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_token;
			return false;
		}

		if ( token == "}" )
		{
			break;
		}
		else if ( token == "," )
		{
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
		{
			m_err = WR_ERR_bad_label;
			return false;
		}

		prefix = token;
		
		WRValue defaultValue;
		defaultValue.init();
		defaultValue.ui = index++;

		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( token == "=" )
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			if ( value.type == WR_REF )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}
		}
		else
		{
			m_loadedToken = token;
			m_loadedValue = value;

			value = defaultValue;
		}

		if ( value.type == WR_INT )
		{
			index = value.ui + 1;
		}
		else if ( value.type != WR_FLOAT )
		{
			m_err = WR_ERR_bad_label;
			return false;
		}
		
		if ( lookupConstantValue(prefix) )
		{
			m_err = WR_ERR_constant_refined;
			return false;
		}

		ConstantValue& newVal = m_units[m_unitTop].constantValues.append();
		newVal.label = prefix;
		newVal.value = value;
	}

	if ( getToken(ex) && token != ";" )
	{
		m_loadedToken = token;
		m_loadedValue = value;
	}

	return true;
}

//------------------------------------------------------------------------------
uint32_t WRCompilationContext::getSingleValueHash( const char* end )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;
	getToken( ex );
	
	if ( token == "(" )
	{
		return getSingleValueHash( ")" );
	}

	if ( value.type == WR_REF )
	{
		m_err = WR_ERR_switch_bad_case_hash;
		return 0;
	}
	
	uint32_t hash = (value.type == WR_COMPILER_LITERAL_STRING) ? wr_hashStr(token) : value.getHash();

	if ( !getToken(ex, end) )
	{
		m_err = WR_ERR_unexpected_token;
		return 0;
	}

	return hash;
}

/*


    continue target:
    targetswitchLinear
    8-bit max
pc> 16-bit default location
    16 bit case offset
    16 bit case offset
    16 bit case offset
    cases...
    [cases]
    break target:


continue target:
switch ins
16-bit mod
16-bit default location
32 hash case mod 0 : 16 bit case offset
32 hash case mod 1 : 16 bit case offset
32 hash case mod 2 : 16 bit case offset
...
[cases]
break target:

*/

//------------------------------------------------------------------------------
struct WRSwitchCase
{
	uint32_t hash; // hash to get this case
	bool occupied; // null hash is legal and common, must mark occupation of a node with extra flag
	bool defaultCase;
	int16_t jumpOffset; // where to go to get to this case
};

//------------------------------------------------------------------------------
bool WRCompilationContext::parseSwitch( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}
	
	WRExpression selectionCriteria( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	selectionCriteria.context[0].token = token;
	selectionCriteria.context[0].value = value;
	
	if ( parseExpression(selectionCriteria) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, selectionCriteria.bytecode );


	if ( !getToken(ex, "{") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	WRarray<WRSwitchCase> cases;
	int16_t defaultOffset = -1;
	WRSwitchCase* swCase = 0; // current case

	int defaultCaseJumpTarget = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int selectionLogicPoint = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, selectionLogicPoint );

	unsigned int startingBytecodeMarker = m_units[m_unitTop].bytecode.all.size();
	
	for(;;)
	{
		if ( !getToken(ex) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if( token == "}" )
		{
			break;
		}

		if ( token == "case" )
		{
			swCase = &cases.append();
			swCase->jumpOffset = m_units[m_unitTop].bytecode.all.size();
			swCase->hash = getSingleValueHash(":");
			if ( m_err )
			{
				return false;
			}

			swCase->occupied = true;
			swCase->defaultCase = false;

			if ( cases.count() > 1 )
			{
				for( unsigned int h = 0; h < cases.count() - 1 ; ++h )
				{
					if ( cases[h].occupied && !cases[h].defaultCase && (swCase->hash == cases[h].hash) )
					{
						m_err = WR_ERR_switch_duplicate_case;
						return false;
					}
				}
			}
		}
		else if ( token == "default" )
		{
			if ( defaultOffset != -1 )
			{
				m_err = WR_ERR_switch_duplicate_case;
				return false;
			}

			if ( !getToken(ex, ":") )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			defaultOffset = m_units[m_unitTop].bytecode.all.size();
			setRelativeJumpTarget( m_units[m_unitTop].bytecode, defaultCaseJumpTarget );
		}
		else
		{
			if ( swCase == 0 && defaultOffset == -1 )
			{
				m_err = WR_ERR_switch_case_or_default_expected;
				return false;
			}

			m_loadedToken = token;
			m_loadedValue = value;

			if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
	}

	if ( startingBytecodeMarker == m_units[m_unitTop].bytecode.all.size() )
	{
		// no code was added so this is one big null operation, go
		// ahead and null it
		m_units[m_unitTop].bytecode.all.shave(3);
		m_units[m_unitTop].bytecode.opcodes.clear();
			
		pushOpcode(m_units[m_unitTop].bytecode, O_PopOne); // pop off the selection criteria

		m_units[m_unitTop].bytecode.jumpOffsetTargets.pop();
		m_breakTargets.pop();
		return true;
	}
	
	// make sure the last instruction is a break (jump) so the
	// selection logic is skipped at the end of the last case/default
	if ( !m_units[m_unitTop].bytecode.opcodes.size() 
		 || (m_units[m_unitTop].bytecode.opcodes.size() && m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_RelativeJump) )
	{
		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, *m_breakTargets.tail() );
	}

	// selection logic jumps HERE
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, selectionLogicPoint );

	// find the highest hash value, and size an array to that
	unsigned int size = 0;
	for( unsigned int d=0; d<cases.count(); ++d )
	{
		if ( cases[d].defaultCase )
		{
			continue;
		}

		if ( cases[d].hash > size )
		{
			size = cases[d].hash;
		}

		if ( size >= 254 )
		{
			break;
		}
	}

	// first try the easy way

	++size;

	WRSwitchCase* table = 0;
	unsigned char packbuf[4];

	if ( size < 254 ) // cases are labeled 0-254, just use a linear jump table
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_SwitchLinear );

		packbuf[0] = size;
		pushData( m_units[m_unitTop].bytecode, packbuf, 1 ); // size

		int currentPos = m_units[m_unitTop].bytecode.all.size();

		if ( defaultOffset == -1 )
		{
			defaultOffset = size*2 + 2;
		}
		else
		{
			defaultOffset -= currentPos;
		}

		table = new WRSwitchCase[size];
		memset( table, 0, size*sizeof(WRSwitchCase) );

		for( unsigned int i = 0; i<size; ++i ) // for each of the possible entries..
		{
			for( unsigned int hash = 0; hash<cases.count(); ++hash ) // if a hash matches it, populate that table entry
			{
				if ( cases[hash].occupied && !cases[hash].defaultCase && (cases[hash].hash == i) )
				{
					table[cases[hash].hash].jumpOffset = cases[hash].jumpOffset - currentPos;
					table[cases[hash].hash].occupied = true;
					break;
				}
			}
		}

		pushData( m_units[m_unitTop].bytecode, pack16(defaultOffset, packbuf), 2 );

		for( unsigned int i=0; i<size; ++i )
		{
			if ( table[i].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, pack16(table[i].jumpOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, pack16(defaultOffset, packbuf), 2 );
			}
		}
	}
	else
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_Switch ); // add switch command
		unsigned char packbuf[4];

		int currentPos = m_units[m_unitTop].bytecode.all.size();

		// find a suitable mod
		uint16_t mod = 1;
		for( ; mod<0x7FFE; ++mod )
		{
			table = new WRSwitchCase[mod];
			memset( table, 0, sizeof(WRSwitchCase)*mod );

			unsigned int c=0;
			for( ; c<cases.count(); ++c )
			{
				if ( cases[c].defaultCase )
				{
					continue;
				}

				if ( table[cases[c].hash % mod].occupied )
				{
					break;
				}

				table[cases[c].hash % mod].hash = cases[c].hash;
				table[cases[c].hash % mod].jumpOffset = cases[c].jumpOffset - currentPos;
				table[cases[c].hash % mod].occupied = true;
			}

			if ( c >= cases.count() )
			{
				break;
			}
			else
			{
				delete[] table;
				table = 0;
			} 
		}

		if ( mod >= 0x7FFE )
		{
			m_err = WR_ERR_switch_construction_error;
			return false;
		}

		if ( defaultOffset == -1 )
		{
			defaultOffset = mod*6 + 4;
		}
		else
		{
			defaultOffset -= currentPos;
		}

		pushData( m_units[m_unitTop].bytecode, pack16(mod, packbuf), 2 ); // mod value
		pushData( m_units[m_unitTop].bytecode, pack16(defaultOffset, packbuf), 2 ); // default offset

		for( uint16_t m = 0; m<mod; ++m )
		{
			pushData( m_units[m_unitTop].bytecode, pack32(table[m].hash, packbuf), 4 );

			if ( !table[m].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, pack16(defaultOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, pack16(table[m].jumpOffset, packbuf), 2 );
			}
		}
	}

	delete[] table;

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	m_breakTargets.pop();

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseIf( bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRValue& value = ex.value;

	if ( !getToken(ex, "(") )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	if ( !getToken(ex) )
	{
		m_err = WR_ERR_unexpected_EOF;
		return false;
	}

	WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
	nex.context[0].token = token;
	nex.context[0].value = value;
	m_loadedToken = token;
	m_loadedValue = value;

	if ( parseExpression(nex) != ')' )
	{
		m_err = WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	int conditionFalseMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, conditionFalseMarker );

	if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
	{
		return false;
	}

 	if ( !getToken(ex) )
	{
		setRelativeJumpTarget(m_units[m_unitTop].bytecode, conditionFalseMarker);
	}
	else if ( token == "else" )
	{
		int conditionTrueMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode ); // when it hits here it will jump OVER this section

		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionTrueMarker );
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );

		if ( !parseStatement(m_unitTop, ';', returnCalled, opcodeToReturn) )
		{
			return false;
		}
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionTrueMarker );
	}
	else
	{
		m_loadedToken = token;
		m_loadedValue = value;
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );
	}

	resolveRelativeJumps( m_units[m_unitTop].bytecode ); // at least do the ones we added

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	returnCalled = false;

	for(;;)
	{
		WRstr& token = ex.token;
		WRValue& value = ex.value;

		if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
		{
			if ( end == '}' )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			break;
		}

		if ( token[0] == end )
		{
			break;
		}

		if ( token == "{" )
		{
			return parseStatement( unitIndex, '}', returnCalled, opcodeToReturn );
		}

		if ( token == "return" )
		{
			returnCalled = true;
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( token == ";" ) // special case of a null return, add the null
			{
				pushOpcode( m_units[unitIndex].bytecode, O_LiteralZero );
			}
			else
			{
				WRExpression nex( m_units[unitIndex].bytecode.localSpace, m_units[unitIndex].bytecode.isStructSpace );
				nex.context[0].token = token;
				nex.context[0].value = ex.value;
				m_loadedToken = token;
				m_loadedValue = ex.value;

				if ( parseExpression( nex ) != ';')
				{
					m_err = WR_ERR_unexpected_token;
					return false;
				}

				appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			}

			pushOpcode( m_units[unitIndex].bytecode, opcodeToReturn );
		}
		else if ( token == "struct" )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}

			if ( !parseUnit(true) )
			{
				return false;
			}
		}
		else if ( token == "function" )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}
			
			if ( !parseUnit(false) )
			{
				return false;
			}
		}
		else if ( token == "if" )
		{
			if ( !parseIf(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "while" )
		{
			if ( !parseWhile(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "for" )
		{
			if ( !parseForLoop(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "enum" )
		{
			if ( !parseEnum(unitIndex) )
			{
				return false;
			}
		}
		else if ( token == "switch" )
		{
			if ( !parseSwitch(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "do" )
		{
			if ( !parseDoWhile(returnCalled, opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( token == "break" )
		{
			if ( !m_breakTargets.count() )
			{
				m_err = WR_ERR_break_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_breakTargets.tail() );
		}
		else if ( token == "continue" )
		{
			if ( !m_continueTargets.count() )
			{
				m_err = WR_ERR_continue_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_continueTargets.tail() );
		}
		else if ( token == "gc_pause" )
		{
			if ( !getToken(ex, "(") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}

			if ( !getToken(ex) || value.type != WR_INT )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}

			uint16_t cycle = value.ui;

			pushOpcode( m_units[unitIndex].bytecode, O_GC_Command );
			unsigned char count[2];
			pack16( cycle, count );
			pushData( m_units[unitIndex].bytecode, count, 2 );

			if ( !getToken(ex, ")") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}
			
			if ( !getToken(ex, ";") )
			{
				m_err = WR_ERR_bad_expression;
				return false;
			}
		}
		else if ( token == "goto" )
		{
			if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			bool isGlobal;
			WRstr prefix;
			if ( !isValidLabel(token, isGlobal, prefix) || isGlobal )
			{
				m_err = WR_ERR_bad_goto_label;
				return false;
			}

			GotoSource& G = m_units[unitIndex].bytecode.gotoSource.append();
			G.hash = wr_hashStr( token );
			G.offset = m_units[unitIndex].bytecode.all.size();
			pushData( m_units[unitIndex].bytecode, "\0\0\0", 3 );

			if ( !getToken(ex, ";"))
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
		}
		else
		{
			WRExpression nex( m_units[unitIndex].bytecode.localSpace, m_units[unitIndex].bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = ex.value;
			m_loadedToken = token;
			m_loadedValue = ex.value;
			if ( parseExpression(nex) != ';' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			pushOpcode( m_units[unitIndex].bytecode, O_PopOne );
		}

		if ( end == ';' ) // single statement
		{
			break;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
void WRCompilationContext::createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size )
{
	if ( unit.bytecode.localSpace.count() == 0 )
	{
		*size = 0;
		*buf = 0;
		return;
	}

	WRHashTable<unsigned char> offsets;
	for( unsigned char i=unit.arguments; i<unit.bytecode.localSpace.count(); ++i )
	{
		offsets.set( unit.bytecode.localSpace[i].hash, i - unit.arguments );
	}
	
	*size = 2;
	*buf = new unsigned char[ (offsets.m_mod * 5) + 4 ];
	(*buf)[(*size)++] = (offsets.m_mod>>8) & 0xFF;
	(*buf)[(*size)++] = offsets.m_mod & 0xFF;

	for( int i=0; i<offsets.m_mod; ++i )
	{
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>24) & 0xFF;
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>16) & 0xFF;
		(*buf)[(*size)++] = (offsets.m_list[i].hash>>8) & 0xFF;
		(*buf)[(*size)++] = offsets.m_list[i].hash & 0xFF;
		(*buf)[(*size)++] = offsets.m_list[i].hash ? offsets.m_list[i].value : (unsigned char)-1;
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::link( unsigned char** out, int* outLen )
{
	WROpcodeStream code;

	code += (unsigned char)(m_units.count() - 1);
	code += (unsigned char)(m_units[0].bytecode.localSpace.count());

	unsigned char data[4];

	// register the function signatures
	for( unsigned int u=1; u<m_units.count(); ++u )
	{
		data[3] = u - 1; // index
		data[2] = m_units[u].arguments; // args
		data[1] = m_units[u].bytecode.localSpace.count(); // local frame size
		data[0] = 0;
		code += O_LiteralInt32;
		code.append( data, 4 ); // placeholder, it doesn't matter

		code += O_LiteralInt32; // hash
		code.append( pack32(m_units[u].hash, data), 4 );

		// offset placeholder
		code += O_LiteralInt16;
		m_units[u].offsetInBytecode = code.size();
		code.append( data, 2 ); // placeholder, it doesn't matter

		code += O_RegisterFunction;
	}

	// append all the unit code
	for( unsigned int u=0; u<m_units.count(); ++u )
	{
		if ( u > 0 ) // for the non-zero unit fill location into the jump table
		{
			int16_t offset = code.size();
			pack16( offset, code.p_str(m_units[u].offsetInBytecode) );
		}

		// fill in relative jumps for the gotos
		for( unsigned int g=0; g<m_units[u].bytecode.gotoSource.count(); ++g )
		{
			unsigned int j=0;
			for( ; j<m_units[u].bytecode.jumpOffsetTargets.count(); j++ )
			{
				if ( m_units[u].bytecode.jumpOffsetTargets[j].gotoHash == m_units[u].bytecode.gotoSource[g].hash )
				{
					int diff = m_units[u].bytecode.jumpOffsetTargets[j].offset - m_units[u].bytecode.gotoSource[g].offset;
					diff -= 2;
					if ( (diff < 128) && (diff > -129) )
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump8;
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset + 1 ) = diff;
					}
					else
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump;
						pack16( diff, m_units[u].bytecode.all.p_str(m_units[u].bytecode.gotoSource[g].offset + 1) );
					}

					break;
				}
			}

			if ( j >= m_units[u].bytecode.jumpOffsetTargets.count() )
			{
				m_err = WR_ERR_goto_target_not_found;
				return;
			}
		}

		int base = code.size();

		code.append( m_units[u].bytecode.all, m_units[u].bytecode.all.size() );

		// load new's
		for( unsigned int f=0; f<m_units[u].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash != N.hash )
					{
						continue;
					}

					if ( m_units[u2].offsetOfLocalHashMap == 0 )
					{
						int size;
						unsigned char* buf = 0;

						createLocalHashMap( m_units[u2], &buf, &size );
						
						if ( size )
						{
							buf[0] = (unsigned char)(m_units[u2].bytecode.localSpace.count() - m_units[u2].arguments); // number of entries
							buf[1] = m_units[u2].arguments;
							m_units[u2].offsetOfLocalHashMap = code.size();
							code.append( buf, size );
						}

						delete[] buf;
					}

					if ( m_units[u2].offsetOfLocalHashMap != 0 )
					{
						int index = base + N.references[r];

						code[index] = (m_units[u2].offsetOfLocalHashMap>>8) & 0xFF;
						code[index+1] = m_units[u2].offsetOfLocalHashMap & 0xFF;
					}

					break;
				}
			}
		}
		
		// load function table
		for( unsigned int x=0; x<m_units[u].bytecode.functionSpace.count(); ++x )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.functionSpace[x];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				unsigned int u2 = 1;
				int index = base + N.references[r];

				for( ; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						code[index] = O_CallFunctionByIndex;

						code[index+2] = (char)(u2 - 1);

						// r+1 = args
						// r+2345 = hash
						// r+6

						if ( code[index+5] == O_PopOne || code[index + 6] == O_NewObjectTable)
						{
							code[index+3] = 3; // skip past the pop, or TO the NewObjectTable
						}
						else 
						{
							code[index+3] = 2; // skip by this push is seen
							code[index+5] = O_PushIndexFunctionReturnValue;
						}
						
						break;
					}
				}

				if ( u2 >= m_units.count() )
				{
					if ( code[index+5] == O_PopOne )
					{
						code[index] = O_CallFunctionByHashAndPop;
					}
					else
					{
						code[index] = O_CallFunctionByHash;
					}

					pack32( N.hash, code.p_str(index+2) );
				}
			}
		}
	}

	uint32_t hash = wr_hash( code, code.size() );

	pack32( hash, data );
	code.append( data, 4 );

	if ( !m_err )
	{
		*outLen = code.size();
		code.release( out );
	}
}

//------------------------------------------------------------------------------
WRError WRCompilationContext::compile( const char* source,
									   const int size,
									   unsigned char** out,
									   int* outLen,
									   char* errorMsg )
{
	m_source = source;
	m_sourceLen = size;

	*outLen = 0;
	*out = 0;
	
	m_pos = 0;
	m_err = WR_ERR_None;
	m_EOF = false;
	m_parsingFor = false;
	m_unitTop = 0;
	m_foreachHash = 0;

	m_units.setCount(1);
	
	bool returnCalled = false;

	m_loadedValue.p2 = INIT_AS_REF;

	do
	{

		WRExpressionContext ex;
		WRstr& token = ex.token;
		WRValue& value = ex.value;
		if ( !getToken(ex) )
		{
			break;
		}
		m_loadedToken = token;
		m_loadedValue = value;
		parseStatement( 0, ';', returnCalled, O_GlobalStop );

	} while ( !m_EOF && (m_err == WR_ERR_None) );

	WRstr msg;
	
	if ( m_err != WR_ERR_None )
	{			
		int onChar = 0;
		int onLine = 1;
		WRstr line;

		for( int p = 0; p<size && source[p] != '\n'; p++ )
		{
			line += (char)source[p];
		}

		for( int i=0; i<m_pos; ++i )
		{
			if ( source[i] == '\n' )
			{
				onLine++;
				onChar = 0;

				line.clear();
				for( int p = i+1; p<size && source[p] != '\n'; p++ )
				{
					line += (char)source[p];
				}
			}
			else
			{
				onChar++;
			}
		}
		
		msg.format( "line:%d\n", onLine );
		msg.appendFormat( "err:%d\n", m_err );
		msg.appendFormat( "%-5d %s\n", onLine, line.c_str() );

		for( int i=0; i<onChar; i++ )
		{
			msg.appendFormat(" ");
		}

		msg.appendFormat( "     ^\n" );

		if ( errorMsg )
		{
			strncpy( errorMsg, msg, msg.size() + 1 );
		}

		printf( "%s", msg.c_str() );
		
		return m_err;
	}
	
	if ( !returnCalled )
	{
		// pop final return value
		pushOpcode( m_units[0].bytecode, O_LiteralZero );
		pushOpcode( m_units[0].bytecode, O_GlobalStop );
	}

	pushOpcode( m_units[0].bytecode, O_Stop );
	
	link( out, outLen );
	if ( m_err )
	{
		printf( "link error [%d]\n", m_err );
		if ( errorMsg )
		{
			snprintf( errorMsg, 32, "link error [%d]\n", m_err );
		}

	}

//	WRstr str;
//	wr_asciiDump( *out, *outLen, str );
//	printf( "%d:\n%s\n", *outLen, str.c_str() );

	return m_err;
}

//------------------------------------------------------------------------------
int wr_compile( const char* source, const int size, unsigned char** out, int* outLen, char* errMsg )
{
	assert( sizeof(float) == 4 );
	assert( sizeof(int) == 4 );
	assert( sizeof(char) == 1 );
	assert( O_LAST < 255 );

	// create a compiler context that has all the necessary stuff so it's completely unloaded when complete
	WRCompilationContext comp; 

	return comp.compile( source, size, out, outLen, errMsg );
}

//------------------------------------------------------------------------------
#ifdef DEBUG_OPCODE_NAMES
const char* c_opcodeName[] = 
{
	"RegisterFunction",

	"LiteralInt32",
	"LiteralZero",
	"LiteralFloat",
	"LiteralString",

	"CallFunctionByHash",
	"CallFunctionByHashAndPop",
	"CallFunctionByIndex",
	"PushIndexFunctionReturnValue",

	"CallLibFunction",

	"NewObjectTable",
	"AssignToObjectTableByOffset",

	"AssignToHashTableAndPop",
	"RemoveFromHashTable",
	"HashEntryExists",

	"PopOne",
	"ReturnZero",
	"Return",
	"Stop",

	"Index",
	"IndexSkipLoad",
	"CountOf",
	"HashOf",

	"StackIndexHash",
	"GlobalIndexHash",
	"LocalIndexHash",

	"StackSwap",
	"SwapTwoToTop",

	"LoadFromLocal",
	"LoadFromGlobal",

	"LLValues",
	"LGValues",
	"GLValues",
	"GGValues",

	"BinaryRightShiftSkipLoad",
	"BinaryLeftShiftSkipLoad",
	"BinaryAndSkipLoad",
	"BinaryOrSkipLoad",
	"BinaryXORSkipLoad",
	"BinaryModSkipLoad",

	"BinaryMultiplication",
	"BinarySubtraction",
	"BinaryDivision",
	"BinaryRightShift",
	"BinaryLeftShift",
	"BinaryMod",
	"BinaryOr",
	"BinaryXOR",
	"BinaryAnd",
	"BinaryAddition",

	"BitwiseNOT",

	"RelativeJump",
	"RelativeJump8",

	"BZ",
	"BZ8",

	"LogicalAnd",
	"LogicalOr",
	"CompareLE",
	"CompareGE",
	"CompareGT",
	"CompareLT",
	"CompareEQ",
	"CompareNE",

	"GGCompareGT",
	"GGCompareGE",
	"GGCompareLT",
	"GGCompareLE",
	"GGCompareEQ", 
	"GGCompareNE", 

	"LLCompareGT",
	"LLCompareGE",
	"LLCompareLT",
	"LLCompareLE",
	"LLCompareEQ", 
	"LLCompareNE", 

	"GSCompareEQ",
	"LSCompareEQ", 
	"GSCompareNE", 
	"LSCompareNE", 
	"GSCompareGE",
	"LSCompareGE",
	"GSCompareLE",
	"LSCompareLE",
	"GSCompareGT",
	"LSCompareGT",
	"GSCompareLT",
	"LSCompareLT",

	"GSCompareEQBZ", 
	"LSCompareEQBZ", 
	"GSCompareNEBZ", 
	"LSCompareNEBZ", 
	"GSCompareGEBZ",
	"LSCompareGEBZ",
	"GSCompareLEBZ",
	"LSCompareLEBZ",
	"GSCompareGTBZ",
	"LSCompareGTBZ",
	"GSCompareLTBZ",
	"LSCompareLTBZ",

	"GSCompareEQBZ8",
	"LSCompareEQBZ8",
	"GSCompareNEBZ8",
	"LSCompareNEBZ8",
	"GSCompareGEBZ8",
	"LSCompareGEBZ8",
	"GSCompareLEBZ8",
	"LSCompareLEBZ8",
	"GSCompareGTBZ8",
	"LSCompareGTBZ8",
	"GSCompareLTBZ8",
	"LSCompareLTBZ8",

	"LLCompareLTBZ",
	"LLCompareLEBZ",
	"LLCompareGTBZ",
	"LLCompareGEBZ",
	"LLCompareEQBZ",
	"LLCompareNEBZ",

	"GGCompareLTBZ",
	"GGCompareLEBZ",
	"GGCompareGTBZ",
	"GGCompareGEBZ",
	"GGCompareEQBZ",
	"GGCompareNEBZ",

	"LLCompareLTBZ8",
	"LLCompareLEBZ8",
	"LLCompareGTBZ8",
	"LLCompareGEBZ8",
	"LLCompareEQBZ8",
	"LLCompareNEBZ8",

	"GGCompareLTBZ8",
	"GGCompareLEBZ8",
	"GGCompareGTBZ8",
	"GGCompareGEBZ8",
	"GGCompareEQBZ8",
	"GGCompareNEBZ8",

	"PostIncrement",
	"PostDecrement",
	"PreIncrement",
	"PreDecrement",

	"PreIncrementAndPop",
	"PreDecrementAndPop",

	"IncGlobal",
	"DecGlobal",
	"IncLocal",
	"DecLocal",

	"Assign",
	"AssignAndPop",
	"AssignToGlobalAndPop",
	"AssignToLocalAndPop",
	"AssignToArrayAndPop",

	"SubtractAssign",
	"AddAssign",
	"ModAssign",
	"MultiplyAssign",
	"DivideAssign",
	"ORAssign",
	"ANDAssign",
	"XORAssign",
	"RightShiftAssign",
	"LeftShiftAssign",

	"SubtractAssignAndPop",
	"AddAssignAndPop",
	"ModAssignAndPop",
	"MultiplyAssignAndPop",
	"DivideAssignAndPop",
	"ORAssignAndPop",
	"ANDAssignAndPop",
	"XORAssignAndPop",
	"RightShiftAssignAndPop",
	"LeftShiftAssignAndPop",

	"LogicalNot", //X
	"Negate",

	"LiteralInt8",
	"LiteralInt16",

	"IndexLiteral8",
	"IndexLiteral16",

	"IndexLocalLiteral8",
	"IndexGlobalLiteral8",
	"IndexLocalLiteral16",
	"IndexGlobalLiteral16",

	"BinaryAdditionAndStoreGlobal",
	"BinarySubtractionAndStoreGlobal",
	"BinaryMultiplicationAndStoreGlobal",
	"BinaryDivisionAndStoreGlobal",

	"BinaryAdditionAndStoreLocal",
	"BinarySubtractionAndStoreLocal",
	"BinaryMultiplicationAndStoreLocal",
	"BinaryDivisionAndStoreLocal",

	"CompareBEQ",
	"CompareBNE",
	"CompareBGE",
	"CompareBLE",
	"CompareBGT",
	"CompareBLT",

	"CompareBEQ8",
	"CompareBNE8",
	"CompareBGE8",
	"CompareBLE8",
	"CompareBGT8",
	"CompareBLT8",

	"BLA",
	"BLA8",
	"BLO",
	"BLO8",

	"LiteralInt8ToGlobal",
	"LiteralInt16ToGlobal",
	"LiteralInt32ToLocal",
	"LiteralInt8ToLocal",
	"LiteralInt16ToLocal",
	"LiteralFloatToGlobal",
	"LiteralFloatToLocal",
	"LiteralInt32ToGlobal",

	"GGBinaryMultiplication",
	"GLBinaryMultiplication",
	"LLBinaryMultiplication",

	"GGBinaryAddition",
	"GLBinaryAddition",
	"LLBinaryAddition",

	"GGBinarySubtraction",
	"GLBinarySubtraction",
	"LGBinarySubtraction",
	"LLBinarySubtraction",

	"GGBinaryDivision",
	"GLBinaryDivision",
	"LGBinaryDivision",
	"LLBinaryDivision",

	"GC_Command",

	"GPushIterator",
	"LPushIterator",
	"GGNextKeyValueOrJump",
	"GLNextKeyValueOrJump",
	"LGNextKeyValueOrJump",
	"LLNextKeyValueOrJump",
	"GNextValueOrJump",
	"LNextValueOrJump",

	"Switch",
	"SwitchLinear",

	"GlobalStop",

	"ToInt",
	"ToFloat",

	"CallLibFunctionAndPop",
};
#endif

#else // WRENCH_WITHOUT_COMPILER

int wr_compile( const char* source, const int size, unsigned char** out, int* outLen, char* errMsg )
{
	return WR_ERR_compiler_not_loaded;
}

	
#endif
