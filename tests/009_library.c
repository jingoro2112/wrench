/*~
1
1
1.5
30
5
8.5
-4
~*/

math::floor();
math::floor(1);
math::floor(1,2);
math::floor(1,2,3);

f = math::floor();
f = math::floor(1);
f = math::floor(1,2);
f = math::floor(1,2,3);

print( math::floor(1.5) );
print( math::floor(1) );

print( math::abs(-1.5) );
print( math::abs(-30) );

print( math::sqrt(25) );
print( math::sqrt(72.25) );

print( math::ceil(-4.1) );

str::sprintf( s, 			"%0s", "x" );if ( s != "x" ) print("bad 001");
str::sprintf( s, 			"%-0s", "x" );if ( s != "x" ) print("bad 002");
str::sprintf( s, 			"%1s", "x" );if ( s != "x" ) print("bad 003");
str::sprintf( s, 			"%2s", "x" );if ( s != " x" ) print("bad 004");
str::sprintf( s, 			"%-2s", "x" );if ( s != "x " ) print("bad 005");
str::sprintf( s, 	"%9s", "x" );if ( s != "        x" ) print("bad 006");
str::sprintf( s, 	"%10s", "x" );if ( s != "         x" ) print("bad 007");
str::sprintf( s, 	"%11s", "x" );if ( s != "          x" ) print("bad 008");
str::sprintf( s, 				"%0s", "" );if ( s != "" ) print("bad 009");
str::sprintf( s, 				"%-0s", "" );if ( s != "" ) print("bad 010");
str::sprintf( s, 			"%1s", "" );if ( s != " " ) print("bad 011");
str::sprintf( s, 			"%2s", "" );if ( s != "  " ) print("bad 012");
str::sprintf( s, 			"%-2s", "" );if ( s != "  " ) print("bad 013");
str::sprintf( s, 	"%9s", "" );if ( s != "         " ) print("bad 014");
str::sprintf( s, 	"%10s", "" );if ( s != "          " ) print("bad 015");
str::sprintf( s, 	"%11s", "" );if ( s != "           " ) print("bad 016");
str::sprintf( s, 			"%0d", 1 );if ( s != "1" ) print("bad 017");
str::sprintf( s, 			"%01d", 1 );if ( s != "1" ) print("bad 018");
str::sprintf( s, 			"%02d", 1 );if ( s != "01" ) print("bad 019");
str::sprintf( s, 	"%09d", 1 );if ( s != "000000001" ) print("bad 020");
str::sprintf( s, 	"%010d", 1 );if ( s != "0000000001" ) print("bad 021");
str::sprintf( s, 	"%011d", 1 );if ( s != "00000000001" ) print("bad 022");
str::sprintf( s, 			"%04s", "abc" );if ( s != " abc" ) print("bad 023");
str::sprintf( s, 				"%s", "" );if ( s != "" ) print("bad 024");
str::sprintf( s, 			"%s", "abc" );if ( s != "abc" ) print("bad 025");
str::sprintf( s, 			"%d", 0 );if ( s != "0" ) print("bad 027");
str::sprintf( s, 			"%d", 1 );if ( s != "1" ) print("bad 028");
str::sprintf( s, 			"%d", 9 );if ( s != "9" ) print("bad 029");
str::sprintf( s, 			"%d", 10 );if ( s != "10" ) print("bad 030");
str::sprintf( s, 			"%d", 101 );if ( s != "101" ) print("bad 031");
str::sprintf( s, 		"%d", 10000 );if ( s != "10000" ) print("bad 032");
str::sprintf( s, 			"%d", 0 );if ( s != "0" ) print("bad 033");
str::sprintf( s, 		"%d", 32767 );if ( s != "32767" ) print("bad 034");
str::sprintf( s, 			"%d", -1 );if ( s != "-1" ) print("bad 035");
str::sprintf( s, 		"%d", -32767 );if ( s != "-32767" ) print("bad 036");
str::sprintf( s, 		"%d", -32768 );if ( s != "-32768" ) print("bad 037");
str::sprintf( s, 			"%u", 0 );if ( s != "0" ) print("bad 038");
str::sprintf( s, 		"%u", 32767 );if ( s != "32767" ) print("bad 039");
str::sprintf( s, 		"%u", 32768 );if ( s != "32768" ) print("bad 040");
str::sprintf( s, 		"%u", 65535 );if ( s != "65535" ) print("bad 041");
str::sprintf( s, 			"%x", 0 );if ( s != "0" ) print("bad 042");
str::sprintf( s, 			"%x", 1 );if ( s != "1" ) print("bad 043");
str::sprintf( s, 			"%x", 9 );if ( s != "9" ) print("bad 044");
str::sprintf( s, 			"%x", 0xa );if ( s != "a" ) print("bad 045");
str::sprintf( s, 			"%x", 0xf );if ( s != "f" ) print("bad 046");
str::sprintf( s, 			"%x", 0x10 );if ( s != "10" ) print("bad 047");
str::sprintf( s, 			"%x", 0xa55a );if ( s != "a55a" ) print("bad 048");
str::sprintf( s, 			"%x", 0xffff );if ( s != "ffff" ) print("bad 049");
str::sprintf( s, 			"%x", 0xA );if ( s != "a" ) print("bad 050");
str::sprintf( s, 			"%X", 0xA );if ( s != "A" ) print("bad 051");
str::sprintf( s, 			"%x", 0x89AB );if ( s != "89ab" ) print("bad 052");
str::sprintf( s, 			"%i", 123 );if ( s != "123" ) print("bad 053");
str::sprintf( s, 			"%d", 1 );if ( s != "1" ) print("bad 054");
str::sprintf( s, 			"%%d", 1 );if ( s != "%d" ) print("bad 055");
str::sprintf( s, 			"%%%d", 1 );if ( s != "%1" ) print("bad 056");
str::sprintf( s, 			"%%%%d", 1 );if ( s != "%%d" ) print("bad 057");
str::sprintf( s, 			"%c", 'a' );if ( s != "a" ) print("bad 058");
str::sprintf( s, 			"%d %d %d", 1, 0, -1 );if ( s != "1 0 -1") print("bad 059");
str::sprintf( s, "%d %d %d %d", -10, -30000, 10, 30000 );if ( s != "-10 -30000 10 30000") print("bad 060");
str::sprintf( s, "%u %u %u %u", -10, -30000, 10, 30000 );if ( s != "4294967286 4294937296 10 30000") print("bad 061");
str::sprintf( s, 			"%p", 0x123A );if ( s != "0x123A") print("bad 062");
str::sprintf( s, 					"%c", 'a' );if ( s != "a") print("bad 063");
str::sprintf( s, 				"%c ", 'a' );if ( s != "a ") print("bad 064");
str::sprintf( s, 				" %c", 'a' );if ( s != " a") print("bad 065");
str::sprintf( s, 		"%b", 0x0A05 );if ( s != "101000000101") print("bad 066");
str::sprintf( s, 	"%b", 0xF00F );if ( s != "1111000000001111") print("bad 067");
str::sprintf( s, 					"%b", 0x1 );if ( s != "1") print("bad 068");
str::sprintf( s, 					"%b", 0x0 );if ( s != "0") print("bad 069");
str::sprintf( s, 					"%s", "" );if ( s != "") print("bad 070");
str::sprintf( s, 				"%s", "test" );if ( s != "test") print("bad 071");
str::sprintf( s, 				"%2s", "test" );if ( s != "test") print("bad 072");
str::sprintf( s, 			"%6s", "test" );if ( s != "  test") print("bad 073");
str::sprintf( s, 			"%06s", "test" );if ( s != "  test") print("bad 074");
str::sprintf( s, 				"%-2s", "test" );if ( s != "test") print("bad 075");
str::sprintf( s, 			"%-6s", "test" );if ( s != "test  ") print("bad 076");
str::sprintf( s, 					"" );if ( s != "") print("bad 077");
str::sprintf( s, 					"%%" );if ( s != "%") print("bad 078");
str::sprintf( s, 				"%d", 123 );if ( s != "123") print("bad 079");
str::sprintf( s, 				"%5d", 123 );if ( s != "  123") print("bad 080");
str::sprintf( s, 				"%%7d", 123 );if ( s != "%7d") print("bad 081");
str::sprintf( s, 				"%%2d", 123 );if ( s != "%2d") print("bad 082");
str::sprintf( s, 				"%%05d", 123 );if ( s != "%05d") print("bad 083");
str::sprintf( s, 				"%%02d", 123 );if ( s != "%02d") print("bad 084");
str::sprintf( s, 				"%%-5d", 123 );if ( s != "%-5d") print("bad 085");
str::sprintf( s, 				"%%-2d", 123 );if ( s != "%-2d") print("bad 086");
str::sprintf( s, 				"%%-05d", 123 );if ( s != "%-05d") print("bad 087");
str::sprintf( s, 				"%%-07d", 123 );if ( s != "%-07d") print("bad 088");
str::sprintf( s, 				"%%-02d", 123 );if ( s != "%-02d") print("bad 089");
str::sprintf( s, 				"%%x", 123 );if ( s != "%x") print("bad 090");
str::sprintf( s, 				"%%5x", 123 );if ( s != "%5x") print("bad 091");
str::sprintf( s, 				"%%7x", 123 );if ( s != "%7x") print("bad 092");
str::sprintf( s, 				"%%2x", 123 );if ( s != "%2x") print("bad 093");
str::sprintf( s, 				"%%05x", 123 );if ( s != "%05x") print("bad 094");
str::sprintf( s, 				"%%02x", 123 );if ( s != "%02x") print("bad 095");
str::sprintf( s, 				"%%-5x", 123 );if ( s != "%-5x") print("bad 096");
str::sprintf( s, 				"%%-2x", 123 );if ( s != "%-2x") print("bad 097");
str::sprintf( s, 				"%%-05x", 123 );if ( s != "%-05x") print("bad 098");
str::sprintf( s, 				"%%-07x", 123 );if ( s != "%-07x") print("bad 099");
str::sprintf( s, 				"%%-02x", 123 );if ( s != "%-02x") print("bad 100");
str::sprintf( s, 				"%%X", 60000 );if ( s != "%X") print("bad 101");
str::sprintf( s, 				"%%5X", 60000 );if ( s != "%5X") print("bad 102");
str::sprintf( s, 				"%%7X", 60000 );if ( s != "%7X") print("bad 103");
str::sprintf( s, 				"%%2X", 60000 );if ( s != "%2X") print("bad 104");
str::sprintf( s, 				"%%05X", 60000 );if ( s != "%05X") print("bad 105");
str::sprintf( s, 				"%%02X", 60000 );if ( s != "%02X") print("bad 106");
str::sprintf( s, 				"%%-5X", 60000 );if ( s != "%-5X") print("bad 107");
str::sprintf( s, 				"%%-2X", 60000 );if ( s != "%-2X") print("bad 108");
str::sprintf( s, 				"%%-05X", 60000 );if ( s != "%-05X") print("bad 109");
str::sprintf( s, 				"%%-07X", 60000 );if ( s != "%-07X") print("bad 110");
str::sprintf( s, 				"%%-02X", 60000 );if ( s != "%-02X") print("bad 111");
str::sprintf( s, 	"%s %-5d %05d", "test", 123, 123 );if ( s != "test 123   00123") print("bad 112");


s0 = "";
s1 = "1";
s2 = "22";
s3 = "1234";
if ( str::strlen("1") != 1 ) print("strlen 1");
if ( str::strlen("") != 0 ) print("strlen 2");
if ( str::strlen("123") != 3 ) print("strlen 3");
if ( str::strlen(s1) != 1 ) print("strlen 1a");
if ( str::strlen(s0) != 0 ) print("strlen 2a");
if ( str::strlen(s3) != 4 ) print("strlen 3a");

if ( str::format( "%s", "hi" ) != "hi" ) print("format 1");

if ( !str::isspace(' ') ) print("isspace 1");
if ( !str::isspace('\t') ) print("isspace 2");
if ( !str::isspace('\n') ) print("isspace 3");
c0 = ' ';
c1 = '\t';
c2 = '\n';
if ( !str::isspace(c0) ) print("isspace 1a");
if ( !str::isspace(c1) ) print("isspace 2a");
if ( !str::isspace(c2) ) print("isspace 3a");
if ( str::isspace('_') ) print("isspace 1b");
if ( str::isspace('\0') ) print("isspace 2b");
if ( str::isspace('A') ) print("isspace 3b");

if ( !str::isdigit('1') ) print("isdigit 1");
if ( !str::isdigit('0') ) print("isdigit 2");
if ( str::isdigit('x') ) print("isdigit 3");

if ( str::isalpha('1') ) print("isalpha 1");
if ( !str::isalpha('a') ) print("isalpha 2");
if ( str::isalpha('_') ) print("isalpha 3");

if ( str::mid("1", 1) != "" ) print("mid 1");
if ( str::mid("22", 1) != "2" ) print("mid 2");
if ( str::mid("22", 0) != "22" ) print("mid 3");
if ( str::mid("1234", 0, 1) != "1" ) print("mid 4");
if ( str::mid("1234", 1, 2) != "23" ) print("mid 5");
if ( str::mid(s1, 1) != "" ) print("mid 1a");
if ( str::mid(s2, 1) != "2" ) print("mid 2a");
if ( str::mid(s2, 0) != "22" ) print("mid 3a");
if ( str::mid(s3, 0, 1) != "1" ) print("mid 4a");
if ( str::mid(s3, 1, 2) != "23" ) print("mid 5a");

if ( str::tolower('A') != 'a' ) print("tolower 1");
if ( str::tolower('a') != 'a' ) print("tolower 2");

if ( str::toupper('a') != 'A' ) print("toupper 1");
if ( str::toupper('A') != 'A' ) print("toupper 2");

s = "#()*+,-/:;<=>?@\\^";
if ( str::strchr("#()*+,-/:;<=>?@\\^", '') != 17 ) print("strchr 2");
if ( str::strchr(s, '#') != 0 ) print("strchr 1");
if ( str::strchr("#()*+,-/:;<=>?@\\^", '\\') != 15 ) print("strchr 3");
if ( str::strchr("#()*+,-/:;<=>?@\\^", '^') != 16 ) print("strchr 4");
if ( str::strchr("#()*+,-/:;<=>?@\\^", 'A') != -1 ) print("strchr 5");

