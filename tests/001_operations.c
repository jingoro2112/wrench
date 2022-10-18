/*~
94
90
hello world
0.1
-0.1
-0.2
0.2
15
-5
4000000
20
0
1.2
18
7
1.7
8
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
1
~*/

if ( 'a' != 0x61 ) print("err a");
if ( '\r' != 0x0d ) print("err a3");
if ( '\n' != 0x0a ) print("err a4");
if ( '' != 0 ) print("err a5");
if ( 1 != 1 ) print("err one");

c = --a + a-- + ++a + a--;

a = 90;
a++;
++a;
b = a++;
c = ++a;
print(a);
a--;
--a;
b = a--;
c = --a;
print(a);

print( "hello world" );

print( .1 );
print( -.1 );
print( -0.2 );
print( 0.2 );
print( 0xF );
print( -5 );

b = 4000000;
print( b );
b = 20;
print( b );

b = 0;
print( b );

b = 1.2;
print( b );


b = 10;
b += 5;
b /= 3;
b -= 2;
b *= 6;
print( b ); // 18

a = 10;
a -= 3;
print(a); // 7
b = 10.2;
b -= .5 + (a += 1);
print(b); // 7
print(a); // 1.7

n = -(-(-(-(-(-10))))); // 10 
print(n);
n = -(-(-(-(-10)))); // -10 
print(n);

a = 25;
print( 1 + ++a - 1 );  // 26
print( ++a + 1 - 1 ); // 27
print( 1 - 1 + ++a ); // 28

print( 10 - 2 );
print( 10 / 5 );

a = 28;
print( 1 + a++ + 10 - 2 * 3 ); // 33

print( (40 + 50) * 4 / (((2 + 1))) ); // 120
print( 3 * 5 - 10 + 10 + 10 - 2 * 3 ); // 19
print( 1 + 1 + a++ ); // 31
print( a++ + 1 + 1 ); // 32 
print( 1 + a++ + 1 ); // 33

d = 90.1;
print( d /= 5.5 );
print( d *= 5.5 );

e = 0x04;
f = e << 1;
g = e >> 2;
print( f );
print( g );
print( e );

a = 10;
b = 12;
print( a += 5 ); // 15
print( a ); // 15
print( a += b ); // 27
print( a ); // 27
print( b ); // 12

a = 13;
b = 10;
print( 13 % 10 ); // 3
print( a % 10 ); // 3
print( a ); // 13
print( a % b ); // 3
print( a ); // 13
print( b ); // 10

print( a %= 10 ); // 3
print( a ); // 3
a = 13; 
print( a %= b ); // 3
print( a ); // 3
print( b ); // 10

a = 1.3;
print( a );
(int)a;
print( a );

// excercise keyhole optimizer loads
local();
ag = 8;
bg = 3;
function local()
{
	a = 8;
	b = 3;
	c = a % b;if ( c != 2 ) print("F1");
	c = a << b;if ( c != 64 ) print("F2");
	c = a >> b;if ( c != 1 ) print("F3");
	
	a = 9;
	c = a & b;if ( c != 1 ) print("F4");
	c = a ^ b;if ( c != 10 ) print("F5");
	a = 8;
	c = a | b;if ( c != 11 ) print("F6");

	::ag = 8;
	b = 3;
	c = ::ag % b;if ( c != 2 ) print("F1");
	c = ::ag << b;if ( c != 64 ) print("F2");
	c = ::ag >> b;if ( c != 1 ) print("F3");

	::ag = 9;
	c = ::ag & b;if ( c != 1 ) print("F4");
	c = ::ag ^ b;if ( c != 10 ) print("F5");
	::ag = 8;
	c = ::ag | b;if ( c != 11 ) print("F6");


	a = 8;
	::ab = 3;
	c = a % ::ab;if ( c != 2 ) print("F1");
	c = a << ::ab;if ( c != 64 ) print("F2");
	c = a >> ::ab;if ( c != 1 ) print("F3");

	a = 9;
	c = a & ::ab;if ( c != 1 ) print("F4");
	c = a ^ ::ab;if ( c != 10 ) print("F5");
	a = 8;
	c = a | ::ab;if ( c != 11 ) print("F6");

	::ag = 8;
	::bg = 3;
	c = ::ag % ::bg;if ( c != 2 ) print("F1");
	c = ::ag << ::bg;if ( c != 64 ) print("F2");
	c = ::ag >> ::bg;if ( c != 1 ) print("F3");

	::ag = 9;
	c = ::ag & ::bg;if ( c != 1 ) print("F4");
	c = ::ag ^ ::bg;if ( c != 10 ) print("F5");
	::ag = 8;
	c = ::ag | ::bg;if ( c != 11 ) print("F6");
	
}



