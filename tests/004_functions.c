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
10
20
~*/


tle(tle());
a = tle();
function tle
{
}


f(f(0));
a = 5;

++::a;
log( a );

test();

function test()
{
	b = 4000000;
	log( b );
	b = 20;
	log( b );
	b = 0;
	log( b );
	b = 1.2;
	log( b );

	b = 10;
	log( b );
	++::a;
	log( ::a );
	
}

test();


function test2(a)
{
	log( a );
	a = 20;
	log( a );
	
	log( ::a );
}

test2();
test2(1);
test2(2,3);

function test3(a,b)
{
	log( a );
	log( b );
	b = 30;
	log( b );
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

log( test5() );

function test6()
{
	b = 2;
	c = 3;
	d = 4;
	return c;
}
log( test6() );

function test7()
{
	return ::a;
}
log( test7() );


function fibonacci( n )
{
	if (n <= 1) return n;
	return fibonacci(n - 2) + fibonacci(n - 1);
}

for ( i = 0; i <= 10; ++i)
{
	log( fibonacci(i) );
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
log(a);
log(::a);
fa();
log(a);
g_fa();
log(a);
