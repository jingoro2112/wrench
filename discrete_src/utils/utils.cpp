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

WR_ALLOC g_malloc = &malloc;
WR_FREE g_free = &free;
//------------------------------------------------------------------------------
void wr_setGlobalAllocator( WR_ALLOC wralloc, WR_FREE wrfree )
{
	g_malloc = wralloc;
	g_free = wrfree;
}

//------------------------------------------------------------------------------
void* wr_malloc( size_t size )
{
	return g_malloc( size );
}

//------------------------------------------------------------------------------
void wr_free( void* ptr )
{
	g_free( ptr );
}

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


// convert the value in-place and return a pointer to it
int32_t* makeInt();
float* makeFloat();

//------------------------------------------------------------------------------
int32_t* WrenchValue::Int()
{
	if ( m_value->type != WR_INT )
	{
		m_value->i = m_value->asInt();
		m_value->p2 = INIT_AS_INT;
	}
	return &(m_value->i);
}

//------------------------------------------------------------------------------
float* WrenchValue::Float()
{
	if ( m_value->type != WR_FLOAT )
	{
		m_value->f = m_value->asFloat();
		m_value->p2 = INIT_AS_FLOAT;
	}
	return &(m_value->f);
}

//------------------------------------------------------------------------------
WRValue* WrenchValue::asArrayMember( const int index )
{
	if ( !IS_ARRAY(m_value->xtype) ) 
	{
		// then make it one!
		m_value->va = m_context->getSVA( index + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !m_value->va )
		{
			m_value->p2 = INIT_AS_INT;
			return m_value;
		}
#endif
		m_value->p2 = INIT_AS_ARRAY;
	}
	else if ( index >= (int)m_value->va->m_size )
	{
		wr_growValueArray( m_value->va, index );
		m_context->allocatedMemoryHint += index * ((m_value->va->m_type == SV_CHAR) ? 1 : sizeof(WRValue));
	}

	return (WRValue*)m_value->va->get( index );
}

//------------------------------------------------------------------------------
WRState* wr_newState( int stackSize )
{
	WRState* w = (WRState *)g_malloc( sizeof(WRState) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !w ) { return 0; }
#endif
																		   
	memset( (unsigned char*)w, 0, sizeof(WRState) );

	w->stackSize = stackSize;
//	w->stack = (WRValue *)((unsigned char *)w + sizeof(WRState));

	w->globalRegistry.init( 0, SV_VOID_HASH_TABLE, false );

	return w;
}

//------------------------------------------------------------------------------
void wr_destroyState( WRState* w )
{
	while( w->libCleanupFunctions )
	{
		w->libCleanupFunctions->cleanupFunction( w, w->libCleanupFunctions->param );
		WRLibraryCleanup *next = w->libCleanupFunctions->next;
		g_free( w->libCleanupFunctions );
		w->libCleanupFunctions = next;
	}

	while( w->contextList )
	{
		wr_destroyContext( w->contextList );
	}

	w->globalRegistry.clear();

	g_free( w );
}

//------------------------------------------------------------------------------
bool wr_getYieldInfo( WRContext* context, int* args, WRValue** firstArg, WRValue** returnValue )
{
	if ( !context || !context->yield_pc )
	{
		return false;
	}

	if ( args )
	{
		*args = context->yieldArgs;
	}

	if ( firstArg )
	{
		*firstArg = context->yield_stackTop - context->yieldArgs;
	}

	if ( returnValue )
	{
		*returnValue = (context->flags & (uint8_t)WRC_ForceYielded) ? 0 : context->yield_stackTop;
	}

	return true;
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
WRContext* wr_createContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership, WRValue* stack )
{
	// CRC the code block, at least is it what the compiler intended?
	uint32_t hash = READ_32_FROM_PC(block + (blockSize - 4));
	if ( hash != wr_hash_read8(block, (blockSize - 4)) + WRENCH_VERSION_MAJOR )
	{
		w->err = WR_ERR_bad_bytecode_CRC;
		return 0;
	}

	int globals = READ_8_FROM_PC( block );
	int localFuncs = READ_8_FROM_PC(block  + 1); // how many?
	
	int needed = sizeof(WRContext) // class
				 + (globals * sizeof(WRValue))  // globals
				 + (localFuncs * sizeof(WRFunction)) // functions
				 + (stack ? 0 : (w->stackSize * sizeof(WRValue))); // stack

	WRContext* C = (WRContext *)g_malloc( needed );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !C )
	{
		w->err = WR_ERR_malloc_failed;
		return 0;
	}
#endif
	
	memset((char*)C, 0, needed);

	C->numLocalFunctions = localFuncs;
	C->localFunctions = (WRFunction *)((uint8_t *)(C + 1) + (globals * sizeof(WRValue)));

	C->globals = globals;
	
	C->stack = stack ? stack : (WRValue *)(C->localFunctions + localFuncs);

	C->allocatedMemoryLimit = WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT;

	C->flags |= takeOwnership ? WRC_OwnsMemory : 0;
	
	C->w = w;

	C->bottom = block;
	C->bottomSize = blockSize;

	C->codeStart = block + 3 + (C->numLocalFunctions * WR_FUNCTION_CORE_SIZE);

	uint8_t compilerFlags = READ_8_FROM_PC( block + 2 );

	if ( compilerFlags & WR_INCLUDE_GLOBALS )
	{
		C->codeStart += C->globals * sizeof(uint32_t); // these are lazily loaded, just skip over them for now
	}

	if ( (compilerFlags & WR_EMBED_DEBUG_CODE) || (compilerFlags & WR_EMBED_SOURCE_CODE) )
	{
		uint16_t symbolsSize = READ_16_FROM_PC( C->codeStart + 4 );
		C->codeStart += 6 + symbolsSize; // 6 = 4 for code hash + 2 for size
	}

	if ( compilerFlags & WR_EMBED_SOURCE_CODE )
	{
		C->codeStart += 4 + READ_32_FROM_PC( C->codeStart );		
	}

	C->registry.init( 0, SV_VOID_HASH_TABLE, false );

	int pos = 3;
	for( int i=0; i<C->numLocalFunctions; ++i )
	{
		C->localFunctions[i].namespaceOffset = READ_16_FROM_PC( block + pos );
		pos += 2;
		C->localFunctions[i].functionOffset = READ_16_FROM_PC( block + pos );
		pos += 2;
		C->localFunctions[i].hash = READ_32_FROM_PC( block + pos );
		pos += 3;
		C->localFunctions[i].arguments = READ_8_FROM_PC( block + ++pos );
		C->localFunctions[i].frameSpaceNeeded = READ_8_FROM_PC( block + ++pos );
		C->localFunctions[i].frameBaseAdjustment = READ_8_FROM_PC( block + ++pos );
		
		++pos;

		C->registry.getAsRawValueHashTable(C->localFunctions[i].hash)->wrf = C->localFunctions + i;
	}

	return C;
}

//------------------------------------------------------------------------------
WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* import = wr_createContext( context->w, block, blockSize, takeOwnership, context->stack );
	if ( !import )
	{
		return 0;
	}

	if ( !wr_callFunction(import, (WRFunction*)0) )
	{ 
		// imported context may not yield
		wr_destroyContext( context );
		return 0;
	}

	import->imported = context->imported;
	context->imported = import;

	// make this a circular linked list
	if ( import->imported == 0 )
	{
		import->imported = context;
	}

	return import;
}

//------------------------------------------------------------------------------
WRContext* wr_newContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* C = wr_createContext( w, block, blockSize, takeOwnership );
	if ( C )
	{
		C->registry.m_vNext = w->contextList;
		w->contextList = C;
	}
	return C;
}

//------------------------------------------------------------------------------
WRValue* wr_executeContext( WRContext* context )
{
	WRState* S = context->w;
	if ( context->stopLocation )
	{
		S->err = WR_ERR_execute_function_zero_called_more_than_once;
		return 0;
	}
	
	return wr_callFunction( context, (WRFunction*)0 );
}

//------------------------------------------------------------------------------
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership )
{
	WRContext* context = wr_newContext( w, block, blockSize, takeOwnership );

	if ( !context )
	{
		return 0;
	}

	if ( !wr_callFunction(context, (WRFunction*)0) )
	{
		if ( !context->yield_pc )
		{
			wr_destroyContext( context );
			context = 0;
		}
	}
		
	return context;
}

//------------------------------------------------------------------------------
void wr_destroyContextEx( WRContext* context )
{
	// g_free all memory allocations by forcing the gc to collect everything
	context->globals = 0;
	context->allocatedMemoryHint = context->allocatedMemoryLimit;
	context->gc( 0 );

	context->registry.clear();

	if ( context->flags & WRC_OwnsMemory )
	{
		g_free( (void*)(context->bottom) );
	}

	g_free( context );
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

			while( context->imported )
			{
				WRContext* next = context->imported->imported;
				wr_destroyContextEx( context->imported );

				context->imported = next == context ? 0 : next; // the list is circular, stop here!
			}

			wr_destroyContextEx( context );

			break;
		}

		prev = c;
	}
}

//------------------------------------------------------------------------------
bool wr_runCommand( WRState* w, const char* sourceCode, const int size )
{
	int len = size == -1 ? strlen(sourceCode) : size;

	unsigned char* outBytes;
	int outLen;
	if ( !wr_compile(sourceCode, len, &outBytes, &outLen) )
	{
		wr_runOnce( w, outBytes, outLen );
		g_free( outBytes );
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
void wr_setAllocatedMemoryGCHint( WRContext* context, const uint16_t bytes )
{
	context->allocatedMemoryLimit = bytes;
}

//------------------------------------------------------------------------------
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr )
{
	WRValue* V = w->globalRegistry.getAsRawValueHashTable( wr_hashStr(name) );
	if ( V != (void*)WRENCH_NULL_HASH )
	{
		V->usr = usr;
		V->ccb = function;
	}
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
bool WRValue::isString( int* len ) const
{
	WRValue& V = deref();
	if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_CHAR )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isWrenchArray( int* len ) const
{
	WRValue& V = deref();
	if ( IS_ARRAY(V.xtype) && V.va->m_type == SV_VALUE )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isRawArray( int* len ) const
{
	WRValue& V = deref();
	if ( IS_RAW_ARRAY(V.xtype) )
	{
		if ( len )
		{
			*len = EX_RAW_ARRAY_SIZE_FROM_P2(V.p2);
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
bool WRValue::isHashTable( int* len ) const
{
	WRValue& V = deref();
	if ( IS_HASH_TABLE(V.xtype) )
	{
		if ( len )
		{
			*len = V.va->m_size;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
WRValue* WRValue::indexArray( WRContext* context, const uint32_t index, const bool create )
{
	WRValue& V = deref();
	
	if ( !IS_ARRAY(V.xtype) || V.va->m_type != SV_VALUE )
	{
		if ( !create )
		{
			return 0;
		}

		V.va = context->getSVA( index + 1, SV_VALUE, true );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !V.va )
		{
			V.p2 = INIT_AS_INT;
			return &V;
		}
#endif

		V.p2 = INIT_AS_ARRAY;

	}

	if ( index >= V.va->m_size )
	{
		if ( !create )
		{
			return 0;
		}
		
		wr_growValueArray( V.va, index );
		context->allocatedMemoryHint += index * ((V.va->m_type == SV_CHAR) ? 1 : sizeof(WRValue));
	}

	return V.va->m_Vdata + index;
}

//------------------------------------------------------------------------------
WRValue* WRValue::indexHash( WRContext* context, const uint32_t hash, const bool create )
{
	WRValue& V = deref();
	
	if ( !IS_HASH_TABLE(V.xtype) )
	{
		if ( !create )
		{
			return 0;
		}

		V.va = context->getSVA( 0, SV_HASH_TABLE, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
		if ( !V.va )
		{
			V.p2 = INIT_AS_INT;
			return &V;
		}
#endif
		
		V.p2 = INIT_AS_HASH_TABLE;
	}

	return create ? (WRValue*)V.va->get(hash) : V.va->exists(hash, false);
}

//------------------------------------------------------------------------------
void* WRValue::array( unsigned int* len, char arrayType ) const
{
	WRValue& V = deref();
	
	if ( (V.xtype != WR_EX_ARRAY) || (V.va->m_type != arrayType) )
	{
		return 0;
	}

	if ( len )
	{
		*len = V.va->m_size;
	}

	return V.va->m_data;
}

//------------------------------------------------------------------------------
int WRValue::arraySize() const
{
	WRValue& V = deref();
	return V.xtype == WR_EX_ARRAY ? V.va->m_size : -1;
}

//------------------------------------------------------------------------------
int wr_technicalAsStringEx( char* string, const WRValue* value, size_t pos, size_t maxLen, bool valuesInHex )
{
	if ( pos >= maxLen )
	{
		return maxLen - 1;
	}
	
	if ( value->type == WR_REF )
	{
		strncpy( (string + pos), "ref: ", maxLen - pos );
		pos += 5;
		return wr_technicalAsStringEx( string, value->r, pos, maxLen, valuesInHex );
	}
	else if ( value->type == WR_FLOAT )
	{
		return pos + snprintf( string + pos, maxLen - pos, "%g", value->f );
	}
	else if ( value->type == WR_INT )
	{
		return pos + (valuesInHex ? snprintf(string + pos, maxLen - pos, "0x%08lX", (unsigned long int)value->ui )
			: snprintf(string + pos, maxLen - pos, "%ld", (long int)value->i));
				
	}
	else if ( value->xtype == WR_EX_ARRAY )
	{
		if ( value->va->m_type == SV_VALUE )
		{
			pos += snprintf( string + pos, maxLen - pos, "[ " );

			bool first = true;
			for( uint32_t i=0; pos<maxLen && i<value->va->m_size; ++i )
			{
				if ( !first )
				{
					pos += snprintf( string + pos, maxLen - pos, ", " );
				}
				first = false;
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + i, pos, maxLen, valuesInHex );
			}

			if ( pos >= maxLen )
			{
				return maxLen;
			}

			pos += snprintf( string + pos, maxLen - pos, " ]" );
		}
		else if ( value->va->m_type == SV_CHAR )
		{
			pos += snprintf( string + pos, maxLen - pos, "\"" );

			for( uint32_t i=0; pos<maxLen && i<value->va->m_size; ++i )
			{
				string[pos++] = value->va->m_SCdata[i];
			}

			if ( pos >= maxLen )
			{
				return maxLen;
			}

			pos += snprintf( string + pos, maxLen - pos, "\"" );
		}
		else
		{
			pos += snprintf( string + pos, maxLen - pos, "<raw array>" );
		}
	}
	else if ( value->xtype == WR_EX_HASH_TABLE )
	{
		pos += snprintf( string + pos, maxLen - pos, "{ " );
		
		bool first = true;
		for( int32_t element=0; element<value->va->m_mod; ++element )
		{
			if ( value->va->m_hashTable[element] != WRENCH_NULL_HASH )
			{
				if ( !first )
				{
					pos += snprintf( string + pos, maxLen - pos, ", " );
				}
				first = false;
				int32_t index = element << 1;
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + index + 1, pos, maxLen, valuesInHex );
				pos += snprintf( string + pos, maxLen - pos, ":" );
				pos = wr_technicalAsStringEx( string, value->va->m_Vdata + index, pos, maxLen, valuesInHex );
			}
		}

		pos += snprintf( string + pos, maxLen - pos, " }" );
	}

	return (pos >= maxLen) ? maxLen : pos;
}

//------------------------------------------------------------------------------
char* WRValue::technicalAsString( char* string, unsigned int maxLen, bool valuesInHex, unsigned int* strLen ) const
{
	unsigned int len = wr_technicalAsStringEx( string, this, 0, maxLen, valuesInHex );
	string[ len ] = 0;
	if ( strLen )
	{
		*strLen = len;
	}
	return string;
}

//------------------------------------------------------------------------------
char* WRValue::asMallocString( unsigned int* strLen ) const
{
	if ( type == WR_REF )
	{
		return r->asMallocString( strLen );
	}

	unsigned int len = 0;
	char* ret = 0;

	if ( type == WR_FLOAT )
	{
		ret = (char*)g_malloc( 10 );
		len = wr_ftoa( f, ret, 11 );
	}
	else if ( type == WR_INT )
	{
		ret = (char*)g_malloc( 13 );
		len = wr_itoa( i, ret, 12 );
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		ret = (char*)g_malloc( va->m_size + 1);
		memcpy( ret, va->m_Cdata, va->m_size );
		ret[va->m_size] = 0;
	}
	else
	{
		return singleValue().asMallocString( strLen );
	}

	if ( strLen )
	{
		*strLen = len;
	}

	return ret;
}

//------------------------------------------------------------------------------
char* WRValue::asString( char* string, unsigned int maxLen, unsigned int* strLen ) const
{
	if ( type == WR_REF )
	{
		return r->asString( string, maxLen, strLen );
	}

	unsigned int len = 0;
	
	if ( type == WR_FLOAT )
	{
		len = wr_ftoa( f, string, maxLen );
	}
	else if ( type == WR_INT )
	{
		len = wr_itoa( i, string, maxLen );
	}
	else if ( xtype == WR_EX_ARRAY && va->m_type == SV_CHAR )
	{
		len = maxLen ? (maxLen > va->m_size ? va->m_size : maxLen) : va->m_size;
		memcpy( string, va->m_Cdata, len );
		string[len] = 0;
	}
	else
	{
		return singleValue().asString( string, maxLen, strLen ); // never give up, never surrender	
	}

	if ( strLen )
	{
		*strLen = len;
	}
	
	return string;
}

//------------------------------------------------------------------------------
WRValue::Iterator::Iterator( WRValue const& V ) : m_va(V.va), m_element(0)
{
	memset((char*)&m_current, 0, sizeof(WRIteratorEntry) );

	if ( (V.xtype != WR_EX_ARRAY && V.xtype != WR_EX_HASH_TABLE)
		 || (m_va->m_type == SV_VOID_HASH_TABLE) )
	{
		m_va = 0;
		m_current.type = 0;
	}
	else
	{
		m_current.type = m_va->m_type;
		++*this;
	}
}

//------------------------------------------------------------------------------
const WRValue::Iterator WRValue::Iterator::operator++()
{
	if ( m_va )
	{
		if ( m_current.type == SV_HASH_TABLE )
		{
			int temp = m_element;
			for( ; temp < m_va->m_mod; ++temp )
			{
				if ( m_va->m_hashTable[temp] != WRENCH_NULL_HASH )
				{
					temp <<= 1;

					m_current.value = m_va->m_Vdata + temp++;
					m_current.key = m_va->m_Vdata + temp;

					m_element = ++temp;

					return *this;
				}
			}
		}
		else if ( m_element < m_va->m_size )
		{
			m_current.index = m_element;
			if ( m_current.type == SV_VALUE )
			{
				m_current.value = m_va->m_Vdata + m_element;
			}
			else
			{
				m_current.character = m_va->m_Cdata[m_element];
			}

			++m_element;

			return *this;
		}
	}

	memset( (char*)&m_current, 0, sizeof(WRIteratorEntry) );
	m_va = 0;
	return *this;
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

	if ( hash )
	{
		if ( context->stopLocation == 0 )
		{
			context->w->err = WR_ERR_run_must_be_called_by_itself_first;
			return 0;
		}

		cF = context->registry.getAsRawValueHashTable( hash );
		if ( !cF->wrf )
		{
			context->w->err = WR_ERR_wrench_function_not_found;
			return 0;
		}
	}

	return wr_callFunction( context, cF ? cF->wrf : 0, argv, argn );
}

//------------------------------------------------------------------------------
WRValue* wr_returnValueFromLastCall( WRContext* context )
{
	return context->stack; // this is where it ends up
}

//------------------------------------------------------------------------------
WRFunction* wr_getFunction( WRContext* context, const char* functionName )
{
	WRValue* f = context->registry.exists(wr_hashStr(functionName), false);
	return f ? f->wrf : 0;
}

//------------------------------------------------------------------------------
WRValue* wr_getGlobalRef( WRContext* context, const char* label )
{
	char globalLabel[64] = "::";
	if ( !label || !(READ_8_FROM_PC(context->bottom + 2) & WR_INCLUDE_GLOBALS) )
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

	const unsigned char* symbolsBlock = context->bottom + 3 + (context->numLocalFunctions * WR_FUNCTION_CORE_SIZE);
	for( unsigned int i=0; i<context->globals; ++i, symbolsBlock += 4 )
	{
		uint32_t symbolHash = READ_32_FROM_PC( symbolsBlock );
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
WRValue& wr_makeString( WRContext* context, WRValue* val, const char* data, const int len )
{
	const int slen = len ? len : strlen(data);
	val->va = context->getSVA( slen, SV_CHAR, false );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !val->va )
	{
		val->p2 = INIT_AS_INT;
		return *val;
	}
#endif
	val->p2 = INIT_AS_ARRAY;
	memcpy( (unsigned char *)val->va->m_data, data, slen );
	return *val;
}

//------------------------------------------------------------------------------
void wr_makeContainer( WRValue* val, const uint16_t sizeHint )
{
	val->va = (WRGCObject*)g_malloc( sizeof(WRGCObject) );
#ifdef WRENCH_HANDLE_MALLOC_FAIL
	if ( !val->va )
	{
		val->init();
		return;
	}
#endif
	val->va->init( sizeHint, SV_VOID_HASH_TABLE, false );
	val->va->m_skipGC = 1;
	val->p2 = INIT_AS_HASH_TABLE;
}

//------------------------------------------------------------------------------
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value )
{
	if ( container->xtype != WR_EX_HASH_TABLE )
	{
		return;
	}
	
	WRValue* entry = container->va->getAsRawValueHashTable( wr_hashStr(name) );
	entry->r = value;
	entry->p2 = INIT_AS_REF;
}

//------------------------------------------------------------------------------
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size )
{
	if ( container->xtype != WR_EX_HASH_TABLE || size > 0x1FFFFF )
	{
		return;
	}

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
	g_free( val->va );
	val->init();
}

#ifndef WRENCH_WITHOUT_COMPILER

//------------------------------------------------------------------------------
const char* c_opcodeName[] = 
{
	"Yield",

	"LiteralInt32",
	"LiteralZero",
	"LiteralFloat",
	"LiteralString",

	"CallFunctionByHash",
	"CallFunctionByHashAndPop",
	"CallFunctionByIndex",
	"PushIndexFunctionReturnValue",

	"CallLibFunction",
	"CallLibFunctionAndPop",

	"NewObjectTable",
	"AssignToObjectTableByOffset",
	"AssignToObjectTableByHash",

	"AssignToHashTableAndPop",
	"Remove",
	"HashEntryExists",

	"PopOne",
	"ReturnZero",
	"Return",
	"Stop",

	"Dereference",
	"Index",
	"IndexSkipLoad",
	"CountOf",
	"HashOf",

	"StackIndexHash",
	"GlobalIndexHash",
	"LocalIndexHash",

	"StackSwap",
	"SwapTwoToTop",

	"LoadFromLocal",
	"LoadFromGlobal",

	"LLValues",
	"LGValues",
	"GLValues",
	"GGValues",

	"BinaryRightShiftSkipLoad",
	"BinaryLeftShiftSkipLoad",
	"BinaryAndSkipLoad",
	"BinaryOrSkipLoad",
	"BinaryXORSkipLoad",
	"BinaryModSkipLoad",

	"BinaryMultiplication",
	"BinarySubtraction",
	"BinaryDivision",
	"BinaryRightShift",
	"BinaryLeftShift",
	"BinaryMod",
	"BinaryOr",
	"BinaryXOR",
	"BinaryAnd",
	"BinaryAddition",

	"BitwiseNOT",

	"RelativeJump",
	"RelativeJump8",

	"BZ",
	"BZ8",

	"LogicalAnd",
	"LogicalOr",
	"CompareLE",
	"CompareGE",
	"CompareGT",
	"CompareLT",
	"CompareEQ",
	"CompareNE",

	"GGCompareGT",
	"GGCompareGE",
	"GGCompareLT",
	"GGCompareLE",
	"GGCompareEQ", 
	"GGCompareNE", 

	"LLCompareGT",
	"LLCompareGE",
	"LLCompareLT",
	"LLCompareLE",
	"LLCompareEQ", 
	"LLCompareNE", 

	"GSCompareEQ",
	"LSCompareEQ", 
	"GSCompareNE", 
	"LSCompareNE", 
	"GSCompareGE",
	"LSCompareGE",
	"GSCompareLE",
	"LSCompareLE",
	"GSCompareGT",
	"LSCompareGT",
	"GSCompareLT",
	"LSCompareLT",

	"GSCompareEQBZ", 
	"LSCompareEQBZ", 
	"GSCompareNEBZ", 
	"LSCompareNEBZ", 
	"GSCompareGEBZ",
	"LSCompareGEBZ",
	"GSCompareLEBZ",
	"LSCompareLEBZ",
	"GSCompareGTBZ",
	"LSCompareGTBZ",
	"GSCompareLTBZ",
	"LSCompareLTBZ",

	"GSCompareEQBZ8",
	"LSCompareEQBZ8",
	"GSCompareNEBZ8",
	"LSCompareNEBZ8",
	"GSCompareGEBZ8",
	"LSCompareGEBZ8",
	"GSCompareLEBZ8",
	"LSCompareLEBZ8",
	"GSCompareGTBZ8",
	"LSCompareGTBZ8",
	"GSCompareLTBZ8",
	"LSCompareLTBZ8",

	"LLCompareLTBZ",
	"LLCompareLEBZ",
	"LLCompareGTBZ",
	"LLCompareGEBZ",
	"LLCompareEQBZ",
	"LLCompareNEBZ",

	"GGCompareLTBZ",
	"GGCompareLEBZ",
	"GGCompareGTBZ",
	"GGCompareGEBZ",
	"GGCompareEQBZ",
	"GGCompareNEBZ",

	"LLCompareLTBZ8",
	"LLCompareLEBZ8",
	"LLCompareGTBZ8",
	"LLCompareGEBZ8",
	"LLCompareEQBZ8",
	"LLCompareNEBZ8",

	"GGCompareLTBZ8",
	"GGCompareLEBZ8",
	"GGCompareGTBZ8",
	"GGCompareGEBZ8",
	"GGCompareEQBZ8",
	"GGCompareNEBZ8",

	"PostIncrement",
	"PostDecrement",
	"PreIncrement",
	"PreDecrement",

	"PreIncrementAndPop",
	"PreDecrementAndPop",

	"IncGlobal",
	"DecGlobal",
	"IncLocal",
	"DecLocal",

	"Assign",
	"AssignAndPop",
	"AssignToGlobalAndPop",
	"AssignToLocalAndPop",
	"AssignToArrayAndPop",

	"SubtractAssign",
	"AddAssign",
	"ModAssign",
	"MultiplyAssign",
	"DivideAssign",
	"ORAssign",
	"ANDAssign",
	"XORAssign",
	"RightShiftAssign",
	"LeftShiftAssign",

	"SubtractAssignAndPop",
	"AddAssignAndPop",
	"ModAssignAndPop",
	"MultiplyAssignAndPop",
	"DivideAssignAndPop",
	"ORAssignAndPop",
	"ANDAssignAndPop",
	"XORAssignAndPop",
	"RightShiftAssignAndPop",
	"LeftShiftAssignAndPop",

	"LogicalNot", //X
	"Negate",

	"LiteralInt8",
	"LiteralInt16",

	"IndexLiteral8",
	"IndexLiteral16",

	"IndexLocalLiteral8",
	"IndexGlobalLiteral8",
	"IndexLocalLiteral16",
	"IndexGlobalLiteral16",

	"BinaryAdditionAndStoreGlobal",
	"BinarySubtractionAndStoreGlobal",
	"BinaryMultiplicationAndStoreGlobal",
	"BinaryDivisionAndStoreGlobal",

	"BinaryAdditionAndStoreLocal",
	"BinarySubtractionAndStoreLocal",
	"BinaryMultiplicationAndStoreLocal",
	"BinaryDivisionAndStoreLocal",

	"CompareBEQ",
	"CompareBNE",
	"CompareBGE",
	"CompareBLE",
	"CompareBGT",
	"CompareBLT",

	"CompareBEQ8",
	"CompareBNE8",
	"CompareBGE8",
	"CompareBLE8",
	"CompareBGT8",
	"CompareBLT8",

	"BLA",
	"BLA8",
	"BLO",
	"BLO8",

	"LiteralInt8ToGlobal",
	"LiteralInt16ToGlobal",
	"LiteralInt32ToLocal",
	"LiteralInt8ToLocal",
	"LiteralInt16ToLocal",
	"LiteralFloatToGlobal",
	"LiteralFloatToLocal",
	"LiteralInt32ToGlobal",

	"GGBinaryMultiplication",
	"GLBinaryMultiplication",
	"LLBinaryMultiplication",

	"GGBinaryAddition",
	"GLBinaryAddition",
	"LLBinaryAddition",

	"GGBinarySubtraction",
	"GLBinarySubtraction",
	"LGBinarySubtraction",
	"LLBinarySubtraction",

	"GGBinaryDivision",
	"GLBinaryDivision",
	"LGBinaryDivision",
	"LLBinaryDivision",

	"GPushIterator",
	"LPushIterator",
	"GGNextKeyValueOrJump",
	"GLNextKeyValueOrJump",
	"LGNextKeyValueOrJump",
	"LLNextKeyValueOrJump",
	"GNextValueOrJump",
	"LNextValueOrJump",

	"Switch",
	"SwitchLinear",

	"GlobalStop",

	"ToInt",
	"ToFloat",

	"LoadLibConstant",
	"InitArray",
	"InitVar",

	"DebugInfo",
};

//------------------------------------------------------------------------------
const char* c_errStrings[]=
{
	"WR_ERR_None",

	"WR_ERR_compiler_not_loaded",
	"WR_ERR_function_not_found",
	"WR_ERR_lib_function_not_found",
	"WR_ERR_hash_not_found",
	"WR_ERR_library_constant_not_loaded",
	"WR_ERR_unknown_opcode",
	"WR_ERR_unexpected_EOF",
	"WR_ERR_unexpected_token",
	"WR_ERR_bad_expression",
	"WR_ERR_bad_label",
	"WR_ERR_statement_expected",
	"WR_ERR_unterminated_string_literal",
	"WR_ERR_newline_in_string_literal",
	"WR_ERR_bad_string_escape_sequence",
	"WR_ERR_tried_to_load_non_resolvable",
	"WR_ERR_break_keyword_not_in_looping_structure",
	"WR_ERR_continue_keyword_not_in_looping_structure",
	"WR_ERR_expected_while",
	"WR_ERR_compiler_panic",
	"WR_ERR_constant_redefined",
	"WR_ERR_struct_in_struct",
	"WR_ERR_var_not_seen_before_label",
	"WR_ERR_unexpected_export_keyword",
	"WR_ERR_new_assign_by_label_or_offset_not_both",
	"WR_ERR_struct_not_exported",

	"WR_ERR_run_must_be_called_by_itself_first",
	"WR_ERR_hash_table_size_exceeded",
	"WR_ERR_hash_table_invalid_key",
	"WR_ERR_wrench_function_not_found",
	"WR_ERR_array_must_be_indexed",
	"WR_ERR_context_not_found",
	"WR_ERR_context_not_yielded",
	"WR_ERR_cannot_call_function_context_yielded",

	"WR_ERR_hash_declaration_in_array",
	"WR_ERR_array_declaration_in_hash",
	"WR_ERR_stack_overflow",

	"WR_ERR_bad_goto_label",
	"WR_ERR_bad_goto_location",
	"WR_ERR_goto_target_not_found",

	"WR_ERR_switch_with_no_cases",
	"WR_ERR_switch_case_or_default_expected",
	"WR_ERR_switch_construction_error",
	"WR_ERR_switch_bad_case_hash",
	"WR_ERR_switch_duplicate_case",

	"WR_ERR_bad_bytecode_CRC",

	"WR_ERR_execute_function_zero_called_more_than_once",

	"WR_ERR_malloc_failed",

	"WR_ERR_USER_err_out_of_range",
};


#endif
