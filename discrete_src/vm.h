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
	
	char arguments;
	char frameSpaceNeeded;
	char frameBaseAdjustment;
	uint32_t hash;
	union
	{
		const unsigned char* offset;
		int offsetI;
	};
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
	WRStaticValueArray* getSVA( int size, WRStaticValueArrayType type, WRValue* stackTop );
	void gcArray( WRStaticValueArray* sva );

	WRState* w;

	WR_LOAD_BLOCK_FUNC loader;
	void* usr;

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

#define GET_LIB_RETURN_VALUE_LOCATION( s, a ) ((s) - 1)
#define INIT_AS_ARRAY    (((uint32_t)WR_EX) | ((uint32_t)WR_EX_ARRAY<<24))
#define INIT_AS_USR      (((uint32_t)WR_EX) | ((uint32_t)WR_EX_USR<<24))
#define INIT_AS_REFARRAY (((uint32_t)WR_EX) | ((uint32_t)WR_EX_REFARRAY<<24))
#define INIT_AS_REF      WR_REF
#define INIT_AS_INT      WR_INT
#define INIT_AS_FLOAT    WR_FLOAT

#define ARRAY_ELEMENT_FROM_P2(P) (((P)&0x00FFFF00) >> 8)
#define ARRAY_ELEMENT_TO_P2(P,E) { (P)->padL = (E); (P)->padH  = ((E)>>8); }

void wr_arrayToValue( const WRValue* array, WRValue* value );
void wr_intValueToArray( const WRValue* array, int32_t I );
void wr_floatValueToArray( const WRValue* array, float F );

typedef void (*WRVoidFunc)( WRValue* to, WRValue* from );
extern WRVoidFunc wr_assign[16];

extern WRVoidFunc wr_SubtractAssign[16];
extern WRVoidFunc wr_AddAssign[16];
extern WRVoidFunc wr_ModAssign[16];
extern WRVoidFunc wr_MultiplyAssign[16];
extern WRVoidFunc wr_DivideAssign[16];
extern WRVoidFunc wr_ORAssign[16];
extern WRVoidFunc wr_ANDAssign[16];
extern WRVoidFunc wr_XORAssign[16];
extern WRVoidFunc wr_RightShiftAssign[16];
extern WRVoidFunc wr_LeftShiftAssign[16];
extern WRVoidFunc wr_postinc[4];
extern WRVoidFunc wr_postdec[4];

typedef void (*WRTargetFunc)( WRValue* to, WRValue* from, WRValue* target );
extern WRTargetFunc wr_AdditionBinary[16];
extern WRTargetFunc wr_MultiplyBinary[16];
extern WRTargetFunc wr_SubtractBinary[16];
extern WRTargetFunc wr_DivideBinary[16];
extern WRTargetFunc wr_LeftShiftBinary[16];
extern WRTargetFunc wr_RightShiftBinary[16];
extern WRTargetFunc wr_ModBinary[16];
extern WRTargetFunc wr_ANDBinary[16];
extern WRTargetFunc wr_ORBinary[16];
extern WRTargetFunc wr_XORBinary[16];

typedef void (*WRStateFunc)( WRContext* c, WRValue* to, WRValue* from );
extern WRStateFunc wr_index[16];

typedef bool (*WRReturnFunc)( WRValue* to, WRValue* from );
extern WRReturnFunc wr_CompareEQ[16];
extern WRReturnFunc wr_CompareGT[16];
extern WRReturnFunc wr_CompareLT[16];
extern WRReturnFunc wr_LogicalAND[16];
extern WRReturnFunc wr_LogicalOR[16];

typedef void (*WRUnaryFunc)( WRValue* value );
extern WRUnaryFunc wr_negate[4];
extern WRUnaryFunc wr_preinc[4];
extern WRUnaryFunc wr_predec[4];
extern WRUnaryFunc wr_toInt[4];
extern WRUnaryFunc wr_toFloat[4];
extern WRUnaryFunc wr_bitwiseNot[4];

typedef bool (*WRReturnSingleFunc)( WRValue* value );

typedef bool (*WRValueCheckFunc)( WRValue* value );
extern WRReturnSingleFunc wr_LogicalNot[4];

typedef void (*WRUserHashFunc)( WRValue* value, WRValue* target, int32_t hash );
extern WRUserHashFunc wr_UserHash[4];


#endif
