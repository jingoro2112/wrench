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
#define STR_FILE_OPERATIONS

#include <wrench.h>

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
void dbprintln( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		unsigned int len = 0;
		char* str = argv[i].asMallocString( &len );
		wr_stdout( str, len );
		g_free( str );
	}
	wr_stdout( "\n", 1 );

	retVal.i = 20; // for argument-return testing
}

//------------------------------------------------------------------------------
void debugPrint( const char* text )
{
	wr_stdout( text, strlen(text) );
	wr_stdout( "\n", 1 );
}

//------------------------------------------------------------------------------
void dbprint( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		unsigned int len = 0;
		char* str = argv[i].asMallocString( &len );
		wr_stdout( str, len );
		g_free( str );
	}
}

//------------------------------------------------------------------------------
WrenchDebugClient::WrenchDebugClient( WRState* w )
{
	m_context = 0;
	
	if ( w )
	{
		m_w = w;
		m_ownState = false;
	}
	else
	{
		m_ownState = true;
		m_w = wr_newState();
		
		wr_loadAllLibs(m_w);
		wr_registerFunction( m_w, "println", dbprintln );
		wr_registerFunction( m_w, "print", dbprint );
	}

	m_printInHex = false;

	m_server = 0;
	m_client = 0;
	m_comm = 0;
	m_code = 0;
}

//------------------------------------------------------------------------------
WrenchDebugClient::~WrenchDebugClient()
{
	if ( m_ownState )
	{
		wr_destroyState( m_w );
	}

	g_free( m_code );
	
	delete m_server;
	delete m_client;
	delete m_comm;
}

//------------------------------------------------------------------------------
bool WrenchDebugClient::init( const char* name, const char* address, const int port )
{
	if ( !name )
	{
		return false;
	}

	g_free( m_code );
	m_code = 0;

	delete m_client;
	delete m_server;
	delete m_comm;
	m_client = 0;
	m_server = 0;
	m_comm = 0;

	if ( address )
	{
		if ( port )
		{
			WrenchDebugTcpInterface* tcp = new WrenchDebugTcpInterface();
			if ( !tcp->connect(address, port) )
			{
				delete tcp;
				return false;
			}

			m_client = new WRDebugClientInterface( tcp );
			m_comm = tcp;
		}
		else
		{
			WrenchDebugSerialInterface* serial = new WrenchDebugSerialInterface();
			if ( !serial->open(address) )
			{
				delete serial;
				return false;
			}

			m_client = new WRDebugClientInterface( serial );
			m_comm = serial;
		}
	}
	else
	{
		WrenchDebugLoopbackInterface* loop = new WrenchDebugLoopbackInterface();
		m_client = new WRDebugClientInterface( loop );
		m_server = new WRDebugServerInterface( m_w, loop );
		m_comm = loop;
	}

	return loadSource( name );
}

//------------------------------------------------------------------------------
bool WrenchDebugClient::loadSource( const char* name )
{
	m_currentDepth = 0;

	m_displayPerStep.clear();
	m_listing.clear();
	m_breakpoints.clear();

	g_free( m_code );
	m_code = 0;
		
	WRstr infile;
	if ( !infile.fileToBuffer(name) )
	{
		printf( "Could not open source file [%s]\n", name );
		return false;
	}

	int err = wr_compile( infile,
						  infile.size(),
						  &m_code,
						  &m_codeLen,
						  0,
						  WR_EMBED_DEBUG_CODE|WR_INCLUDE_GLOBALS|WR_EMBED_SOURCE_CODE );
	if ( err )
	{
#ifdef WRENCH_WITHOUT_COMPILER
		printf( "compile error [#%d]\n", err );
#else
		printf( "compile error [%s]\n", c_errStrings[err] );
#endif
		return false;
	}

	WRstr str;
	wr_asciiDump( m_code, m_codeLen, str );
	printf( "%d:\n%s\n", m_codeLen, str.c_str() );

	m_client->load( m_code, m_codeLen );

	m_client->getFunctions();

	const char* sbuf = 0;
	int slen;
	if ( !m_client->getSourceCode(&sbuf, &slen) )
	{
		printf( "internal err<1>\n" );
		return false;
	}

	splitNL( sbuf );

	return true;
}

//------------------------------------------------------------------------------
void WrenchDebugClient::enter( const char* sourceFileName, const char* address, const int port )
{
	WRstr currentName = sourceFileName;
	
	bool state = init( currentName, address, port );

	bool showCodePos = false;
	bool showCallStack = false;
	bool showVars = false;

	char buf[200];
	
	SimpleLL<WRstr> cmd;
	for(;;)
	{
		SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
		if ( stack )
		{
			m_currentDepth = stack->count() - 1;
		}

		if ( showCodePos )
		{
			showCodePos = false;
			showCodePosition();
		}

		if ( showCallStack )
		{
			showCallStack = false;
//			showCallStack();
		}
		
		if ( showVars )
		{
			showVars = false;
			for( WRstr* S = m_displayPerStep.first(); S; S = m_displayPerStep.next() )
			{
				if ( !printVariable(*S) )
				{
					m_displayPerStep.popItem( S );
				}
			}
		}

/*
		char stk[64000] = "null";
		m_client->getStackDump( stk, 64000 );
		printf( "%s\n\n", stk );
*/
		
		if ( !state )
		{
			printf("'R' needed > ");
		}
		else
		{
			printf( "> " );
		}

		if ( !fgets(buf, 200, stdin) )
		{
			printf( "quit\n" );
			break;
		}

		int count = splitArgs( buf, cmd );
		if ( !count )
		{
			printf( "err?\n" );
			continue;
		}

		WRstr& c = *cmd.first();

		if ( isdigit(c[0]) )
		{
			int stackLevel = atoi(c);
			int count = (int)stack->count() - 1;
			if ( (count < 0) || (stackLevel > count) )
			{
				printf( "invalid frame [%d]\n", stackLevel );
			}
			else
			{
				m_currentDepth = stackLevel;
				printf( "frame [%d]\n", stackLevel );
			}

			showCallStack = true;

			continue;
		}
		
		switch( tolower(c[0]) )
		{
			case 'r':
			{
				if ( c[0] == 'R' )
				{
					if ( count > 1 )
					{
						currentName = *cmd.next();
					}

					if ( loadSource(currentName) )
					{
						printf( "[%s] ready to run\n", currentName.c_str() );
					}
					else
					{
						printf( "[%s] failed to load\n", currentName.c_str() );
					}
				}
				else
				{
					int toLine = count > 1 ? atoi(*cmd.next()) : 0;

					m_client->run( toLine );

					showCodePos = true;
					showCallStack = true;
					showVars = true;
				}
				break;
			}

			case 'l':
			{
				showCodePos = true;
				showCallStack = true;
				showVars = true;
				break;
			}

			case 's':
			{
				if ( c[0] == 's' )
				{
					m_client->stepOver();
					showCodePos = true;
					showCallStack = true;
					showVars = true;
					break;
				}
				// otherwise treat it as "i" for step INTO
			}

			case 'i':
			{
				m_client->stepInto();
				showCodePos = true;
				showCallStack = true;
				showVars = true;
				break;
			}

			case 'b':
			{
				if ( count == 1 )
				{
					showBreakpoint();
				}
				else
				{
					int bp = atoi( *cmd.next() );
					if ( bp < 1 || bp > (int)m_listing.count() )
					{
						printf( "[%d] invalid\n", bp );
					}
					else
					{
						for( int *b = m_breakpoints.first(); b; b = m_breakpoints.next() )
						{
							if ( *b == bp )
							{
								bp = -1;
								showBreakpoint( bp );
								printf( "removed %d\n", *b );
								m_breakpoints.popItem( b );
								m_client->clearBreakpoint( *b );
								break;
							}
						}

						if ( bp != -1 )
						{
							printf( "added %d\n", bp );
							*m_breakpoints.addHead() = bp;
							showBreakpoint( bp );
							m_client->setBreakpoint( bp );
						}
					}
				}
				
				break;
			}

			case 'x':
			{
				m_printInHex = !m_printInHex;
				printf( "hex: %s\n", m_printInHex ? "true":"false" );
				break;
			}

			case 'd':
			{
				if ( count == 1 )
				{
					if ( m_displayPerStep.head()
						 && *m_displayPerStep.head() == "*" )
					{
						printf( "display none\n" );
						m_displayPerStep.clear();
					}
					else
					{
						printf( "display all\n" );
						m_displayPerStep.clear();
						*m_displayPerStep.addHead() = "*";
					}
				}
				else
				{
					if ( m_displayPerStep.head()
						 && *m_displayPerStep.head() == "*" )
					{
						m_displayPerStep.clear();
					}

					WRstr* toAdd = cmd.next();
					for( WRstr* S = m_displayPerStep.first(); S; S = m_displayPerStep.next() )
					{
						if ( *S == *toAdd )
						{
							printf( "Removing [%s]\n", S->c_str() );
							m_displayPerStep.popItem( S );
							toAdd = 0;
							break;
						}
					}

					if ( toAdd )
					{
						printf( "Adding [%s]\n", toAdd->c_str() );
						*m_displayPerStep.addHead() = *toAdd;
					}
				}

				showVars = true;

				break;
			}

			case 'p':
			{
				if ( count == 1 )
				{
					showVars = true;
				}
				else
				{
					printVariable( *cmd.next() );
				}
				
				break;
			}

			case 'g':
			{
				printf( "\n" );

				SimpleLL<WrenchFunction>& funcs = m_client->getFunctions();
				WrenchFunction* f = funcs.first(); // first is global

				if ( f )
				{
					int i = 0;
					for( WrenchSymbol* s = f->vars->first(); s; s = f->vars->next() )
					{
						printf( "%d: %s\n", i, s->label );
						++i;
					}
				}
					
				printf( "\n");
				break;
			}

			case 'f':
			{
				if ( count <= 1 )
				{
					printf( "Please provide valid index, 0 for global\n" );
					break;
				}
				
				int index = atoi( *cmd.next() );
				
				printf( "\n" );
				SimpleLL<WrenchFunction>& funcs = m_client->getFunctions();
				WrenchFunction* f = funcs[index];

				if ( !f )
				{
					printf( "invalid function [%d]\n", index );
					break;
				}

				printf( "[%s:%d]\n", f->name, index );
				
				int i = 0;
				for( WrenchSymbol* s = f->vars->first(); s; s = f->vars->next() )
				{
					if ( i < f->arguments )
					{
						printf( "%d arg: %s\n", i, s->label );
					}
					else
					{
						printf( "%d: %s\n", i, s->label );
					}
					++i;
				}
				printf( "-------------------\n\n");
				
				break;
			}
			
			case 'q': break;

			default:
			{
				printf( "Command summary:\n"
						"r [line#] run/continue, optionally until line # is reached\n"
						"R [file]  load and run file, if not provided repeat last file\n"  
						"s         step over\n"
						"i or S    step into\n"
						"--------------\n"
						"w         where (show callstack)\n"
						"l         show listing\n"
						"--------------\n"
						"b #       break add/remove line #\n"
						"--------------\n"
						"x         toggle show values in hex\n"
						"p [var]   print locals or var if provided\n"
						"d [var]   display every step. All if var not specified\n"
						"          to cancel, repeat command\n"
						"g         print globals\n"
						"f [#]     print functions, optional select function index\n"
						"--------------\n"
						"q         quit\n"
					  );
				break;
			}
		}

		if ( c[0] == 'q' )
		{
			printf( "<q>uit\n" );
			break;
		}
	}

	printf( "\n" );
}

//------------------------------------------------------------------------------
int WrenchDebugClient::splitArgs( const char* str, SimpleLL<WRstr>& args )
{
	args.clear();

	int pos = 0;
	int count = 0;
	while( str[pos] )
	{
		++count;
		WRstr& s = *args.addTail();
		while( str[pos] && !isspace(str[pos]) )
		{
			s += str[pos];
			++pos;
		}

		while( str[pos] && isspace(str[pos]) ) { ++pos; }
	}

	return count;
}

//------------------------------------------------------------------------------
void WrenchDebugClient::splitNL( const char* str )
{
	m_listing.clear();
	
	if ( !str )
	{
		return;
	}
	
	int pos = 0;
	while( str[pos] )
	{
		WRstr* line = m_listing.addTail();
		for( ; str[pos]; ++pos )
		{
			if ( str[pos] == '\r' )
			{
				if ( str[pos + 1] == '\n' )
				{
					++pos;
				}

				break;
			}
			else if ( str[pos] == '\n' )
			{
				break;
			}

			line->append( str[pos] );
		}
		if ( !str[pos] )
		{
			break;
		}

		++pos;
	}
}

//------------------------------------------------------------------------------
bool WrenchDebugClient::printVariable( WRstr& str )
{
	SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
	if ( !stack )
	{	
		return false;
	}

	int unitToSearch = (*stack)[ m_currentDepth ]->thisUnitIndex;
	WRstr actual = str;

	if ( str.size() > 1
		 && *str.c_str(0) == ':'
		 && *str.c_str(1) == ':')
	{
		unitToSearch = 0;
	}
	else if ( unitToSearch == 0
			  && (str.size() < 3
				  || str[0] != ':'
				  || str[1] != ':') )
	{
		actual = "::" + str;
	}

	WrenchFunction* f = m_client->getFunctions()[unitToSearch];

	int i = 0;
	for( WrenchSymbol* s = f->vars->first(); s; s = f->vars->next(), ++i )
	{
		if ( actual.isMatch(s->label) )
		{
			WRValue val;
			char buf[512];
			m_client->getValue( val, i, unitToSearch );
			
			printf( "%s : %s\n", actual.c_str(), val.technicalAsString(buf, 511, m_printInHex) );
			
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
WrenchCallStackEntry* WrenchDebugClient::getCurrentFrame()
{
	SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
	if ( !stack )
	{
		return 0;
	}

	return (*stack)[m_currentDepth];
}

//------------------------------------------------------------------------------
void WrenchDebugClient::showLocals()
{
	SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
	if ( stack->count() == 1 )
	{
		return;
	}

	SimpleLL<WrenchDebugValue> values;
	m_client->I->getValues( values, stack->count() - 1 );
	printf("locals:\n");
	for( WrenchDebugValue* d = values.first(); d; d = values.next() )
	{
		char buf[512];
		printf( "%s : %s\n", d->name, d->value.technicalAsString(buf, 511, m_printInHex) );
	}	
}

//------------------------------------------------------------------------------
void WrenchDebugClient::showGlobals()
{
	SimpleLL<WrenchDebugValue> globals;
	m_client->I->getValues( globals, 0 );
	printf("globals: \n");
	for( WrenchDebugValue* d = globals.first(); d; d = globals.next() )
	{
		char buf[512];
		printf( "%s : %s\n", d->name, d->value.technicalAsString(buf, 511, m_printInHex) );
	}	
}

//------------------------------------------------------------------------------
void WrenchDebugClient::showCallStack()
{
	SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
	WrenchCallStackEntry* frame = getCurrentFrame();

	if ( !stack || !frame )
	{
		return;
	}

	WRstr output = "call stack ------------------------\n";
	int level = 0;
	for( WrenchCallStackEntry* e = stack->first(); e; e = stack->next() )
	{
		const char* f = m_client->getFunctionLabel( e->thisUnitIndex );
		output.appendFormat( "[%d] \"%s\" :%d\n",
							 level,
							 f ? f : "<NUL>",
							 e->onLine );

		++level;
	}

	printf( "\n%s\n", output.c_str() );
}

//------------------------------------------------------------------------------
void WrenchDebugClient::showCodePosition()
{
	SimpleLL<WrenchCallStackEntry>* stack = m_client->getCallstack();
	
	switch( m_client->getProcState() )
	{
		case WRP_Unloaded:
		{
			printf( "no code\n" );
			break;
		}
		
		case WRP_Loaded:
		{
			int l = 1;
			for( WRstr* line = m_listing.first(); line; ++l, line = m_listing.next() )
			{
				printf( "%-4d : %s\n", l, line->c_str() );
			}
			break;
		}

		case WRP_Running:
		{
			if ( !stack )
			{
				printf( "show code err<1>\n" );
				break;
			}

			WrenchCallStackEntry* e = getCurrentFrame();
			if ( !e )
			{
				printf( "show code err<2>\n" );
				break;
			}

			int start = (int)e->onLine - 3;
			int end = (int)e->onLine + 3;
			if ( start < 1 )
			{
				start = 1;
			}

			WRstr* line = m_listing.first();
			int	l = 1;
			for( ; line; ++l, line = m_listing.next() )
			{
				if ( l >= start && l <= end )
				{
					bool bp = false;
					for( int* b = m_breakpoints.first(); b; b = m_breakpoints.next() )
					{
						if ( *b == l )
						{
							bp = true;
							break;
						}
					}

					printf( "%c %s%-4d : %s\n",
							bp ? '*' : ' ',
							(e && (l == e->onLine)) ? ">>> " : "    ",
							l, line->c_str() );
				}	
			}

			showGlobals();
			showLocals();

			break;
		}
				

		case WRP_Complete:
		{
			printf( "complete\n" );
			break;
		}
	}
}

//------------------------------------------------------------------------------
void WrenchDebugClient::showBreakpoint( int bp )
{
	printf( "\n" );
	
	for( int* b = m_breakpoints.first(); b; b = m_breakpoints.next() )
	{
		if ( bp != -1 && *b != bp )
		{
			continue;
		}
		
		int	l = 1;
		for( WRstr* line = m_listing.first(); line; ++l, line = m_listing.next() )
		{
			if ( *b == l )
			{
				printf( "* %-4d : %s\n\n", l, line->c_str() );
				break;
			}
		}
	}
}

#endif