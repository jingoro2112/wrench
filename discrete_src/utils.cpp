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
unsigned char* wr_pack16( int16_t i, unsigned char* buf )
{
	*buf = i & 0xFF;
	*(buf + 1) = (i>>8) & 0xFF;
	return buf;
}

//------------------------------------------------------------------------------
unsigned char* wr_pack32( int32_t l, unsigned char* buf )
{
	*buf = l & 0xFF;
	*(buf + 1) = (l>>8) & 0xFF;
	*(buf + 2) = (l>>16) & 0xFF;
	*(buf + 3) = (l>>24) & 0xFF;
	return buf;
}

#ifdef WRENCH_BIG_ENDIAN
//------------------------------------------------------------------------------
int32_t wr_x32( const int32_t val )
{
	int32_t v = READ_32_FROM_PC( (const unsigned char *)&val );
	return v;
}

//------------------------------------------------------------------------------
int16_t wr_x16( const int16_t val )
{
	int16_t v = READ_16_FROM_PC( (const unsigned char *)&val );
	return v;
}
#endif

//------------------------------------------------------------------------------
int32_t* WrenchValue::makeInt()
{
	m_value->i = m_value->asInt();
	m_value->p2 = INIT_AS_INT;
	return &(m_value->i);
}

//------------------------------------------------------------------------------
float* WrenchValue::makeFloat()
{
	m_value->f = m_value->asFloat();
	m_value->p2 = INIT_AS_FLOAT;
	return &(m_value->f);
}

//------------------------------------------------------------------------------
WRValue* WrenchValue::asArrayMember( const int index )
{
	if ( !IS_ARRAY(m_value->xtype) ) 
	{
		// then make it one!
		m_value->p2 = INIT_AS_ARRAY;
		m_value->va = m_context->getSVA( index + 1, SV_VALUE, true );
	}
	else if ( index >= (int)m_value->va->m_size )
	{
		m_value->va = wr_growValueArray( m_value->va, index );
	}

	return (WRValue*)m_value->va->get( index );
}

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
bool wr_executeFunctionZero( WRContext* context )
{
	return wr_executeContext(context) ? true : false;
}

//------------------------------------------------------------------------------
WRContext* wr_newContext( WRState* w, const unsigned char* block, const int blockSize )
{
	// CRC the code block, at least is it what the compiler intended?
	const unsigned char* p = block + (blockSize - 4);
	uint32_t hash = READ_32_FROM_PC( p );
	if ( hash != wr_hash_read8(block, (blockSize - 4)) )
	{
		w->err = WR_ERR_bad_bytecode_CRC;
		return 0;
	}

	int needed = sizeof(WRContext) // class
				 + READ_8_FROM_PC(block) * sizeof(WRFunction) // local functions
				 + READ_8_FROM_PC(block+1) * sizeof(WRValue);  // globals
				 
	WRContext* C = (WRContext *)malloc( needed );

	memset((char*)C, 0, needed);

	C->globals = READ_8_FROM_PC(block+1);
	C->w = w;

	C->localFunctions = (WRFunction*)((unsigned char *)C + sizeof(WRContext) + C->globals * sizeof(WRValue));

	C->registry.init( 0, SV_VOID_HASH_TABLE );
	C->registry.m_vNext = w->contextList;
	C->bottom = block;
	C->bottomSize = blockSize;

	return (w->contextList = C);
}

//------------------------------------------------------------------------------
WRValue* wr_executeContext( WRContext* context )
{
	WRState* w = context->w;
	if ( context->stopLocation )
	{
		w->err = WR_ERR_execute_function_zero_called_more_than_once;
		return 0;
	}
	
	return wr_callFunction( context, (int32_t)0 );
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize )
{
	WRContext* C = wr_newContext( w, block, blockSize );

	if ( C && !wr_executeContext(C) )
	{
		wr_destroyContext( C );
		return 0;
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

#ifdef WRENCH_INCLUDE_DEBUG_CODE
	free( context->debugInterface ); // in case it had been created
#endif

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

			// free all memory allocations by forcing the gc to collect everything
			context->gcPauseCount = 0; 
			context->globals = 0;
			context->gc( 0 );

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
void wr_registerLibraryConstant( WRState* w, const char* signature, const WRValue& value )
{
	if ( value.p2 == WR_INT || value.p2 == WR_FLOAT )
	{
		WRValue* C = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(signature) );
		C->p2 = value.p2 | INIT_AS_LIB_CONST;
		C->p = value.p;
	}
}

//------------------------------------------------------------------------------
int WRValue::asInt() const
{
	if ( type == WR_INT )
	{
		return i;
	}
	else if ( type == WR_FLOAT )
	{
		return (int)f;
	}

	return singleValue().asInt();
}

//------------------------------------------------------------------------------
float WRValue::asFloat() const
{
	if ( type == WR_FLOAT )
	{
		return f;
	}
	else if ( type == WR_INT )
	{
		return (float)i;
	}

	return singleValue().asFloat();
}

//------------------------------------------------------------------------------
void WRValue::setInt( const int val )
{
	if ( type == WR_REF )
	{
		r->setInt( val );
	}
	else
	{
		p2 = INIT_AS_INT;
		i = val;
	}
}

//------------------------------------------------------------------------------
void WRValue::setFloat( const float val )
{
	if ( type == WR_REF )
	{
		r->setFloat( val );
	}
	else
	{
		p2 = INIT_AS_FLOAT;
		f = val;
	}
}

//------------------------------------------------------------------------------
void* WRValue::array( unsigned int* len, char* arrayType ) const
{
	if ( type == WR_REF )
	{
		return r->array( len, arrayType );
	}

	if ( (xtype != WR_EX_ARRAY) || (va->m_type != SV_CHAR) )
	{
		return 0;
	}

	if ( arrayType )
	{
		*arrayType = va->m_type;
	}

	if ( len )
	{
		*len = va->m_size;
	}

	return va->m_data;
}

//------------------------------------------------------------------------------
const char* WRValue::c_str( unsigned int* len ) const
{
	char arrayType = 0;
	void* ret = array( len, &arrayType );
	return (arrayType == SV_CHAR) ? (char*)ret : 0;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, size_t len ) const
{
	if ( type == WR_REF )
	{
		return r->asString( string, len );
	}
	else if ( type == WR_FLOAT )
	{
		wr_ftoa( f, string, len );
	}
	else if ( type == WR_INT )
	{
		wr_itoa( i, string, len );
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
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
	else
	{
		singleValue().asString( string, len ); // never give up, never surrender	
	}

	return string;
}

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, const char* functionName, const WRValue* argv, const int argn )
{
	return wr_callFunction( context, wr_hashStr(functionName), argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_callFunction( WRContext* context, const int32_t hash, const WRValue* argv, const int argn )
{
	WRValue* cF = 0;
	WRState* w = context->w;

	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			w->err = WR_ERR_run_must_be_called_by_itself_first;
			return 0;
		}

		cF = context->registry.getAsRawValueHashTable( hash );
		if ( !cF->wrf )
		{
			w->err = WR_ERR_wrench_function_not_found;
			return 0;
		}
	}

	return wr_callFunction( context, cF ? cF->wrf : 0, argv, argn );
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
	if ( hash != wr_hash_read8( symbolsBlock, context->globals * sizeof(uint32_t) ) )
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
WRValue& wr_makeInt( WRValue* val, int i )
{
	val->p2 = INIT_AS_INT;
	val->i = i;
	return *val;
}

//------------------------------------------------------------------------------
WRValue& wr_makeFloat( WRValue* val, float f )
{
	val->p2 = INIT_AS_FLOAT;
	val->f = f;
	return *val;
}

//------------------------------------------------------------------------------
WRValue& wr_makeString( WRContext* context, WRValue* val, const unsigned char* data, const int len )
{
	val->p2 = INIT_AS_ARRAY;
	val->va = (WRGCObject*)malloc( sizeof(WRGCObject) );
	val->va->init( len, SV_CHAR );
	val->va->m_skipGC = 1;
	memcpy( (unsigned char *)val->va->m_data, data, len );
	return *val;
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

#ifndef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
const char* wr_asciiDump( const void* d, unsigned int len, WRstr& str, int markByte )
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


#endif
