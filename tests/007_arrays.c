/*~
one
two
3
6
6
7
seven
~*/


sb[5] = { 3 };
sa[sb[0]] = {1};
if(sa._count != 3 ) println("size 10");


var s1000[1000];
if ( s1000._count != 1000 ) println("size 0");
var s10[10];
if ( s10._count != 10 ) println("size 1");
s2000[2000];
if ( s2000._count != 2000 ) println("size 10");

s3[10];
if ( s3._count != 10 ) println("size 2");
s1[] = {};
if ( s1._count != 0 ) println("size 3");
s2[] = {0};
if ( s2._count != 1 ) println("size 4");
s22[] = {0,1,2,3};
if ( s22._count != 4 ) println("size 5");

function retCheck()
{
	a[] = { 1, 2, 3 };
	return a;
}

b = retCheck();
if ( b[0] != 1 ) println( "rc 3");
if ( b[1] != 2 ) println( "rc 4");
if ( retCheck()[0] != 1 ) println("rc 0");
if ( retCheck()[1] != 2 ) println("rc 1");
if ( retCheck()[2] != 3 ) println("rc 2");


 
hashTable = { 1:"one", 2:"two", 3:3, "str":6 };

if ( hashTable[1] != "one" ) println("h0");
println( hashTable[1] ); // "one"
if ( hashTable[2] != "two" ) println("h1");
println( hashTable[2] ); // "two"
if ( hashTable[3] != 3 ) println("h2");
println( hashTable[3] ); // 3

println( hashTable["str"] ); // 6

println( hashTable.str ); // 6

hashTable.str = 7;
println( hashTable.str ); // 6
hashTable.str = "seven";
println( hashTable.str ); // 6


array = { 20 };
if ( array[0] != 20 ) println("lk1");
for( a : array )
{
	if ( a != 20 ) println("lk2");
	array = 0; // null the memory so it can be gc'ed (but shouldn't!)
	s = "allocate memory";
	if ( a != 20 ) println("lk2");
}

ff[] = { 1.2, 1.3, 1.4, 1.5 };
if ( ff[0] != 1.2 ) { println("ff1"); }
if ( ff[2] != 1.4 ) { println("ff2"); }
ff[0] = 1.7;
ff[2] = 45.76;
if ( ff[0] != 1.7 ) { println("ff3"); }
if ( ff[2] != 45.76 ) { println("ff4"); }
ff[1] = ff[1] + ff[3];
if ( ff[1] != 2.8 ) { println("ff5"); }
a = 7.8;
ff[1] = a;
if ( ff[1] != 7.8 ) { println("ff6"); }


aa[10];
a[500] = { aa };
if ( a[0]._count != 10 ) println("bad size 10");
if ( a._count!= 500 ) println("bad size 500");



for( gi=0; gi<500; ++gi )
{
	allocArray();
}
for( gi=0; gi<500; ++gi )
{
	allocArray();
}




b[40000];
b[1] = 10;
b[35000] = 20;
if ( b[1] != 10 || b[35000] != 20 )
{
	println("bad b");
}

c[2000000];
c[1] = 11;
c[1913500] = 21;
if ( c[1] != 11 || c[1913500] != 21 )
{
	println("bad c");
}

a = { 4, 2, 3, 4, 5 };
if ( a[2] != 3 )
{
	println("bad a1");
}

b[] = { 4, 2, 3, 4, 5 };
if ( b[2] != 3 )
{
	println("bad b1");
}

c[100] = { 4, 2, 3, 4, 5, };
if ( c[2] != 3 )
{
	println("bad c1");
}

d[1] = { 4, 2, 3, 4, 5 };
if ( d[2] != 3 )
{
	println("bad d1");
}

a[400] = 4;
if ( a[400] != 4 )
{
	println("bad a2");
}

ggi = 0;

grow[1] = 10;
grow[2] = 11;
grow[3] = 12;
grow[4] = 13;
grow[5] = 14;
grow[6] = 15;
if ( grow[6] != 15 ) println("badref0");
if ( grow[5] != 14 ) println("badref1");
if ( grow[4] != 13 ) println("badref2");
if ( grow[3] != 12 ) println("badref3");
if ( grow[2] != 11 ) println("badref4");



function allocArray()
{

	_a = { 4, 2, 3, 4, 5 };
	if ( _a[2] != 3 )
	{
		println("F bad a1");
	}

	_b[] = { 4, 2, 3, 4, 5 };
	if ( _b[2] != 3 )
	{
		println("F bad b1");
	}

	_c[100] = { 4, 2, 3, 4, 5 };
	if ( _c[2] != 3 )
	{
		println("F bad c1");
	}

	_d[1] = { 4, 2, 3, 4, 5 };
	if ( _d[2] != 3 )
	{
		println("F bad d1");
	}


	ab[500];
	i = 2;
	ab = { 0, 2, 3 };
	if ( ab[2] != 3 )
	{
		println("bad ab2");
	}



	
	ab[401] = 41;
	if ( ab[401] != 41 )
	{
		println("bad ab41");
	}


	ac[50];

	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}


	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			println( "oops" );
		}
	}


	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}


	for( i=0; i<50; ++i )
	{
		::a[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			println( "oops_LL" );
		}
	}


	for( i=0; i<50; ++i )
	{
		if ( ::a[i] != i+1 )
		{
			println( "oops_GL" );
		}
	}


	for( ::ggi=0; ::ggi<50; ++::ggi )
	{
		if ( ac[::ggi] != ::ggi+1 )
		{
			println( "oops_LG" );
		}
	}

	for( ::ggi=0; ::ggi<50; ++::ggi )
	{
		if ( ::a[::ggi] != ::ggi+1 )
		{
			println( "oops_GG" );
		}
	}
}

a[10] = { 1, 2, 3 };
leakCheck( a[0] );

function leakCheck( value )
{
	::a = 10;
	b[20]; // cause GC to run

	value = 20;
	if ( value != 20 ) println("B1");
	if ( 20 != value ) println("B2");
}


