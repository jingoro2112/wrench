// g++ -o wmin -m32 -O3 wrench_min.cpp

#define WRENCH_WITHOUT_COMPILER
#define WRENCH_COMPACT
#define WRENCH_REALLY_COMPACT

#include "src/wrench.h"
#include "src/wrench.cpp"


//------------------------------------------------------------------------------
int main( int argn, char* argv[] )
{
	WRState* w = wr_newState();
	
	unsigned char buf[1] = {0};
	wr_run( w, buf, 0 );
	wr_destroyState( w );
	return 0;
}
