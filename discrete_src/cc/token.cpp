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
#ifndef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
int wr_strnlen( const char* s, int len )
{
	const char* found = (const char *)memchr( s, '\0', len );
	return found ? (found - s) : len;
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
		m_quoted = m_loadedQuoted;
	}
	else
	{
		m_quoted = false;
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
			int len = wr_strnlen( c_operations[t].token, 20 );
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
				m_quoted = true;
				
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
				else if (token[0] == '0' && isdigit(m_source[m_pos]) ) // octal
				{
					token.clear();

					for(;;)
					{
						if ( m_pos >= m_sourceLen )
						{
							m_err = WR_ERR_unexpected_EOF;
							return false;
						}

						if ( !isdigit(m_source[m_pos]) )
						{
							break;
						}

						token += m_source[m_pos++];
					}

					value.p2 = INIT_AS_INT;
					value.i = strtol( token, 0, 8 );
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
						value.f = strtof( token, 0 );
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
	m_loadedQuoted = m_quoted;
	m_loadedValue.p2 = INIT_AS_REF;

	if ( expect && (token != expect) )
	{
		return false;
	}

	return true;
}

#endif
