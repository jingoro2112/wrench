/*~ ~*/

a = 10;
b = 20;
c = 30;
d = 40;
e = 50;
f = 600;

function testFunc( a1, b1, c1, d1, e1 )
{
	if ( b1 / e != 4 ) println("d0");
	if ( f / c1 != 2 ) println("d1");
			
	if ( a + a1 != 110 ) println( "1" );
	if ( b + a1 != 120 ) println( "2" );
	if ( c + a1 != 130 ) println( "3" );
	if ( d + a1 != 140 ) println( "4" );
	if ( e + a1 != 150 ) println( "5" );
	
	if ( a1 + a != 110 ) println( "6" );
	if ( a1 + b != 120 ) println( "7" );
	if ( a1 + c != 130 ) println( "8" );
	if ( a1 + d != 140 ) println( "9" );
	if ( a1 + e != 150 ) println( "10" );

	if ( a + c1 != 310 ) println( "11" );
	if ( b + c1 != 320 ) println( "12" );
	if ( c + c1 != 330 ) println( "13" );
	if ( d + c1 != 340 ) println( "14" );
	if ( e + c1 != 350 ) println( "15" );

	if ( c1 + a != 310 ) println( "16" );
	if ( c1 + b != 320 ) println( "17" );
	if ( c1 + c != 330 ) println( "18" );
	if ( c1 + d != 340 ) println( "19" );
	if ( c1 + e != 350 ) println( "20" );


	if ( a - a1 != -90 ) println( "-1" );
	if ( b - a1 != -80 ) println( "-2" );
	if ( c - a1 != -70 ) println( "-3" );
	if ( d - a1 != -60 ) println( "-4" );
	if ( e - a1 != -50 ) println( "-5" );

	if ( a1 - a != 90 ) println( "-6" );
	if ( a1 - b != 80 ) println( "-7" );
	if ( a1 - c != 70 ) println( "-8" );
	if ( a1 - d != 60 ) println( "-9" );
	if ( a1 - e != 50 ) println( "-10" );

	if ( a - c1 != -290 ) println( "-11" );
	if ( b - c1 != -280 ) println( "-12" );
	if ( c - c1 != -270 ) println( "-13" );
	if ( d - c1 != -260 ) println( "-14" );
	if ( e - c1 != -250 ) println( "-15" );

	if ( c1 - a != 290 ) println( "-16" );
	if ( c1 - b != 280 ) println( "-17" );
	if ( c1 - c != 270 ) println( "-18" );
	if ( c1 - d != 260 ) println( "-19" );
	if ( c1 - e != 250 ) println( "-20" );

}

testFunc( 100, 200, 300, 400, 500 );
