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
#ifndef _OPCODE_H
#define _OPCODE_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROpcode
{
	O_Yield =0,

	O_LiteralInt32,
	O_LiteralZero,
	O_LiteralFloat,
	O_LiteralString,

	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,
	O_PushIndexFunctionReturnValue,

	O_CallLibFunction,
	O_CallLibFunctionAndPop,

	O_NewObjectTable,
	O_AssignToObjectTableByOffset,
	O_AssignToObjectTableByHash,

	O_AssignToHashTableAndPop,
	O_Remove,
	O_HashEntryExists,

	O_PopOne,
	O_ReturnZero,
	O_Return,
	O_Stop,

	O_Dereference,
	O_Index,
	O_IndexSkipLoad,
	O_CountOf,
	O_HashOf,

	O_StackIndexHash,
	O_GlobalIndexHash,
	O_LocalIndexHash,

	O_StackSwap,
	O_SwapTwoToTop,

	O_LoadFromLocal,
	O_LoadFromGlobal,

	O_LLValues,
	O_LGValues,
	O_GLValues,
	O_GGValues,

	O_BinaryRightShiftSkipLoad,
	O_BinaryLeftShiftSkipLoad,
	O_BinaryAndSkipLoad,
	O_BinaryOrSkipLoad,
	O_BinaryXORSkipLoad,
	O_BinaryModSkipLoad,

	O_BinaryMultiplication,
	O_BinarySubtraction,
	O_BinaryDivision,
	O_BinaryRightShift,
	O_BinaryLeftShift,
	O_BinaryMod,
	O_BinaryOr,
	O_BinaryXOR,
	O_BinaryAnd,
	O_BinaryAddition,

	O_BitwiseNOT,

	O_RelativeJump,
	O_RelativeJump8,

	O_BZ,
	O_BZ8,

	O_LogicalAnd,
	O_LogicalOr,
	O_CompareLE,
	O_CompareGE,
	O_CompareGT,
	O_CompareLT,
	O_CompareEQ,
	O_CompareNE,

	O_GGCompareGT,
	O_GGCompareGE,
	O_GGCompareLT,
	O_GGCompareLE,
	O_GGCompareEQ, 
	O_GGCompareNE, 

	O_LLCompareGT,
	O_LLCompareGE,
	O_LLCompareLT,
	O_LLCompareLE,
	O_LLCompareEQ, 
	O_LLCompareNE, 

	O_GSCompareEQ,
	O_LSCompareEQ, 
	O_GSCompareNE, 
	O_LSCompareNE, 
	O_GSCompareGE,
	O_LSCompareGE,
	O_GSCompareLE,
	O_LSCompareLE,
	O_GSCompareGT,
	O_LSCompareGT,
	O_GSCompareLT,
	O_LSCompareLT,

	O_GSCompareEQBZ, 
	O_LSCompareEQBZ, 
	O_GSCompareNEBZ, 
	O_LSCompareNEBZ, 
	O_GSCompareGEBZ,
	O_LSCompareGEBZ,
	O_GSCompareLEBZ,
	O_LSCompareLEBZ,
	O_GSCompareGTBZ,
	O_LSCompareGTBZ,
	O_GSCompareLTBZ,
	O_LSCompareLTBZ,

	O_GSCompareEQBZ8,
	O_LSCompareEQBZ8,
	O_GSCompareNEBZ8,
	O_LSCompareNEBZ8,
	O_GSCompareGEBZ8,
	O_LSCompareGEBZ8,
	O_GSCompareLEBZ8,
	O_LSCompareLEBZ8,
	O_GSCompareGTBZ8,
	O_LSCompareGTBZ8,
	O_GSCompareLTBZ8,
	O_LSCompareLTBZ8,

	O_LLCompareLTBZ,
	O_LLCompareLEBZ,
	O_LLCompareGTBZ,
	O_LLCompareGEBZ,
	O_LLCompareEQBZ,
	O_LLCompareNEBZ,

	O_GGCompareLTBZ,
	O_GGCompareLEBZ,
	O_GGCompareGTBZ,
	O_GGCompareGEBZ,
	O_GGCompareEQBZ,
	O_GGCompareNEBZ,

	O_LLCompareLTBZ8,
	O_LLCompareLEBZ8,
	O_LLCompareGTBZ8,
	O_LLCompareGEBZ8,
	O_LLCompareEQBZ8,
	O_LLCompareNEBZ8,

	O_GGCompareLTBZ8,
	O_GGCompareLEBZ8,
	O_GGCompareGTBZ8,
	O_GGCompareGEBZ8,
	O_GGCompareEQBZ8,
	O_GGCompareNEBZ8,

	O_PostIncrement,
	O_PostDecrement,
	O_PreIncrement,
	O_PreDecrement,

	O_PreIncrementAndPop,
	O_PreDecrementAndPop,

	O_IncGlobal,
	O_DecGlobal,
	O_IncLocal,
	O_DecLocal,

	O_Assign,
	O_AssignAndPop,
	O_AssignToGlobalAndPop,
	O_AssignToLocalAndPop,
	O_AssignToArrayAndPop,

	O_SubtractAssign,
	O_AddAssign,
	O_ModAssign,
	O_MultiplyAssign,
	O_DivideAssign,
	O_ORAssign,
	O_ANDAssign,
	O_XORAssign,
	O_RightShiftAssign,
	O_LeftShiftAssign,

	O_SubtractAssignAndPop,
	O_AddAssignAndPop,
	O_ModAssignAndPop,
	O_MultiplyAssignAndPop,
	O_DivideAssignAndPop,
	O_ORAssignAndPop,
	O_ANDAssignAndPop,
	O_XORAssignAndPop,
	O_RightShiftAssignAndPop,
	O_LeftShiftAssignAndPop,

	O_LogicalNot, //X
	O_Negate,

	O_LiteralInt8,
	O_LiteralInt16,

	O_IndexLiteral8,
	O_IndexLiteral16,

	O_IndexLocalLiteral8,
	O_IndexGlobalLiteral8,
	O_IndexLocalLiteral16,
	O_IndexGlobalLiteral16,

	O_BinaryAdditionAndStoreGlobal,
	O_BinarySubtractionAndStoreGlobal,
	O_BinaryMultiplicationAndStoreGlobal,
	O_BinaryDivisionAndStoreGlobal,

	O_BinaryAdditionAndStoreLocal,
	O_BinarySubtractionAndStoreLocal,
	O_BinaryMultiplicationAndStoreLocal,
	O_BinaryDivisionAndStoreLocal,

	O_CompareBEQ,
	O_CompareBNE,
	O_CompareBGE,
	O_CompareBLE,
	O_CompareBGT,
	O_CompareBLT,

	O_CompareBEQ8,
	O_CompareBNE8,
	O_CompareBGE8,
	O_CompareBLE8,
	O_CompareBGT8,
	O_CompareBLT8,

	O_BLA,
	O_BLA8,
	O_BLO,
	O_BLO8,

	O_LiteralInt8ToGlobal,
	O_LiteralInt16ToGlobal,
	O_LiteralInt32ToLocal,
	O_LiteralInt8ToLocal,
	O_LiteralInt16ToLocal,
	O_LiteralFloatToGlobal,
	O_LiteralFloatToLocal,
	O_LiteralInt32ToGlobal,

	O_GGBinaryMultiplication,
	O_GLBinaryMultiplication,
	O_LLBinaryMultiplication,

	O_GGBinaryAddition,
	O_GLBinaryAddition,
	O_LLBinaryAddition,

	O_GGBinarySubtraction,
	O_GLBinarySubtraction,
	O_LGBinarySubtraction,
	O_LLBinarySubtraction,

	O_GGBinaryDivision,
	O_GLBinaryDivision,
	O_LGBinaryDivision,
	O_LLBinaryDivision,

	O_GPushIterator,
	O_LPushIterator,
	O_GGNextKeyValueOrJump,
	O_GLNextKeyValueOrJump,
	O_LGNextKeyValueOrJump,
	O_LLNextKeyValueOrJump,
	O_GNextValueOrJump,
	O_LNextValueOrJump,

	O_Switch,
	O_SwitchLinear,

	O_GlobalStop,

	O_ToInt,
	O_ToFloat,

	O_LoadLibConstant,
	O_InitArray,
	O_InitVar,

	O_DebugInfo,
				
	// non-interpreted opcodes
	O_HASH_PLACEHOLDER,
	O_FUNCTION_CALL_PLACEHOLDER,

	O_LAST,
};

extern const char* c_opcodeName[];

#endif
