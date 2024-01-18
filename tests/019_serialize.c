/*~ ~*/

// check HASH
d = { 1:100, "2":200, "3":"300", 40:400, 50:500 };
d1 = std::serialize( d );
d2 = std::deserialize( d1 );
if ( d2[1] != 100 ) println("d0");
if ( d2["2"] != 200 ) println("d1");
if ( d2[50] != 500 ) println("d2");
if ( d2["3"] != "300" ) println("d3");

// check ARRAY
c[] = { 1, 2, 3 };
c1 = std::serialize( c );
c2 = std::deserialize( c1 );
if ( c2[1] != 2 ) println( "c0" );
if ( c2._count != 3 ) println( "c1" );

// check int and float
a = 10;
a1 = std::serialize( a );
a2 = std::deserialize( a1 );
if ( a2 != 10 ) println("a0");
a = 10.5;
a1 = std::serialize( a );
a2 = std::deserialize( a1 );
if ( a2 != 10.5 ) println("a1");

// String (char array)
b = "a test string";
b1 = std::serialize( b );
b2 = std::deserialize( b1 );
if ( b2 != "a test string" ) println( "b0" );

// and now ALL OF THEM
e = { b, a, c, d };
f = { 1:e, 2:e, 3:d };

f1 = std::serialize( f );
f2 = std::deserialize( f1 );

if( f2[1][1] != 10.5 ) println( "f0" );
if( f2[2][0] != "a test string" ) println( "f1" );
if( f2[3]["3"] != "300" ) println( "f2" );

// BTW encode/decode the 'busiest' one to make sure it plays well
// with the file system
io::writeFile( "_serialize_test.bin", std::serialize(f) );
g = std::deserialize( io::readFile( "_serialize_test.bin" ) );
if( g[1][1] != 10.5 ) println( "g0" );
if( g[2][0] != "a test string" ) println( "g1" );
if( g[3]["3"] != "300" ) println( "g2" );

