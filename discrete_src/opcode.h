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
#ifndef _OPCODE_H
#define _OPCODE_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WROpcode
{
	O_RegisterFunction = 0,
	O_FunctionListSize,
	
	O_LiteralZero,
	O_LiteralInt8,
	O_LiteralInt32,
	O_LiteralFloat,
	O_LiteralString,

	O_LoadLabel, // [label key:] push this label onto the stack
	O_CallFunctionByHash,
	O_CallFunctionByHashAndPop,
	O_CallFunctionByIndex,

	O_Index,
	O_IndexHash,
	
	O_Assign,
	O_AssignAndPop,
	O_StackSwap,

	O_ReserveFrame,
	O_ReserveGlobalFrame,

	O_LoadFromLocal,
	O_LoadFromGlobal,

	O_PopOne, // pop exactly one value from stack
	O_Return, // end of a unit
	O_Stop, // end of a program

	O_BinaryAddition,
	O_BinarySubtraction,
	O_BinaryMultiplication,
	O_BinaryDivision,
	O_BinaryMod,
	O_BinaryAnd,
	O_BinaryOr,
	O_BinaryXOR,
	O_BitwiseNOT,

	O_CoerceToInt,
	O_CoerceToString,
	O_CoerceToFloat,

	O_RelativeJump,
	O_BZ,
	O_BNZ,

	O_CompareEQ, 
	O_CompareNE, 
	O_CompareGE,
	O_CompareLE,
	O_CompareGT,
	O_CompareLT,

	O_PostIncrement, // a++
	O_PostDecrement, // a--
	O_PreIncrement, // ++a
	O_PreDecrement, // ++a
	O_Negate, // -a

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

	O_LogicalAnd, // &&
	O_LogicalOr, // ||
	O_LogicalNot, // !

	O_BitwiseRightShift,
	O_BitwiseLeftShift,

	O_LAST,
};

//#define DEBUG_OPCODE_NAMES
#ifdef DEBUG_OPCODE_NAMES
#define D_OPCODE(a) a
const char* c_opcodeName[]=
{
	"O_RegisterFunction",
	"O_FunctionListSize",
	"O_LiteralZero",
	"O_LiteralInt8",
	"O_LiteralInt32",
	"O_LiteralFloat",
	"O_LiteralString",
	"O_LoadLabel",
	"O_CallFunctionByHash",
	"O_CallFunctionByHashAndPop",
	"O_CallFunctionByIndex",
	"O_Index",
	"O_IndexHash",
	"O_Assign",
	"O_AssignAndPop",
	"O_StackSwap",
	"O_ReserveFrame",
	"O_ReserveGlobalFrame",
	"O_LoadFromLocal",
	"O_LoadFromGlobal",
	"O_PopOne",
	"O_Return",
	"O_Stop",
	"O_BinaryAddition",
	"O_BinarySubtraction",
	"O_BinaryMultiplication",
	"O_BinaryDivision",
	"O_BinaryMod",
	"O_BinaryAnd",
	"O_BinaryOr",
	"O_BinaryXOR",
	"O_BitwiseNOT",
	"O_CoerceToInt",
	"O_CoerceToString",
	"O_CoerceToFloat",
	"O_RelativeJump",
	"O_BZ",
	"O_BNZ",
	"O_CompareEQ",
	"O_CompareNE",
	"O_CompareGE",
	"O_CompareLE",
	"O_CompareGT",
	"O_CompareLT",
	"O_PostIncrement",
	"O_PostDecrement",
	"O_PreIncrement",
	"O_PreDecrement",
	"O_Negate",
	"O_SubtractAssign",
	"O_AddAssign",
	"O_ModAssign",
	"O_MultiplyAssign",
	"O_DivideAssign",
	"O_ORAssign",
	"O_ANDAssign",
	"O_XORAssign",
	"O_RightShiftAssign",
	"O_LeftShiftAssign",
	"O_SubtractAssignAndPop",
	"O_AddAssignAndPop",
	"O_ModAssignAndPop",
	"O_MultiplyAssignAndPop",
	"O_DivideAssignAndPop",
	"O_ORAssignAndPop",
	"O_ANDAssignAndPop",
	"O_XORAssignAndPop",
	"O_RightShiftAssignAndPop",
	"O_LeftShiftAssignAndPop",
	"O_LogicalAnd",
	"O_LogicalOr",
	"O_LogicalNot",
	"O_BitwiseRightShift",
	"O_BitwiseLeftShift",
};
#else
#define D_OPCODE(a)
#endif

#endif
