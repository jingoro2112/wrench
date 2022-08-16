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

#include "wrench.h"

//------------------------------------------------------------------------------
WRValue* WRValue::asValueArray( int* len )
{
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_VALUE )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (WRValue*)va->m_data;
}

//------------------------------------------------------------------------------
unsigned char* WRValue::asCharArray( int* len )
{
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_CHAR )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (unsigned char*)va->m_data;
}

//------------------------------------------------------------------------------
int* WRValue::asIntArray( int* len )
{
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_INT )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}
	
	return (int*)va->m_data;
}

//------------------------------------------------------------------------------
float* WRValue::asFloatArray( int* len )
{
	if ( type != WR_ARRAY )
	{
		return 0;
	}

	if ( (va->m_type&0x3) != SV_FLOAT )
	{
		return 0;
	}

	if ( len )
	{
		*len = (int)va->m_size;
	}

	return (float*)va->m_data;
}

//------------------------------------------------------------------------------
WRState* wr_newState( int stackSize )
{
	return new WRState( stackSize );
}

//------------------------------------------------------------------------------
WRRunContext::WRRunContext( WRState* state ) : w(state)
{
	localFunctions = 0;
	globalSpace = 0;
	globals = 0;
	svAllocated = 0;
	stopLocation = 0;
	bottom = 0;
}

//------------------------------------------------------------------------------
WRRunContext::~WRRunContext()
{
	delete[] globalSpace;
	delete[] localFunctions;

	while( svAllocated )
	{
		WRStaticValueArray* next = svAllocated->m_next;
		delete svAllocated;
		svAllocated = next;
	}
}

//------------------------------------------------------------------------------
WRState::WRState( int EntriesInStack )
{
	contextIdGenerator = 0;
	
	err = WR_ERR_None;

	stackSize = EntriesInStack;
	stack = new WRValue[ stackSize ];
	for( int i=0; i<stackSize; ++i )
	{
		stack[i].init();
	}
	stackTop = stack;

	loader = 0;
	usr = 0;
	returnValue = 0;
	contextList = 0;
}

//------------------------------------------------------------------------------
WRState::~WRState()
{
	while( contextList )
	{
		WRRunContext* next = contextList->next;
		delete contextList;
		contextList = next;
	}

	for( int i=0; i<stackSize; ++i )
	{
		stack[i].init();
	}
	delete[] stack;
}

//------------------------------------------------------------------------------
WRStaticValueArray* WRRunContext::getSVA( int size, WRStaticValueArrayType type )
{
	// gc before every alloc may seem a bit much but we want to be miserly
	if ( svAllocated )
	{
		// mark stack
		for( WRValue* s=w->stack; s<w->stackTop; ++s)
		{
			// an array in the chain?
			if ( s->type == WR_ARRAY && !(s->va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( s->va );
			}
		}
		
		// mark context's global
		for( int i=0; i<globals; ++i )
		{
			// an array in the chain?
			if ( globalSpace[i].type == WR_ARRAY && !(globalSpace[i].va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( globalSpace[i].va );
			}
		}

		// sweep
		WRStaticValueArray* current = svAllocated;
		WRStaticValueArray* prev = 0;
		while( current )
		{
			// if set, clear it
			if ( current->m_size & 0x40000000 )
			{
				current->m_size &= ~0x40000000;
				prev = current;
				current = current->m_next;
			}
			// otherwise nuke it as unreferenced
			else if ( prev == 0 )
			{
				svAllocated = current->m_next;
				delete current;
				current = svAllocated;
			}
			else
			{
				prev->m_next = current->m_next;
				delete current;
				current = prev->m_next;
			}
		}
	}

	
	WRStaticValueArray* ret = new WRStaticValueArray( size, type );
	if ( type == SV_VALUE )
	{
		WRValue *array = (WRValue *)ret->m_data;
		for( int i=0; i<size; ++i )
		{
			array[i].init();
		}
	}

	ret->m_next = svAllocated;
	svAllocated = ret;

	return ret;
}

//------------------------------------------------------------------------------
void WRRunContext::gcArray( WRStaticValueArray* sva )
{
	sva->m_size |= 0x40000000;

	if ( (sva->m_type&0x3) == SV_VALUE )
	{
		// this is an array of values, check them for array-ness too
		
		WRValue* top = (WRValue*)sva->m_data + (sva->m_size & ~0x40000000);
		for( WRValue* i = (WRValue*)sva->m_data; i<top; ++i )
		{
			if ( i->type == WR_ARRAY && !(i->va->m_type & SV_PRE_ALLOCATED) )
			{
				gcArray( i->va );
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	delete w;
}

//------------------------------------------------------------------------------
unsigned int wr_loadSingleBlock( int offset, const unsigned char** block, void* usr )
{
	*block = (unsigned char *)usr;
	return 0xFFFFFFF; // larger than any bytecode possible
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return w->err;
}

//------------------------------------------------------------------------------
int wr_runEx( WRState* w )
{
	int context_id = ++w->contextIdGenerator;
	if ( context_id == 0 ) // catch wraparound
	{
		++context_id;
	}
	
	WRRunContext* C = new WRRunContext( w );
	C->next = w->contextList;
	w->contextList = C;
	
	w->contexts.set( context_id, C );

	if ( wr_callFunction(w, context_id, (int32_t)0) )
	{
		w->contextIdGenerator--;
		delete C;
		w->contexts.remove( context_id );
		return -1;
	}

	return context_id;
}

//------------------------------------------------------------------------------
int wr_run( WRState* w, const unsigned char* block, const int size )
{
	w->loader = wr_loadSingleBlock;
	w->usr = (void*)block;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
int wr_run( WRState* w, WR_LOAD_BLOCK_FUNC loader, void* usr )
{
	w->loader = loader;
	w->usr = usr;
	return wr_runEx( w );
}

//------------------------------------------------------------------------------
void wr_destroyContext( WRState* w, const int contextId )
{
	WRRunContext* context;
	if ( !contextId || !(context = w->contexts.getItem(contextId)) )
	{
		return;
	}

	w->contexts.remove( contextId );

	WRRunContext* prev = 0;

	// unlink it
	for( WRRunContext* c = w->contextList; c; c = c->next )
	{
		if ( c == context )
		{
			if ( prev )
			{
				prev->next = c->next;
			}
			else
			{
				w->contextList = w->contextList->next;
			}

			break;
		}
		prev = c;
	}

	delete context;
}

//------------------------------------------------------------------------------
int wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRCFunctionCallback callback;
	callback.usr = usr;
	callback.function = function;

	uint32_t hash = wr_hashStr( name );
	if ( !w->c_functionRegistry.set(hash, callback) )
	{
		return w->err = WR_ERR_hash_table_size_exceeded;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
char* wr_valueToString( WRValue const& value, char* string )
{
	return value.asString( string );
}


#ifdef SPRINTF_OPERATIONS
#include <stdio.h>
#endif
//------------------------------------------------------------------------------
char* WRValue::asString( char* string ) const
{
	switch( type )
	{
#ifdef SPRINTF_OPERATIONS
		case WR_INT: { sprintf( string, "%d", i ); break; }
		case WR_FLOAT: { sprintf( string, "%g", f ); break; }
#else
		case WR_INT: 
		case WR_FLOAT:
		{
			string[0] = 0;
			break;
		}
#endif
		case WR_REF:
		{
			if ( r->type == WR_ARRAY )
			{
				WRValue temp;
				arrayToValue( this, &temp );
				return temp.asString( string );
			}
			else
			{
				return r->asString( string );
			}
		}
		case WR_USR:
		{
			return string;
		}

		case WR_ARRAY:
		{
			unsigned int s = 0;

			for( ; s<va->m_size; ++s )
			{
				switch( va->m_type & 0x3)
				{
					case SV_VALUE: string[s] = ((WRValue *)va->m_data)[s].i; break;
					case SV_CHAR: string[s] = ((char *)va->m_data)[s]; break;
					default: break;
				}
			}
			string[s] = 0;
			break;
		}
	}
	
	return string;
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, const int contextId, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( w, contextId, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, const int contextId, const int32_t hash, const WRValue* argv, const int argn )
{
	const unsigned char* pc;
	WRValue* tempValue;
	WRValue* tempValue2;
	WRValue* frameBase = 0;
	WRFunctionRegistry* F;
	unsigned char args;
	WRValue* stackTop = w->stack;
#ifdef _WIN32
	WROpcode opcode;
#endif

	w->err = WR_ERR_None;
	
	WRRunContext* context = w->contexts.getItem( contextId );
	if ( !context )
	{
		return w->err = WR_ERR_context_not_found;
	}
	
	union
	{
		WRVoidFunc (*voidFunc)[5];
		WRReturnFunc (*returnFunc)[5];
		WRTargetFunc (*targetFunc)[5];
	};

#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
	if ( !(pc = context->bottom) )
	{
		w->loader( 0, &pc, w->usr );
		context->bottom = pc;
	}
#else
	// cache these values they are used a lot
	WR_LOAD_BLOCK_FUNC loader = w->loader;
	void* usr = w->usr;
	const unsigned char* top = 0;
	pc = 0;
	int absoluteBottom = 0; // pointer to where in the codebase out bottom actually points to
#endif
		
	if ( hash )
	{
		// if a function name is provided it is meant to be called
		// directly. Set that up with a return vector of "stop"
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_execute_must_be_called_by_itself_first;
		}

		F = context->localFunctionRegistry.getItem( hash );

		if ( !F )
		{
			return w->err = WR_ERR_wrench_function_not_found;
		}
		
		args = 0;
		if ( argv && argn )
		{
			for( ; args < argn; ++args )
			{
				*stackTop++ = argv[args];
			}
		}

#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
		pc = context->bottom + context->stopLocation;
#else
		unsigned int size = loader( absoluteBottom = context->stopLocation, &pc, usr );
		top = pc + (size - 6);
		context->bottom = pc;
#endif
		goto callFunction;
	}

	for(;;)
	{

		if ( w->err )
		{
			if ( w->err > WR_warning_enums_follow )
			{
				//if ( !w->ignoreWarnings )
				{
					// then do something I guess
				}
				//else
				{
				}
			}
			else
			{
				return w->err;
			}

			w->err = WR_ERR_None;
		}

#ifndef SINGLE_COMPLETE_BYTECODE_LOAD
		if ( pc >= top )
		{
			unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
			top = pc + (size - 6);
			context->bottom = pc;
		}
#endif

#ifdef _WIN32
		opcode = (WROpcode)*pc;
#endif

		D_OPCODE(printf( "s[%p] top[%p] size[%d] %d:%s\n", w->stack, stackTop, (int)(stackTop - w->stack), (int)*pc, c_opcodeName[*pc]));
		
		switch( *pc++)
		{
			case O_RegisterFunction:
			{
				int index = (stackTop - 5)->i;
				context->localFunctions[ index ].arguments = (stackTop - 4)->i;
				context->localFunctions[ index ].frameSpaceNeeded = (stackTop - 3)->i;
				context->localFunctions[ index ].hash = (stackTop - 2)->i;
				
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				context->localFunctions[index].offset = (stackTop - 1)->i + context->bottom; // absolute
#else
				context->localFunctions[index].offsetI = (stackTop - 1)->i; // relative
#endif
				
				context->localFunctions[ index ].frameBaseAdjustment = 2
																	   + context->localFunctions[ index ].frameSpaceNeeded
																	   + context->localFunctions[ index ].arguments;

				context->localFunctionRegistry.set( context->localFunctions[ index ].hash,
													context->localFunctions + index );
				stackTop -= 5;
				continue;
			}

			case O_FunctionListSize:
			{
				delete context->localFunctions;
				context->localFunctionRegistry.clear();
				context->localFunctions = new WRFunctionRegistry[ *pc++ ];
				continue;
			}

			case O_LiteralZero:
			{
				(stackTop++)->init();
				continue;
			}
			
			case O_LiteralInt8:
			{
				stackTop->type = WR_INT;
				(stackTop++)->i = (int32_t)(int8_t)*pc++;
				continue;
			}
			
			case O_LiteralFloat:
			{
				stackTop->type = WR_FLOAT;
				(stackTop++)->i = (((int32_t)*pc) << 24)
								  | (((int32_t)*(pc+1)) << 16)
								  | (((int32_t)*(pc+2)) << 8)
								  | ((int32_t)*(pc+3));
				pc += 4;
				continue;
			}
			case O_LiteralInt32:
			{
				stackTop->type = WR_INT;
				(stackTop++)->i = (((int32_t)*pc) << 24)
								  | (((int32_t)*(pc+1)) << 16)
								  | (((int32_t)*(pc+2)) << 8)
								  | ((int32_t)*(pc+3));
				pc += 4;
				continue;
			}

			case O_LiteralString:
			{
				int16_t len = (((int16_t)*pc)<<8) | (int16_t)*(pc + 1);
				pc += 2;
				stackTop->type = WR_ARRAY;
				stackTop->va = context->getSVA( len, SV_CHAR );
				
				for( int c=0; c<len; ++c )
				{
					((unsigned char *)stackTop->va->m_data)[c] = *pc++;

#ifndef SINGLE_COMPLETE_BYTECODE_LOAD
					if ( pc >= top )
					{
						unsigned int size = loader( absoluteBottom += (pc - context->bottom), &pc, usr );
						top = pc + (size - 6);
						context->bottom = pc;
					}
#endif
				}
				++stackTop;
				continue;
			}
			
			case O_ReserveFrame:
			{
				frameBase = stackTop;
				for( unsigned char i=0; i<*pc; ++i )
				{
					(++stackTop)->init();
				}
				++pc;
				continue;
			}

			case O_ReserveGlobalFrame:
			{
				delete[] context->globalSpace;
				context->globals = *pc++;
				context->globalSpace = new WRValue[ context->globals ];
				for( unsigned char i=0; i<context->globals; ++i )
				{
					context->globalSpace[i].init();
				}
				continue;
			}

			case O_LoadFromLocal:
			{
				stackTop->type = WR_REF;
				(stackTop++)->p = frameBase + *pc++;
				continue;
			}

			case O_LoadFromGlobal:
			{
				/*
				stackTop->type = WR_REF;
				(stackTop++)->p = w->stack + *pc++;
				*/
				stackTop->type = WR_REF;
				(stackTop++)->p = context->globalSpace + *pc++;

				continue;
			}

			case O_Index:
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_index[tempValue->type][tempValue2->type]( context, tempValue, tempValue2 );
				continue;
			}

			case O_IndexHash:
			{
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				wr_UserHash[ tempValue->type ]( tempValue, tempValue2, tempValue2->i );
				continue;
			}

			case O_BinaryMultiplication: { targetFunc = wr_binaryMultiply; goto targetFuncOp; }
			case O_BinarySubtraction: { targetFunc = wr_binarySubtract; goto targetFuncOp; }
			case O_BinaryDivision: { targetFunc = wr_binaryDivide; goto targetFuncOp; }
			case O_BinaryRightShift: { targetFunc = wr_binaryRightShift; goto targetFuncOp; }
			case O_BinaryLeftShift: { targetFunc = wr_binaryLeftShift; goto targetFuncOp; }
			case O_BinaryMod: { targetFunc = wr_binaryMod; goto targetFuncOp; }
			case O_BinaryOr: { targetFunc = wr_binaryOr; goto targetFuncOp; }
			case O_BinaryXOR: { targetFunc = wr_binaryXOR; goto targetFuncOp; }
			case O_BinaryAnd: { targetFunc = wr_binaryAnd; goto targetFuncOp; }
			case O_BinaryAddition:
			{
				targetFunc = wr_binaryAddition;
targetFuncOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				targetFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2, tempValue2 );
				continue;
			}

			case O_SubtractAssign: { voidFunc = wr_SubtractAssign; goto binaryTableOp; }
			case O_AddAssign: { voidFunc = wr_AddAssign; goto binaryTableOp; }
			case O_ModAssign: { voidFunc = wr_ModAssign; goto binaryTableOp; }
			case O_MultiplyAssign: { voidFunc = wr_MultiplyAssign; goto binaryTableOp; }
			case O_DivideAssign: { voidFunc = wr_DivideAssign; goto binaryTableOp; }
			case O_ORAssign: { voidFunc = wr_ORAssign; goto binaryTableOp; }
			case O_ANDAssign: { voidFunc = wr_ANDAssign; goto binaryTableOp; }
			case O_XORAssign: { voidFunc = wr_XORAssign; goto binaryTableOp; }
			case O_RightShiftAssign: { voidFunc = wr_RightShiftAssign; goto binaryTableOp; }
			case O_LeftShiftAssign: { voidFunc = wr_LeftShiftAssign; goto binaryTableOp; }
			case O_Assign:
			{
				voidFunc = wr_assign;
binaryTableOp:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				voidFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				continue;
			}

			case O_SubtractAssignAndPop: { voidFunc = wr_SubtractAssign; goto binaryTableOpAndPop; }
			case O_AddAssignAndPop: { voidFunc = wr_AddAssign; goto binaryTableOpAndPop; }
			case O_ModAssignAndPop: { voidFunc = wr_ModAssign; goto binaryTableOpAndPop; }
			case O_MultiplyAssignAndPop: { voidFunc = wr_MultiplyAssign; goto binaryTableOpAndPop; }
			case O_DivideAssignAndPop: { voidFunc = wr_DivideAssign; goto binaryTableOpAndPop; }
			case O_ORAssignAndPop: { voidFunc = wr_ORAssign; goto binaryTableOpAndPop; }
			case O_ANDAssignAndPop: { voidFunc = wr_ANDAssign; goto binaryTableOpAndPop; }
			case O_XORAssignAndPop: { voidFunc = wr_XORAssign; goto binaryTableOpAndPop; }
			case O_RightShiftAssignAndPop: { voidFunc = wr_RightShiftAssign; goto binaryTableOpAndPop; }
			case O_LeftShiftAssignAndPop: { voidFunc = wr_LeftShiftAssign; goto binaryTableOpAndPop; }
			case O_AssignAndPop:
			{
				voidFunc = wr_assign;
binaryTableOpAndPop:
				tempValue = --stackTop;
				tempValue2 = --stackTop;
				voidFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				continue;
			}

			case O_StackSwap:
			{
				WRValue v = *(stackTop - 1);
				unsigned char offset = *pc++;
				*(stackTop - 1) = *(stackTop - offset);
				*(stackTop - offset) = v;
				continue;
			}

			case O_CallFunctionByHash:
			{
				int32_t hash = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				args = *(pc+4); // which have already been pushed
				pc += 5;

				WRCFunctionCallback* cF = w->c_functionRegistry.get( hash );
				if ( cF )
				{
					if ( !args )
					{
						cF->function( w, 0, 0, *stackTop++, cF->usr );
					}
					else
					{
						stackTop->init();
						cF->function( w, stackTop - args, args, *stackTop, cF->usr );
						*(stackTop - args) = *stackTop;
						stackTop -= args - 1;
					}
				}
				else
				{
					w->err = WR_WARN_c_function_not_found;
					stackTop -= args;
					(stackTop++)->init(); // push a fake return value
				}
				continue;
			}

			case O_CallFunctionByHashAndPop:
			{
				// don't care about return value
				int32_t hash = (((int32_t)*pc) << 24)
							   | (((int32_t)*(pc+1)) << 16)
							   | (((int32_t)*(pc+2)) << 8)
							   | ((int32_t)*(pc+3));
				args = *(pc+4); // which have already been pushed
				pc += 6; // skip past the pop operation that is next in line (linking can't remove the instruction... yet)

				WRCFunctionCallback* cF = w->c_functionRegistry.get( hash );
				if ( cF )
				{
					if ( !args )
					{
						cF->function( w, 0, 0, *stackTop, cF->usr );
					}
					else
					{
						cF->function( w, stackTop - args, args, *stackTop, cF->usr );
						stackTop -= args;
					}
				}
				else
				{
					w->err = WR_WARN_c_function_not_found;
					stackTop -= args;
				}
				continue;
			}

			case O_CallFunctionByIndex:
			{
				F = context->localFunctions + *pc;
				pc += 4;
				args = *pc++;
callFunction:				
				// rectify arg count
				if ( args != F->arguments )
				{
					if ( args > F->arguments )
					{
						stackTop -= args - F->arguments; // poof
					}
					else
					{
						for( char a=args; a < F->arguments; ++a )
						{
							(stackTop++)->init();
						}
					}
				}

				// the arguments are at framepace 0, add more to make
				// up for the locals in the function
				for( int i=0; i<F->frameSpaceNeeded; ++i )
				{
					(stackTop++)->init();
				}

				// temp value contains return vector
				tempValue = stackTop++; // return vector
				tempValue->type = WR_INT;
				
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				tempValue->p = pc; // simple for big-blob case
				pc = F->offset;
#else
				// relative position in the code to return to
				tempValue->i = absoluteBottom + (pc - context->bottom);

				if ( F->offsetI >= absoluteBottom )
				{
					// easy, function is within this loaded block
					pc = context->bottom + (F->offsetI - absoluteBottom);
				}
				else
				{
					// less easy, have to load the new block
					unsigned int size = loader( absoluteBottom = F->offsetI, &pc, usr );
					top = pc + (size - 6);
					context->bottom = pc;
				}
#endif

				stackTop->type = WR_INT;
				(stackTop++)->p = frameBase; // very top is the old framebase

				// set the new frame base to the base arguments the function is expecting
				frameBase = stackTop - F->frameBaseAdjustment;

				continue;
			}

			case O_Stop:
			{
				// leave the global space allocated at the top alone so
				// functions can be called BACK when necessary, but
				// return the top of the stack which is where the
				// "return value" will be
				w->returnValue = stackTop--;
				context->stopLocation = (pc - 1) - context->bottom;
				return WR_ERR_None;
			}

			case O_Return:
			{
#ifdef SINGLE_COMPLETE_BYTECODE_LOAD
				// copy the return value
				pc = (unsigned char*)((stackTop - 3)->p); // grab new PC in case function clobbers it
#else
				int returnOffset = (stackTop - 3)->i;
				if ( returnOffset >= absoluteBottom )
				{
					// easy, function is within this loaded block
					pc = context->bottom + (returnOffset - absoluteBottom);
				}
				else
				{
					// less easy, have to load the new block
					unsigned int size = loader( absoluteBottom = returnOffset, &pc, usr );
					top = pc + (size - 6);
					context->bottom = pc;
				}

#endif
				if ( (--stackTop)->type == WR_REF )
				{
					frameBase->type = stackTop->r->type;
					frameBase->p = stackTop->r->p;
				}
				else
				{
					*frameBase = *stackTop; // copy return value down
				}
				
				tempValue = (WRValue*)(stackTop - 1)->p;
				stackTop = frameBase + 1;
				frameBase = tempValue;
				continue;
			}
			
			case O_PopOne:
			{
				--stackTop;
				continue;
			}

			case O_CoerceToInt:
			{
				break;
			}
			
			case O_CoerceToFloat:
			{
				break;
			}

			case O_RelativeJump:
			{
				int16_t offset = *pc;
				offset = (offset<<8) | *(pc+1);
				pc += offset;
				continue;
			}
			
			case O_BZ:
			{
				tempValue = --stackTop;
				if ( wr_ZeroCheck[tempValue->type](tempValue) )
				{
					int16_t offset = *pc;
					offset = (offset<<8) | *(pc+1);
					pc += offset;
					continue;
				}

				pc += 2;
				continue;
			}
			case O_BNZ:
			{
				tempValue = --stackTop;
				if ( wr_ZeroCheck[tempValue->type](tempValue) )
				{
					pc += 2;
					continue;
				}

				int16_t offset = *pc;
				offset = (offset<<8) | *(pc+1);
				pc += offset;
				continue;
			}

			case O_LogicalAnd: { returnFunc = wr_LogicalAnd; goto returnFuncNormal; }
			case O_LogicalOr: { returnFunc = wr_LogicalOr; goto returnFuncNormal; }
			case O_CompareLE: { returnFunc = wr_CompareGT; goto returnFuncInverted; }
			case O_CompareGE: { returnFunc = wr_CompareLT; goto returnFuncInverted; }
			case O_CompareGT: { returnFunc = wr_CompareGT; goto returnFuncNormal; }
			case O_CompareLT: { returnFunc = wr_CompareLT; goto returnFuncNormal; }
			case O_CompareEQ:
			{
				returnFunc = wr_CompareEQ;
returnFuncNormal:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)returnFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				continue;
			}
			
			case O_CompareNE:
			{
				returnFunc = wr_CompareEQ;
returnFuncInverted:
				tempValue = --stackTop;
				tempValue2 = stackTop - 1;
				tempValue2->i = (int)!returnFunc[tempValue->type][tempValue2->type]( tempValue, tempValue2 );
				tempValue2->type = WR_INT;
				continue;
			}

			case O_PostIncrement:
			{
				tempValue = stackTop - 1;
				wr_postinc[ tempValue->type ]( tempValue, tempValue );
				continue;
			}

			case O_PostDecrement:
			{
				tempValue = stackTop - 1;
				wr_postdec[ tempValue->type ]( tempValue, tempValue );
				continue;
			}
			
			case O_PreIncrement:
			{
				tempValue = stackTop - 1;
				wr_preinc[ tempValue->type ]( tempValue );
				continue;
			}
			
			case O_PreDecrement:
			{
				tempValue = stackTop - 1;
				wr_predec[ tempValue->type ]( tempValue );
				continue;
			}

			case O_LogicalNot:
			{
				tempValue = stackTop - 1;
				tempValue->i = wr_LogicalNot[ tempValue->type ]( tempValue );
				tempValue->type = WR_INT;
				continue;
			}

			case O_Negate:
			{
				tempValue = stackTop - 1;
				wr_negate[ tempValue->type ]( tempValue );
				continue;
			}

			default:
			{
				return w->err = WR_ERR_unknown_opcode;
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_makeInt( WRValue* val, int i )
{
	val->type = WR_INT;
	val->i = i;
}

//------------------------------------------------------------------------------
void wr_makeFloat( WRValue* val, float f )
{
	val->type = WR_FLOAT;
	val->f = f;
}

//------------------------------------------------------------------------------
void wr_makeUserData( WRValue* val )
{
	val->type = WR_USR;
	val->u = new WRUserData;
}

//------------------------------------------------------------------------------
void wr_addUserValue( WRValue* userData, const char* key, WRValue* value )
{
	userData->u->registerValue( key, value );
}

//------------------------------------------------------------------------------
void wr_addUserCharArray( WRValue* userData, const char* name, const unsigned char* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_CHAR, data );
}

//------------------------------------------------------------------------------
void wr_addUserIntArray( WRValue* userData, const char* name, const int* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_INT, data );
}

//------------------------------------------------------------------------------
void wr_addUserFloatArray( WRValue* userData, const char* name, const float* data, const int len )
{
	WRValue* val = userData->u->addValue( name );
	val->type = WR_ARRAY;
	val->va = new WRStaticValueArray( len, SV_FLOAT, data );
}

//------------------------------------------------------------------------------
void WRValue::free()
{
	if ( type == WR_USR )
	{
		delete u;
	}
	else if ( type == WR_ARRAY && (va->m_type & SV_PRE_ALLOCATED) )
	{
		delete va;
	}
}

