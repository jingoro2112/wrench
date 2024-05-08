#include <wrench.h>
#include <string.h>
#include <stdio.h>

//------------------------------------------------------------------------------
void print( WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[1024];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf, 1024) );
	}
}

//------------------------------------------------------------------------------
class EventHandler
{
	public:

		void callFunction()
		{
			if ( m_func )
			{
				wr_callFunction( m_context, m_func );
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
				delete[] m_byteCode;
				m_byteCode = 0;
				return 0;
			}

			return this;
		}

		EventHandler() { m_byteCode = 0; }
		~EventHandler() { delete[] m_byteCode; }

	private:
		WRContext* m_context;
		WRFunction* m_func;
		unsigned char* m_byteCode;
};


const char* wrenchCode1 = "print(\"into 1\\n\"); function handler() { print(\"I am event 1\\n\"); }";
const char* wrenchCode2 = "print(\"into 2\\n\"); function handler() { print(\"I am event 2\\n\"); }";
const char* wrenchCode3 = "print(\"into 3\\n\"); function handler() { print(\"I am event 3\\n\"); }";

//------------------------------------------------------------------------------
int main( int argn, char** argv )
{
	WRState* w = wr_newState(); // create the state
	wr_registerFunction( w, "print", print ); // bind a function

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

