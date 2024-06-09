/*~ ~*/


sb[5] = { 3 };
sa[sb[0]] = { 1 };
if(sa[0] != 1 ) println("rese size 3");


var s1000[1000];
if ( s1000._count != 1000 ) println("size 0");
var s10[10];
if ( s10._count != 10 ) println("size 1");
s2000[2000];
if ( s2000._count != 2000 ) println("size 2000");

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
if ( aa._count!= 10 ) println("bad size aa 10");
a[500] = { aa };
if ( a[0]._count != 10 ) println("bad size 10");
if ( a._count!= 1 ) println("bad size 500");



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


var a = {10};
if ( a[0] != 10 ) println("err 0");

var b[] = {11};
if ( b[0] != 11 ) println("err 1");

var c[100] = {12};
if ( c[0] != 12 ) println("err 2");

var d = {13,14};
if ( d[0] != 13 ) println("err 3");
if ( d[1] != 14 ) println("err 4");

var e[] = {13,14};
if ( e[0] != 13 ) println("err 5");
if ( e[1] != 14 ) println("err 6");

var f[100] = {13,14};
if ( f[0] != 13 ) println("err 7");
if ( f[1] != 14 ) println("err 8");

var g = { { 15 } };
if ( g[0][0] != 15 ) println("err 9");

var h = { { 16, 17 } };
if ( h[0][0] != 16 ) println("err 10");
if ( h[0][1] != 17 ) println("err 11");

var i = { { 18, 19 }, { 20 } };
if ( i[0][0] != 18 ) println("err 12");
if ( i[0][1] != 19 ) println("err 13");
if ( i[1][0] != 20 ) println("err 14");

var j = { { 21 }, { 22, 23 } };
if ( j[0][0] != 21 ) println("err 15");
if ( j[1][0] != 22 ) println("err 16");
if ( j[1][1] != 23 ) println("err 17");

var k = { { { 24 } } };
if ( k[0][0][0] != 24 ) println("err 18");

var l = { { 25 }, {} };
if ( l[0][0] != 25 ) println("err 19");
if ( l[1][0] != 0 ) println("err 20");

var m = { { 15, 16 }, {}, 19 };
if ( m[0][0] != 15 ) println("err 21");
if ( m[2] != 19 ) println("err 22");

var n = { {}, { 15, 16 }, {} };
if ( n[1][1] != 16 ) println("err 23");

var o = { n, m, l, "hello" };
if ( o[0][1][1] != 16 ) println("err 24");
if ( o[3] != "hello" ) println("err 25");

