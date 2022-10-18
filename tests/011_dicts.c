/*~ ~*/


table =
{
	"1":1,
	"2":2,
	3:"3",
	4:"4"
};

if ( table["1"] != 1 ) print( "bad 1" );
if ( table["2"] != 2 ) print( "bad 2" );
if ( table[3] != "3" ) print( "bad 3" );
if ( table[4] != "4" ) print( "bad 4" );
if ( table[5] != 0 ) print( "bad 5" );

table2 =
{
	"1":1,
	"2":2,
};

if ( table2["2"] != 2 ) print( "bad 22" );

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
if ( table[6] != "10" ) print( "bad 6" );


table[7] = "10";
table[8] = "10";
table[9] = "10";
table["a"] = "10";
table["b"] = "10";
table["c"] = "10";
table["d"] = "10";
if ( table[4] != "4" ) { print( "bad 4d" ); print(table[4]); }
goto j;
j:
table["e"] = "10";
if ( table[4] != "4" ) { print( "bad 4d" ); print(table[4]); }
table["f"] = "10";
table["g"] = "10";
table["h"] = "10";
table["i"] = "10";
table["j"] = "10";
table["k"] = "10";
table["7"] = "11";
table["m"] = "10";

if ( table.m != "10" ) print( "bad m" );

if ( table["1"] != 1 ) print( "bad 1a" );
if ( table["2"] != 2 ) print( "bad 2b" );
if ( table[3] != "3" ) print( "bad 3c" );
if ( table[4] != "4" ) { print( "bad 4d" ); print(table[4]); }
if ( table[5] != 0 ) print( "bad 5e" );

if ( table["7"] != "11" ) print( "bad 71" );

aa = {:};

a = { "a" : 2 }; // creating a blank hash table

a = "hello";
b = "hello";
if ( a != "hello" ) print("string bad 1");
if ( "hello" != a ) print("string bad 2");
if ( b != a ) print("string bad 3");
if ( "why?" != "why?" ) print("string bad 4");



a = 0;
if ( a._hash != 0 ) print( "bad 1") ;

a = 20;
if ( a._hash != 20 ) print( "bad 2") ;

a = "hello";
if ( a._hash != 1335831723 ) print("bad 3");

// TODO: for "hash table" which the code has and I can basically do for
// just the cost of adding to the compiler


/* 

a = { key:value, key:value }; // initting a hash table

a._size(); // how many value sare in it
a.key ; // syntactic sugar for a[key] ?


for( key,value : a ) // iterating key and value
{
	
}


for( value : a ) // iterating value only
{
	_.remove(); // removing the current entry
}

*/
