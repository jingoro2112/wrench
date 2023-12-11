/*~
~*/

// I have some ideas that will allow the following to compile and
// function, but not there yet
/*
struct SomeStruct
{
	m = 10;
	
	function member()
	{
		m = 20;
		n = 20;
	}

	n = 10;
};


SomeStruct s = new SomeStruct();

s.member();
*/
// I have some ideas that will allow the following to compile and
// function, but not there yet
/*
out = 12;

struct SomeStruct
{
	a = 5;
	
	function member()
	{
		b += a;
		return b + out;
	}

	function member2( d )
	{
		return d + b;
	}

	b = 10;
};


SomeStruct s = new SomeStruct();

if ( s.member() != 27 ) { print("f1"); }
if ( s.member2( 5 ) != 20 ) { print("f2"); }
*/
