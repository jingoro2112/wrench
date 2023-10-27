/*~ ~*/

//------------------------------------------------------------------------------
function userCheck( data )
{
	if ( data.b != data.c ) print("b001");
	
	if ( data.b != 0x99 ) print("b99");
	data.b = 10;
	if ( data.b != 10 ) print("b0");
	if ( (data.b += 20) != 30 ) print ("b1");
	if ( data.b != 30 ) print("b2");
	if ( 30 != data.b ) print("b3");
	if ( 2 + data.b != 32 ) print("b4");
	if ( data.b + 4 != 34 ) print("b5");
	if ( ++data.b != 31 ) print("b6");
	if ( data.b != 31 ) print("b7");
	if ( data.b++ != 31 ) print("b8");
	if ( data.b != 32 ) print("b9");

	if ( data.name[0] != 'h' ) print("f5");
	if ( data.name[1] != 'e' ) print("f6");

	i = 0;
	j = 1;
	if ( data.name[i++] != 'h' ) print("f5i");
	if ( data.name[i] != 'e' ) print("f5i");
	if ( data.name[j] != 'e' ) print("f6j");

	rem::setRGB( -1, -1, data.name[i], data.name[i], data.name[i] );

	i = math::sin( data.name[i] );

	if ( data.name[1] < 'd' ) print("f7");
	if ( data.name[1] > 'f' ) print("f8");

	if ( data.name[1] != data.name[1] ) print("f71");
	if ( data.name[1] < data.name[1] ) print("f72");
	if ( data.name[1] > data.name[1] ) print("f73");
	a = 0x64;
	if ( data.name[1] == a ) print("f74");
	if ( a == data.name[1] ) print("f75");
	if ( !(a < data.name[1]) ) print("f76");
	if ( !(data.name[1] > a) ) print("f77");

	if ( data.big[0] != 10 ) print("f9");
	if ( data.big[10000] != 20 ) print("f10");
	if ( data.big[100000] != 30 ) print("f11");
	if ( data.big[0x1FFFFE] != 40 ) print("f12");
	if ( data.big[0x2FFFFE] != 0 ) print("f13");

	if ( -data.big[0] != -10 ) print("f14");
	if ( data.big[0] != 10 ) print("f15");

	data.name[1] = 'c';
	
	if ( data.integer != 2456 ) print("f1");
	if ( data["integer"] != 2456 ) print("f2");

	data.integer = 56789;
	
	return 77;
}

function stringCheck( string )
{
	if ( string != "test string" ) print("bs1");
	c = string;
	if ( c != "test string" ) print ("bs2" );
}
