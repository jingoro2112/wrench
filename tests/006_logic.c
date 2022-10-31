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

abc();

a = 10;
b = 5;
c = 0;

if ( 10 <= a ) { c += 0x1000; }
if ( 10 >= a ) { c += 0x1000; }
if ( 10 == b ) { print("CB1"); }
if ( 10 != a ) { print("CB2"); }
if ( 10 < b )  { print("CB3"); }
if ( b > 10 )  { print("CB4"); }
if ( 10 <= b ) { print("CB8"); }
if ( b >= 10 ) { print("CB8"); }

if ( 10 <= a && 10 <= a  ) { c += 0x10; }
if ( 10 >= a && 10 >= a ) { c += 0x10; }
if ( 10 == b && 10 == b ) { print("BB1"); }
if ( 10 != a && 10 != a ) { print("BB2"); }
if ( 10 < b && 10 < b ) { print("BB3"); }
if ( b > 10 && b > 10 ) { print("BB4"); }
if ( 10 <= b && 10 <= b ) { print("BB8"); }
if ( b >= 10 && b >= 10 ) { print("BB8"); }

if ( a <= a && a <= a  ) { c += 0x1; }
if ( a >= a && a >= a ) { c += 0x1; }
if ( a == b && a == b ) { print("AB1"); }
if ( a != a && a != a ) { print("AB2"); }
if ( a < b && a < b ) { print("AB3"); }
if ( b > a && b > a ) { print("AB4"); }
if ( a <= b && a <= b ) { print("AB8"); }
if ( b >= a && b >= a ) { print("AB8"); }


if ( a <= a ) { c += 0x100; }
if ( a >= a ) { c += 0x100; }
if ( a == b ) { print("CB1"); }
if ( a != a ) { print("CB2"); }
if ( a < b ) { print("CB3"); }
if ( b > a ) { print("CB4"); }
if ( a <= b ) { print("CB8"); }
if ( b >= a ) { print("CB8"); }


if ( c != 0x2222 ) { print("err C:"); print(c); }

function abc()
{
	aa = 10;
	ba = 5;
	ca = 0;

	if ( 10 <= aa ) { ca += 0x1000; }
	if ( 10 >= aa ) { ca += 0x1000; }
	if ( 10 == ba ) { print("L CB1"); }
	if ( 10 != aa ) { print("L CB2"); }
	if ( 10 < ba )  { print("L CB3"); }
	if ( ba > 10 )  { print("L CB4"); }
	if ( 10 <= ba ) { print("L CB8"); }
	if ( ba >= 10 ) { print("L CB8"); }
	
	if ( 10 <= aa && 10 <= aa  ) { ca += 0x10; }
	if ( 10 >= aa && 10 >= aa ) { ca += 0x10; }
	if ( 10 == ba && 10 == ba ) { print("L BB1"); }
	if ( 10 != aa && 10 != aa ) { print("L BB2"); }
	if ( 10 < ba && 10 < ba ) { print("L BB3"); }
	if ( ba > 10 && ba > 10 ) { print("L BB4"); }
	if ( 10 <= ba && 10 <= ba ) { print("L BB8"); }
	if ( ba >= 10 && ba >= 10 ) { print("L BB8"); }

	if ( aa >= aa && aa >= aa ) { ca += 1; }
	if ( aa <= aa && aa <= aa ) { ca += 1; }
	if ( aa == ba && aa == ba ) { print("L AB1"); }
	if ( aa != aa && aa != aa ) { print("L AB2"); }
	if ( aa < ba && aa < ba ) { print("L AB3"); }
	if ( ba > aa && ba > aa ) { print("L AB4"); }
	if ( aa <= ba && aa <= ba ) { print("L AB8"); }
	if ( ba >= aa && ba >= aa ) { print("L AB8"); }

	if ( aa >= aa ) { ca += 0x100; }
	if ( aa <= aa ) { ca += 0x100; }
	if ( aa == ba ) { print("L AB1"); }
	if ( aa != aa ) { print("L AB2"); }
	if ( aa < ba ) { print("L AB3"); }
	if ( ba > aa ) { print("L AB4"); }
	if ( aa <= ba ) { print("L AB8"); }
	if ( ba >= aa ) { print("L AB8"); }

	if ( ca != 0x2222 ) { print("L err C:"); print(ca); }
}

a = 10.5;
if ( 10.5 < 10 || 10.5 < 10 ) print("E T1");
if ( 10.5 <= 10 || 10.5 <= 10 ) print("E T11");
if ( 10 > 10.5 || 10 > 10.5 ) print("E T2");
if ( 10 >= 10.5 || 10 >= 10.5 ) print("E T21");
if ( 10.5 == 10 || 10.5 == 10 ) print("E 3");
if ( 10 != 10 || 10 != 10 ) print("E 4");
if ( !(10 <= 10 || 10 <= 10) ) print("E 5");
if ( !(10 >= 10 || 10 >= 10) ) print("E 6");



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
