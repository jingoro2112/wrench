/*~
777
33
222
1
1
2
3
4
5
6
7
8
9
A
B
C
100
100
95
100
100
200
205
205
85
80
80
20
4
15
60
60
120
24
1
1
1
1
6
6
5
5
4
17
1
254
~*/

//------------------------------------------------------------------------------
function userCheck( data )
{
	print( data.subUser.data2 );
	data.subUser.data2 = 28;
	print( 1 + data.subUser.data2++ + 10 - 2 * 3 ); // 33

	print( data.subUser.data3[1] );

	print( data.value );
	if ( data.value == 1 ) print("1");
	if ( data.value != 1 ) print("1 FAIL");
	if ( data.value != 2 ) print("2");
	if ( data.value > 0 ) print("3");
	if ( data.value > 1 ) print("3 FAIL");
	if ( data.value < 2 ) print("4");
	if ( data.value < 1 ) print("4 FAIL");
	if ( data.value >= 1 ) print("5");
	if ( data.value >= 2 ) print("5 FAIL");
	if ( data.value <= 1 ) print("6");
	if ( data.value <= 0 ) print("6 FAIL");

	if ( 1 ==data.value ) print("7");
	if ( 1 != data.value ) print("7 FAIL");
	if ( 2 != data.value ) print("8");
	if ( 0 < data.value ) print("9");
	if ( 1 < data.value ) print("9 FAIL");
	if ( 2 > data.value ) print("A");
	if ( 1 > data.value ) print("A FAIL");
	if ( 1 <= data.value ) print("B");
	if ( 2 <= data.value ) print("B FAIL");
	if ( 1 >= data.value ) print("C");
	if ( 0 >= data.value ) print("C FAIL");

	
	data.ac[9] = 100;
	data.ac[8] = 90;

	print( data.ac[8] = data.ac[9] ); // 100
	print( data.ac[8] ); // 100
	data.ac[8] = 90;

	a = 5;
	print( data.ac[8] += 5 ); // 95
	print( data.ac[8] += a ); // 100
	print( data.ac[8] ); // 100
	print( data.ac[9] += data.ac[8] ); // 200
	print( a += data.ac[9]); // 205
	print( a ); // 205

	data.ac[9] = 100;
	data.ac[8] = 90;
	a = 5;
	print( data.ac[8] -= 5 ); // 85
	print( data.ac[8] -= a ); // 80
	print( data.ac[8] ); // 80
	print( data.ac[9] -= data.ac[8] ); // 20
	data.ac[9] = 1;
	print( a -= data.ac[9]); // 4

	data.ac[9] = 2;
	data.ac[8] = 3;
	a = 4;
	print( data.ac[8] *= 5 ); // 15
	print( data.ac[8] *= a ); // 60
	print( data.ac[8] ); // 60
	print( data.ac[9] *= data.ac[8] ); // 120
	data.ac[9] = 6;
	print( a *= data.ac[9]); // 24

	data.ac[9] = 10;
	print( data.ac[9] %= 3 ); // 1

	data.ac[9] = 10;
	data.ac[8] = 3;
	print( data.ac[9] %= data.ac[8] ); // 1

	data.ac[9] = 10;
	a = 3;
	print( data.ac[9] %= a ); // 1

	data.ac[9] = 3;
	a = 10;
	print( a %= data.ac[9] ); // 1

	data.ac[4] = 1;
	data.ac[5] = 3;
	a = 5;
	print( data.ac[4] + a ); // 6
	print( a + data.ac[4] ); // 6
	print( data.ac[4] + 4 ); // 5
	print( 4 + data.ac[4] ); // 5
	print( data.ac[4] + data.ac[5] ); // 4


	a = 3;
	data.ac[4] = 10;
	data.ac[2] = 3;
	if ( data.ac[4] < a ) print( "bad1" );
	if ( a > data.ac[4] ) print( "bad2" );
	if ( data.ac[4] < 3 ) print( "bad3" );
	if ( 3 > data.ac[4] ) print( "bad4" );
	if ( data.ac[4] < data.ac[2] ) print( "bad5" );
	if ( data.ac[2] > data.ac[4] ) print( "bad6" );

	a = 10;
	data.ac[4] = 10;
	data.ac[2] = 10;
	if ( data.ac[4] != a ) print( "bad7" );
	if ( a != data.ac[4] ) print( "bad8" );
	if ( data.ac[4] != 10 ) print( "bad9" );
	if ( 10 != data.ac[4] ) print( "bad10" );
	if ( data.ac[4] != data.ac[2] ) print( "bad11" );
	if ( data.ac[2] != data.ac[4] ) print( "bad12" );

	a = 11;
	data.ac[4] = 12;
	data.ac[2] = 10;
	if ( data.ac[4] == a ) print( "bad13" );
	if ( a == data.ac[4] ) print( "bad14" );
	if ( data.ac[4] == 10 ) print( "bad15" );
	if ( 10 == data.ac[4] ) print( "bad16" );
	if ( data.ac[4] == data.ac[2] ) print( "bad17" );
	if ( data.ac[2] == data.ac[4] ) print( "bad18" );

	data.ac[4] = 0x10;
	print( data.ac[4] |= 0x01 );
	a = 0x01;
	print( data.ac[4] &= a );
	data.ac[5] = 0xFF;
	print( data.ac[4] ^= data.ac[5] );
}
