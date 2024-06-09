/*~
one
two
3
6
6
7
seven
~*/

hashTable = { 1:"one", 2:"two", 3:3, "str":6 };

if ( hashTable[1] != "one" ) println("h0");
println( hashTable[1] ); // "one"
if ( hashTable[2] != "two" ) println("h1");
println( hashTable[2] ); // "two"
if ( hashTable[3] != 3 ) println("h2");
println( hashTable[3] ); // 3

println( hashTable["str"] ); // 6

println( hashTable.str ); // 6

hashTable.str = 7;
println( hashTable.str ); // 6
hashTable.str = "seven";
println( hashTable.str ); // 6
