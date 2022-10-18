/*~
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


/*
a = 10;
test2(20);
if ( a != 10 ) print("a fail");
function test2(a)
{
	a = 30;
}
*/


glob = 10;
incGlob();
function incGlob()
{
	glob++;
}
if ( glob != 11 ) print("g1");


tle(tle());
a = tle();
function tle
{
}




f(f(0));
a = 5;

++::a;
print( a );

test();

function test()
{
	b = 4000000;
	print( b );
	b = 20;
	print( b );
	b = 0;
	print( b );
	b = 1.2;
	print( b );

	b = 10;
	print( b );
	++::a;
	print( ::a );
	
}

test();


function test2(a)
{
	print( a );
	a = 20;
	print( a );
	
	print( ::a );
}

test2();
test2(1);
test2(2,3);

function test3(a,b)
{
	print( a );
	print( b );
	b = 30;
	print( b );
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

print( test5() );

function test6()
{
	b = 2;
	c = 3;
	d = 4;
	return c;
}
print( test6() );

function test7()
{
	return ::a;
}
print( test7() );


function fibonacci( n )
{
	if (n <= 1) return n;
	return fibonacci(n - 2) + fibonacci(n - 1);
}

for ( i = 0; i <= 10; ++i)
{
	print( fibonacci(i) );
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
print(a);
print(::a);
fa();
print(a);
g_fa();
print(a);
