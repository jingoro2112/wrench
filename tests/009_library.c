/*~
1
1
1.5
30
5
8.5
-4
~*/

//var s;
//str::sprintf( s, 			"%0d", 1 );if ( s != "1" ) println("bad 017");

if ( io::O_RDWR != io::O_RDWR ) println( "c_err" );

msg::write( "key1", 10 );
if ( msg::peek("test") ) println("m1");
msg::write( 0, 20 );
msg::write( 1.4, 30 );
if ( msg::read("none") ) println("m2");
if ( msg::read("key1") != 10 ) println("m3");
if ( msg::read("key1") != 10 ) println("m4");
if ( msg::read("key1", true) != 10 ) println("m5");
if ( msg::read("key1", true) == 10 ) println("m6");
if ( !msg::peek(1.4) ) println("m7");
msg::clear(1.4);
if ( msg::peek(1.4) ) println("m8");


math::floor();
math::floor(1);
math::floor(1,2);
math::floor(1,2,3);

f = math::floor();
f = math::floor(1);
f = math::floor(1,2);
f = math::floor(1,2,3);

println( math::floor(1.5) );
println( math::floor(1) );

println( math::abs(-1.5) );
println( math::abs(-30) );

println( math::sqrt(25) );
println( math::sqrt(72.25) );

println( math::ceil(-4.1) );

//str::sprintf( s, 			"%-10.4g", 12.45 );if ( s != "12.45" ) println("bad 1245 ");

str::sprintf( s, 			"%0s", "x" );if ( s != "x" ) println("bad 001");
str::sprintf( s, 			"%-0s", "x" );if ( s != "x" ) println("bad 002");
str::sprintf( s, 			"%1s", "x" );if ( s != "x" ) println("bad 003");
str::sprintf( s, 			"%2s", "x" );if ( s != " x" ) println("bad 004");
str::sprintf( s, 			"%-2s", "x" );if ( s != "x " ) println("bad 005");
str::sprintf( s, 	"%9s", "x" );if ( s != "        x" ) println("bad 006");
str::sprintf( s, 	"%10s", "x" );if ( s != "         x" ) println("bad 007");
str::sprintf( s, 	"%11s", "x" );if ( s != "          x" ) println("bad 008");
str::sprintf( s, 				"%0s", "" );if ( s != "" ) println("bad 009");
str::sprintf( s, 				"%-0s", "" );if ( s != "" ) println("bad 010");
str::sprintf( s, 			"%1s", "" );if ( s != " " ) println("bad 011");
str::sprintf( s, 			"%2s", "" );if ( s != "  " ) println("bad 012");
str::sprintf( s, 			"%-2s", "" );if ( s != "  " ) println("bad 013");
str::sprintf( s, 	"%9s", "" );if ( s != "         " ) println("bad 014");
str::sprintf( s, 	"%10s", "" );if ( s != "          " ) println("bad 015");
str::sprintf( s, 	"%11s", "" );if ( s != "           " ) println("bad 016");
str::sprintf( s, 			"%0d", 1 );if ( s != "1" ) println("bad 017");
str::sprintf( s, 			"%01d", 1 );if ( s != "1" ) println("bad 018");
str::sprintf( s, 			"%02d", 1 );if ( s != "01" ) println("bad 019");
str::sprintf( s, 	"%09d", 1 );if ( s != "000000001" ) println("bad 020");
str::sprintf( s, 	"%010d", 1 );if ( s != "0000000001" ) println("bad 021");
str::sprintf( s, 	"%011d", 1 );if ( s != "00000000001" ) println("bad 022");
str::sprintf( s, 			"%04s", "abc" );if ( s != " abc" ) println("bad 023");
str::sprintf( s, 				"%s", "" );if ( s != "" ) println("bad 024");
str::sprintf( s, 			"%s", "abc" );if ( s != "abc" ) println("bad 025");
str::sprintf( s, 			"%d", 0 );if ( s != "0" ) println("bad 027");
str::sprintf( s, 			"%d", 1 );if ( s != "1" ) println("bad 028");
str::sprintf( s, 			"%d", 9 );if ( s != "9" ) println("bad 029");
str::sprintf( s, 			"%d", 10 );if ( s != "10" ) println("bad 030");
str::sprintf( s, 			"%d", 101 );if ( s != "101" ) println("bad 031");
str::sprintf( s, 		"%d", 10000 );if ( s != "10000" ) println("bad 032");
str::sprintf( s, 			"%d", 0 );if ( s != "0" ) println("bad 033");
str::sprintf( s, 		"%d", 32767 );if ( s != "32767" ) println("bad 034");
str::sprintf( s, 			"%d", -1 );if ( s != "-1" ) println("bad 035");
str::sprintf( s, 		"%d", -32767 );if ( s != "-32767" ) println("bad 036");
str::sprintf( s, 		"%d", -32768 );if ( s != "-32768" ) println("bad 037");
str::sprintf( s, 			"%u", 0 );if ( s != "0" ) println("bad 038");
str::sprintf( s, 		"%u", 32767 );if ( s != "32767" ) println("bad 039");
str::sprintf( s, 		"%u", 32768 );if ( s != "32768" ) println("bad 040");
str::sprintf( s, 		"%u", 65535 );if ( s != "65535" ) println("bad 041");
str::sprintf( s, 			"%x", 0 );if ( s != "0" ) println("bad 042");
str::sprintf( s, 			"%x", 1 );if ( s != "1" ) println("bad 043");
str::sprintf( s, 			"%x", 9 );if ( s != "9" ) println("bad 044");
str::sprintf( s, 			"%x", 0xa );if ( s != "a" ) println("bad 045");
str::sprintf( s, 			"%x", 0xf );if ( s != "f" ) println("bad 046");
str::sprintf( s, 			"%x", 0x10 );if ( s != "10" ) println("bad 047");
str::sprintf( s, 			"%x", 0xa55a );if ( s != "a55a" ) println("bad 048");
str::sprintf( s, 			"%x", 0xffff );if ( s != "ffff" ) println("bad 049");
str::sprintf( s, 			"%x", 0xA );if ( s != "a" ) println("bad 050");
str::sprintf( s, 			"%X", 0xA );if ( s != "A" ) println("bad 051");
str::sprintf( s, 			"%x", 0x89AB );if ( s != "89ab" ) println("bad 052");
str::sprintf( s, 			"%i", 123 );if ( s != "123" ) println("bad 053");
str::sprintf( s, 			"%d", 1 );if ( s != "1" ) println("bad 054");
str::sprintf( s, 			"%%d", 1 );if ( s != "%d" ) println("bad 055");
str::sprintf( s, 			"%%%d", 1 );if ( s != "%1" ) println("bad 056");
str::sprintf( s, 			"%%%%d", 1 );if ( s != "%%d" ) println("bad 057");
str::sprintf( s, 			"%c", 'a' );if ( s != "a" ) println("bad 058");
str::sprintf( s, 			"%d %d %d", 1, 0, -1 );if ( s != "1 0 -1") println("bad 059");
str::sprintf( s, "%d %d %d %d", -10, -30000, 10, 30000 );if ( s != "-10 -30000 10 30000") println("bad 060");
str::sprintf( s, "%u %u %u %u", -10, -30000, 10, 30000 );if ( s != "4294967286 4294937296 10 30000") println("bad 061");
str::sprintf( s, 			"%p", 0x123A );if ( s != "0x123A") println("bad 062");
str::sprintf( s, 					"%c", 'a' );if ( s != "a") println("bad 063");
str::sprintf( s, 				"%c ", 'a' );if ( s != "a ") println("bad 064");
str::sprintf( s, 				" %c", 'a' );if ( s != " a") println("bad 065");
str::sprintf( s, 		"%b", 0x0A05 );if ( s != "101000000101") println("bad 066");
str::sprintf( s, 	"%b", 0xF00F );if ( s != "1111000000001111") println("bad 067");
str::sprintf( s, 					"%b", 0x1 );if ( s != "1") println("bad 068");
str::sprintf( s, 					"%b", 0x0 );if ( s != "0") println("bad 069");
str::sprintf( s, 					"%s", "" );if ( s != "") println("bad 070");
str::sprintf( s, 				"%s", "test" );if ( s != "test") println("bad 071");
str::sprintf( s, 				"%2s", "test" );if ( s != "test") println("bad 072");
str::sprintf( s, 			"%6s", "test" );if ( s != "  test") println("bad 073");
str::sprintf( s, 			"%06s", "test" );if ( s != "  test") println("bad 074");
str::sprintf( s, 				"%-2s", "test" );if ( s != "test") println("bad 075");
str::sprintf( s, 			"%-6s", "test" );if ( s != "test  ") println("bad 076");
str::sprintf( s, 					"" );if ( s != "") println("bad 077");
str::sprintf( s, 					"%%" );if ( s != "%") println("bad 078");
str::sprintf( s, 				"%d", 123 );if ( s != "123") println("bad 079");
str::sprintf( s, 				"%5d", 123 );if ( s != "  123") println("bad 080");
str::sprintf( s, 				"%%7d", 123 );if ( s != "%7d") println("bad 081");
str::sprintf( s, 				"%%2d", 123 );if ( s != "%2d") println("bad 082");
str::sprintf( s, 				"%%05d", 123 );if ( s != "%05d") println("bad 083");
str::sprintf( s, 				"%%02d", 123 );if ( s != "%02d") println("bad 084");
str::sprintf( s, 				"%%-5d", 123 );if ( s != "%-5d") println("bad 085");
str::sprintf( s, 				"%%-2d", 123 );if ( s != "%-2d") println("bad 086");
str::sprintf( s, 				"%%-05d", 123 );if ( s != "%-05d") println("bad 087");
str::sprintf( s, 				"%%-07d", 123 );if ( s != "%-07d") println("bad 088");
str::sprintf( s, 				"%%-02d", 123 );if ( s != "%-02d") println("bad 089");
str::sprintf( s, 				"%%x", 123 );if ( s != "%x") println("bad 090");
str::sprintf( s, 				"%%5x", 123 );if ( s != "%5x") println("bad 091");
str::sprintf( s, 				"%%7x", 123 );if ( s != "%7x") println("bad 092");
str::sprintf( s, 				"%%2x", 123 );if ( s != "%2x") println("bad 093");
str::sprintf( s, 				"%%05x", 123 );if ( s != "%05x") println("bad 094");
str::sprintf( s, 				"%%02x", 123 );if ( s != "%02x") println("bad 095");
str::sprintf( s, 				"%%-5x", 123 );if ( s != "%-5x") println("bad 096");
str::sprintf( s, 				"%%-2x", 123 );if ( s != "%-2x") println("bad 097");
str::sprintf( s, 				"%%-05x", 123 );if ( s != "%-05x") println("bad 098");
str::sprintf( s, 				"%%-07x", 123 );if ( s != "%-07x") println("bad 099");
str::sprintf( s, 				"%%-02x", 123 );if ( s != "%-02x") println("bad 100");
str::sprintf( s, 				"%%X", 60000 );if ( s != "%X") println("bad 101");
str::sprintf( s, 				"%%5X", 60000 );if ( s != "%5X") println("bad 102");
str::sprintf( s, 				"%%7X", 60000 );if ( s != "%7X") println("bad 103");
str::sprintf( s, 				"%%2X", 60000 );if ( s != "%2X") println("bad 104");
str::sprintf( s, 				"%%05X", 60000 );if ( s != "%05X") println("bad 105");
str::sprintf( s, 				"%%02X", 60000 );if ( s != "%02X") println("bad 106");
str::sprintf( s, 				"%%-5X", 60000 );if ( s != "%-5X") println("bad 107");
str::sprintf( s, 				"%%-2X", 60000 );if ( s != "%-2X") println("bad 108");
str::sprintf( s, 				"%%-05X", 60000 );if ( s != "%-05X") println("bad 109");
str::sprintf( s, 				"%%-07X", 60000 );if ( s != "%-07X") println("bad 110");
str::sprintf( s, 				"%%-02X", 60000 );if ( s != "%-02X") println("bad 111");
str::sprintf( s, 	"%s %-5d %05d", "test", 123, 123 );if ( s != "test 123   00123") println("bad 112");


s0 = "";
s1 = "1";
s2 = "22";
s3 = "1234";
if ( str::strlen("1") != 1 ) println("strlen 1");
if ( str::strlen("") != 0 ) println("strlen 2");
if ( str::strlen("123") != 3 ) println("strlen 3");
if ( str::strlen(s1) != 1 ) println("strlen 1a");
if ( str::strlen(s0) != 0 ) println("strlen 2a");
if ( str::strlen(s3) != 4 ) println("strlen 3a");

if ( str::format( "%s", "hi" ) != "hi" ) println("format 1");

if ( !str::isspace(' ') ) println("isspace 1");
if ( !str::isspace('\t') ) println("isspace 2");
if ( !str::isspace('\n') ) println("isspace 3");
c0 = ' ';
c1 = '\t';
c2 = '\n';
if ( !str::isspace(c0) ) println("isspace 1a");
if ( !str::isspace(c1) ) println("isspace 2a");
if ( !str::isspace(c2) ) println("isspace 3a");
if ( str::isspace('_') ) println("isspace 1b");
if ( str::isspace('\0') ) println("isspace 2b");
if ( str::isspace('A') ) println("isspace 3b");

if ( !str::isdigit('1') ) println("isdigit 1");
if ( !str::isdigit('0') ) println("isdigit 2");
if ( str::isdigit('x') ) println("isdigit 3");

if ( str::isalpha('1') ) println("isalpha 1");
if ( !str::isalpha('a') ) println("isalpha 2");
if ( str::isalpha('_') ) println("isalpha 3");

if ( str::mid("1", 1) != "" ) println("mid 1");
if ( str::mid("22", 1) != "2" ) println("mid 2");
if ( str::mid("22", 0) != "22" ) println("mid 3");
if ( str::mid("1234", 0, 1) != "1" ) println("mid 4");
if ( str::mid("1234", 1, 2) != "23" ) println("mid 5");
if ( str::mid(s1, 1) != "" ) println("mid 1a");
if ( str::mid(s2, 1) != "2" ) println("mid 2a");
if ( str::mid(s2, 0) != "22" ) println("mid 3a");
if ( str::mid(s3, 0, 1) != "1" ) println("mid 4a");
if ( str::mid(s3, 1, 2) != "23" ) println("mid 5a");

if ( str::tolower('A') != 'a' ) println("tolower 1");
if ( str::tolower('a') != 'a' ) println("tolower 2");

if ( str::toupper('a') != 'A' ) println("toupper 1");
if ( str::toupper('A') != 'A' ) println("toupper 2");

s = "#()*+,-/:;<=>?@\\^";
if ( str::chr("#()*+,-/:;<=>?@\\^", '') != -1 ) println("strchr 2");
if ( str::chr(s, '#') != 0 ) println("strchr 1");

if ( str::chr("#()*+,-/:;<=>?@\\^", '\\') != 15 ) println("strchr 3");
if ( str::chr("#()*+,-/:;<=>?@\\^", '^') != 16 ) println("strchr 4");
if ( str::chr("#()*+,-/:;<=>?@\\^", 'A') != -1 ) println("strchr 5");

a = "hello";
b = "world";
if ( str::concat( "hello", "world" ) != "helloworld" ) println( "concat0" );
if ( str::concat( a, "world" ) != "helloworld" ) println( "concat1" );
if ( str::concat( "hello", b ) != "helloworld" ) println( "concat2" );
if ( str::concat( a, b ) != "helloworld" ) println( "concat3" );

if ( str::left( "hello", 0 ) != "" ) println( "left0");
if ( str::left( "hello", 1 ) != "h" ) println( "left1");
if ( str::left( "hello", 2 ) != "he" ) println( "left2");
if ( str::left( "hello", 5 ) != "hello" ) println( "left3");
if ( str::left( "hello", 15 ) != "hello" ) println( "left4");
if ( str::left( a, 0 ) != "" ) println( "left5");
if ( str::left( a, 1 ) != "h" ) println( "left6");
if ( str::left( a, 2 ) != "he" ) println( "left7");
if ( str::left( a, 5 ) != "hello" ) println( "left8");
if ( str::left( a, 15 ) != "hello" ) println( "left9");

if ( str::trunc( "hello", 0 ) != "" ) println( "trunc0");
if ( str::trunc( "hello", 1 ) != "h" ) println( "trunc1");
if ( str::trunc( "hello", 2 ) != "he" ) println( "trunc2");
if ( str::trunc( "hello", 5 ) != "hello" ) println( "trunc3");
if ( str::trunc( "hello", 15 ) != "hello" ) println( "trunc4");
if ( str::trunc( a, 0 ) != "" ) println( "trunc5");
if ( str::trunc( a, 1 ) != "h" ) println( "trunc6");
if ( str::trunc( a, 2 ) != "he" ) println( "trunc7");
if ( str::trunc( a, 5 ) != "hello" ) println( "trunc8");
if ( str::trunc( a, 15 ) != "hello" ) println( "trunc9");

if ( str::trimright("") != "" ) println("tright0");
if ( str::trimright(" ") != "" ) println("tright01");
if ( str::trimright("123") != "123" ) println("tright1");
if ( str::trimright("123 ") != "123" ) println("tright2");
if ( str::trimright(" 123 ") != " 123" ) println("tright3");
if ( str::trimright(" 123") != " 123" ) println("tright4");

if ( str::trimleft("") != "" ) println("tleft0");
if ( str::trimleft(" ") != "" ) println("tleft10");
if ( str::trimleft("123") != "123" ) println("tleft1");
if ( str::trimleft("123 ") != "123 " ) println("tleft2");
if ( str::trimleft(" 123 ") != "123 " ) println("tleft3");
if ( str::trimleft(" 123") != "123" ) println("tleft4");

if ( str::trim("") != "" ) println("t0");
if ( str::trim(" ") != "" ) println("t10");
if ( str::trim("123") != "123" ) println("t1");
if ( str::trim("123 ") != "123" ) println("t2");
if ( str::trim(" 123 ") != "123" ) println("t3");
if ( str::trim(" 123") != "123" ) println("t4");

if ( str::insert( "", "", 0 ) != "" ) println("ins0");
if ( str::insert( "", "", 10 ) != "" ) println("ins1");
if ( str::insert( "11", "", 0 ) != "11" ) println("ins2");
if ( str::insert( "11", "", 10 ) != "11" ) println("ins3");
if ( str::insert( "", "11", 0 ) != "11" ) println("ins4");
if ( str::insert( "", "11", 10 ) != "11" ) println("ins5");
if ( str::insert( "11", "22", 0 ) != "2211" ) println("ins6");
if ( str::insert( "11", "22", 1 ) != "1221" ) println("ins7");
if ( str::insert( "11", "22", 2 ) != "1122" ) println("ins8");
if ( str::insert( "11", "22", 12 ) != "1122" ) println("ins9");

if ( str::tolower( "PPlPP" ) != "pplpp" ) println("tol0");
if ( str::toupper( "hNh" ) != "HNH" ) println("tou0");

function f1() { }

if ( sys::isFunction("println") != 1 ) println("sys0");
if ( sys::isFunction("foo") != 0 ) println("sys1");
if ( sys::isFunction("f1") != 2 ) println("sys2");
if ( sys::isFunction("f2") != 2 ) println("sys3");
if ( sys::isFunction() != 0 ) println("sys4");

function f2() { }

