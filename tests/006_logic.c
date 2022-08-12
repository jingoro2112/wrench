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
log( f += i ); // 3.5
log( f );
log( i );

// int <- int
i = 10;
j = 3;
log( i += j ); // 13
log( i );
log( j );

// int <- float
i = 10;
f = .5;
log( i += f ); // 10.5
log( i );
log( f );

// float <- float
g = 10.2;
f = .5;
log( g += f ); // 10.7
log( g );
log( f );

if ( !1 )
{
	log("1");
}

a = 1;
if ( a )
{
	log("T");
}
else
{
	log("F");
}

if ( !a )
{
	log("T");
}
else
{
	log("F");
}



a = 10.5;

if ( a > 10 ) log("T1");
if ( a < 11 ) log("T2");
if ( a >= 10.5 ) log("T3");
if ( a >= 10 ) log("T4");
if ( a < 10.6 ) log("T5");
if ( a <= 10.5 ) log("T6");
if ( a <= 11 ) log("T7");
if ( a == 10.5 ) log("T8");
if ( a != 1) log("T9");

if ( a <= 10 ) log("T1");
if ( a >= 11 ) log("T2");
if ( a < 10.5 ) log("T3");
if ( a < 10 ) log("T4");
if ( a >= 10.6 ) log("T5");
if ( a > 10.5 ) log("T6");
if ( a > 11 ) log("T7");
if ( a != 10.5 ) log("T8");
if ( a == 1) log("T9");

b = 0;
if ( a && b ) log("ab");
if ( b && a ) log("ab");
if ( a && b ) log("ab"); else log("ba");
if ( b && a ) log("ab"); else log("ba");
if ( a || b ) log("a||b");
if ( b || a ) log("a||b");

a = 0;
b = 1;
if ( a && b ) log("ab");
if ( b && a ) log("ab");
if ( a && b ) log("ab"); else log("ba");
if ( b && a ) log("ab"); else log("ba");
if ( a || b ) log("a||b");
if ( b || a ) log("a||b");

b = 0;
if ( a && b ) log("ab");
if ( b && a ) log("ab");
if ( a && b ) log("ab"); else log("ba");
if ( b && a ) log("ab"); else log("ba");
if ( a || b ) log("a||b");
if ( b || a ) log("a||b");

a = 1;
b = 1;
if ( a && b ) log("ab");
if ( b && a ) log("ab");
if ( a && b ) log("ab"); else log("ba");
if ( b && a ) log("ab"); else log("ba");

a = 0;
if ( a && b ) log("ab");
if ( b && a ) log("ab");

a = 1;
b = 0;
if ( a && b ) log("ab");
if ( b && a ) log("ab");

a = 0;
if ( a || b ) log("a||b");
if ( b || a ) log("a||b");

a = 1;
b = 0;
if ( a || b ) log("a||b");
if ( b || a ) log("a||b");