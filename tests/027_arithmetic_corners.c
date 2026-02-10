/*~ ~*/

// 32-bit signed boundary wrap checks
var imax = 2147483647;
if ( imax + 1 != -2147483648 ) println("ac1");
if ( imax + 2 != -2147483647 ) println("ac2");

var imin = -2147483648;
if ( imin - 1 != 2147483647 ) println("ac3");
if ( -imin != -2147483648 ) println("ac4");

// Signed division/modulo corner behavior
if ( -7 / 3 != -2 ) println("ac5");
if ( 7 / -3 != -2 ) println("ac6");
if ( -7 / -3 != 2 ) println("ac7");

if ( -13 % 5 != -3 ) println("ac8");
if ( 13 % -5 != 3 ) println("ac9");
if ( -13 % -5 != -3 ) println("ac10");

// Operator precedence/associativity corners
if ( 1 + 2 << 3 != 24 ) println("ac11");
if ( (1 | 2 & 4) != 1 ) println("ac12");
if ( (1 ^ 2 & 3) != 3 ) println("ac13");
if ( 16 >> 1 + 1 != 4 ) println("ac14");
if ( 3 + 4 * 5 - 6 / 2 != 20 ) println("ac15");

// Compound assignment around boundaries
var a = 0x7FFFFFFE;
a += 1;
if ( a != 2147483647 ) println("ac16");
a += 1;
if ( a != -2147483648 ) println("ac17");
a -= 2;
if ( a != 2147483646 ) println("ac18");

// Mixed int/float arithmetic around truncation and sign
var f = -5;
f /= 2;
if ( f != -2 ) println("ac19");

f = -5.0;
f /= 2;
if ( f != -2.5 ) println("ac20");

var m = (int)(-1.9) * 3 + (int)(2.9);
if ( m != -1 ) println("ac21");
