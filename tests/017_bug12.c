/*~
0.2
5
0.1
-8
8
-18
4
~*/

var b = 0.0;
var b1 = 0.0;
var b2 = 0.0;
var b3 = 0.0;
var a = 10.0;

function init()
{ 
	b = millis()/200.0;
}

function test( i,j )
{
	println( i / a );
	println( a / i );
	println( i / j );

	println( i - a );
	println( a - i );
	println( i - j );

}

test( 2.0, 20.0 );

function faulty_func()
{
	var t = 1.0;
	var x = t * 4.0;  // <-- any arithmetic operation involving a literal
	// can be x = 1.0 + 2.0;
	var z = x * t; // <-- any binary operation on two variables
	return z;
}

println( faulty_func() );
