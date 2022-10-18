/*~
1
10
100
1.01
1.00001
0.00004
hello
hello
~*/



print( 1.0 );
print( 10.0 );
print( 100.0 );
print( 1.01 );
print( 1.00001 );
print( .00004 );



/***//*
 */
// comment

print( "hello" ); // eol comment
print( /**/"hello"  /*  */ ); /*  *//*
*/

a = 10 / 2;
if ( a != 5 ) print("err 1");
a = 10/	/*h*/2;
if ( a != 5 ) print("err 1");
a = 10/**//	/*h*/2;
if ( a != 5 ) print("err 1");


if ( 0b010 != 2 ) print( "err bin" );
if ( 0b != 0 ) print( "err bin2" );
