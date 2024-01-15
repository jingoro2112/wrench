/*~ ~*/

hashTest = { 1:10, 2:20, "three":30 };
if ( hashTest._exists(1) != 1 ) println("x1");
if ( hashTest._exists(3) != 0 ) println("x2");
if ( hashTest._exists("three") != 1 ) println("x3");
somethingH._remove( 1 );
if ( somethingH._exists(1) != 0 ) println("x4");
somethingH._remove( "three" );
if ( somethingH._exists("three") != 0 ) println("x4");

blue   = { 10:"one", 20:"two", "three":30 };
green[]  = { 1, 2, blue };
red[]    = { green, 3 };

colors[] = { red, red };

if ( colors[0][0][1] != 2 ) println("sub 0");
if ( colors[0][0][1] != 2 ) println("sub 1");
if ( colors[0][1] != 3 ) println("sub 2");
if ( blue[10] != "one" ) println("sub 3");
if ( colors[0][0][2]["three"] != 30 ) println("sub 4");
if ( colors[0][1][2]["three"] != 0 ) println("sub 5");


table =
{
	"1":1,
	"2":2,
	3:"3",
	4:"4"
};

if ( table["1"] != 1 ) println( "bad 1" );
if ( table["2"] != 2 ) println( "bad 2" );
if ( table[3] != "3" ) println( "bad 3" );
if ( table[4] != "4" ) println( "bad 4" );
if ( table[5] != 0 ) println( "bad 5" );


table2 =
{
	"1":1,
	"2":2,
};

if ( table2["2"] != 2 ) println( "bad 22" );

table[6] = 6;
table[6] = "7";
table[6] = 8;
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "9";
table[6] = "10";
if ( table[6] != "10" ) println( "bad 6" );


table[7] = "10";
table[8] = "10";
table[9] = "10";
table["a"] = "10";
table["b"] = "10";
table["c"] = "10";
table["d"] = "10";
if ( table[4] != "4" ) { println( "bad 4d" ); println(table[4]); }
goto j;
j:
table["e"] = "10";
if ( table[4] != "4" ) { println( "bad 4d" ); println(table[4]); }
table["f"] = "10";
table["g"] = "10";
table["h"] = "10";
table["i"] = "10";
table["j"] = "10";
table["k"] = "10";
table["7"] = "11";
table["m"] = "10";

if ( table.m != "10" ) println( "bad m" );

if ( table["1"] != 1 ) println( "bad 1a" );
if ( table["2"] != 2 ) println( "bad 2b" );
if ( table[3] != "3" ) println( "bad 3c" );
if ( table[4] != "4" ) { println( "bad 4d" ); println(table[4]); }
if ( table[5] != 0 ) println( "bad 5e" );

if ( table["7"] != "11" ) println( "bad 71" );

aa = {:};

a = { "a" : 2 }; // creating a blank hash table

a = "hello";
b = "hello";
if ( a != "hello" ) println("string bad 1");
if ( "hello" != a ) println("string bad 2");
if ( b != a ) println("string bad 3");
if ( "why?" != "why?" ) println("string bad 4");



a = 0;
if ( a._hash != 0 ) println( "bad 1") ;

a = 20;
if ( a._hash != 20 ) println( "bad 2") ;

a = "hello";
if ( a._hash != 1335831723 ) println("bad 3");


hash1 = { "one":100 };
hash2 = { "two":10, 1:hash1, "member":hash3 };
hash3 = { "three":30, 2:hash2 };

array1 = { 1, 2, hash3 };
array2 = { hash2, array1, 4 };
array3 = { 50, 60, array2, hash3, hash2 };

if ( array3[2][0]["two"] != 10 ) println("hsh 0");
if ( array3[2][0][1]["one"] != 100 ) println("hsh 1");
if ( array1[0] != 1 ) println("hsh 2" );
if ( array1[1] != 2 ) println("hsh 3" );
if ( 2 != array1[1] ) println("hsh 4" );
if ( 1 != array1[0] ) println("hsh 5" );
if ( array1[2]["three"] != 30 ) println("hsh 6" );
if ( array1[2][2]["two"] != 10 ) println("hsh 7" );
if ( hash1.one != 100 ) println("hsh 8" );
if ( hash2.two != 10 ) println("hsh 9" );

//if ( hash2.member["three"] != 30 ) println("hsh 9" );  // TODO (yuck)

