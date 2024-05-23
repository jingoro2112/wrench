#include <wrench.h>
#include <string.h>
#include <stdio.h>

//------------------------------------------------------------------------------
void println( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[1024];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s\n", argv[i].asString(buf, 1024) );
	}
}

const char* wrenchCode1 = "println(\"into 1\");"
						  "function handler()"
						  "{"
						  "		println(\"I am event 1\");\n"
						  "		for( var l = 1; l<10; ++l )"
						  "		{"
						  "			println( yield(l) );"
						  "		}"
						  "}";
const char* wrenchCode2 = "println(\"into 2\"); function handler() { println(\"I am event 2\"); }";
const char* wrenchCode3 = "println(\"into 3\"); function handler() { println(\"I am event 3\"); }";

//------------------------------------------------------------------------------
class EventHandler
{
public:

	void callFunction()
	{
		if ( m_func )
		{
			for(;;)
			{
				wr_callFunction( m_context, m_func );
				
				int args;
				WRValue* firstArg;
				WRValue* returnValue;
				if( wr_getYieldInfo(m_context, &args, &firstArg, &returnValue) )
				{
					if ( args == 1 )
					{
						printf( "yield passed [%d], adding 1 and passing back:\n", firstArg->asInt() );
						returnValue->setInt( firstArg->asInt() + 1 );
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	EventHandler* registerHandler( WRState* w, const char* code, const char* functionName )
	{
		int outLen;
		
		if ( wr_compile(code, strlen(code), &m_byteCode, &outLen) )
		{
			return 0;
		}

		if ( !(m_context = wr_run(w, m_byteCode, outLen))
			 || !(m_func = wr_getFunction(m_context, functionName)) )
		{
			free( m_byteCode );
			m_byteCode = 0;
			return 0;
		}

		return this;
	}

	EventHandler() { m_byteCode = 0; }
	~EventHandler() { free( m_byteCode ); }

private:
	WRContext* m_context;
	WRFunction* m_func;
	unsigned char* m_byteCode;
};


//------------------------------------------------------------------------------
int main( int argn, char** argv )
{
	WRState* w = wr_newState(); // create the state
	wr_registerFunction( w, "println", println ); // bind a function

	EventHandler events[3];
	events[0].registerHandler( w, wrenchCode1, "handler" );
	events[1].registerHandler( w, wrenchCode2, "handler" );
	events[2].registerHandler( w, wrenchCode3, "handler" );

	events[0].callFunction();
	events[1].callFunction();
	events[2].callFunction();
	events[1].callFunction();
	events[1].callFunction();
	events[0].callFunction();

	wr_destroyState( w );
	return 0;
}

