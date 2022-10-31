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
#ifndef _CONTAINER_DATA_H
#define _CONTAINER_DATA_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
class WRContainerData
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

	WRContainerData( int sizeHint =0 ) : m_head(0), m_nodeOnlyHead(0), m_index(sizeHint) {}
	~WRContainerData();

private:
	struct UDNode
	{
		WRValue* val;
		UDNode* next;
	};
	UDNode* m_head; // values that might have been handed to this structure so it does not necessarily own (see below)
	UDNode* m_nodeOnlyHead; // values created by this structure (so are destroyed with it)
	WRHashTable<UDNode*> m_index;
};

#endif
