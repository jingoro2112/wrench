/*~
6
32000
-32000
500000
-500000
32001
-32001
500001
-500001
~*/

U();

function U()
{
	enum
	{
		value2 = 6,
		value3 = 6.5,
	};

	if ( value3 != 6.5 )
	{
		println("float");
	}
	
	enum
	{
		a16 = 32000,
		a162 = -32000,
		a32 = 500000,
		a322 = -500000,
	}

	println( value2 );
	println( a16 );
	println( a162 );
	println( a32 );
	println( a322 );
}

enum
{
	name1
}

enum
{
	n1,
	n2,
	n3
}

enum
{
	name2 = 2
};

enum
{
	name3 = 3,
}

enum
{
	name4 = 4,
	name5
}

enum
{
	name6 = 6,
	name7 = 7
};

enum
{
	name8 = 8,
	name9 = 9,
}

enum
{
	name10 = 6,
	name11 = 6,
	name12,
	name13,
	name14 = -1,
	name15,
}

enum
{
	a16 = 32001,
	a162 = -32001,
	a32 = 500001,
	a322 = -500001,
};


if( name1 != 0 ) { println("bad 1"); }
if( n1 != 0 ) { println("bad 2"); }
if( n2 != 1 ) { println("bad 3"); }
if( n3 != 2) { println("bad 4"); }
if( name2 != 2 ) { println("bad 5"); }
if( name3 != 3 ) { println("bad 6"); }
if( name4 != 4 ) { println("bad 7"); }
if( name5 != 5 ) { println("bad 8"); }
if( name6 != 6 ) { println("bad 9"); }
if( name7 != 7 ) { println("bad a"); }
if( name8 != 8 ) { println("bad b"); }
if( name9 != 9 ) { println("bad c"); }
if( name10 != 6 ) { println("bad d"); }
if( name11 != 6 ) { println("bad e"); }
if( name12 != 7 ) { println("bad f"); }
if( name13 != 8 ) { println("bad g"); }
if( name14 != -1 ) { println("bad h"); }
if( name15 != 0 ) { println("bad i"); }
println( a16 );
println( a162 );
println( a32 );
println( a322 );
