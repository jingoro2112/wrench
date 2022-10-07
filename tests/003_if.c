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
if ( a == b && 1 ) print("G1");
if ( a != a && 1 ) print("G2");
if ( a < b && 1 )  print("G3");
if ( b > a && 1 )  print("G4");
if ( b >= a && 1 ) print("G5");
if ( a <= b && 1 ) print("G6");

localIf();
function localIf()
{
	a = 8;
	b = 7;
	if ( a == b && 1 ) print("L1");
	if ( a != a && 1 ) print("L2");
	if ( a < b && 1 )  print("L3");
	if ( b > a && 1 )  print("L4");
	if ( b >= a && 1 ) print("L5");
	if ( a <= b && 1 ) print("L6");

	if ( 11 < 10 ) print("La fail");
	if ( 9 > 10 ) print("Lb fail");
	if ( 11 <= 10 ) print("Lc fail");
	if ( 9 >= 10 ) print("Ld fail");
	if ( 9 == 10 ) print("Le fail");
	if ( 10 != 10 ) print("Lf faile");
}

if ( n != n )
{
	print( "null NOT okay" );
}

if ( 9 < 10 ) print("a");
if ( 11 < 10 ) print("a fail");
if ( 9 > 10 ) print("b fail");
if ( 11 > 10 ) print("b");
if ( 10 <= 10 ) print("c");
if ( 11 <= 10 ) print("c fail");
if ( 10 >= 10 ) print("d");
if ( 9 >= 10 ) print("d fail");
if ( 10 == 10 ) print("e");
if ( 9 == 10 ) print("e fail");
if ( 9 != 10 ) print("f");
if ( 10 != 10 ) print("f fail");


a = 10;


if ( a == 10 )
	print( "yes" );
else
	print( "no" );


if ( a != 10 )
	print( "yes" );
else
{
	print( "no" );
}

if ( a != 10 )
{
	print( "yes" );
}
else
	print( "no" );

if ( a == 10 )
	print( "yes" );

print("yes2");

if ( n != n )
{
	print( "null NOT okay" );
}

c = 4;
d = 6;

print(8 + 3 + c++ - --d) ;

			
c = 4;
d = 6;
a = 10;
b = 20;

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 10) )
	if ( b = 5 )
		if ( c = 5 )
		{
			if ( b + c == 10 )
				print( "made it" );

			print( "here too" );
		}
	else
		print( "no" );

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 1) )
{
	if ( b = 5 )
		
		if ( c = 5 )
		{
			if ( b + c == 10 )
				print( "made it" );
			print( "here too" );
		}
		else
			print( "no" );
}
else
{
	print( "yay" );
}

a = 10;
b = 2.3;
if ( !a || !b )
{
	print( "boo" );
}

a = 0;
if ( !a || !b )
{
	print( "yay2" );
}

if ( !a && !b )
{
	print( "boo" );
}

b = 0;
if ( !a && !b )
{
	print( "yay3" );
}
