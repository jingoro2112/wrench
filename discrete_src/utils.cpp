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

//------------------------------------------------------------------------------
WRState* wr_newState( int stackSize )
{
	WRState* state = (WRState *)malloc( stackSize*sizeof(WRValue) + sizeof(WRState) );
	memset( (unsigned char*)state, 0, stackSize*sizeof(WRValue) + sizeof(WRState) );

	state->stackSize = stackSize;
	state->stack = (WRValue *)((unsigned char *)state + sizeof(WRState));

	state->globalRegistry.init( 0, SV_VOID_HASH_TABLE );

	return state;
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	while( w->contextList )
	{
		wr_destroyContext( w->contextList );
	}

	w->globalRegistry.clear();

	free( w );
}

//------------------------------------------------------------------------------
WRError wr_getLastError( WRState* w )
{
	return (WRError)w->err;
}

//------------------------------------------------------------------------------
WRContext* wr_allocateNewScript( WRState* w, const unsigned char* block, const int blockSize )
{
	// CRC the code block, at least is it what the compiler intended?
	const unsigned char* p = block + (blockSize - 4);
	uint32_t hash = READ_32_FROM_PC( p );
	if ( hash != wr_hash(block, (blockSize - 4)) )
	{
		w->err = WR_ERR_bad_bytecode_CRC;
		return 0;
	}

	int needed = sizeof(WRContext) // class
				 + block[0] * sizeof(WRFunction) // local functions
				 + block[1] * sizeof(WRValue);  // globals
				 
	WRContext* C = (WRContext *)malloc( needed );

	memset((char*)C, 0, needed);

	C->globals = block[1];
	C->w = w;

	C->localFunctions = (WRFunction*)((unsigned char *)C + sizeof(WRContext) + block[1] * sizeof(WRValue));

	C->registry.init( 0, SV_VOID_HASH_TABLE );
	C->registry.m_vNext = w->contextList;
	C->bottom = block;
	C->bottomSize = blockSize;

	return (w->contextList = C);
}

//------------------------------------------------------------------------------
bool wr_executeFunctionZero( WRState* w, WRContext* context )
{
	if ( context->stopLocation )
	{
		w->err = WR_ERR_execute_function_zero_called_more_than_once;
		return false;
	}
	
	if ( wr_callFunction(w, context, (int32_t)0) ) 
	{
		wr_destroyContext( context );
		return false;
	}
	
	return true;
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize )
{
	WRContext* C = wr_allocateNewScript( w, block, blockSize );

	if ( C && !wr_executeFunctionZero(w, C) )
	{
		C = 0;
	}

	return C;
}


//------------------------------------------------------------------------------
void wr_destroyContext( WRContext* context )
{
	if ( !context )
	{
		return;
	}

	WRContext* prev = 0;

	// unlink it
	for( WRContext* c = context->w->contextList; c; c = (WRContext*)c->registry.m_vNext )
	{
		if ( c == context )
		{
			if ( prev )
			{
				prev->registry.m_vNext = c->registry.m_vNext;
			}
			else
			{
				context->w->contextList = (WRContext*)context->w->contextList->registry.m_vNext;
			}

			while ( context->svAllocated )
			{
				WRGCObject* next = context->svAllocated->m_next;
				context->svAllocated->clear();
				free( context->svAllocated );
				context->svAllocated = next;
			}

			context->registry.clear();

			free( context );

			break;
		}
		prev = c;
	}
}

//------------------------------------------------------------------------------
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRValue* V = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(name) );
	V->usr = usr;
	V->ccb = function;
}

//------------------------------------------------------------------------------
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function )
{
	w->globalRegistry.getAsRawValueHashTable(wr_hashStr(signature))->lcb = function;
}

//------------------------------------------------------------------------------
int WRValue::asInt() const
{
	if ( type == WR_INT )
	{
		return i;
	}
	else if ( type == WR_REF )
	{
		return r->asInt();
	}
	else if ( type == WR_FLOAT )
	{
		return (int)f;
	}
	else if ( IS_REFARRAY(xtype) )
	{
		WRValue temp;
		arrayValue( &temp );
		return temp.asInt();
	}

	return 0;
}

//------------------------------------------------------------------------------
float WRValue::asFloat() const
{
	if ( type == WR_FLOAT )
	{
		return f;
	}
	else if ( type == WR_REF )
	{
		return r->asFloat();
	}
	else if ( type == WR_INT )
	{
		return (float)i;
	}
	else if ( IS_REFARRAY(xtype) )
	{
		WRValue temp;
		arrayValue( &temp );
		return temp.asFloat();
	}

	return 0;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, size_t len ) const
{
	switch( type )
	{
		case WR_FLOAT: { wr_ftoa( f, string, len ); break; }
		case WR_INT: { wr_itoa( i, string, len ); break; }
		case WR_REF: { return r->asString( string, len ); }
		case WR_EX:
		{
			switch( xtype & EX_TYPE_MASK )
			{
				case WR_EX_ARRAY:
				{
					if ( va->m_type == SV_CHAR )
					{
						unsigned int s = 0;
						while( (string[s] = va->m_Cdata[s]) )
						{
							if ( s >= len )
							{
								string[s] = '\0';
								break;
							}
							
							++s;
						}
					}
					break;
				}

				case WR_EX_REFARRAY:
				{
					if ( IS_EXARRAY_TYPE(r->xtype) )
					{
						WRValue temp;
						wr_arrayToValue(this, &temp);
						return temp.asString(string, len);
					}
					else if ( IS_RAW_ARRAY(r->xtype) )
					{
						uint32_t i = ARRAY_ELEMENT_FROM_P2(p2);
						wr_itoa( (i < (uint32_t)(EX_RAW_ARRAY_SIZE_FROM_P2(r->p2))) ? (uint32_t)(unsigned char)(r->c[i]) : 0,
								 string, 
								 len);
					}
					else
					{
						return r->asString(string, len);
					}

					break;
				}

				case WR_EX_RAW_ARRAY:
				case WR_EX_STRUCT:
				case WR_EX_NONE:
				{
					string[0] = 0;
					break;
				}
			}
		}
	}

	return string;
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( w, context, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
int wr_callFunction( WRState* w, WRContext* context, const int32_t hash, const WRValue* argv, const int argn )
{
	WRValue* cF = 0;
	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			return w->err = WR_ERR_run_must_be_called_by_itself_first;
		}

		cF = context->registry.getAsRawValueHashTable( hash );
		if ( !cF->wrf )
		{
			return w->err = WR_ERR_wrench_function_not_found;
		}
	}

	return wr_callFunction( w, context, cF ? cF->wrf : 0, argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_returnValueFromLastCall( WRState* w )
{
	return w->stack; // this is where it ends up
}

//------------------------------------------------------------------------------
WRFunction* wr_getFunction( WRContext* context, const char* functionName )
{
	return context->registry.getAsRawValueHashTable(wr_hashStr(functionName))->wrf;
}

//------------------------------------------------------------------------------
WRValue* wr_getGlobalRef( WRContext* context, const char* label )
{
	char globalLabel[64] = "::";
	if ( !label )
	{
		return 0;
	}
	size_t len = strlen(label);
	uint32_t match;
	if ( len < 3 || (label[0] == ':' && label[1] == ':') )
	{
		match = wr_hashStr( label );
	}
	else
	{
		strncpy( globalLabel + 2, label, 61 );
		match = wr_hashStr( globalLabel );
	}
	
	// grab the globals block if it exists (check hash)
	if ( (int)((context->globals + 2) * sizeof(uint32_t)) > context->bottomSize )
	{
		return 0; // not enough room for globals to exist
	}

	const unsigned char* symbolsBlock = (context->bottom + (context->bottomSize -
											 (sizeof(uint32_t) // code hash
											  + sizeof(uint32_t) // symbols hash
											  + context->globals * sizeof(uint32_t)))); // globals
	
	uint32_t hash = READ_32_FROM_PC( symbolsBlock + context->globals*sizeof(uint32_t) );
	if ( hash != wr_hash( symbolsBlock, context->globals * sizeof(uint32_t) ) )
	{
		return 0; // bad CRC
	}

	for( unsigned int i=0; i<context->globals; ++i )
	{
		uint32_t symbolHash = READ_32_FROM_PC(symbolsBlock + i*sizeof(uint32_t));
		if ( match == symbolHash )
		{
			return ((WRValue *)(context + 1)) + i; // global space lives immediately past the context
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
void wr_makeInt( WRValue* val, int i )
{
	val->p2 = INIT_AS_INT;
	val->i = i;
}

//------------------------------------------------------------------------------
void wr_makeFloat( WRValue* val, float f )
{
	val->p2 = INIT_AS_FLOAT;
	val->f = f;
}

//------------------------------------------------------------------------------
void wr_makeString( WRContext* context, WRValue* val, const unsigned char* data, const int len )
{
	val->p2 = INIT_AS_ARRAY;
	val->va = (WRGCObject*)malloc( sizeof(WRGCObject) );
	val->va->init( len, SV_CHAR );
	val->va->m_skipGC = 1;
	memcpy( (unsigned char *)val->va->m_data, data, len );
}

//------------------------------------------------------------------------------
void wr_freeString( WRValue* val )
{
	val->va->clear();
	free( val->va );
}

//------------------------------------------------------------------------------
void wr_makeContainer( WRValue* val, const uint16_t sizeHint )
{
	val->p2 = INIT_AS_HASH_TABLE;
	val->va = (WRGCObject*)malloc( sizeof(WRGCObject) );
	val->va->init( sizeHint, SV_VOID_HASH_TABLE );
	val->va->m_skipGC = 1;
}

//------------------------------------------------------------------------------
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value )
{
	WRValue* entry = container->va->getAsRawValueHashTable( wr_hashStr(name) );
	entry->r = value;
	entry->p2 = INIT_AS_REF;
}

//------------------------------------------------------------------------------
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size )
{
	assert( size <= 0x1FFFFF );

	WRValue* entry = container->va->getAsRawValueHashTable( wr_hashStr(name) );
	entry->c = array;
	entry->p2 = INIT_AS_RAW_ARRAY | (size<<8);
}

//------------------------------------------------------------------------------
void wr_destroyContainer( WRValue* val )
{
	if ( val->xtype != WR_EX_HASH_TABLE )
	{
		return;
	}

	val->va->clear();
	free( val->va );
	val->init();
}

