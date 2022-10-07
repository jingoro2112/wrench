/*~
3.5
3.5
3
13
13
3
10.5
10.5
0.5
10.7
10.7
0.5
T
F
T1
T2
T3
T4
T5
T6
T7
T8
T9
ba
ba
a||b
a||b
ba
ba
a||b
a||b
ba
ba
ab
ab
ab
ab
a||b
a||b
~*/

// float <- int
f = .5;
i = 3;
print( f += i ); // 3.5
print( f );
print( i );

// int <- int
i = 10;
j = 3;
print( i += j ); // 13
print( i );
print( j );

// int <- float
i = 10;
f = .5;
print( i += f ); // 10.5
print( i );
print( f );

// float <- float
g = 10.2;
f = .5;
print( g += f ); // 10.7
print( g );
print( f );

if ( !1 )
{
	print("1");
}

a = 1;
if ( a )
{
	print("T");
}
else
{
	print("F");
}

if ( !a )
{
	print("T");
}
else
{
	print("F");
}



a = 10.5;

if ( a > 10 ) print("T1");
if ( a < 11 ) print("T2");
if ( a >= 10.5 ) print("T3");
if ( a >= 10 ) print("T4");
if ( a < 10.6 ) print("T5");
if ( a <= 10.5 ) print("T6");
if ( a <= 11 ) print("T7");
if ( a == 10.5 ) print("T8");
if ( a != 1) print("T9");

if ( a <= 10 ) print("B1");
if ( a >= 11 ) print("B2");
if ( a < 10.5 ) print("B3");
if ( a < 10 ) print("B4");
if ( a >= 10.6 ) print("B5");
if ( a > 10.5 ) print("B6");
if ( a > 11 ) print("B7");
if ( a != 10.5 ) print("B8");
if ( a == 1) print("B9");


a = 1000;
if ( a > 9000 ) print("B10");
if ( a >= 9000 ) print("B11");
if ( a < 900 ) print("B12");
if ( a <= 900 ) print("B13");
if ( a == 900 ) print("B14");
if ( a != 1000 ) print("B15");
if ( 9000 < a ) print("B101");
if ( 9000 <= a ) print("B111");
if ( 900 > a ) print("B121");
if ( 900 >= a ) print("B131");
if ( 900 == a ) print("B141");
if ( 1000 != a ) print("B151");


a = 100000;
if ( a > 900000 ) print("B14");
if ( a >= 900000 ) print("B15");
if ( a < 90000 ) print("B16");
if ( a <= 90000 ) print("B17");
if ( a == 90000 ) print("B18");
if ( a != 100000 ) print("B19");
if ( 900000 < a ) print("C10");
if ( 900000 <= a ) print("C11");
if ( 90000 > a ) print("C12");
if ( 90000 >= a ) print("C13");
if ( 90000 == a ) print("C14");
if ( 100000 != a ) print("C15");


b = 0;
if ( a && b ) print("ab");
if ( b && a ) print("ab");
if ( a && b ) print("ab"); else print("ba");
if ( b && a ) print("ab"); else print("ba");
if ( a || b ) print("a||b");
if ( b || a ) print("a||b");

a = 0;
b = 1;
if ( a && b ) print("ab");
if ( b && a ) print("ab");
if ( a && b ) print("ab"); else print("ba");
if ( b && a ) print("ab"); else print("ba");
if ( a || b ) print("a||b");
if ( b || a ) print("a||b");

b = 0;
if ( a && b ) print("ab");
if ( b && a ) print("ab");
if ( a && b ) print("ab"); else print("ba");
if ( b && a ) print("ab"); else print("ba");
if ( a || b ) print("a||b");
if ( b || a ) print("a||b");

a = 1;
b = 1;
if ( a && b ) print("ab");
if ( b && a ) print("ab");
if ( a && b ) print("ab"); else print("ba");
if ( b && a ) print("ab"); else print("ba");

a = 0;
if ( a && b ) print("ab");
if ( b && a ) print("ab");

a = 1;
b = 0;
if ( a && b ) print("ab");
if ( b && a ) print("ab");

a = 0;
if ( a || b ) print("a||b");
if ( b || a ) print("a||b");

a = 1;
b = 0;
if ( a || b ) print("a||b");
if ( b || a ) print("a||b");
