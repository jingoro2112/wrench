/*~ ~*/

//------------------------------------------------------------------------------
function userCheck( data )
{

	if ( data.name[0] != 'h' ) print("f5");
	if ( data.name[1] != 'e' ) print("f6");

	if ( data.name[1] < 'd' ) print("f7");
	if ( data.name[1] > 'f' ) print("f8");

	data.name[1] = 'c';
	
	if ( data.integer != 2456 ) print("f1");
	if ( data["integer"] != 2456 ) print("f2");

	data.integer = 56789;

	return 77;
}
