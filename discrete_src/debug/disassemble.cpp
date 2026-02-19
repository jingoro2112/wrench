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

#include "wrench.h"

#ifdef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, WRstr& out, const bool includeComments )
{
	(void)bytecode;
	(void)len;
	(void)includeComments;
	out = "disassembler unavailable in WRENCH_WITHOUT_COMPILER build\n";
}

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen )
{
	WRstr listing;
	wr_disassemble( bytecode, len, listing, true );
	listing.release( out, outLen );
}

#else

//------------------------------------------------------------------------------
bool wr_startsWith( const char* s, const char* p )
{
	while( *p )
	{
		if ( *s++ != *p++ )
		{
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
bool wr_endsWith( const char* s, const char* suffix )
{
	unsigned int ls = (unsigned int)strlen(s);
	unsigned int le = (unsigned int)strlen(suffix);
	return (ls >= le) && (strncmp(s + (ls - le), suffix, le) == 0);
}

//------------------------------------------------------------------------------
bool wr_contains( const char* s, const char* needle )
{
	return strstr(s, needle) ? true : false;
}

//------------------------------------------------------------------------------
const char* wr_safeOpcodeName( const uint8_t op )
{
	return (op < O_HASH_PLACEHOLDER) ? c_opcodeName[op] : "<meta>";
}

//------------------------------------------------------------------------------
int wr_operandSizeForOpcode( const uint8_t* opPtr, const uint8_t* end, const uint8_t op )
{
	switch( op )
	{
		case O_LiteralZero:
		case O_PushIndexFunctionReturnValue:
		case O_AssignToHashTableAndPop:
		case O_Remove:
		case O_HashEntryExists:
		case O_PopOne:
		case O_ReturnZero:
		case O_Return:
		case O_Stop:
		case O_Dereference:
		case O_Index:
		case O_IndexSkipLoad:
		case O_CountOf:
		case O_HashOf:
		case O_LLValues:
		case O_LGValues:
		case O_GLValues:
		case O_GGValues:
		case O_BinaryRightShiftSkipLoad:
		case O_BinaryLeftShiftSkipLoad:
		case O_BinaryAndSkipLoad:
		case O_BinaryOrSkipLoad:
		case O_BinaryXORSkipLoad:
		case O_BinaryModSkipLoad:
		case O_BinaryMultiplication:
		case O_BinarySubtraction:
		case O_BinaryDivision:
		case O_BinaryRightShift:
		case O_BinaryLeftShift:
		case O_BinaryMod:
		case O_BinaryOr:
		case O_BinaryXOR:
		case O_BinaryAnd:
		case O_BinaryAddition:
		case O_BitwiseNOT:
		case O_LogicalAnd:
		case O_LogicalOr:
		case O_CompareLE:
		case O_CompareGE:
		case O_CompareGT:
		case O_CompareLT:
		case O_CompareEQ:
		case O_CompareNE:
		case O_PostIncrement:
		case O_PostDecrement:
		case O_PreIncrement:
		case O_PreDecrement:
		case O_PreIncrementAndPop:
		case O_PreDecrementAndPop:
		case O_Assign:
		case O_AssignAndPop:
		case O_SubtractAssign:
		case O_AddAssign:
		case O_ModAssign:
		case O_MultiplyAssign:
		case O_DivideAssign:
		case O_ORAssign:
		case O_ANDAssign:
		case O_XORAssign:
		case O_RightShiftAssign:
		case O_LeftShiftAssign:
		case O_SubtractAssignAndPop:
		case O_AddAssignAndPop:
		case O_ModAssignAndPop:
		case O_MultiplyAssignAndPop:
		case O_DivideAssignAndPop:
		case O_ORAssignAndPop:
		case O_ANDAssignAndPop:
		case O_XORAssignAndPop:
		case O_RightShiftAssignAndPop:
		case O_LeftShiftAssignAndPop:
		case O_LogicalNot:
		case O_Negate:
		case O_ToInt:
		case O_ToFloat:
		case O_InitArray:
		{
			return 0;
		}

		case O_LiteralInt8:
		case O_IndexLiteral8:
		case O_GlobalStop:
		case O_Yield:
		case O_AssignToObjectTableByOffset:
		case O_StackSwap:
		case O_LoadFromLocal:
		case O_LoadFromGlobal:
		case O_AssignToGlobalAndPop:
		case O_AssignToLocalAndPop:
		case O_IncGlobal:
		case O_DecGlobal:
		case O_IncLocal:
		case O_DecLocal:
		case O_BinaryAdditionAndStoreGlobal:
		case O_BinarySubtractionAndStoreGlobal:
		case O_BinaryMultiplicationAndStoreGlobal:
		case O_BinaryDivisionAndStoreGlobal:
		case O_BinaryAdditionAndStoreLocal:
		case O_BinarySubtractionAndStoreLocal:
		case O_BinaryMultiplicationAndStoreLocal:
		case O_BinaryDivisionAndStoreLocal:
		case O_GSCompareEQ:
		case O_LSCompareEQ:
		case O_GSCompareNE:
		case O_LSCompareNE:
		case O_GSCompareGE:
		case O_LSCompareGE:
		case O_GSCompareLE:
		case O_LSCompareLE:
		case O_GSCompareGT:
		case O_LSCompareGT:
		case O_GSCompareLT:
		case O_LSCompareLT:
		{
			return 1;
		}

		case O_LiteralInt16:
		case O_IndexLiteral16:
		case O_AssignToArrayAndPop:
		case O_RelativeJump:
		case O_BZ:
		case O_CompareBEQ:
		case O_CompareBNE:
		case O_CompareBGE:
		case O_CompareBLE:
		case O_CompareBGT:
		case O_CompareBLT:
		case O_BLA:
		case O_BLO:
		case O_NewObjectTable:
		case O_DebugInfo:
		case O_RelativeJump8:
		case O_BZ8:
		case O_CompareBEQ8:
		case O_CompareBNE8:
		case O_CompareBGE8:
		case O_CompareBLE8:
		case O_CompareBGT8:
		case O_CompareBLT8:
		case O_BLA8:
		case O_BLO8:
		case O_LiteralInt8ToGlobal:
		case O_LiteralInt8ToLocal:
		case O_IndexLocalLiteral8:
		case O_IndexGlobalLiteral8:
		case O_SwapTwoToTop:
		case O_GPushIterator:
		case O_LPushIterator:
		{
			return 2;
		}

		case O_LiteralInt16ToGlobal:
		case O_LiteralInt16ToLocal:
		case O_IndexLocalLiteral16:
		case O_IndexGlobalLiteral16:
		case O_CallFunctionByIndex:
		{
			return 3;
		}

		case O_LiteralInt32:
		case O_LiteralFloat:
		case O_LoadLibConstant:
		case O_AssignToObjectTableByHash:
		case O_StackIndexHash:
		{
			return 4;
		}

		case O_LiteralInt32ToLocal:
		case O_LiteralInt32ToGlobal:
		case O_LiteralFloatToGlobal:
		case O_LiteralFloatToLocal:
		case O_CallFunctionByHash:
		case O_CallFunctionByHashAndPop:
		case O_CallLibFunction:
		case O_CallLibFunctionAndPop:
		case O_GlobalIndexHash:
		case O_LocalIndexHash:
		{
			return 5;
		}

		case O_LiteralString:
		{
			if ( opPtr + 2 > end )
			{
				return -1;
			}
			uint16_t l = (uint16_t)READ_16_FROM_PC(opPtr);
			return 2 + (int)l;
		}

		case O_Switch:
		{
			if ( opPtr + 2 > end )
			{
				return -1;
			}
			uint16_t mod = (uint16_t)READ_16_FROM_PC(opPtr);
			return 4 + (int)(mod * 6U);
		}

		case O_SwitchLinear:
		{
			if ( opPtr + 1 > end )
			{
				return -1;
			}
			uint8_t count = READ_8_FROM_PC(opPtr);
			return 3 + (int)(count * 2U);
		}

		default:
		{
			break;
		}
	}

	// Optimized families with regular encodings.
	const char* name = wr_safeOpcodeName(op);

	if ( wr_startsWith(name, "GG") || wr_startsWith(name, "LL") || wr_startsWith(name, "GL") || wr_startsWith(name, "LG") )
	{
		if ( wr_contains(name, "NextKeyValueOrJump") )
		{
			return 5; // idx, idx, dest, rel16
		}
		if ( wr_contains(name, "BZ8") )
		{
			return 4; // idx, idx, rel8, pad8
		}
		if ( wr_contains(name, "BZ") )
		{
			return 4; // idx, idx, rel16
		}
		return 2; // idx, idx
	}

	if ( wr_startsWith(name, "GS") || wr_startsWith(name, "LS") )
	{
		if ( wr_contains(name, "BZ8") )
		{
			return 3; // idx, rel8, pad8
		}
		if ( wr_contains(name, "BZ") )
		{
			return 3; // idx, rel16
		}
		return 1; // idx
	}

	if ( wr_startsWith(name, "GNextValueOrJump") || wr_startsWith(name, "LNextValueOrJump") )
	{
		return 4; // iterIdx, destIdx, rel16
	}

	if ( wr_startsWith(name, "LocalBZ8") || wr_startsWith(name, "GlobalBZ8") )
	{
		return 3; // idx, rel8, pad8
	}

	if ( wr_startsWith(name, "LocalBZ") || wr_startsWith(name, "GlobalBZ") )
	{
		return 3;
	}

	if ( wr_startsWith(name, "GGBinary") || wr_startsWith(name, "GLBinary") || wr_startsWith(name, "LGBinary") || wr_startsWith(name, "LLBinary") )
	{
		return 2;
	}

	return 0;
}

//------------------------------------------------------------------------------
void wr_appendBytes( WRstr& out, const uint8_t* p, int n )
{
	for( int i=0; i<n; ++i )
	{
		out.appendFormat( "%02X ", (unsigned int)p[i] );
	}
	for( int i=n; i<12; ++i )
	{
		out += "   ";
	}
}

//------------------------------------------------------------------------------
void wr_appendEscapedBytes( WRstr& out, const uint8_t* p, int n )
{
	for( int i=0; i<n; ++i )
	{
		uint8_t c = p[i];
		if ( c == '\\' || c == '"' )
		{
			out += '\\';
			out += (char)c;
		}
		else if ( c >= 32 && c < 127 )
		{
			out += (char)c;
		}
		else
		{
			out.appendFormat( "\\x%02X", (unsigned int)c );
		}
	}
}

//------------------------------------------------------------------------------
void wr_appendRawOperandHex( WRstr& ops, const uint8_t* opPtr, int opLen )
{
	for( int i=0; i<opLen; ++i )
	{
		if ( i )
		{
			ops += " ";
		}
		ops.appendFormat( "0x%02X", (unsigned int)opPtr[i] );
	}
}

//------------------------------------------------------------------------------
void wr_decodeOperands( const uint8_t op,
							const uint8_t* opPtr,
							int opLen,
							int instOffset,
							WRstr& ops,
							WRstr& note )
{
	const char* name = c_opcodeName[op];
	ops.clear();
	note.clear();

	if ( opLen <= 0 )
	{
		return;
	}

	if ( op == O_CallFunctionByHash || op == O_CallFunctionByHashAndPop || op == O_CallLibFunction || op == O_CallLibFunctionAndPop )
	{
		ops.appendFormat( "argc=0x%02X hash=0x%08X", (unsigned int)opPtr[0], (unsigned int)READ_32_FROM_PC(opPtr + 1) );
		return;
	}
	if ( op == O_CallFunctionByIndex )
	{
		ops.appendFormat( "argc=0x%02X fnIdx=0x%02X pcSkip=0x%02X",
						  (unsigned int)opPtr[0], (unsigned int)opPtr[1], (unsigned int)opPtr[2] );
		return;
	}
	if ( op == O_LoadLibConstant || op == O_AssignToObjectTableByHash || op == O_StackIndexHash )
	{
		ops.appendFormat( "hash=0x%08X", (unsigned int)READ_32_FROM_PC(opPtr) );
		return;
	}

	if ( op == O_GlobalIndexHash || op == O_LocalIndexHash )
	{
		char scope = (op == O_GlobalIndexHash) ? 'g' : 'l';
		ops.appendFormat( "%c[0x%02X] keyHash=0x%08X", scope, (unsigned int)opPtr[0], (unsigned int)READ_32_FROM_PC(opPtr + 1) );
		return;
	}

	if ( op == O_LiteralString && opLen >= 2 )
	{
		uint16_t l = (uint16_t)READ_16_FROM_PC(opPtr);
		ops.appendFormat( "len=0x%04X", (unsigned int)l );
		wr_appendEscapedBytes( note, opPtr + 2, (int)l );
		return;
	}

	if ( op == O_LiteralInt32 || op == O_LiteralInt32ToLocal || op == O_LiteralInt32ToGlobal )
	{
		if ( op == O_LiteralInt32 )
		{
			ops.appendFormat( "imm=0x%08X", (unsigned int)READ_32_FROM_PC(opPtr) );
		}
		else
		{
			char scope = (op == O_LiteralInt32ToGlobal) ? 'g' : 'l';
			ops.appendFormat( "%c[0x%02X] imm=0x%08X", scope, (unsigned int)opPtr[0], (unsigned int)READ_32_FROM_PC(opPtr + 1) );
		}
		return;
	}

	if ( op == O_LiteralFloat || op == O_LiteralFloatToGlobal || op == O_LiteralFloatToLocal )
	{
		union { uint32_t u; float f; } v;
		if ( op == O_LiteralFloat )
		{
			v.u = (uint32_t)READ_32_FROM_PC(opPtr);
			ops.appendFormat( "imm=0x%08X", (unsigned int)v.u );
		}
		else
		{
			v.u = (uint32_t)READ_32_FROM_PC(opPtr + 1);
			char scope = (op == O_LiteralFloatToGlobal) ? 'g' : 'l';
			ops.appendFormat( "%c[0x%02X] imm=0x%08X", scope, (unsigned int)opPtr[0], (unsigned int)v.u );
		}
		note.appendFormat( "float=%g", v.f );
		return;
	}

	if ( op == O_RelativeJump || op == O_BZ
		 || (wr_startsWith(name, "CompareB") && !wr_endsWith(name, "8"))
		 || (wr_startsWith(name, "BLA") && !wr_endsWith(name, "8"))
		 || (wr_startsWith(name, "BLO") && !wr_endsWith(name, "8")) )
	{
		int rel = (int)READ_16_FROM_PC(opPtr);
		ops.appendFormat( "rel=0x%04X ->0x%04X", (unsigned int)(uint16_t)rel, (unsigned int)(instOffset + 1 + rel) );
		return;
	}

	if ( op == O_RelativeJump8 || op == O_BZ8
		 || ((wr_startsWith(name, "CompareB") || wr_startsWith(name, "BLA") || wr_startsWith(name, "BLO")) && wr_endsWith(name, "8")) )
	{
		int rel8 = (int)(int8_t)READ_8_FROM_PC(opPtr);
		if ( opLen >= 2 )
		{
			ops.appendFormat( "rel8=0x%02X pad=0x%02X ->0x%04X",
							  (unsigned int)READ_8_FROM_PC(opPtr),
							  (unsigned int)READ_8_FROM_PC(opPtr + 1),
							  (unsigned int)(instOffset + 1 + rel8) );
		}
		else
		{
			ops.appendFormat( "rel8=0x%02X ->0x%04X",
							  (unsigned int)READ_8_FROM_PC(opPtr),
							  (unsigned int)(instOffset + 1 + rel8) );
		}
		return;
	}

	if ( op == O_LocalBZ || op == O_GlobalBZ )
	{
		int16_t rel = READ_16_FROM_PC(opPtr + 1);
		char scope = (op == O_GlobalBZ) ? 'g' : 'l';
		ops.appendFormat( "%c[0x%02X] rel=0x%04X ->0x%04X",
						  scope, (unsigned int)opPtr[0], (unsigned int)(uint16_t)rel, (unsigned int)(instOffset + 2 + rel) );
		return;
	}
	if ( op == O_LocalBZ8 || op == O_GlobalBZ8 )
	{
		int rel8 = (int)(int8_t)opPtr[1];
		char scope = (op == O_GlobalBZ8) ? 'g' : 'l';
		ops.appendFormat( "%c[0x%02X] rel8=0x%02X pad=0x%02X ->0x%04X",
						  scope, (unsigned int)opPtr[0], (unsigned int)opPtr[1], (unsigned int)opPtr[2], (unsigned int)(instOffset + 2 + rel8) );
		return;
	}

	if ( op == O_Switch && opLen >= 4 )
	{
		uint16_t buckets = (uint16_t)READ_16_FROM_PC(opPtr);
		int16_t d = READ_16_FROM_PC(opPtr + 2);
		ops.appendFormat( "buckets=0x%04X defaultRel=0x%04X ->0x%04X",
						  (unsigned int)buckets,
						  (unsigned int)(uint16_t)d,
						  (unsigned int)(instOffset + 1 + d) );
		return;
	}

	if ( op == O_SwitchLinear && opLen >= 3 )
	{
		uint8_t count = READ_8_FROM_PC(opPtr);
		int16_t d = READ_16_FROM_PC(opPtr + 1);
		ops.appendFormat( "count=0x%02X defaultRel=0x%04X ->0x%04X",
						  (unsigned int)count,
						  (unsigned int)(uint16_t)d,
						  (unsigned int)(instOffset + 2 + d) );
		return;
	}

	if ( wr_startsWith(name, "GG") || wr_startsWith(name, "LL") || wr_startsWith(name, "GL") || wr_startsWith(name, "LG") )
	{
		char a = (name[0] == 'G') ? 'g' : 'l';
		char b = (name[1] == 'G') ? 'g' : 'l';
		if ( wr_contains(name, "NextKeyValueOrJump") )
		{
			int16_t rel = READ_16_FROM_PC(opPtr + 3);
			ops.appendFormat( "%c[0x%02X] %c[0x%02X] g[0x%02X] rel=0x%04X ->0x%04X",
							  a, (unsigned int)opPtr[0], b, (unsigned int)opPtr[1], (unsigned int)opPtr[2],
							  (unsigned int)(uint16_t)rel, (unsigned int)(instOffset + 4 + rel) );
			return;
		}
		if ( wr_contains(name, "BZ") )
		{
			if ( wr_endsWith(name, "BZ8") )
			{
				int rel8 = (int)(int8_t)opPtr[2];
				ops.appendFormat( "%c[0x%02X] %c[0x%02X] rel8=0x%02X pad=0x%02X ->0x%04X",
								  a, (unsigned int)opPtr[0], b, (unsigned int)opPtr[1], (unsigned int)opPtr[2], (unsigned int)opPtr[3],
								  (unsigned int)(instOffset + 3 + rel8) );
			}
			else
			{
				int16_t rel = READ_16_FROM_PC(opPtr + 2);
				ops.appendFormat( "%c[0x%02X] %c[0x%02X] rel=0x%04X ->0x%04X",
								  a, (unsigned int)opPtr[0], b, (unsigned int)opPtr[1], (unsigned int)(uint16_t)rel,
								  (unsigned int)(instOffset + 3 + rel) );
			}
			return;
		}
		ops.appendFormat( "%c[0x%02X] %c[0x%02X]", a, (unsigned int)opPtr[0], b, (unsigned int)opPtr[1] );
		return;
	}

	if ( wr_startsWith(name, "GS") || wr_startsWith(name, "LS") )
	{
		char a = name[0];
		if ( wr_contains(name, "BZ") )
		{
			if ( wr_endsWith(name, "BZ8") )
			{
				int rel8 = (int)(int8_t)opPtr[1];
				ops.appendFormat( "%c[0x%02X] rel8=0x%02X pad=0x%02X ->0x%04X", a, (unsigned int)opPtr[0], (unsigned int)opPtr[1], (unsigned int)opPtr[2],
								  (unsigned int)(instOffset + 2 + rel8) );
			}
			else
			{
				int16_t rel = READ_16_FROM_PC(opPtr + 1);
				ops.appendFormat( "%c[0x%02X] rel=0x%04X ->0x%04X", a, (unsigned int)opPtr[0], (unsigned int)(uint16_t)rel,
								  (unsigned int)(instOffset + 2 + rel) );
			}
			return;
		}
		ops.appendFormat( "%c[0x%02X]", a, (unsigned int)opPtr[0] );
		return;
	}

	if ( wr_startsWith(name, "GNextValueOrJump") || wr_startsWith(name, "LNextValueOrJump") )
	{
		int16_t rel = READ_16_FROM_PC(opPtr + 2);
		ops.appendFormat( "%c[0x%02X] g[0x%02X] rel=0x%04X ->0x%04X",
						  name[0], (unsigned int)opPtr[0], (unsigned int)opPtr[1], (unsigned int)(uint16_t)rel,
						  (unsigned int)(instOffset + 3 + rel) );
		return;
	}

	if ( wr_startsWith(name, "Local") || wr_contains(name, "Local") || op == O_AssignToLocalAndPop || op == O_IncLocal || op == O_DecLocal )
	{
		if ( opLen >= 1 && (opLen == 1 || opLen == 2 || opLen == 3) )
		{
			if ( opLen == 1 ) ops.appendFormat( "l[0x%02X]", (unsigned int)opPtr[0] );
			else if ( opLen == 2 ) ops.appendFormat( "l[0x%02X] imm=0x%02X", (unsigned int)opPtr[0], (unsigned int)opPtr[1] );
			else ops.appendFormat( "l[0x%02X] imm=0x%04X", (unsigned int)opPtr[0], (unsigned int)READ_16_FROM_PC(opPtr + 1) );
			return;
		}
	}
	if ( wr_startsWith(name, "Global") || wr_contains(name, "Global") || op == O_AssignToGlobalAndPop || op == O_IncGlobal || op == O_DecGlobal )
	{
		if ( opLen >= 1 && (opLen == 1 || opLen == 2 || opLen == 3) )
		{
			if ( opLen == 1 ) ops.appendFormat( "g[0x%02X]", (unsigned int)opPtr[0] );
			else if ( opLen == 2 ) ops.appendFormat( "g[0x%02X] imm=0x%02X", (unsigned int)opPtr[0], (unsigned int)opPtr[1] );
			else ops.appendFormat( "g[0x%02X] imm=0x%04X", (unsigned int)opPtr[0], (unsigned int)READ_16_FROM_PC(opPtr + 1) );
			return;
		}
	}

	wr_appendRawOperandHex( ops, opPtr, opLen );
}

//------------------------------------------------------------------------------
void wr_disassembleRange( WRstr& out,
							  const uint8_t* base,
							  const uint8_t* begin,
							  const uint8_t* end,
							  const bool includeComments )
{
	const uint8_t* pc = begin;
	while( pc < end )
	{
		const uint8_t* inst = pc;
		uint8_t op = READ_8_FROM_PC(pc++);
		if ( op >= O_LAST || op >= O_HASH_PLACEHOLDER )
		{
			out.appendFormat( "  %04X: %02X                                  <non-opcode data>\n",
							  (unsigned int)(inst - base),
							  (unsigned int)op );
			out.appendFormat( "  ....: skipping raw bytes [0x%04X..0x%04X)\n",
							  (unsigned int)(inst - base),
							  (unsigned int)(end - base) );
			break;
		}

		int opLen = wr_operandSizeForOpcode( pc, end, op );
		if ( opLen < 0 || (pc + opLen) > end )
		{
			out.appendFormat( "  %04X: %02X                                  %-30s ; truncated\n",
							  (unsigned int)(inst - base),
							  (unsigned int)op,
							  wr_safeOpcodeName(op) );
			out.appendFormat( "  ....: skipping raw bytes [0x%04X..0x%04X)\n",
							  (unsigned int)(inst - base),
							  (unsigned int)(end - base) );
			break;
		}

		WRstr ops;
		WRstr note;
		wr_decodeOperands( op, pc, opLen, (int)(inst - base), ops, note );

		out.appendFormat( "  %04X: ", (unsigned int)(inst - base) );
		wr_appendBytes( out, inst, opLen + 1 );
		out.appendFormat( "%-30s ", wr_safeOpcodeName(op) );
		out.appendFormat( "%-44s", ops.c_str() );
		if ( includeComments && note.size() && op != O_LiteralString )
		{
			out.appendFormat( " ; %s", note.c_str() );
		}
		out += "\n";
		if ( op == O_LiteralString && note.size() )
		{
			out += "        string: \"";
			out += note;
			out += "\"\n";
		}

		pc += opLen;
	}
}

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, WRstr& out, const bool includeComments )
{
	out.clear();

	WRState* w = wr_newState();
	if ( !w )
	{
		out = "err: wr_newState() failed\n";
		return;
	}

	WRContext* context = wr_createContext( w, bytecode, (int)len, false );
	if ( !context )
	{
		out.appendFormat( "err: context failed (%s)\n", c_errStrings[wr_getLastError(w)] );
		wr_destroyState( w );
		return;
	}

	const uint8_t* codeEnd = context->bottom + context->bottomSize - 4; // trailing CRC
	uint8_t compilerFlags = READ_8_FROM_PC( context->bottom + 2 );
	out += "Wrench Bytecode Disassembly\n";
	out += "===========================\n";
	out.appendFormat( "globals : 0x%02X\n", (unsigned int)context->globals );
	out.appendFormat( "units   : 0x%02X\n", (unsigned int)context->numLocalFunctions );
	out.appendFormat( "flags   : 0x%02X\n", (unsigned int)compilerFlags );
	out.appendFormat( "code    : [0x%04X..0x%04X)\n\n",
					  (unsigned int)(context->codeStart - context->bottom),
					  (unsigned int)(codeEnd - context->bottom) );

	if ( compilerFlags & WR_INCLUDE_GLOBALS )
	{
		out += "Globals Table\n";
		out += "-------------\n";
		const uint8_t* gsym = context->bottom + 3 + (context->numLocalFunctions * WR_FUNCTION_CORE_SIZE);
		for( unsigned int g=0; g<context->globals; ++g, gsym += 4 )
		{
			out.appendFormat( "  g[0x%02X] hash=0x%08X\n", g, (unsigned int)READ_32_FROM_PC(gsym) );
		}
		out += "\n";
	}

	if ( context->numLocalFunctions )
	{
		out += "Function Table\n";
		out += "--------------\n";
		out += "  idx  hash         args frame baseAdj offset\n";
		for( int i=0; i<context->numLocalFunctions; ++i )
		{
			const WRFunction* f = context->localFunctions + i;
			out.appendFormat( "  0x%02X 0x%08X 0x%02X 0x%02X  0x%02X   0x%04X\n",
							  i,
							  (unsigned int)f->hash,
							  (unsigned int)f->arguments,
							  (unsigned int)f->frameSpaceNeeded,
							  (unsigned int)f->frameBaseAdjustment,
							  (unsigned int)f->functionOffset );
		}
		out += "\n";
	}

	const uint8_t* globalStart = context->codeStart;
	const uint8_t* globalEnd = codeEnd;
	for( int i=0; i<context->numLocalFunctions; ++i )
	{
		const uint8_t* candidate = context->bottom + context->localFunctions[i].functionOffset;
		if ( candidate > globalStart && candidate < globalEnd )
		{
			globalEnd = candidate;
		}
	}

	out.appendFormat( "unit global @0x%04X\n", (unsigned int)(globalStart - context->bottom) );
	wr_disassembleRange( out, context->bottom, globalStart, globalEnd, includeComments );
	out += "\n";

	for( int i=0; i<context->numLocalFunctions; ++i )
	{
		const WRFunction* f = context->localFunctions + i;
		const uint8_t* unitStart = context->bottom + f->functionOffset;
		const uint8_t* unitEnd = codeEnd;

		for( int j=0; j<context->numLocalFunctions; ++j )
		{
			const WRFunction* n = context->localFunctions + j;
			const uint8_t* candidate = context->bottom + n->functionOffset;
			if ( candidate > unitStart && candidate < unitEnd )
			{
				unitEnd = candidate;
			}
		}

		out.appendFormat( "unit %d hash=0x%08X args=%u frame=%u baseAdj=%u @0x%04X\n",
						  i,
						  (unsigned int)f->hash,
						  (unsigned int)f->arguments,
						  (unsigned int)f->frameSpaceNeeded,
						  (unsigned int)f->frameBaseAdjustment,
						  (unsigned int)(unitStart - context->bottom) );

		wr_disassembleRange( out, context->bottom, unitStart, unitEnd, includeComments );
		out += "\n";
	}

	wr_destroyState( w );
}

//------------------------------------------------------------------------------
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen )
{
	WRstr listing;
	wr_disassemble( bytecode, len, listing, true );
	listing.release( out, outLen );
}

#endif
