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
		UDNode* node = new UDNode;
		node->val = value;
		node->next = m_nodeOnlyHead;
		m_nodeOnlyHead = node;

		m_index.set( wr_hashStr(key), node );
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

	WRUserData() : m_head(0), m_nodeOnlyHead(0) {}
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
struct WRFunctionRegistry
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
struct WRRunContext
{
	WRFunctionRegistry* localFunctions;
	WRHashTable<WRFunctionRegistry*> localFunctionRegistry;
	WRValue* globalSpace;
	int globals;

	const unsigned char* bottom;
	int32_t stopLocation;

	WRStaticValueArray* svAllocated;
	WRStaticValueArray* getSVA( int size, WRStaticValueArrayType type = SV_VALUE );
	void gcArray( WRStaticValueArray* sva );

	WRState* w;
	
	WRRunContext* next;
	
	WRRunContext( WRState* state );
	~WRRunContext();
};

//------------------------------------------------------------------------------
struct WRState
{
	int contextIdGenerator;
	WRHashTable<WRRunContext*> contexts;

	WRHashTable<WRCFunctionCallback> c_functionRegistry;

	WRError err;

	WRValue* stack;
	int stackSize;
	WRValue* stackTop;
	
	WR_LOAD_BLOCK_FUNC loader;
	void* usr;
	
	WRValue* returnValue;
	WRRunContext* contextList;

	WRState( int EntriesInStack =DEFAULT_STACK_SIZE );
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

void arrayToValue( const WRValue* array, WRValue* value );
void intValueToArray( const WRValue* array, int32_t I );
void floatValueToArray( const WRValue* array, float F );

typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[5][5];
extern WRVoidFunc wr_SubtractAssign[5][5];
extern WRVoidFunc wr_AddAssign[5][5];
extern WRVoidFunc wr_ModAssign[5][5];
extern WRVoidFunc wr_MultiplyAssign[5][5];
extern WRVoidFunc wr_DivideAssign[5][5];
extern WRVoidFunc wr_ORAssign[5][5];
extern WRVoidFunc wr_ANDAssign[5][5];
extern WRVoidFunc wr_XORAssign[5][5];
extern WRVoidFunc wr_RightShiftAssign[5][5];
extern WRVoidFunc wr_LeftShiftAssign[5][5];


typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_binaryAddition[5][5];
extern WRTargetFunc wr_binaryMultiply[5][5];
extern WRTargetFunc wr_binarySubtract[5][5];
extern WRTargetFunc wr_binaryDivide[5][5];
extern WRTargetFunc wr_binaryLeftShift[5][5];
extern WRTargetFunc wr_binaryRightShift[5][5];
extern WRTargetFunc wr_binaryMod[5][5];
extern WRTargetFunc wr_binaryAnd[5][5];
extern WRTargetFunc wr_binaryOr[5][5];
extern WRTargetFunc wr_binaryXOR[5][5];

typedef void (*WRStateFunc)( WRRunContext* c, WRValue* to, WRValue* from );
extern WRStateFunc wr_index[5][5];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[5][5];
extern WRReturnFunc wr_CompareGT[5][5];
extern WRReturnFunc wr_CompareLT[5][5];
extern WRReturnFunc wr_LogicalAnd[5][5];
extern WRReturnFunc wr_LogicalOr[5][5];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_BitwiseNot[5];
extern WRUnaryFunc wr_negate[5];
extern WRUnaryFunc wr_preinc[5];
extern WRUnaryFunc wr_predec[5];
extern WRVoidFunc wr_postinc[5];
extern WRVoidFunc wr_postdec[5];

typedef bool (*WRReturnSingleFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[5];

typedef bool (*WRValueCheckFunc)( WRValue* value );
extern WRValueCheckFunc wr_ZeroCheck[5];

typedef void (*WRUserHashFunc)( WRValue* value, WRValue* target, int32_t hash );
extern WRUserHashFunc wr_UserHash[5];


#endif
