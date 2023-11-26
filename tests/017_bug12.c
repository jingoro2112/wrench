/*~
0.2
5
0.1
-8
8
-18
4
~*/

b = 0.0;
b1 = 0.0;
b2 = 0.0;
b3 = 0.0;
a = 10.0;

function init()
{ 
	b = millis()/200.0;
}

function test( i,j )
{
	print( i / a );
	print( a / i );
	print( i / j );

	print( i - a );
	print( a - i );
	print( i - j );

}

test( 2.0, 20.0 );

function faulty_func()
{
	t = 1.0;
	x = t * 4.0;  // <-- any arithmetic operation involving a literal
	// can be x = 1.0 + 2.0;
	z = x * t; // <-- any binary operation on two variables
	return z;
}

print( faulty_func() );
