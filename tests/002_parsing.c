/*~
)
;
}
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

println(")");
println(";");
println("}");

var a = {};

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
if ( 0x13 != 19 ) println("err hex");
if ( 023 != 19 ) println("err octal");

// test crazy comment combinations

/***//*
 */
// comment

println( "hello" ); // eol comment
println( /**/"hello"  /*  */ ); /*  *//*
*/

a = 10 / 2;
if ( a != 5 ) println("err 1");
a = 10/	/*h*/2;
if ( a != 5 ) println("err 1");
a = 10/**//	/*h*/2;
if ( a != 5 ) println("err 1");

