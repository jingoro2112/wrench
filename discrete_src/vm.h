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

#ifndef _VM_H
#define _VM_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
class WRUserData
{
public:

	//------------------------------------------------------------------------------
	void registerValue( const char* key, WRValue* value )
	{
		int32_t hash = wr_hashStr(key);
		
		UDNode* node = m_index.getItem( hash );
		if ( !node )
		{
			node = new UDNode;
			node->next = m_nodeOnlyHead;
			m_nodeOnlyHead = node;
			m_index.set( wr_hashStr(key), node );
		}

		node->val = value;
	}
	
	//------------------------------------------------------------------------------
	WRValue* addValue( const char* key )
	{
		UDNode* node = new UDNode;
		node->val = new WRValue;
		node->val->type = 0;
		node->next = m_head;
		m_head = node;
		m_index.set( wr_hashStr(key), node );
		return node->val;
	}
	
	//------------------------------------------------------------------------------
	WRValue* get( const char* key ) { UDNode* N = m_index.getItem( wr_hashStr(key) ); return N ? N->val : 0; }
	WRValue* get( const int32_t hash ) { UDNode* N = m_index.getItem(hash); return N ? N->val : 0; }

	WRUserData( int sizeHint =0 ) : m_head(0), m_nodeOnlyHead(0), m_index(sizeHint) {}
	~WRUserData()
	{
		while( m_head )
		{
			UDNode* next = m_head->next;
			delete m_head->val;
			delete m_head;
			m_head = next;
		}

		while (m_nodeOnlyHead)
		{
			UDNode* next = m_nodeOnlyHead->next;
			delete m_nodeOnlyHead;
			m_nodeOnlyHead = next;
		}
	}

private:
	struct UDNode
	{
		WRValue* val;
		UDNode* next;
	};
	UDNode* m_head;
	UDNode* m_nodeOnlyHead;
	WRHashTable<UDNode*> m_index;
};

//------------------------------------------------------------------------------
struct WRFunction
{
	union
	{
		const unsigned char* offset;
		int offsetI;
	};
	
	uint32_t hash;
	char arguments;
	char frameSpaceNeeded;
	char frameBaseAdjustment;
};

//------------------------------------------------------------------------------
struct WRCFunctionCallback
{
	WR_C_CALLBACK function;
	void* usr;
};

//------------------------------------------------------------------------------
class WRGCValueArray;
enum WRStaticValueArrayType
{
	SV_VALUE = 0x00,
	SV_CHAR = 0x01,
	SV_INT = 0x02,
	SV_FLOAT = 0x03,

	SV_PRE_ALLOCATED = 0x10,
};

//------------------------------------------------------------------------------
struct WRContext
{
	WRFunction* localFunctions;
	WRHashTable<WRFunction*> localFunctionRegistry;
	WRValue* globalSpace;
	int globals;

	const unsigned char* bottom;
	int32_t stopLocation;

	WRStaticValueArray* svAllocated;
	WRStaticValueArray* getSVA( int size, WRStaticValueArrayType type = SV_VALUE );
	void gcArray( WRStaticValueArray* sva );

	WRState* w;
	
	WRContext* next;
	
	WRContext( WRState* state );
	~WRContext();
};

//------------------------------------------------------------------------------
struct WRState
{
	WRHashTable<WRCFunctionCallback> c_functionRegistry;
	WRHashTable<WR_LIB_CALLBACK> c_libFunctionRegistry;

	WRError err;

	WRValue* stack;
	int stackSize;
	WRValue* stackTop;
	
	WR_LOAD_BLOCK_FUNC loader;
	void* usr;
	
	WRValue* returnValue;
	WRContext* contextList;

	WRState( int EntriesInStack =WRENCH_DEFAULT_STACK_SIZE );
	~WRState();
};

//------------------------------------------------------------------------------
class WRStaticValueArray
{
public:

	WRStaticValueArray* m_next;
	unsigned int m_size;
	char m_type;
	const void* m_data;
	
	//------------------------------------------------------------------------------
	WRStaticValueArray( const unsigned int size,
						const WRStaticValueArrayType type,
						const void* preAlloc =0 )
	{
		m_type = (char)type;
		m_size = size;
		if ( preAlloc )
		{
			m_type |= (char)SV_PRE_ALLOCATED;
			m_data = preAlloc;
		}
		else
		{
			switch( type )
			{
				case SV_VALUE: { m_data = new WRValue[size]; break; }
				case SV_CHAR: { m_data = new char[size]; break; }
				case SV_INT: { m_data = new int[size]; break; }
				case SV_FLOAT: { m_data = new float[size]; break; }
				default: m_data = 0;
			}
		}
	}
	
	//------------------------------------------------------------------------------
	~WRStaticValueArray()
	{
		if ( !(m_type & (char)SV_PRE_ALLOCATED) )
		{
			switch( m_type & 0x3 )
			{
				case SV_VALUE: { delete[] (WRValue *)m_data; break; }
				case SV_CHAR: { delete[] (char *)m_data; break; }
				case SV_INT: { delete[] (int *)m_data; break; }
				case SV_FLOAT: { delete[] (float *)m_data; break; }
				default: break;
			}
		}
	}
	
	//------------------------------------------------------------------------------
	void* operator[]( const unsigned int l ) { return get( l ); }
		
	void* get( const unsigned int l ) const
	{
		int s = l < m_size ? l : m_size - 1;

		switch( m_type & 0x3 )
		{
			case SV_VALUE: { return ((WRValue *)m_data) + s; }
			case SV_CHAR: { return ((char *)m_data) + s; }
			case SV_INT: { return ((int *)m_data) + s; }
			case SV_FLOAT: { return ((float *)m_data) + s; }
			default: return 0;
		}
	}
};

void wr_arrayToValue( const WRValue* array, WRValue* value );
void wr_intValueToArray( const WRValue* array, int32_t I );
void wr_floatValueToArray( const WRValue* array, float F );

typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[36];

extern WRVoidFunc wr_SubtractAssign[36];
extern WRVoidFunc wr_AddAssign[36];
extern WRVoidFunc wr_ModAssign[36];
extern WRVoidFunc wr_MultiplyAssign[36];
extern WRVoidFunc wr_DivideAssign[36];
extern WRVoidFunc wr_ORAssign[36];
extern WRVoidFunc wr_ANDAssign[36];
extern WRVoidFunc wr_XORAssign[36];
extern WRVoidFunc wr_RightShiftAssign[36];
extern WRVoidFunc wr_LeftShiftAssign[36];
extern WRVoidFunc wr_postinc[6];
extern WRVoidFunc wr_postdec[6];


typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_binaryAddition[36];

extern WRTargetFunc wr_binaryAddition2[36];


extern WRTargetFunc wr_binaryMultiply[36];
extern WRTargetFunc wr_binarySubtract[36];
extern WRTargetFunc wr_binaryDivide[36];
extern WRTargetFunc wr_binaryLeftShift[36];
extern WRTargetFunc wr_binaryRightShift[36];
extern WRTargetFunc wr_binaryMod[36];
extern WRTargetFunc wr_binaryAND[36];
extern WRTargetFunc wr_binaryOR[36];
extern WRTargetFunc wr_binaryXOR[36];

typedef void (*WRStateFunc)( WRContext* c, WRValue* to, WRValue* from );
extern WRStateFunc wr_index[36];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[36];
extern WRReturnFunc wr_CompareGT[36];
extern WRReturnFunc wr_CompareLT[36];
extern WRReturnFunc wr_LogicalAND[36];
extern WRReturnFunc wr_LogicalOR[36];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_negate[6];
extern WRUnaryFunc wr_preinc[6];
extern WRUnaryFunc wr_predec[6];
extern WRUnaryFunc wr_toInt[6];
extern WRUnaryFunc wr_toFloat[6];
extern WRUnaryFunc wr_bitwiseNot[6];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[6];

typedef bool (*WRValueCheckFunc)( WRValue* value );
extern WRValueCheckFunc wr_ZeroCheck[6];

typedef void (*WRUserHashFunc)( WRValue* value, WRValue* target, int32_t hash );
extern WRUserHashFunc wr_UserHash[6];


#endif
