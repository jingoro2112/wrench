/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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
#ifndef _CC_H
#define _CC_H
#ifndef WRENCH_WITHOUT_COMPILER
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROperationType
{
	WR_OPER_PRE,
	WR_OPER_BINARY,
	WR_OPER_BINARY_COMMUTE, // binary but operation order doesn't matter
	WR_OPER_POST,
};

//------------------------------------------------------------------------------
struct WROperation
{
	const char* token;
	int precedence; // higher number if lower precedence
	WROpcode opcode;
	bool leftToRight;
	WROperationType type;
};

//------------------------------------------------------------------------------
// reference:
// https://en.cppreference.com/w/cpp/language/operator_precedence
const WROperation c_operations[] =
{
//       precedence                      L2R      type

	{ "==",  10, O_CompareEQ,           true,  WR_OPER_BINARY_COMMUTE },
	{ "!=",  10, O_CompareNE,           true,  WR_OPER_BINARY },
	{ ">=",   9, O_CompareGE,           true,  WR_OPER_BINARY },
	{ "<=",   9, O_CompareLE,           true,  WR_OPER_BINARY },
	{ ">",    9, O_CompareGT,           true,  WR_OPER_BINARY },
	{ "<",    9, O_CompareLT,           true,  WR_OPER_BINARY },
	{ "&&",  14, O_LogicalAnd,          true,  WR_OPER_BINARY_COMMUTE },
	{ "||",  15, O_LogicalOr,           true,  WR_OPER_BINARY_COMMUTE },

	{ "++",   3, O_PreIncrement,        true,  WR_OPER_PRE },
	{ "++",   2, O_PostIncrement,       true,  WR_OPER_POST },

	{ "--",   3, O_PreDecrement,        true,  WR_OPER_PRE },
	{ "--",   2, O_PostDecrement,       true,  WR_OPER_POST },

	{ ".",    2, O_IndexHash,           true,  WR_OPER_BINARY },

	{ "!",    3, O_LogicalNot,         false,  WR_OPER_PRE },
	{ "~",    3, O_BitwiseNOT,         false,  WR_OPER_PRE },
	{ "-",    3, O_Negate,             false,  WR_OPER_PRE },

	{ "+",    6, O_BinaryAddition,      true,  WR_OPER_BINARY_COMMUTE },
	{ "-",    6, O_BinarySubtraction,   true,  WR_OPER_BINARY },
	{ "*",    5, O_BinaryMultiplication,true,  WR_OPER_BINARY_COMMUTE },
	{ "/",    5, O_BinaryDivision,      true,  WR_OPER_BINARY },
	{ "%",    6, O_BinaryMod,           true,  WR_OPER_BINARY },

	{ "|",   13, O_BinaryOr,            true,  WR_OPER_BINARY_COMMUTE },
	{ "&",   11, O_BinaryAnd,           true,  WR_OPER_BINARY_COMMUTE },
	{ "^",   11, O_BinaryXOR,           true,  WR_OPER_BINARY_COMMUTE },

	{ ">>",   7, O_BinaryRightShift,    true,  WR_OPER_BINARY },
	{ "<<",   7, O_BinaryLeftShift,     true,  WR_OPER_BINARY },

	{ "+=",  16, O_AddAssign,           true,  WR_OPER_BINARY },
	{ "-=",  16, O_SubtractAssign,      true,  WR_OPER_BINARY },
	{ "%=",  16, O_ModAssign,           true,  WR_OPER_BINARY },
	{ "*=",  16, O_MultiplyAssign,      true,  WR_OPER_BINARY },
	{ "/=",  16, O_DivideAssign,        true,  WR_OPER_BINARY },
	{ "|=",  16, O_ORAssign,            true,  WR_OPER_BINARY },
	{ "&=",  16, O_ANDAssign,           true,  WR_OPER_BINARY },
	{ "^=",  16, O_XORAssign,           true,  WR_OPER_BINARY },
	{ ">>=", 16, O_RightShiftAssign,   false,  WR_OPER_BINARY },
	{ "<<=", 16, O_LeftShiftAssign,    false,  WR_OPER_BINARY },

	{ "=",   16, O_Assign,             false,  WR_OPER_BINARY },

	{ "@i",   3, O_CoerceToInt,         true,  WR_OPER_PRE },
	{ "@f",   3, O_CoerceToFloat,       true,  WR_OPER_PRE },
	{ "@[]",  2, O_Index,               true,  WR_OPER_POST },

	{ 0, 0, O_LAST, false, WR_OPER_PRE },
};
const int c_highestPrecedence = 17; // one higher than the highest entry above, things that happen absolutely LAST

//------------------------------------------------------------------------------
enum WRExpressionType
{
	EXTYPE_NONE =0,
	EXTYPE_LITERAL,
	EXTYPE_LABEL,
	EXTYPE_OPERATION,
	EXTYPE_RESOLVED,
	EXTYPE_BYTECODE_RESULT,

	EXTYPE_LVALUE = 0x1000, // LValue can be ANDED with anything to indicate it is a reference
};

//------------------------------------------------------------------------------
struct WRNamespaceLookup
{
	uint32_t hash; // hash of symbol
	WRarray<int> references; // where this symbol is referenced (loaded) in the bytecode

	WRNamespaceLookup() { reset(0); }
	void reset( uint32_t h )
	{
		hash = h;
		references.clear();
	}
};

//------------------------------------------------------------------------------
struct BytecodeJumpOffset
{
	int offset;
	WRarray<int> references;
};

//------------------------------------------------------------------------------
struct WRBytecode
{
	WRstr all;
	WRstr opcodes;
	WRarray<WRNamespaceLookup> localSpace;
	WRarray<WRNamespaceLookup> functionSpace;

	void invalidateOpcodeCache() { opcodes.clear(); }
	
	WRarray<BytecodeJumpOffset> jumpOffsetTargets;
	
	void clear() { all.clear(); opcodes.clear(); localSpace.clear(); jumpOffsetTargets.clear(); }
};

//------------------------------------------------------------------------------
struct WRExpressionContext
{
	WRExpressionType type;

	bool spaceBefore;
	bool spaceAfter;
	bool global;
	WRstr token;
	WRValue value;
	const WROperation* operation;
	
	int stackPosition;
	
	WRBytecode bytecode;

	WRExpressionContext() { reset(); }

	void setLocalSpace( WRarray<WRNamespaceLookup>& localSpace )
	{
		bytecode.localSpace.clear();
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
		type = EXTYPE_NONE;
	}

	WRExpressionContext* reset()
	{
		type = EXTYPE_NONE;

		spaceBefore = false;
		spaceAfter = false;
		global = false;
		stackPosition = -1;
		token.clear();
		value.init();
		bytecode.clear();

		return this;
	}
};

//------------------------------------------------------------------------------
struct WRExpression
{
	WRarray<WRExpressionContext> context;

	WRBytecode bytecode;

	//------------------------------------------------------------------------------
	void pushToStack( int index )
	{
		int highest = 0;
		for( unsigned int i=0; i<context.count(); ++i )
		{
			if ( context[i].stackPosition != -1 && (i != (unsigned int)index) )
			{
				++context[i].stackPosition;
				highest = context[i].stackPosition > highest ? context[i].stackPosition : highest;
			}
		}

		context[index].stackPosition = 0;

		// now compact the stack
		for( int h=0; h<highest; ++h )
		{
			unsigned int i = 0;
			bool found = false;
			for( ; i<context.count(); ++i )
			{
				if ( context[i].stackPosition == h )
				{
					found = true;
					break;
				}
			}

			if ( !found && i == context.count() )
			{
				for( unsigned int j=0; j<context.count(); ++j )
				{
					if ( context[j].stackPosition > h )
					{
						--context[j].stackPosition;
					}
				}
				--highest;
				--h;
			}
		}
	}

	//------------------------------------------------------------------------------
	void popFrom( int index )
	{
		context[index].stackPosition = -1;

		for( unsigned int i=0; i<context.count(); ++i )
		{
			if ( context[i].stackPosition != -1 )
			{
				--context[i].stackPosition;
			}
		}
	}

	//------------------------------------------------------------------------------
	void swapWithTop( int stackPosition );
	
	WRExpression() { reset(); }
	WRExpression( WRarray<WRNamespaceLookup>& localSpace )
	{
		reset();
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
	}

	void reset()
	{
		context.clear();
		bytecode.clear();
	}
};

//------------------------------------------------------------------------------
struct WRUnitContext
{
	uint32_t hash;
	int arguments;
	int offsetInBytecode;
	
	WRBytecode bytecode;

	WRUnitContext() { reset(); }
	void reset()
	{
		hash = 0;
		arguments = 0;
		offsetInBytecode = 0;
	}
};

//------------------------------------------------------------------------------
struct WRCompilationContext
{
public:
	WRError compile( const char* data, const int size, unsigned char** out, int* outLen );
	
private:
	
	bool isReserved( const char* token );
	bool isValidLabel( WRstr& token, bool& isGlobal );
	bool getToken( WRExpressionContext& ex, const char* expect =0 );
	char* pack16( int16_t i, char* buf )
	{
		*buf = (i>>8) & 0xFF;
		*(buf + 1) = i & 0xFF;
		return buf;
	}
	char* pack32( int32_t l, char* buf )
	{
		*buf = (l>>24) & 0xFF;
		*(buf + 1) = (l>>16) & 0xFF;
		*(buf + 2) = (l>>8) & 0xFF;
		*(buf + 3) = l & 0xFF;
		return buf;
	}

	friend struct WRExpression;
	static void pushOpcode( WRBytecode& bytecode, WROpcode opcode );
	static void pushData( WRBytecode& bytecode, const char* data, const int len ) { bytecode.all.append( data, len ); }

	int getBytecodePosition( WRBytecode& bytecode ) { return bytecode.all.size(); }
	
	int addRelativeJumpTarget( WRBytecode& bytecode );
	void setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget );
	void addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget );
	void resolveRelativeJumps( WRBytecode& bytecode );

	void appendBytecode( WRBytecode& bytecode, WRBytecode& addMe );
	
	void pushLiteral( WRBytecode& bytecode, WRValue& value );
	void addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly =false );
	void addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token );
	void addFunctionToHashSpace( WRBytecode& result, WRstr& token );
	void loadExpressionContext( WRExpression& expression, int depth, int operation );
	void resolveExpression( WRExpression& expression );
	void resolveExpressionEx( WRExpression& expression, int o, int p );

	bool operatorFound( WRstr& token, WRarray<WRExpressionContext>& context, int depth );
	char parseExpression( WRExpression& expression );
	bool parseUnit();
	bool parseWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseForLoop( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseIf( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn );
	
	void link( unsigned char** out, int* outLen );
	
	const char* m_source;
	int m_sourceLen;
	int m_pos;

	WRstr m_loadedToken;
	WRValue m_loadedValue;

	WRError m_err;
	bool m_EOF;

	int m_unitTop;
	WRarray<WRUnitContext> m_units;

	WRarray<int> m_continueTargets;
	WRarray<int> m_breakTargets;


/*

jumpOffsetTarget-> code
                   code
                   code
fill 1             jump to jumpOffset
                   code
                   code
fill 2             jump to jumpOffset

  jumpOffset is a list of fills

 bytecode has a list of jump offsets it added so it can increment them
  when appended

*/
};

//------------------------------------------------------------------------------
enum ScopeContextType
{
	Unit,
	Switch,
};

//------------------------------------------------------------------------------
struct ScopeContext
{
	int type;
};

#endif // WRENCH_WITHOUT_COMPILER

#endif
