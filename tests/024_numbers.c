/*~ ~*/

var n = -123.456;
n += 0.456;
if ( n != -123 ) println("n0");
n = -1;
n += 3;
if ( n != 2 ) println("n1");
n = -400;
n += 500;
if ( n != 100 ) println("n2");
n = -500000;
n += 600000;
if ( n != 100000 ) println("n3");

var a = 0xFFFFFFFF;
var b = 4294967295;
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
