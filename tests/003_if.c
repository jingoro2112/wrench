/*~
a
b
c
d
e
f
yes
no
no
yes
yes2
10
made it
here too
yay
yay2
yay3
~*/

a = 8;
b = 7;
if ( a == b && 1 ) println("G1");
if ( a != a && 1 ) println("G2");
if ( a < b && 1 )  println("G3");
if ( b > a && 1 )  println("G4");
if ( b >= a && 1 ) println("G5");
if ( a <= b && 1 ) println("G6");

localIf();
function localIf()
{
	a = 8;
	b = 7;
	if ( a == b && 1 ) println("L1");
	if ( a != a && 1 ) println("L2");
	if ( a < b && 1 )  println("L3");
	if ( b > a && 1 )  println("L4");
	if ( b >= a && 1 ) println("L5");
	if ( a <= b && 1 ) println("L6");

	if ( 11 < 10 ) println("La fail");
	if ( 9 > 10 ) println("Lb fail");
	if ( 11 <= 10 ) println("Lc fail");
	if ( 9 >= 10 ) println("Ld fail");
	if ( 9 == 10 ) println("Le fail");
	if ( 10 != 10 ) println("Lf faile");
}

if ( n != n )
{
	println( "null NOT okay" );
}

if ( 9 < 10 ) println("a");
if ( 11 < 10 ) println("a fail");
if ( 9 > 10 ) println("b fail");
if ( 11 > 10 ) println("b");
if ( 10 <= 10 ) println("c");
if ( 11 <= 10 ) println("c fail");
if ( 10 >= 10 ) println("d");
if ( 9 >= 10 ) println("d fail");
if ( 10 == 10 ) println("e");
if ( 9 == 10 ) println("e fail");
if ( 9 != 10 ) println("f");
if ( 10 != 10 ) println("f fail");


a = 10;


if ( a == 10 )
	println( "yes" );
else
	println( "no" );


if ( a != 10 )
	println( "yes" );
else
{
	println( "no" );
}

if ( a != 10 )
{
	println( "yes" );
}
else
	println( "no" );

if ( a == 10 )
	println( "yes" );

println("yes2");

if ( n != n )
{
	println( "null NOT okay" );
}

c = 4;
d = 6;

println(8 + 3 + c++ - --d) ;

			
c = 4;
d = 6;
a = 10;
b = 20;

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 10) )
	if ( b = 5 )
		if ( c = 5 )
		{
			if ( b + c == 10 )
				println( "made it" );

			println( "here too" );
		}
	else
		println( "no" );

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 1) )
{
	if ( b = 5 )
		
		if ( c = 5 )
		{
			if ( b + c == 10 )
				println( "made it" );
			println( "here too" );
		}
		else
			println( "no" );
}
else
{
	println( "yay" );
}

a = 10;
b = 2.3;
if ( !a || !b )
{
	println( "boo" );
}

a = 0;
if ( !a || !b )
{
	println( "yay2" );
}

if ( !a && !b )
{
	println( "boo" );
}

b = 0;
if ( !a && !b )
{
	println( "yay3" );
}
