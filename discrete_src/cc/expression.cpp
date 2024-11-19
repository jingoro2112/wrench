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
char WRCompilationContext::parseExpression( WRExpression& expression )
{
	int depth = 0;
	char end = 0;

	m_newHashValue = 0;

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
			if ( (uint8_t)value.type == WR_COMPILER_LITERAL_STRING )
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

		pushDebug( WRD_LineNumber, expression.bytecode, getSourcePosition() );
		
		if ( token == "var" )
		{
			expression.context[depth].varSeen = true;
			continue;
		}
		else if ( token == "{" )
		{
			depth = parseInitializer( expression, depth ) + 1;
			if ( depth == 0 )
			{
				return false;
			}

			continue;
		}
		
		if ( operatorFound(token, expression.context, depth) )
		{
			++depth;
			continue;
		}
		
		if ( !m_quoted && token == "new" )
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
			bool isLibConstant;

			if ( !getToken(expression.context[depth]) )
			{
				m_err = WR_ERR_bad_expression;
				return 0;
			}

			if ( !isValidLabel(token2, isGlobal, prefix, isLibConstant) || isLibConstant )
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

			if ( !m_quoted && token2 == ";" )
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}

				m_loadedToken = ";";
				m_loadedValue.p2 = INIT_AS_REF;
				m_loadedQuoted = m_quoted;
			}
			else if ( !m_quoted && token2 == "(" )
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
					m_loadedQuoted = m_quoted;
				}
			}
			else if (!m_quoted && token2 == "{")
			{
				if ( !parseCallFunction(expression, functionName, depth, false) )
				{
					return 0;
				}
				token2 = "{";
			}
			else if ( !m_quoted && token2 == "[" )
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

				if ( !getToken(expression.context[depth], "{") )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

				uint16_t initializer = 0;

				if ( !m_quoted && token3 == "{" )
				{
					for(;;)
					{
						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( !m_quoted && token3 == "}" )
						{
							break;
						}

						if ( !parseCallFunction(expression, functionName, depth, false) )
						{
							return 0;
						}

						if (!m_quoted && token3 != "{")
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
						wr_pack16( initializer, data );
						++initializer;

						pushData( expression.context[depth].bytecode, data, 2 );

						if ( !getToken(expression.context[depth]) )
						{
							m_err = WR_ERR_unexpected_EOF;
							return 0;
						}

						if ( !m_quoted && token3 == "," )
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
					m_loadedQuoted = m_quoted;
				}
				else if ( token2 != ";" )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}

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

		if ( !m_quoted && token == "(" )
		{
			// might be cast, call or sub-expression
			
			if ( (depth > 0) &&
				 (expression.context[depth - 1].type == EXTYPE_LABEL
				 || expression.context[depth - 1].type == EXTYPE_LIB_CONSTANT) )
			{
				// always only a call
				expression.context[depth - 1].type = EXTYPE_LABEL;
				
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

				operatorFound( WRstr("@i"), expression.context, depth );
			}
			else if ( token == "float" )
			{
				if ( !getToken(expression.context[depth], ")") )
				{
					m_err = WR_ERR_bad_expression;
					return 0;
				}

				operatorFound( WRstr("@f"), expression.context, depth );
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
				m_loadedQuoted = m_quoted;
				if ( parseExpression(nex) != ')' )
				{
					m_err = WR_ERR_unexpected_token;
					return 0;
				}


				if ( depth > 0 && expression.context[depth - 1].operation
					 && expression.context[depth - 1].operation->opcode == O_Remove )
				{
					--depth;
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( WRstr("._remove"), expression.context, depth );
				}
				else if ( depth > 0 && expression.context[depth - 1].operation
						  && expression.context[depth - 1].operation->opcode == O_HashEntryExists )
				{
					--depth;
					expression.context[depth].bytecode = nex.bytecode;
					operatorFound( WRstr("._exists"), expression.context, depth );
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

		if ( !m_quoted && token == "[" )
		{
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
				operatorFound( WRstr("@[]"), expression.context, depth );
				expression.context[depth].bytecode.all.clear();
				expression.context[depth].bytecode.opcodes.clear();
				pushOpcode( expression.context[depth].bytecode, O_LiteralZero );
				pushOpcode( expression.context[depth].bytecode, O_InitArray );
			}
			else 
			{
				expression.context[depth].bytecode = nex.bytecode;
				operatorFound( WRstr("@[]"), expression.context, depth );
			}

			++depth;
			continue;
		}

		bool isGlobal;
		WRstr prefix;
		bool isLibConstant;

		if ( isValidLabel(token, isGlobal, prefix, isLibConstant) )
		{
			if ( (depth > 0) 
				&& ((expression.context[depth - 1].type == EXTYPE_LABEL) 
					|| (expression.context[depth - 1].type == EXTYPE_LITERAL)
					|| (expression.context[depth - 1].type == EXTYPE_LIB_CONSTANT)) )
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

				if ( m_parsingNew )
				{
					if ( token == "=" )
					{
						m_newHashValue = wr_hashStr( label );

						continue;
//						getToken(expression.context[depth]) );
//						--depth;
						
					}
				}
				
				if ( !m_quoted && token == ":" )
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
					m_loadedQuoted = m_quoted;
				}
			}
			

			expression.context[depth].type = isLibConstant ? EXTYPE_LIB_CONSTANT : EXTYPE_LABEL;
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

	if ( depth == 2
		 && expression.lValue
		 && expression.context[0].type == EXTYPE_LABEL
		 && expression.context[1].type == EXTYPE_OPERATION
		 && expression.context[1].operation->opcode == O_Index
		 && expression.context[1].bytecode.opcodes.size() > 0 )
	{
		unsigned int i = expression.context[1].bytecode.opcodes.size() - 1;
		unsigned int a = expression.context[1].bytecode.all.size() - 1;
		WROpcode o = (WROpcode)(expression.context[1].bytecode.opcodes[ i ]);
		
		switch( o )
		{
			case O_Index:
			{
				expression.context[1].bytecode.all[a] = O_InitArray;
				break;
			}

			case O_IndexLiteral16:
			{
				if (a > 1)
				{
					expression.context[1].bytecode.all[a - 2] = O_LiteralInt16;
					pushOpcode(expression.context[1].bytecode, O_InitArray);
				}
				break;
			}
			
			case O_IndexLiteral8:
			{
				if (a > 0)
				{
					expression.context[1].bytecode.all[a - 1] = O_LiteralInt8;
					pushOpcode(expression.context[1].bytecode, O_InitArray);
				}
				break;
			}

			default: break;
		}
	}

	resolveExpression( expression );

	return end;
}

#endif
