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
#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
class WrenchDebugClient
{
public:

	void enter( const char* sourceFileName, const char* port =0 );

	WrenchDebugClient( WRState* w =0 ); // use provided state or make a new one
	~WrenchDebugClient();

private:

	bool init( const char* name, const char* port =0 );
	bool loadSource( const char* name );

	bool printVariable( WRstr& str );
	WrenchCallStackEntry* getCurrentFrame();

	void showGlobals();
	void showLocals();
	void showCallStack();
	void showCodePosition();

	void showBreakpoint( int bp =-1 );

	int splitArgs( const char* str, SimpleLL<WRstr>& args );
	void splitNL( const char* str );

	bool m_printInHex;
	int m_currentDepth;

	SimpleLL<WRstr> m_listing;
	SimpleLL<int> m_breakpoints;
	SimpleLL<WRstr> m_displayPerStep;

	WRState* m_w;
	WRContext* m_context;
	bool m_ownState;

	unsigned char* m_code;
	int m_codeLen;

	WRDebugServerInterface* m_server;
	WRDebugClientInterface* m_client;
};

#endif

#endif