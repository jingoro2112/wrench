/*~ ~*/


//------------------------------------------------------------------------------
function userCheck( data )
{
	if ( data.name[0] != 'h' ) print("f5");
	if ( data.name[1] != 'e' ) print("f6");

	if ( data.name[1] < 'd' ) print("f7");
	if ( data.name[1] > 'f' ) print("f8");

	if ( data.integer != 765 ) print("f1");
	if ( data["integer"] != 765 ) print("f2");

	if ( data.integer2 != 222 ) print("f3");
	if ( data.fl != 1.123 ) print("f4");

	return;
}
