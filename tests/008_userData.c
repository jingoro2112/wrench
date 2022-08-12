/*~
95
100
100
200
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
	data.ac[9] = 100;
	data.ac[8] = 90;
	a = 5;
	log( data.ac[8] += 5 ); // 95
	log( data.ac[8] += a ); // 100
	log( data.ac[8] ); // 100
	log( data.ac[9] += data.ac[8] ); // 200
	log( a += data.ac[9]); // 205

	data.ac[9] = 100;
	data.ac[8] = 90;
	a = 5;
	log( data.ac[8] -= 5 ); // 85
	log( data.ac[8] -= a ); // 80
	log( data.ac[8] ); // 80
	log( data.ac[9] -= data.ac[8] ); // 20
	data.ac[9] = 1;
	log( a -= data.ac[9]); // 4

	data.ac[9] = 2;
	data.ac[8] = 3;
	a = 4;
	log( data.ac[8] *= 5 ); // 15
	log( data.ac[8] *= a ); // 60
	log( data.ac[8] ); // 60
	log( data.ac[9] *= data.ac[8] ); // 120
	data.ac[9] = 6;
	log( a *= data.ac[9]); // 24

	data.ac[9] = 10;
	log( data.ac[9] %= 3 ); // 1

	data.ac[9] = 10;
	data.ac[8] = 3;
	log( data.ac[9] %= data.ac[8] ); // 1

	data.ac[9] = 10;
	a = 3;
	log( data.ac[9] %= a ); // 1

	data.ac[9] = 3;
	a = 10;
	log( a %= data.ac[9] ); // 1

	data.ac[4] = 1;
	data.ac[5] = 3;
	a = 5;
	log( data.ac[4] + a ); // 6
	log( a + data.ac[4] ); // 6
	log( data.ac[4] + 4 ); // 5
	log( 4 + data.ac[4] ); // 5
	log( data.ac[4] + data.ac[5] ); // 4


	a = 3;
	data.ac[4] = 10;
	data.ac[2] = 3;
	if ( data.ac[4] < a ) log( "bad" );
	if ( a > data.ac[4] ) log( "bad" );
	if ( data.ac[4] < 3 ) log( "bad" );
	if ( 3 > data.ac[4] ) log( "bad" );
	if ( data.ac[4] < data.ac[2] ) log( "bad" );
	if ( data.ac[2] > data.ac[4] ) log( "bad" );

	a = 10;
	data.ac[4] = 10;
	data.ac[2] = 10;
	if ( data.ac[4] != a ) log( "bad" );
	if ( a != data.ac[4] ) log( "bad" );
	if ( data.ac[4] != 10 ) log( "bad" );
	if ( 10 != data.ac[4] ) log( "bad" );
	if ( data.ac[4] != data.ac[2] ) log( "bad" );
	if ( data.ac[2] != data.ac[4] ) log( "bad" );


	a = 11;
	data.ac[4] = 12;
	data.ac[2] = 10;
	if ( data.ac[4] == a ) log( "bad" );
	if ( a == data.ac[4] ) log( "bad" );
	if ( data.ac[4] == 10 ) log( "bad" );
	if ( 10 == data.ac[4] ) log( "bad" );
	if ( data.ac[4] == data.ac[2] ) log( "bad" );
	if ( data.ac[2] == data.ac[4] ) log( "bad" );

	data.ac[4] = 0x10;
	log( data.ac[4] |= 0x01 );
	a = 0x01;
	log( data.ac[4] &= a );
	data.ac[5] = 0xFF;
	log( data.ac[4] ^= data.ac[5] );
}
