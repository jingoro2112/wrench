/*~ ~*/

a = 10;
b = 20;
c = 30;
d = 40;
e = 50;
f = 600;

function testFunc( a1, b1, c1, d1, e1 )
{
	if ( b1 / e != 4 ) print("d0");
	if ( f / c1 != 2 ) print("d1");
			
	if ( a + a1 != 110 ) print( "1" );
	if ( b + a1 != 120 ) print( "2" );
	if ( c + a1 != 130 ) print( "3" );
	if ( d + a1 != 140 ) print( "4" );
	if ( e + a1 != 150 ) print( "5" );
	
	if ( a1 + a != 110 ) print( "6" );
	if ( a1 + b != 120 ) print( "7" );
	if ( a1 + c != 130 ) print( "8" );
	if ( a1 + d != 140 ) print( "9" );
	if ( a1 + e != 150 ) print( "10" );

	if ( a + c1 != 310 ) print( "11" );
	if ( b + c1 != 320 ) print( "12" );
	if ( c + c1 != 330 ) print( "13" );
	if ( d + c1 != 340 ) print( "14" );
	if ( e + c1 != 350 ) print( "15" );

	if ( c1 + a != 310 ) print( "16" );
	if ( c1 + b != 320 ) print( "17" );
	if ( c1 + c != 330 ) print( "18" );
	if ( c1 + d != 340 ) print( "19" );
	if ( c1 + e != 350 ) print( "20" );


	if ( a - a1 != -90 ) print( "-1" );
	if ( b - a1 != -80 ) print( "-2" );
	if ( c - a1 != -70 ) print( "-3" );
	if ( d - a1 != -60 ) print( "-4" );
	if ( e - a1 != -50 ) print( "-5" );

	if ( a1 - a != 90 ) print( "-6" );
	if ( a1 - b != 80 ) print( "-7" );
	if ( a1 - c != 70 ) print( "-8" );
	if ( a1 - d != 60 ) print( "-9" );
	if ( a1 - e != 50 ) print( "-10" );

	if ( a - c1 != -290 ) print( "-11" );
	if ( b - c1 != -280 ) print( "-12" );
	if ( c - c1 != -270 ) print( "-13" );
	if ( d - c1 != -260 ) print( "-14" );
	if ( e - c1 != -250 ) print( "-15" );

	if ( c1 - a != 290 ) print( "-16" );
	if ( c1 - b != 280 ) print( "-17" );
	if ( c1 - c != 270 ) print( "-18" );
	if ( c1 - d != 260 ) print( "-19" );
	if ( c1 - e != 250 ) print( "-20" );

}

testFunc( 100, 200, 300, 400, 500 );
