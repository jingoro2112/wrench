/*~
6
10
7
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
~*/

f(f(0));
a = 5;

++::a;
log( a );
test();

unit test()
{
	b = 10;
	log( b );
	++::a;
	log( ::a );
}

test();


unit test2(a)
{
	log( a );
	a = 20;
	log( a );
	
	log( ::a );
}

test2();
test2(1);
test2(2,3);

unit test3(a,b)
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


unit test4()
{
}

test4();
test4(1);
test4(2,3);
test4(4,5,6 );


unit test5()
{
	return 5;
}

log( test5() );

unit test6()
{
	b = 2;
	c = 3;
	d = 4;
	return c;
}
log( test6() );

unit test7()
{
	return ::a;
}
log( test7() );


unit fibonacci( n )
{
	if (n <= 1) return n;
	return fibonacci(n - 2) + fibonacci(n - 1);
}

for ( i = 0; i <= 10; ++i)
{
	log( fibonacci(i) );
}
