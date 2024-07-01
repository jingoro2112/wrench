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

#ifndef _CC_H
#define _CC_H
/*------------------------------------------------------------------------------*/

#ifndef WRENCH_WITHOUT_COMPILER

#define WR_COMPILER_LITERAL_STRING 0x10 

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
	WROpcode alt;
};

//------------------------------------------------------------------------------
struct NamespacePush
{
	int unit;
	int location;
	NamespacePush* next;
};

//------------------------------------------------------------------------------
// reference:
// https://en.cppreference.com/w/cpp/language/operator_precedence
const WROperation c_operations[] =
{
//       precedence                      L2R      type             alt

	{ "==",  10, O_CompareEQ,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "!=",  10, O_CompareNE,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ ">=",   9, O_CompareGE,           true,  WR_OPER_BINARY, O_CompareLE },
	{ "<=",   9, O_CompareLE,           true,  WR_OPER_BINARY, O_CompareGE },
	{ ">",    9, O_CompareGT,           true,  WR_OPER_BINARY, O_CompareLT },
	{ "<",    9, O_CompareLT,           true,  WR_OPER_BINARY, O_CompareGT },
	{ "&&",  14, O_LogicalAnd,          true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "||",  15, O_LogicalOr,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },

	{ "++",   3, O_PreIncrement,        true,  WR_OPER_PRE, O_LAST },
	{ "++",   2, O_PostIncrement,       true,  WR_OPER_POST, O_LAST },

	{ "--",   3, O_PreDecrement,        true,  WR_OPER_PRE, O_LAST },
	{ "--",   2, O_PostDecrement,       true,  WR_OPER_POST, O_LAST },

	{ ".",    2, O_HASH_PLACEHOLDER,    true,  WR_OPER_BINARY, O_LAST },

	{ "!",    3, O_LogicalNot,         false,  WR_OPER_PRE, O_LAST },
	{ "~",    3, O_BitwiseNOT,         false,  WR_OPER_PRE, O_LAST },
	{ "-",    3, O_Negate,             false,  WR_OPER_PRE, O_LAST },

	{ "+",    6, O_BinaryAddition,      true,  WR_OPER_BINARY, O_LAST },
	{ "-",    6, O_BinarySubtraction,   true,  WR_OPER_BINARY, O_LAST },
	{ "*",    5, O_BinaryMultiplication,true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "/",    5, O_BinaryDivision,      true,  WR_OPER_BINARY, O_LAST },
	{ "%",    6, O_BinaryMod,           true,  WR_OPER_BINARY, O_LAST },

	{ "|",   13, O_BinaryOr,            true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "&",   11, O_BinaryAnd,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },
	{ "^",   11, O_BinaryXOR,           true,  WR_OPER_BINARY_COMMUTE, O_LAST },

	{ ">>",   7, O_BinaryRightShift,    true,  WR_OPER_BINARY, O_LAST },
	{ "<<",   7, O_BinaryLeftShift,     true,  WR_OPER_BINARY, O_LAST },

	{ "+=",  16, O_AddAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "-=",  16, O_SubtractAssign,      true,  WR_OPER_BINARY, O_LAST },
	{ "%=",  16, O_ModAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "*=",  16, O_MultiplyAssign,      true,  WR_OPER_BINARY, O_LAST },
	{ "/=",  16, O_DivideAssign,        true,  WR_OPER_BINARY, O_LAST },
	{ "|=",  16, O_ORAssign,            true,  WR_OPER_BINARY, O_LAST },
	{ "&=",  16, O_ANDAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ "^=",  16, O_XORAssign,           true,  WR_OPER_BINARY, O_LAST },
	{ ">>=", 16, O_RightShiftAssign,   false,  WR_OPER_BINARY, O_LAST },
	{ "<<=", 16, O_LeftShiftAssign,    false,  WR_OPER_BINARY, O_LAST },

	{ "=",   16, O_Assign,             false,  WR_OPER_BINARY, O_LAST },

	{ "@i",  3, O_ToInt,               false,  WR_OPER_PRE, O_LAST },
	{ "@f",  3, O_ToFloat,             false,  WR_OPER_PRE, O_LAST },

	{ "@[]",  2, O_Index,		        true,  WR_OPER_POST, O_LAST },
	{ "@init", 2, O_InitArray,          true,  WR_OPER_POST, O_LAST },

	{ "@macroBegin", 0, O_LAST,         true,  WR_OPER_POST, O_LAST },

	{ "._count", 2, O_CountOf,          true,  WR_OPER_POST, O_LAST },
	{ "._hash",  2, O_HashOf,           true,  WR_OPER_POST, O_LAST },
	{ "._remove", 2, O_Remove,			true,  WR_OPER_POST, O_LAST },
	{ "._exists", 2, O_HashEntryExists, true,  WR_OPER_POST, O_LAST },
	
	{ 0, 0, O_LAST, false, WR_OPER_PRE, O_LAST },
};
const int c_highestPrecedence = 17; // one higher than the highest entry above, things that happen absolutely LAST

//------------------------------------------------------------------------------
enum WRExpressionType
{
	EXTYPE_NONE =0,
	EXTYPE_LITERAL,
	EXTYPE_LIB_CONSTANT,
	EXTYPE_LABEL,
	EXTYPE_LABEL_AND_NULL,
	EXTYPE_OPERATION,
	EXTYPE_RESOLVED,
	EXTYPE_BYTECODE_RESULT,
};

//------------------------------------------------------------------------------
struct WRNamespaceLookup
{
	uint32_t hash; // hash of symbol
	WRarray<int> references; // where this symbol is referenced (loaded) in the bytecode
	WRstr label;
	
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
	uint32_t gotoHash;
	
	BytecodeJumpOffset() : offset(0), gotoHash(0) {}
};

//------------------------------------------------------------------------------
struct GotoSource
{
	uint32_t hash;
	int offset;
};

//------------------------------------------------------------------------------
struct WRBytecode
{
	WROpcodeStream all;
	WROpcodeStream opcodes;

	bool isStructSpace;

	WRarray<WRNamespaceLookup> localSpace;
	WRarray<WRNamespaceLookup> functionSpace;
	WRarray<WRNamespaceLookup> unitObjectSpace;

	void invalidateOpcodeCache() { opcodes.clear(); }
	
	WRarray<BytecodeJumpOffset> jumpOffsetTargets;
	WRarray<GotoSource> gotoSource;
	
	void clear()
	{
		all.clear();
		opcodes.clear();
		isStructSpace = false;
		localSpace.clear();
		
		functionSpace.clear();
		unitObjectSpace.clear();
		
		jumpOffsetTargets.clear();
		gotoSource.clear();
		
		isStructSpace = false;
	}
};

//------------------------------------------------------------------------------
struct WRExpressionContext
{
	WRExpressionType type;

	bool spaceBefore;
	bool spaceAfter;
	bool global;
	bool varSeen;
	WRstr prefix;
	WRstr token;
	WRValue value;
	WRstr literalString;
	const WROperation* operation;
	
	int stackPosition;
	
	WRBytecode bytecode;

	WRExpressionContext() { reset(); }

	void setLocalSpace( WRarray<WRNamespaceLookup>& localSpace, bool isStructSpace )
	{
		bytecode.localSpace.clear();
		bytecode.isStructSpace = isStructSpace;
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
		type = EXTYPE_NONE;
	}

	WRExpressionContext* reset()
	{
		type = EXTYPE_NONE;
		varSeen = false;
		spaceBefore = false;
		spaceAfter = false;
		global = false;
		stackPosition = -1;
		token.clear();
		value.init();
		bytecode.clear();
		operation = 0;

		return this;
	}
};

//------------------------------------------------------------------------------
class WRExpression
{
public:
	WRarray<WRExpressionContext> context;

	WRBytecode bytecode;
	bool lValue;

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
	void swapWithTop( int stackPosition, bool addOpcodes =true );
	
	WRExpression() { reset(); }
	WRExpression( WRarray<WRNamespaceLookup>& localSpace, bool isStructSpace )
	{
		reset();
		bytecode.isStructSpace = isStructSpace;
		for( unsigned int l=0; l<localSpace.count(); ++l )
		{
			bytecode.localSpace.append().hash = localSpace[l].hash;
		}
	}

	void reset()
	{
		context.clear();
		bytecode.clear();
		lValue = false;
	}
};

//------------------------------------------------------------------------------
struct ConstantValue
{
	WRValue value;
	WRstr label;
	ConstantValue() { value.init(); }
};

//------------------------------------------------------------------------------
struct WRUnitContext
{
	WRstr name;
	uint32_t hash; // hashed name of this unit
	uint32_t arguments; // how many arguments it expects
	int offsetInBytecode; // where in the bytecode it resides

	bool exportNamespace;

	WRarray<ConstantValue> constantValues;

	int16_t offsetOfLocalHashMap;
	
	// the code that runs when it loads
	// the locals it has
	WRBytecode bytecode;

	int parentUnitIndex;
	
	WRUnitContext() { reset(); }
	void reset()
	{
		name = "";
		exportNamespace = false;
		hash = 0;
		arguments = 0;
		arguments = 0;
		parentUnitIndex = 0;
		constantValues.clear();
		offsetOfLocalHashMap = 0;
		bytecode.clear();
		parentUnitIndex = 0;
	}
};

//------------------------------------------------------------------------------
struct WRCompilationContext
{
public:
	WRError compile( const char* data,
					 const int size,
					 unsigned char** out,
					 int* outLen,
					 char* erroMsg,
					 const uint8_t compilerOptionFlags );

private:
	
	bool isReserved( const char* token );
	bool isValidLabel( WRstr& token, bool& isGlobal, WRstr& prefix, bool& isLibConstant );

	bool getToken( WRExpressionContext& ex, const char* expect =0 );

	static bool CheckSkipLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool CheckFastLoad( WROpcode opcode, WRBytecode& bytecode, int a, int o );
	static bool IsLiteralLoadOpcode( unsigned char opcode );
	static bool CheckCompareReplace( WROpcode LS, WROpcode GS, WROpcode ILS, WROpcode IGS, WRBytecode& bytecode, unsigned int a, unsigned int o );

	friend class WRExpression;
	static void pushOpcode( WRBytecode& bytecode, WROpcode opcode );
	static void pushData( WRBytecode& bytecode, const unsigned char* data, const int len ) { bytecode.all.append( data, len ); }
	static void pushData( WRBytecode& bytecode, const char* data, const int len ) { bytecode.all.append( (unsigned char*)data, len ); }

	int getBytecodePosition( WRBytecode& bytecode ) { return bytecode.all.size(); }

	bool m_addDebugSymbols;
	bool m_embedGlobalSymbols;
	bool m_embedSourceCode;
	bool m_needVar;
	bool m_exportNextUnit;
	
	uint16_t m_lastCode;
	uint16_t m_lastParam;
	void pushDebug( uint16_t code, WRBytecode& bytecode,int param );
	int getSourcePosition();
	int getSourcePosition( int& onLine, int& onChar, WRstr* line =0 );
	
	int addRelativeJumpTarget( WRBytecode& bytecode );
	void setRelativeJumpTarget( WRBytecode& bytecode, int relativeJumpTarget );
	void addRelativeJumpSourceEx( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget, const unsigned char* data, const int dataSize );
	void addRelativeJumpSource( WRBytecode& bytecode, WROpcode opcode, int relativeJumpTarget );
	void resolveRelativeJumps( WRBytecode& bytecode );

	void appendBytecode( WRBytecode& bytecode, WRBytecode& addMe );
	
	void pushLiteral( WRBytecode& bytecode, WRExpressionContext& context );
	void pushLibConstant( WRBytecode& bytecode, WRExpressionContext& context );
	int addLocalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen );
	int addGlobalSpaceLoad( WRBytecode& bytecode, WRstr& token, bool addOnly, bool varSeen );
	void addFunctionToHashSpace( WRBytecode& result, WRstr& token );
	void loadExpressionContext( WRExpression& expression, int depth, int operation );
	void resolveExpression( WRExpression& expression );
	unsigned int resolveExpressionEx( WRExpression& expression, int o, int p );

	bool operatorFound( WRstr const& token, WRarray<WRExpressionContext>& context, int depth );
	bool parseCallFunction( WRExpression& expression, WRstr functionName, int depth, bool parseArguments );
	bool pushObjectTable( WRExpressionContext& context, WRarray<WRNamespaceLookup>& localSpace, uint32_t hash );
	int parseInitializer( WRExpression& expression, int depth );
	char parseExpression( WRExpression& expression );
	bool parseUnit( bool isStruct, int parentUnitIndex );
	bool parseWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseDoWhile( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseForLoop( bool& returnCalled, WROpcode opcodeToReturn );
	bool lookupConstantValue( WRstr& prefix, WRValue* value =0 );
	bool parseEnum( int unitIndex );
	uint32_t getSingleValueHash( const char* end );
	bool parseSwitch( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseIf( bool& returnCalled, WROpcode opcodeToReturn );
	bool parseStatement( int unitIndex, char end, bool& returnCalled, WROpcode opcodeToReturn );

	void createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size );
	void link( unsigned char** out, int* outLen, const uint8_t compilerOptionFlags );

	const char* m_source;
	int m_sourceLen;
	int m_pos;

	bool getChar( char &c ) { c = m_source[m_pos++]; return m_pos < m_sourceLen; }
	bool checkAsComment( char lead );
	bool readCurlyBlock( WRstr& block );
	struct TokenBlock
	{
		WRstr data;
		TokenBlock* next;
	};

	WRstr m_loadedToken;
	WRValue m_loadedValue;
	bool m_loadedQuoted;
	
	WRError m_err;
	bool m_EOF;
	bool m_LastParsedLabel;
	bool m_parsingFor;
	bool m_parsingNew;
	bool m_quoted;

	uint32_t m_newHashValue;
	
	int m_unitTop;
	WRarray<WRUnitContext> m_units;

	WRarray<int> m_continueTargets;
	WRarray<int> m_breakTargets;

	int m_foreachHash;
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

//------------------------------------------------------------------------------
inline const char* wr_asciiDump( const void* d, unsigned int len, WRstr& str, int markByte =-1 )
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
			if ( i == (unsigned int)markByte )
			{
				str.shave(1);
				str.appendFormat( "[%02X]", (unsigned char)data[i] );
			}
			else
			{
				str.appendFormat( "%02X ", (unsigned char)data[i] );
			}
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

#endif // WRENCH_WITHOUT_COMPILER

#endif
