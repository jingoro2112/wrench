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
		value2 = 6
	};

	enum
	{
		a16 = 32000,
		a162 = -32000,
		a32 = 500000,
		a322 = -500000,
	}

	print( value2 );
	print( a16 );
	print( a162 );
	print( a32 );
	print( a322 );
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


if( name1 != 0 ) { print("bad 1"); }
if( n1 != 0 ) { print("bad 2"); }
if( n2 != 1 ) { print("bad 3"); }
if( n3 != 2) { print("bad 4"); }
if( name2 != 2 ) { print("bad 5"); }
if( name3 != 3 ) { print("bad 6"); }
if( name4 != 4 ) { print("bad 7"); }
if( name5 != 5 ) { print("bad 8"); }
if( name6 != 6 ) { print("bad 9"); }
if( name7 != 7 ) { print("bad a"); }
if( name8 != 8 ) { print("bad b"); }
if( name9 != 9 ) { print("bad c"); }
if( name10 != 6 ) { print("bad d"); }
if( name11 != 6 ) { print("bad e"); }
if( name12 != 7 ) { print("bad f"); }
if( name13 != 8 ) { print("bad g"); }
if( name14 != -1 ) { print("bad h"); }
if( name15 != 0 ) { print("bad i"); }
print( a16 );
print( a162 );
print( a32 );
print( a322 );
