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

a = 0xFFFFFFFF;
b = 4294967295;
if ( a != b ) println("r1");
a++;
b++;
if ( a != b ) println("r2");
a = -2147483648;
b = 0x80000000;
if ( a != b ) println("r3");
a++;
b++;
if ( a != b ) println("r4");

if ( 0b010 != 2 ) println( "err bin" );
if ( 0b != 0 ) println( "err bin2" );

if ( 10f != 10.f ) { println("f1"); }

// test floating/integer point string converter
println( 0.0 );
println( 1.0 );
println( 10.0 );
println( 100.0 );
println( 1.01 );
println( 1.00001 );
println( .00004 );

if ( "\097" != "a" ) println("a ar");
if ( '\0' != 0 ) println("zero char");
if ( 'a' != 0x61 ) println("err a");
if ( '\r' != 0x0d ) println("err a3");
if ( '\n' != 0x0a ) println("err a4");
if ( '' != 0 ) println("err a5");
if ( 1 != 1 ) println("err one");


// test crazy comment combinations

/***//*
 */
// comment

gc_pause( 0xABCD );

println( "hello" ); // eol comment
println( /**/"hello"  /*  */ ); /*  *//*
*/

a = 10 / 2;
if ( a != 5 ) println("err 1");
a = 10/	/*h*/2;
if ( a != 5 ) println("err 1");
a = 10/**//	/*h*/2;
if ( a != 5 ) println("err 1");

