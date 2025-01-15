/*~
argtest
1
2
3
4
5
6
6
4000000
20
0
1.2
10
7
4000000
20
0
1.2
10
8
0
20
8
1
20
8
2
20
8
0
0
30
1
0
30
2
3
30
4
5
30
5
3
8
0
1
1
2
3
5
8
13
21
34
55
10
10
20
20
~*/


var x = -2;
if ( x != -2 ) println( "xfail1");
incGlob(y)-1;


a = 10;
a = println("argtest");
if ( a != 20 ) { println("afail1"); }
a = 10;
a = println();
if ( a != 20 ) { println("afail2"); }
a = 10;
a = println(1,2,3,4,5,6);
if ( a != 20 ) { println("afail3"); }

glob = 10;
incGlob();
function incGlob()
{
	glob++;
}
if ( glob != 11 ) println("g1");


tle(tle());
a = tle();
function tle
{
}


a = 5;

++::a;
println( a );

test();

function test()
{
	b = 4000000;
	println( b );
	b = 20;
	println( b );
	b = 0;
	println( b );
	b = 1.2;
	println( b );

	b = 10;
	println( b );
	++::a;
	println( ::a );
	
}

test();


function test2(a)
{
	println( a );
	a = 20;
	println( a );
	
	println( ::a );
}

test2();
test2(1);
test2(2,3);

function test3(a,b)
{
	println( a );
	println( b );
	b = 30;
	println( b );
}

test3();
test3(1);
test3(2,3);
test3(4,5,6 );


function test4()
{
}

test4();
test4(1);
test4(2,3);
test4(4,5,6 );


function test5()
{
	return 5;
}

println( test5() );

function test6()
{
	b = 2;
	c = 3;
	d = 4;
	return c;
}
println( test6() );

function test7()
{
	return ::a;
}
println( test7() );


function fibonacci( n )
{
	if (n <= 1) return n;
	return fibonacci(n - 2) + fibonacci(n - 1);
}

for ( i = 0; i <= 10; ++i)
{
	println( fibonacci(i) );
}


a = 10;
function fa()
{
	a = 20;
}
function g_fa()
{
	::a = 20;
}
println(a);
println(::a);
fa();
println(a);
g_fa();
println(a);


function farray( a )
{
	a[0] = 21;
}

function farg( a )
{
	a = 25;
}

a[1] = {0};
farray( a );
if ( a[0] != 21 ) println("ferr 0");

//work = 40;
//println( work );
//farg( work );
//println( work );
