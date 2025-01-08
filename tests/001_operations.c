/*~
10
-10
26
27
28
8
2
33
120
19
31
32
33
16.38181
90.1
8
1
4
15
15
27
27
12
3
3
13
3
13
10
3
3
3
3
10
1.3
~*/

//if ( () ) println ("");

if ( 10 / 0 ) {}
if ( 10.1 / 0 ) {}
var zero = 0;
if( 10 / zero ) {}
if( 10.1 / zero ) {}
zero = 0.5;
zero = 0;
if( 10 / zero ) {}
if( 10.1 / zero ) {}

if ( 10 /= 0 ){}
if ( 10.1 /= 0 ){}
zero = 0;
if( 10 /= zero ) {}
if( 10.1 /= zero ) {}
zero = 0.5;
zero = 0;
if( 10 /= zero ) {}
if( 10.1 /= zero ) {}




y = "BB";
var z = "CC";
var y1 = y + z;
y = y + z;
if ( y1 != "BBCC" ) print("fail bin1");
if ( y != "BBCC" ) print("fail bin1");



a = "BB";
b = "CC";
d = a + b;
a = a + b;
b = a + b;
if ( d != "BBCC" ) println("op 0");
if ( a != "BBCC" ) println("op 1");
if ( b != "BBCCCC" ) println("op 2");


two = 0;
s = 0xAABBCCDD;
r = s + two;
s = 0xA;
r = s + two;
s = 1.0;
r = s + two;

two = 0;
s = 0xAABBCCDD;
r = s + two;
if ( r != 0xAABBCCDD ) { println("o1"); }
s = 0xA;
r = s + two;
if ( r != 0xA ) println("o2");

s = 1.0;
r = s + two;
if ( r != 1.0) println(r);

loc();

function loc()
{
	ltwo = 0;
	ls = 0xAABBCCDD;
	lr = ls + ltwo;
	ls = 0xA;
	lr = ls + ltwo;
	ls = 1.0;
	lr = ls + ltwo;

	ltwo = 0;
	ls = 0xAABBCCDD;
	lr = ls + ltwo;
	if ( lr != 0xAABBCCDD ) { println("l1"); }
	ls = 0xA;
	lr = ls + ltwo;
	if ( lr != 0xA ) println("l2");
	ls = 1.0;
	lr = ls + ltwo;
	if ( lr != 1.0) println("l3");
}



// test casting
ii = 10.5;
j = (int)ii;
if ( ii != 10.5 ) { println("j0"); }
if ( j != 10 ) { println("j1"); }
ii = 10 / 1000;
if ( ii != 0 ) { println("ii 1"); }
ii = (float)10 / 1000;
if ( ii != .01 ) { println("ii 2"); }
ii = 10 / (float)1000;
if ( ii != .01 ) { println("ii 3"); }
ii = 67.2;
ii = (int)ii;
if ( ii != 67 ) { println("ii 4"); }


a[10] = { 10 };
if ( a[0] / 1000 != 0 ) { println("ii 4"); }
if ( (float)a[0] / 1000 != .01 ) { println("ii 5"); }
if ( a[0] / (float)1000 != .01 ) { println("ii 5"); }


not = 1;
if ( !not ) println("n1");
if ( not != 1 ) println("n2");
flip = 0x0000FFFF;
if ( ~flip != 0xFFFF0000 ) println("flip1");
if ( flip != 0x0000FFFF ) println("flip2");

a1 = b1 = c1 = 10;
if ( a1 != 10 || b1 != 10 || c1 != 10 ) println("err 0");

a = 1;
b = -1;
c = --a + ++b;
if ( c != 0 ) { println("err00"); }

a = 90;
a++;
++a;
b = a++;
c = ++a;
if ( a != 94 ) println("err 1");

a--;
--a;
b = a--;
c = --a;
if ( a != 90 ) println("err 2");

if ( .1 != 0.1 ) println( "val 0" );
if ( -.1 != -0.1 ) println( "val 1" );
if ( -.2 != -0.2 ) println( "val 2" );
if ( 0xF != 15 ) println( "val 3" );

b = 4000000;
if ( b != 4000000 ) println( "assign 0" );
if ( 4000000 != b ) println( "assign 1" );
b = 20;
if ( b != 20 ) println( "assign 2" );
if ( 20 != b ) println( "assign 3" );

b = 0;
if ( b != 0 ) println( "assign 4" );
if ( 0 != b ) println( "assign 5" );

b = 1.2;
if ( b != 1.2 ) println( "assign 6" );
if ( 1.2000 != b ) println( "assign 7" );


b = 10;
b += 5;
b /= 3;
b -= 2;
b *= 6;
if ( b != 18) println( "oper 0" );

a = 10;
a -= 3;
if ( a != 7 ) println( "oper 1" );
b = 10.2;
b -= .5 + (a += 1);

if ( b != 1.7 ) println( "oper 2" );
if ( a != 8 ) println( "oper 3" );

n = -(-(-(-(-(-10))))); // 10 
println(n);
n = -(-(-(-(-10)))); // -10 
println(n);

a = 25;
println( 1 + ++a - 1 );  // 26
println( ++a + 1 - 1 ); // 27
println( 1 - 1 + ++a ); // 28

println( 10 - 2 );
println( 10 / 5 );

a = 28;
println( 1 + a++ + 10 - 2 * 3 ); // 33

println( (40 + 50) * 4 / (((2 + 1))) ); // 120
println( 3 * 5 - 10 + 10 + 10 - 2 * 3 ); // 19
println( 1 + 1 + a++ ); // 31
println( a++ + 1 + 1 ); // 32 
println( 1 + a++ + 1 ); // 33

d = 90.1;
println( d /= 5.5 );
println( d *= 5.5 );

e = 0x04;
f = e << 1;
g = e >> 2;
println( f );
println( g );
println( e );

a = 10;
b = 12;
println( a += 5 ); // 15
println( a ); // 15
println( a += b ); // 27
println( a ); // 27
println( b ); // 12

a = 13;
b = 10;
println( 13 % 10 ); // 3
println( a % 10 ); // 3
println( a ); // 13
println( a % b ); // 3
println( a ); // 13
println( b ); // 10

println( a %= 10 ); // 3
println( a ); // 3
a = 13; 
println( a %= b ); // 3
println( a ); // 3
println( b ); // 10

a = 1.3;
println( a );

// // excercise keyhole optimizer loads

local();
ag = 8;
bg = 3;

function local()
{
	a = 8;
	b = 3;
	c = a % b;if ( c != 2 ) println("F1");
	c = a << b;if ( c != 64 ) println("F2");
	c = a >> b;if ( c != 1 ) println("F3");
	
	a = 9;
	c = a & b;if ( c != 1 ) println("F4");
	c = a ^ b;if ( c != 10 ) println("F5");
	a = 8;
	c = a | b;if ( c != 11 ) println("F6");

	::ag = 8;
	b = 3;
	c = ::ag % b;if ( c != 2 ) println("F1");
	c = ::ag << b;if ( c != 64 ) println("F2");
	c = ::ag >> b;if ( c != 1 ) println("F3");

	::ag = 9;
	c = ::ag & b;if ( c != 1 ) println("F4");
	c = ::ag ^ b;if ( c != 10 ) println("F5");
	::ag = 8;
	c = ::ag | b;if ( c != 11 ) println("F6");


	a = 8;
	::ab = 3;
	c = a % ::ab;if ( c != 2 ) println("F1");
	c = a << ::ab;if ( c != 64 ) println("F2");
	c = a >> ::ab;if ( c != 1 ) println("F3");

	a = 9;
	c = a & ::ab;if ( c != 1 ) println("F4");
	c = a ^ ::ab;if ( c != 10 ) println("F5");
	a = 8;
	c = a | ::ab;if ( c != 11 ) println("F6");

	::ag = 8;
	::bg = 3;
	c = ::ag % ::bg;if ( c != 2 ) println("F1");
	c = ::ag << ::bg;if ( c != 64 ) println("F2");
	c = ::ag >> ::bg;if ( c != 1 ) println("F3");

	::ag = 9;
	c = ::ag & ::bg;if ( c != 1 ) println("F4");
	c = ::ag ^ ::bg;if ( c != 10 ) println("F5");
	::ag = 8;
	c = ::ag | ::bg;if ( c != 11 ) println("F6");
	
}

var gn = 30;
function foo()
{
	var gn = 2;  // local 'n' is 2
	if ( ::gn != 30 ) println("gn0");
	::gn = 40;   // global 'n' was 30, will now be 40
	if ( gn != 2 ) println("gn1");
	if ( ::gn != 40 ) println("gn2");
}

if ( gn != 30 ) println("gn3");
foo();
if ( gn != 40 ) println("gn4");
if ( ::gn != 40 ) println("gn5");
