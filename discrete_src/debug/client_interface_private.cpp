/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::WRDebugClientInterfacePrivate()
{
	init();
}

//------------------------------------------------------------------------------
void wr_FunctionClearFunc( WrenchFunction& F )
{
	delete F.vars;
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::init()
{
	memset( this, 0, sizeof(*this) );
	m_functions = new SimpleLL<WrenchFunction>( wr_FunctionClearFunc );
	m_callStack = new SimpleLL<WrenchCallStackEntry>();
}

//------------------------------------------------------------------------------
WRDebugClientInterfacePrivate::~WRDebugClientInterfacePrivate()
{
	g_free( m_sourceBlock );

	delete m_functions;
	delete m_callStack;
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::getValues( SimpleLL<WrenchDebugValue>& values, const int depth )
{
	values.clear();

	WrenchFunction* g = (*m_functions)[depth];


	for( unsigned int i = 0; i<g->vars->count(); ++i )
	{
		WrenchDebugValue* v = values.addTail();

		strncpy( v->name, (*g->vars)[i]->label, 63 );
		m_parent->getValue( v->value, i, depth );
	}
}

//------------------------------------------------------------------------------
WrenchPacket* WRDebugClientInterfacePrivate::getPacket( const int timeoutMilliseconds )
{
	for(;;)
	{
		WrenchPacket* packet = m_comm->receive( timeoutMilliseconds );

		if ( !packet || (packet->_type != WRD_DebugOut) )
		{
			return packet;
		}

		if ( outputDebug )
		{
			outputDebug( (const char*)packet->payload() );
		}

		g_free( packet );
	}
}

//------------------------------------------------------------------------------
void WRDebugClientInterfacePrivate::populateSymbols()
{
	if ( m_symbolsLoaded )
	{
		return;
	}

	m_functions->clear();

	m_comm->send( WrenchPacket(WRD_RequestSymbolBlock) );

	WrenchPacketScoped r( m_comm->receive() );

	if ( !r || r.packet->_type != WRD_ReplySymbolBlock )
	{
		return;
	}

	m_symbolsLoaded = true;

	const unsigned char *block = (const unsigned char *)r.packet->payload();

	int functionCount = READ_16_FROM_PC( block );
	int pos = 2;

	for( int f=0; f<functionCount; ++f )
	{
		uint8_t count = READ_8_FROM_PC( block + pos );
		++pos;

		WrenchFunction* func = m_functions->addTail(); // so first is always "global"

		func->arguments = READ_8_FROM_PC( block + pos++ );
		sscanf( (const char *)(block + pos), "%63s", func->name );

		while( block[pos++] ); // skip to next null

		func->vars = new SimpleLL<WrenchSymbol>();
		for( uint8_t v=0; v<count; ++v )
		{
			WrenchSymbol* s = func->vars->addTail();
			sscanf( (const char*)(block + pos), "%63s", s->label );
			while( block[pos++] );
		}
	}
}


#endif
