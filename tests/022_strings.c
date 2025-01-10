/*~ ~*/

var s = "string";
if ( s + 123 != "string123" ) println( "st_err0" );
if ( "string" + 123 != "string123" ) println( "st_err1" );
if ( 123 + "string" != "123string" ) println( "st_err2" );
if ( 123 + s != "123string" ) println( "st_err3" );
if ( s != "string" ) println( "st_err4" );
if ( (s += 123) != "string123" ) println ( "st_err5" );
var i = 123;
if ( (i += "string") != "123string" ) println ( "st_err6" );
if ( (123 + "string" + 34 + "hi") != "123string34hi" ) println ( "st_err7" );

	
var h = "Hello " + "World";
if ( h != "Hello World" ) println( "hw_err0" );
var h1 = "Hello ";
var w1 = "World";
if ( h1 + w1 != "Hello World" ) println( "hw_err1" );
if ( h1 + "World" != "Hello World" ) println( "hw_err2" );
if ( "Hello " + w1 != "Hello World" ) println( "hw_err3" );
if ( "Hello " + "World" != "Hello World" ) println( "hw_err4" );
if ( h1 != "Hello " ) println( "hw_err5" );
if ( w1 != "World" ) println( "hw_err5" );


var h2 = "Hello ";
var w2 = "World";
if ( (h2 += w2) != "Hello World" ) println( "hw_err6" );
h2 = "Hello ";
if ( (h2 += "World") != "Hello World" ) println( "hw_err7" );

h2 = "Hello ";
var h3 = "foo";
if ( (h3 = h2 + w2) != "Hello World" ) println( "hw_err8" );
h3 = "foo";
if ( (h3 = h2 + "World") != "Hello World" ) println( "hw_err9" );
h3 = "foo";
if ( h3 = "Hello " + "World" != "Hello World" ) println( "hw_err10" );

var a1 = { "Hello " };
var a2 = { "w":"World" };
if ( a1[0] + "World" != "Hello World" ) println( "hw_err11" );
if ( a1[0] + a2["w"] != "Hello World" ) println( "hw_err12" );
if ( "Hello " + a2["w"] != "Hello World" ) println( "hw_err13" );


var y = "hello";
y += "hi";
if ( y != "hellohi" ) print("hw_err14");
