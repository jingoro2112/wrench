/*~ ~*/

var res[] = { "a", 2, 0.3 };
checkIsWrenchArray( res, 3 );
checkIter( res, "a", 2, 0.3 );

var h = {:};
checkIsHashTable( h, 0 );
h["one"] = 1;
h["two"] = 2;
checkIsHashTable( h, 2 );


var s = "a string";
checkIsString( s, s._count );


//------------------------------------------------------------------------------
function userCheck( data )
{
	checkIsRawArray( data.big, data.big._count );
return;	
	if ( data.b != data.c ) println("b001");
	
	if ( data.b != 0x99 ) println("b99");
	data.b = 10;
	if ( data.b != 10 ) println("b0");
	if ( (data.b += 20) != 30 ) println ("b1");
	
	if ( data.b != 30 ) println("b2");
	if ( 30 != data.b ) println("b3");
	if ( 2 + data.b != 32 ) println("b4");
	if ( data.b + 4 != 34 ) println("b5");

	if ( ++data.b != 31 ) println("b6");
	if ( data.b != 31 ) println("b7");
	if ( data.b++ != 31 ) println("b8");
	if ( data.b != 32 ) println("b9");

	if ( data.name[0] != 'h' ) println("f5");
	if ( data.name[1] != 'e' ) println("f6");

	i = 0;
	j = 1;
	if ( data.name[i++] != 'h' ) println("f5i");
	if ( data.name[i] != 'e' ) println("f5i");
	if ( data.name[j] != 'e' ) println("f6j");

	if ( data.name[1] < 'd' ) println("f7");
	if ( data.name[1] > 'f' ) println("f8");

	if ( data.name[1] != data.name[1] ) println("f71");
	if ( data.name[1] < data.name[1] ) println("f72");
	if ( data.name[1] > data.name[1] ) println("f73");
	a = 0x64;
	if ( data.name[1] == a ) println("f74");
	if ( a == data.name[1] ) println("f75");
	if ( !(a < data.name[1]) ) println("f76");
	if ( !(data.name[1] > a) ) println("f77");

	if ( data.big[0] != 10 ) println("f9");
	if ( data.big[10000] != 20 ) println("f10");
	if ( data.big[100000] != 30 ) println("f11");
	if ( data.big[0x1FFFFE] != 40 ) println("f12");
	if ( data.big[0x2FFFFE] != 0 ) println("f13");

	if ( -data.big[0] != -10 ) println("f14");
	if ( data.big[0] != 10 ) println("f15");

	data.name[1] = 'c';
	
	if ( data.integer != 2456 ) println("f1");
	if ( data["integer"] != 2456 ) println("f2");

	if ( data._i != 1001 ) println("ii1");
	if ( data._f != 20.02 ) println("ff1");
	data.integer = 56789;
	
	return 77;
}

function stringCheck( string )
{
	if ( string != "test string" ) println("bs1");
	c = string;
	if ( c != "test string" ) println ("bs2" );
}

function arrayCheck()
{
	newState = "some string";
	newStateDuration = 0.0;
	resetCollision = false;

	res[] = { newState, newStateDuration, resetCollision };

	return  res;
}

