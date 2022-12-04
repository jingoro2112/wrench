/*~ ~*/

//------------------------------------------------------------------------------
function userCheck( data )
{
	if ( data.name[0] != 'h' ) print("f5");
	if ( data.name[1] != 'e' ) print("f6");

	if ( data.name[1] < 'd' ) print("f7");
	if ( data.name[1] > 'f' ) print("f8");
	
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
