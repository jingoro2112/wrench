/*~
0
1
10
100
1.01
1.00001
0.00004
hello
hello
~*/


// test floating/integer point string converter
print( 0.0 );
print( 1.0 );
print( 10.0 );
print( 100.0 );
print( 1.01 );
print( 1.00001 );
print( .00004 );

if ( "\097" != "a" ) print("a ar");
if ( '\0' != 0 ) print("zero char");
if ( 'a' != 0x61 ) print("err a");
if ( '\r' != 0x0d ) print("err a3");
if ( '\n' != 0x0a ) print("err a4");
if ( '' != 0 ) print("err a5");
if ( 1 != 1 ) print("err one");


// test crazy comment combinations

/***//*
 */
// comment

gc_pause( 0xABCD );

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
