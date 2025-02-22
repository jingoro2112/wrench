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
#include <assert.h>

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
	"export",
	"unit",
	""
};

//------------------------------------------------------------------------------
WRError WRCompilationContext::compile( const char* source,
									   const int size,
									   unsigned char** out,
									   int* outLen,
									   char* errorMsg,
									   const uint8_t compilerOptionFlags )
{
	m_source = source;
	m_sourceLen = size;

	*outLen = 0;
	*out = 0;

	m_lastParam = 0;
	m_lastCode = 0;

	m_pos = 0;
	m_err = WR_ERR_None;
	m_EOF = false;
	m_parsingFor = false;
	m_parsingNew = false;
	m_unitTop = 0;
	m_foreachHash = 0;

	m_units.setCount(1);

	m_loadedValue.p2 = INIT_AS_REF;

	m_addDebugSymbols = compilerOptionFlags & WR_EMBED_DEBUG_CODE;
	m_embedSourceCode = compilerOptionFlags & WR_EMBED_SOURCE_CODE;
	m_embedGlobalSymbols = compilerOptionFlags != 0; // (WR_INCLUDE_GLOBALS)
	m_needVar = !(compilerOptionFlags & WR_NON_STRICT_VAR);

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
		m_loadedQuoted = m_quoted;
		
		parseStatement( 0, ';', O_GlobalStop );

	} while ( !m_EOF && (m_err == WR_ERR_None) );

	WRstr msg;

	if ( m_err != WR_ERR_None )
	{
		int onChar;
		int onLine;
		WRstr line;
		getSourcePosition( onLine, onChar, &line );

		msg.format( "line:%d\n", onLine );
		msg.appendFormat( "err: %s[%d]\n", c_errStrings[m_err], m_err );
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

	if ( !m_units[0].bytecode.opcodes.size()
		 || m_units[0].bytecode.opcodes[m_units[0].bytecode.opcodes.size() - 1] != O_GlobalStop )
	{
		// pop final return value
		pushOpcode( m_units[0].bytecode, O_LiteralZero );
		pushOpcode( m_units[0].bytecode, O_GlobalStop );
	}

	pushOpcode( m_units[0].bytecode, O_Stop );

	link( out, outLen, compilerOptionFlags );
	if ( m_err )
	{
		printf( "link error [%d]\n", m_err );
		if ( errorMsg )
		{
			snprintf( errorMsg, 32, "link error [%d]\n", m_err );
		}

	}

	return m_err;
}

//------------------------------------------------------------------------------
WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					char* errMsg,
					const uint8_t compilerOptionFlags )
{
	assert( sizeof(float) == 4 );
	assert( sizeof(int) == 4 );
	assert( sizeof(char) == 1 );
	assert( O_LAST < 255 );

	// create a compiler context that has all the necessary stuff so it's completely unloaded when complete
	WRCompilationContext comp; 

	return comp.compile( source, size, out, outLen, errMsg, compilerOptionFlags );
}

//------------------------------------------------------------------------------
void streamDump( WROpcodeStream const& stream )
{
	WRstr str;
	wr_asciiDump( stream, stream.size(), str );
	printf( "%d:\n%s\n", stream.size(), str.c_str() );
}

//------------------------------------------------------------------------------
bool WRCompilationContext::isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix, bool& isLibConstant )
{
	isLibConstant = false;
	
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

	bool foundColon = false;
	for( unsigned int i=0; i<token.size(); i++ ) // entire token alphanumeric or '_'?
	{
		if ( token[i] == ':' )
		{
			if ( token[++i] != ':' )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}

			foundColon = true;

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

	if ( foundColon && token[token.size() - 1] == ':' )
	{
		return false;
	}
	
	if ( foundColon && token[0] != ':' )
	{
		isLibConstant = true;
	}

	return true;
}


//------------------------------------------------------------------------------
void WRCompilationContext::pushDebug( uint16_t code, WRBytecode& bytecode, int param )
{
	if ( !m_addDebugSymbols )
	{
		return;
	}

	if ( code == m_lastCode && (uint16_t)param == m_lastParam )
	{
		return;
	}

	m_lastParam = param;
	m_lastCode = code;
	
	pushOpcode( bytecode, O_DebugInfo );

	uint16_t codeword = code | ((uint16_t)param & WRD_PayloadMask);
	unsigned char data[2];
	pushData( bytecode, wr_pack16(codeword, data), 2 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::getSourcePosition()
{
	int onChar;
	int onLine;
	return getSourcePosition( onLine, onChar, 0 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::getSourcePosition( int& onLine, int& onChar, WRstr* line )
{
	onChar = 0;
	onLine = 1;

	for( int p = 0; line && p<m_sourceLen && m_source[p] != '\n'; p++ )
	{
		(*line) += (char)m_source[p];
	}

	for( int i=0; i<m_pos; ++i )
	{
		if ( m_source[i] == '\n' )
		{
			onLine++;
			onChar = 0;

			if( line )
			{
				line->clear();
				for( int p = i+1; p<m_sourceLen && m_source[p] != '\n'; p++ )
				{
					(*line) += (char)m_source[p];
				}
			}
		}
		else
		{
			onChar++;
		}
	}

	return onLine;
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
				
				wr_pack16( diff, bytecode.all.p_str(offset) );
			}
		}
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLiteral( WRBytecode& bytecode, WRExpressionContext& context )
{
	WRValue& value = context.value;
	unsigned char data[4];
	
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
			int16_t be = value.i;
			pushData( bytecode, wr_pack16(be, data), 2 );
		}
		else
		{
			pushOpcode( bytecode, O_LiteralInt32 );
			unsigned char data[4];
			pushData( bytecode, wr_pack32(value.i, data), 4 );
		}
	}
	else if ( (uint8_t)value.type == WR_COMPILER_LITERAL_STRING )
	{
		pushOpcode( bytecode, O_LiteralString );
		int16_t be = context.literalString.size();
		pushData( bytecode, wr_pack16(be, data), 2 );
		for( unsigned int i=0; i<context.literalString.size(); ++i )
		{
			pushData( bytecode, context.literalString.c_str(i), 1 );
		}
	}
	else
	{
		pushOpcode( bytecode, O_LiteralFloat );
		int32_t be = value.i;
		pushData( bytecode, wr_pack32(be, data), 4 );
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::pushLibConstant( WRBytecode& bytecode, WRExpressionContext& context )
{
	pushOpcode( bytecode, O_LoadLibConstant );
	unsigned char data[4];
	uint32_t hash = wr_hashStr(context.prefix + "::" + context.token);
	pushData( bytecode, wr_pack32(hash, data), 4 );
}

//------------------------------------------------------------------------------
int WRCompilationContext::addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen )
{
	if ( m_unitTop == 0 )
	{
		return addGlobalSpaceLoad( bytecode, token, addOnly, varSeen );
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

	if ( i >= bytecode.localSpace.count() )
	{
		if ( !addOnly )
		{
			WRstr t2;
			if ( varSeen || (token[0] == ':' && token[1] == ':') )
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
			if ( !bytecode.isStructSpace || varSeen ) // structs and explicit 'var' are immune to this
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

		if ( m_needVar && !varSeen )
		{
			m_err = WR_ERR_var_not_seen_before_label;
			return 0;
		}
	}

	bytecode.localSpace[i].hash = hash;
	bytecode.localSpace[i].label = token;
	
	if ( !addOnly )
	{
		pushOpcode( bytecode, O_LoadFromLocal );
		unsigned char c = i;
		pushData( bytecode, &c, 1 );
	}
	
	return i;
}

//------------------------------------------------------------------------------
int WRCompilationContext::addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen )
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

	if ( m_needVar && !varSeen && i >= m_units[0].bytecode.localSpace.count() )
	{
		m_err = WR_ERR_var_not_seen_before_label;
		return 0;
	}
	
	m_units[0].bytecode.localSpace[i].hash = hash;
	m_units[0].bytecode.localSpace[i].label = t2;

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
		wr_pack32( wr_hash( expression.context[depth].token,
							expression.context[depth].token.size()),
				   buf );
		pushOpcode( expression.bytecode, O_LiteralInt32 );
		pushData( expression.bytecode, buf, 4 );
	}
	else
	{
		switch( expression.context[depth].type )
		{
			case EXTYPE_LIB_CONSTANT:
			{
				pushLibConstant( expression.bytecode, expression.context[depth] );
				break;
			}
			
			case EXTYPE_LITERAL:
			{
				pushLiteral( expression.bytecode, expression.context[depth] );
				break;
			}

			case EXTYPE_LABEL_AND_NULL:
			case EXTYPE_LABEL:
			{
				if ( expression.context[depth].global )
				{
					addGlobalSpaceLoad( expression.bytecode,
										expression.context[depth].token,
										false,
										expression.context[depth].varSeen );
				}
				else
				{
					addLocalSpaceLoad( expression.bytecode,
									   expression.context[depth].token,
									   false,
									   expression.context[depth].varSeen );
				}

				if ( expression.context[depth].type == EXTYPE_LABEL_AND_NULL )
				{
					expression.bytecode.all += O_InitVar;
					expression.bytecode.opcodes += O_InitVar;
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


				if ( expression.context[first].stackPosition == 1 )
				{
					// lucky! a simple single-swap will put them in the
					// right order (turns out this is common)
					expression.swapWithTop( 1 );
				}
				else
				{
					// can still do it in one opcode
					
					WRCompilationContext::pushOpcode( expression.bytecode, O_SwapTwoToTop );
					unsigned char pos = expression.context[first].stackPosition + 1;
					WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
					pos = 2;
					WRCompilationContext::pushData( expression.bytecode, &pos, 1 );
					
					expression.swapWithTop( 1, false );
					expression.swapWithTop( expression.context[first].stackPosition, false );
				}
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
bool WRCompilationContext::operatorFound( WRstr const& token, WRarray<WRExpressionContext>& context, int depth )
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

			if ( !m_quoted && token2 == ")" )
			{
				break;
			}

			++argsPushed;

			WRExpression nex( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;
			m_loadedToken = token2;
			m_loadedValue = value2;
			m_loadedQuoted = m_quoted;
			
			char end = parseExpression( nex );

			if ( nex.bytecode.opcodes.size() > 0 )
			{
				uint8_t op = nex.bytecode.opcodes[nex.bytecode.opcodes.size() - 1];
				if ( op == O_Index
					 || op == O_IndexSkipLoad
					 || op == O_IndexLiteral8
					 || op == O_IndexLiteral16
					 || op == O_IndexLocalLiteral8
					 || op == O_IndexGlobalLiteral8
					 || op == O_IndexLocalLiteral16
					 || op == O_IndexGlobalLiteral16 )
				{
					nex.bytecode.opcodes += O_Dereference;
					nex.bytecode.all += O_Dereference;
				}
			}
			
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

	pushDebug( WRD_LineNumber, expression.context[depth].bytecode, getSourcePosition() );
	pushDebug( WRD_FunctionCall, expression.context[depth].bytecode, WRD_ExternalFunction );

	if ( prefix.size() )
	{
		prefix += "::";
		prefix += functionName;

		unsigned char buf[4];
		wr_pack32( wr_hashStr(prefix), buf );

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

		if ( hash == wr_hashStr("yield") )
		{
			pushOpcode( expression.context[depth].bytecode, O_Yield );
			pushData( expression.context[depth].bytecode, &argsPushed, 1 );
		}
		else
		{
			pushOpcode( expression.context[depth].bytecode, O_FUNCTION_CALL_PLACEHOLDER );
			pushData( expression.context[depth].bytecode, &argsPushed, 1 );
			pushData( expression.context[depth].bytecode, "XXXX", 4 ); // TBD opcode plus index, OR hash if index was not found
		}
		
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

	bool byHash = false;
	bool byOrder = false;
	
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

	if ( !m_quoted && token2 == "{" )
	{
		unsigned char offset = 0;
		for(;;)
		{
			WRExpression nex( localSpace, context.bytecode.isStructSpace );
			nex.context[0].token = token2;
			nex.context[0].value = value2;

			m_parsingNew = true;

			bool v = m_needVar;
			m_needVar = false;
			char end = parseExpression( nex );
			m_needVar = v;

			m_parsingNew = false;

			if ( nex.bytecode.all.size() )
			{
				appendBytecode( context.bytecode, nex.bytecode );

				if ( m_newHashValue )
				{
					if ( byOrder )
					{
						m_err = WR_ERR_new_assign_by_label_or_offset_not_both;
						return false;
					}
					byHash = true;
					
					pushOpcode( context.bytecode, O_AssignToObjectTableByHash );
					uint8_t dat[4];
					pushData( context.bytecode, wr_pack32(m_newHashValue, dat), 4 );
				}
				else
				{
					if ( byHash )
					{
						m_err = WR_ERR_new_assign_by_label_or_offset_not_both;
						return false;
					}
					byOrder = true;

					pushOpcode( context.bytecode, O_AssignToObjectTableByOffset );
					pushData( context.bytecode, &offset, 1 );
				}
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
int WRCompilationContext::parseInitializer( WRExpression& expression, int depth )
{
	bool inHashTable = false;
	bool inArray = false;
	
	// start a value ... was something declared in front of us?
	if ( depth == 3
		 && expression.context[0].type == EXTYPE_LABEL
		 && expression.context[1].type == EXTYPE_OPERATION
		 && expression.context[1].operation->opcode
		 && expression.context[1].operation->opcode == O_Index )
	{
		expression.context.remove( depth - 2, 1 );
		depth -= 1;
	}

	WRValue& value = expression.context[depth].value;
	WRstr& token = expression.context[depth].token;

	expression.context[depth].type = EXTYPE_BYTECODE_RESULT;

	pushOpcode( expression.context[depth].bytecode, O_LiteralZero ); // push the value we're going to create

	// knock off the initial create
	uint16_t initializer = 0;
	unsigned char idat[2];
	for(;;)
	{
		if ( !getToken(expression.context[depth]) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return -1;
		}

		bool subTableValue = false;

		if ( token == '{' )
		{
			subTableValue = true;
			if ( parseInitializer(expression, depth) == -1 )
			{
				return -1;
			}

			pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
			wr_pack16( initializer++, idat );
			pushData( expression.context[depth].bytecode, idat, 2 );

			continue;
		}
	
		if ( token == ',' )
		{
			if ( initializer == 0 )
			{
				m_err = WR_ERR_bad_expression;
				return -1;
			}
		}
		else
		{
			m_loadedToken = token;
			m_loadedValue = value;
			m_loadedQuoted = m_quoted;
		}

		WRExpression val( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
		val.context[0].token = token;
		val.context[0].value = value;

		char end = parseExpression( val );
		if ( end == ':' )
		{
			bool nullKey = false;
			
			// we are in a hash table!
			inHashTable = true;
			if ( inArray )
			{
				m_err = WR_ERR_hash_declaration_in_array;
				return -1;
			}

			if ( !val.bytecode.all.size() )
			{
				nullKey = true;
				pushOpcode( val.bytecode, O_LiteralZero );
			}
			
			if ( subTableValue ) // sub-table keys don't make sense
			{
				m_err = WR_ERR_hash_table_invalid_key;
				return -1;
			}

			appendBytecode( expression.context[depth].bytecode, val.bytecode );
			
			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return -1;
			}

			if ( token == '{' )
			{
				if ( parseInitializer(expression, depth) == -1 )
				{
					return -1;
				}

				pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
				continue;
			}
			else
			{
				m_loadedToken = token;
				m_loadedValue = value;
				m_loadedQuoted = m_quoted;
				
				WRExpression key( expression.bytecode.localSpace, expression.bytecode.isStructSpace );
				key.context[0].token = token;
				key.context[0].value = value;
				end = parseExpression( key );
				
				if ( key.bytecode.all.size() ) // nor do null keys
				{
					if ( nullKey )
					{
						m_err = WR_ERR_hash_table_invalid_key;
						return -1;
					}
					
					appendBytecode( expression.context[depth].bytecode, key.bytecode );
					pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
				}
				else
				{
					if ( nullKey )
					{
						// null key AND value means null table, just create a zero-size table, by indexing as
						// hash, which creates the table, then removing the entry we just added, nulling it
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_Remove );
					}
					else
					{
						pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
						pushOpcode( expression.context[depth].bytecode, O_AssignToHashTableAndPop );
					}
				}
			}
		}
		else
		{
			if ( val.bytecode.all.size() )
			{
				inArray = true;
				if ( inHashTable )
				{
					m_err = WR_ERR_array_declaration_in_hash;
					return -1;
				}

				appendBytecode( expression.context[depth].bytecode, val.bytecode );
				pushOpcode( expression.context[depth].bytecode, O_AssignToArrayAndPop );
				wr_pack16( initializer++, idat );
				pushData( expression.context[depth].bytecode, idat, 2 );
			}
		}
		
		if ( end == '}' )
		{
			break;
		}
		else if ( end == ',' )
		{
			continue;
		}
		else
		{
			m_err = WR_ERR_bad_expression;
			return -1;
		}
	}

	return depth;
}


//------------------------------------------------------------------------------
bool WRCompilationContext::parseUnit( bool isStruct, int parentUnitIndex )
{
	int previousIndex = m_unitTop;
	m_unitTop = m_units.count();

	if ( parentUnitIndex && isStruct )
	{
		m_err = WR_ERR_struct_in_struct;
		return false;
	}

	bool isGlobal;

	WRExpressionContext ex;
	WRstr& token = ex.token;
	WRstr prefix;
	bool isLibConstant;

	// get the function name
	if ( !getToken(ex)
		 || !isValidLabel(token, isGlobal, prefix, isLibConstant)
		 || isGlobal
		 || isLibConstant )
	{
		m_err = WR_ERR_bad_label;
		return false;
	}

	m_units[m_unitTop].exportNamespace = m_exportNextUnit;
	m_exportNextUnit = false;
	m_units[m_unitTop].name = token;
	m_units[m_unitTop].hash = wr_hash( token, token.size() );
	m_units[m_unitTop].bytecode.isStructSpace = isStruct;

	m_units[m_unitTop].parentUnitIndex = parentUnitIndex; // if non-zero, must be called from a new'ed structure!
	
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
			
			if ( !m_quoted && token == ")" )
			{
				break;
			}
			
			if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			++m_units[m_unitTop].arguments;

			// register the argument on the hash stack
			addLocalSpaceLoad( m_units[m_unitTop].bytecode,
							   token,
							   true,
							   true );

			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( !m_quoted && token == ")" )
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
		
	parseStatement( m_unitTop, '}', O_Return );

	if ( !m_units[m_unitTop].bytecode.opcodes.size()
		 || (m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_Return
			 && m_units[m_unitTop].bytecode.opcodes[m_units[m_unitTop].bytecode.opcodes.size() - 1] != O_ReturnZero) )
	{
		pushOpcode( m_units[m_unitTop].bytecode, O_ReturnZero );
	}

	m_unitTop = previousIndex;

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseWhile( WROpcode opcodeToReturn )
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
	m_loadedQuoted = m_quoted;

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

	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
bool WRCompilationContext::parseDoWhile( WROpcode opcodeToReturn )
{
	*m_continueTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	*m_breakTargets.push() = addRelativeJumpTarget( m_units[m_unitTop].bytecode );

	int jumpToTop = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	setRelativeJumpTarget( m_units[m_unitTop].bytecode, jumpToTop );
	
	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
bool WRCompilationContext::parseForLoop( WROpcode opcodeToReturn )
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

		if ( !m_quoted && token == ";" )
		{
			break;
		}

		WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
		nex.context[0].token = token;
		nex.context[0].value = value;
		m_loadedToken = token;
		m_loadedValue = value;
		m_loadedQuoted = m_quoted;

		char end = parseExpression( nex );

		if ( foreachPossible )
		{
			if ( foreachLoadI < 3 )
			{
				if (nex.bytecode.opcodes.size() == 1
					  && nex.bytecode.all.size() == 2
					  && ((nex.bytecode.all[0] == O_LoadFromLocal) || (nex.bytecode.all[0] == O_LoadFromGlobal)) )
				{
					foreachLoad[foreachLoadI++] = nex.bytecode.all[0];
					foreachLoad[foreachLoadI++] = nex.bytecode.all[1];
				}
				else if ( nex.bytecode.opcodes.size() == 2
						  && nex.bytecode.all.size() == 5
						  && nex.bytecode.all[0] == O_DebugInfo
						  && ((nex.bytecode.all[3] == O_LoadFromLocal) || (nex.bytecode.all[3] == O_LoadFromGlobal)) )
				{
					foreachLoad[foreachLoadI++] = nex.bytecode.all[3];
					foreachLoad[foreachLoadI++] = nex.bytecode.all[4];
				}
				else
				{
					foreachPossible = false;
				}
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
					g = (unsigned char)(addGlobalSpaceLoad(m_units[0].bytecode, T, true, true)); // #25 force "var seen" true since we are runtime adding the temporary ourselves

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
		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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
			m_loadedQuoted = m_quoted;

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

			if ( !m_quoted && token == ")" )
			{
				break;
			}

			WRExpression nex( m_units[m_unitTop].bytecode.localSpace, m_units[m_unitTop].bytecode.isStructSpace );
			nex.context[0].token = token;
			nex.context[0].value = value;
			m_loadedToken = token;
			m_loadedValue = value;
			m_loadedQuoted = m_quoted;

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
		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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

		if ( !m_quoted && token == "}" )
		{
			break;
		}
		else if ( !m_quoted && token == "," )
		{
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		bool isLibConstant;
		if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
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

		bool negative = false;

		if ( !m_quoted && token == "=" )
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_bad_label;
				return false;
			}

			if ( token == '-' )
			{
				negative = true;
				if ( !getToken(ex) )
				{
					m_err = WR_ERR_bad_label;
					return false;
				}
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
			m_loadedQuoted = m_quoted;

			value = defaultValue;
		}

		if ( value.type == WR_INT )
		{
			if ( negative )
			{
				value.i = -value.i;
			}
			
			index = value.i + 1;
		}
		else if (value.type == WR_FLOAT)
		{
			if ( negative )
			{
				value.f = -value.f;
			}
		}
		else
		{
			m_err = WR_ERR_bad_label;
			return false;
		}
		
		if ( lookupConstantValue(prefix) )
		{
			m_err = WR_ERR_constant_redefined;
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
		m_loadedQuoted = m_quoted;
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
	
	if ( !m_quoted && token == "(" )
	{
		return getSingleValueHash( ")" );
	}

	if ( value.type == WR_REF )
	{
		m_err = WR_ERR_switch_bad_case_hash;
		return 0;
	}
	
	uint32_t hash = ((uint8_t)value.type == WR_COMPILER_LITERAL_STRING) ? wr_hashStr(token) : value.getHash();

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
bool WRCompilationContext::parseSwitch( WROpcode opcodeToReturn )
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

		if( !m_quoted && token == "}" )
		{
			break;
		}

		if ( !m_quoted && token == "case" )
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
		else if ( !m_quoted && token == "default" )
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
			m_loadedQuoted = m_quoted;

			if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
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

		table = (WRSwitchCase *)g_malloc(size * sizeof(WRSwitchCase));
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

		pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );

		for( unsigned int i=0; i<size; ++i )
		{
			if ( table[i].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(table[i].jumpOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );
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
			table = (WRSwitchCase *)g_malloc(mod * sizeof(WRSwitchCase));
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
				g_free( table );
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

		pushData( m_units[m_unitTop].bytecode, wr_pack16(mod, packbuf), 2 ); // mod value
		pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 ); // default offset

		for( uint16_t m = 0; m<mod; ++m )
		{
			pushData( m_units[m_unitTop].bytecode, wr_pack32(table[m].hash, packbuf), 4 );

			if ( !table[m].occupied )
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(defaultOffset, packbuf), 2 );
			}
			else
			{
				pushData( m_units[m_unitTop].bytecode, wr_pack16(table[m].jumpOffset, packbuf), 2 );
			}
		}
	}

	g_free( table );

	setRelativeJumpTarget( m_units[m_unitTop].bytecode, *m_breakTargets.tail() );

	resolveRelativeJumps( m_units[m_unitTop].bytecode );

	m_breakTargets.pop();

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseIf( WROpcode opcodeToReturn )
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
	m_loadedQuoted = m_quoted;

	if ( parseExpression(nex) != ')' || m_err )
	{
		m_err = m_err ? m_err : WR_ERR_unexpected_token;
		return false;
	}

	appendBytecode( m_units[m_unitTop].bytecode, nex.bytecode );

	int conditionFalseMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode );
	
	addRelativeJumpSource( m_units[m_unitTop].bytecode, O_BZ, conditionFalseMarker );

	if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
	{
		return false;
	}

 	if ( !getToken(ex) )
	{
		setRelativeJumpTarget(m_units[m_unitTop].bytecode, conditionFalseMarker);
	}
	else if ( !m_quoted && token == "else" )
	{
		int conditionTrueMarker = addRelativeJumpTarget( m_units[m_unitTop].bytecode ); // when it hits here it will jump OVER this section

		addRelativeJumpSource( m_units[m_unitTop].bytecode, O_RelativeJump, conditionTrueMarker );
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );

		if ( !parseStatement(m_unitTop, ';', opcodeToReturn) )
		{
			return false;
		}
		
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionTrueMarker );
	}
	else
	{
		m_loadedToken = token;
		m_loadedValue = value;
		m_loadedQuoted = m_quoted;
		setRelativeJumpTarget( m_units[m_unitTop].bytecode, conditionFalseMarker );
	}

	resolveRelativeJumps( m_units[m_unitTop].bytecode ); // at least do the ones we added

	return true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::parseStatement( int unitIndex, char end, WROpcode opcodeToReturn )
{
	WRExpressionContext ex;
	bool varSeen = false;
	m_exportNextUnit = false;

	for(;;)
	{
		WRstr& token = ex.token;
//		WRValue& value = ex.value;

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

		if ( m_exportNextUnit
			 && token != "function"
			 && token != "struct"
			 && token != "unit" )
		{
			m_err = WR_ERR_unexpected_export_keyword;
			break;
		}
		
		if ( !m_quoted && token == "{" )
		{
			return parseStatement( unitIndex, '}', opcodeToReturn );
		}

		if ( !m_quoted && token == "return" )
		{
			if ( !getToken(ex) )
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			if ( !m_quoted && token == ";" ) // special case of a null return, add the null
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
				m_loadedQuoted = m_quoted;

				if ( parseExpression( nex ) != ';')
				{
					m_err = WR_ERR_unexpected_token;
					return false;
				}

				appendBytecode( m_units[unitIndex].bytecode, nex.bytecode );
			}

			pushDebug( WRD_LineNumber, m_units[m_unitTop].bytecode, getSourcePosition() );
			pushOpcode( m_units[unitIndex].bytecode, opcodeToReturn );
		}
		else if ( !m_quoted && token == "struct" )
		{
			m_exportNextUnit = true; // always export structs
			
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}

			if ( !parseUnit(true, unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && (token == "function" || token == "unit") )
		{
			if ( unitIndex != 0 )
			{
				m_err = WR_ERR_statement_expected;
				return false;
			}
			
			if ( !parseUnit(false, unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "if" )
		{
			if ( !parseIf(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "while" )
		{
			if ( !parseWhile(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "for" )
		{
			if ( !parseForLoop(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "enum" )
		{
			if ( !parseEnum(unitIndex) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "export" )
		{
			m_exportNextUnit = true;
			continue;
		}
		else if ( !m_quoted && token == "switch" )
		{
			if ( !parseSwitch(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "do" )
		{
			if ( !parseDoWhile(opcodeToReturn) )
			{
				return false;
			}
		}
		else if ( !m_quoted && token == "break" )
		{
			if ( !m_breakTargets.count() )
			{
				m_err = WR_ERR_break_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_breakTargets.tail() );
		}
		else if ( !m_quoted && token == "continue" )
		{
			if ( !m_continueTargets.count() )
			{
				m_err = WR_ERR_continue_keyword_not_in_looping_structure;
				return false;
			}

			addRelativeJumpSource( m_units[unitIndex].bytecode, O_RelativeJump, *m_continueTargets.tail() );
		}
		else if ( !m_quoted && token == "goto" )
		{
			if ( !getToken(ex) ) // if we run out of tokens that's fine as long as we were not waiting for a }
			{
				m_err = WR_ERR_unexpected_EOF;
				return false;
			}

			bool isGlobal;
			WRstr prefix;
			bool isLibConstant;
			if ( !isValidLabel(token, isGlobal, prefix, isLibConstant) || isGlobal || isLibConstant )
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
			nex.context[0].varSeen = varSeen;
			nex.context[0].token = token;
			nex.context[0].value = ex.value;
			nex.lValue = true;
			m_loadedToken = token;
			m_loadedValue = ex.value;
			m_loadedQuoted = m_quoted;
			if ( parseExpression(nex) != ';' )
			{
				if ( !m_err )
				{
					m_err = WR_ERR_unexpected_token;
				}
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

	return m_err ? false : true;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::checkAsComment( char lead )
{
	char c;
	if ( lead == '/' )
	{
		if ( !getChar(c) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		if ( c == '/' )
		{
			while( getChar(c) && c != '\n' );
			return true;
		}
		else if ( c == '*' )
		{
			for(;;)
			{
				if ( !getChar(c) )
				{
					m_err = WR_ERR_unexpected_EOF;
					return false;
				}

				if ( c == '*' )
				{
					if ( !getChar(c) )
					{
						m_err = WR_ERR_unexpected_EOF;
						return false;
					}

					if ( c == '/' )
					{
						break;
					}
				}
			}

			return true;
		}

		// else not a comment
	}

	return false;
}

//------------------------------------------------------------------------------
bool WRCompilationContext::readCurlyBlock( WRstr& block )
{
	char c;
	int closesNeeded = 0;

	for(;;)
	{
		if ( !getChar(c) )
		{
			m_err = WR_ERR_unexpected_EOF;
			return false;
		}

		// read past comments
		if ( checkAsComment(c) )
		{
			continue;
		}

		if ( m_err )
		{
			return false;
		}

		block += c;
		
		if ( c == '{' )
		{
			++closesNeeded;
		}
		
		if ( c == '}' )
		{
			if ( closesNeeded == 0 )
			{
				m_err = WR_ERR_unexpected_token;
				return false;
			}
			
			if ( !--closesNeeded )
			{
				break;
			}
		}
	}

	return true;
}


#else // WRENCH_WITHOUT_COMPILER

WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					char* errMsg,
					const uint8_t compilerOptionFlags )
{
	return WR_ERR_compiler_not_loaded;
}
	
#endif
