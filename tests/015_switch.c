/*~ ~*/


switch( f )
{
	default: break;
}


switch( f )
{
	case 1000:
}

f = 1000;

switch( f )
{
	case 1000: break;
}


switch( f )
{
	case 1000:
}


f = 2;
while( f )
{
	switch( f )
	{
	}
	f--;
}


for( k = 0; k<3; ++k )
{
	switch( k )
	{
	}
}



f = 1000;

switch( f )
{
	case 1000: break;
}

switch( f )
{
	case 1000:
}


switch( f )
{
	case 0: break;
}


f = 5;
switch( f )
{
}

switch( f )
{
	default:
}

switch( f )
{
	default: break;
}


switch( f )
{
	case 0:
}


switch( f )
{
	case 5:
}


switch( f )
{
	case 5: break;
}



switch( f )
{
	default: f = 2; break;
}
if ( f != 2 ) print("d0");


switch( f )
{
	case 1:
	default: f = 21; break;
}
if ( f != 21 ) print("d1");


switch( f )
{
	default: f = 201; break;
	case 1:
}
if ( f != 201 ) print("d01");


f = 21;
switch( f )
{
	case 21:
	default: f = 22; break;
}
if ( f != 22 ) print("d2");

switch( f )
{
	case 22: f = 23;
}
if ( f != 23 ) print("d3");



switch( f )
{
	case 23: f = 24;
	case 22:
}
if ( f != 24 ) print("d4");

switch( f )
{
	case 22:
	case 24: f = 25; break;
}
if ( f != 25 ) print("d5");



switch( f )
{
	default: f = 2000; break;
}
if ( f != 2000 ) print("b0");

switch( f )
{
	case 1:
	default: f = 21000; break;
}
if ( f != 21000 ) print("b1");

switch( f )
{
	default: f = 201000; break;
	case 1:
}
if ( f != 201000 ) print("b01");


f = 21000;
switch( f )
{
	case 21000:
	default: f = 22000; break;
}
if ( f != 22000 ) print("d2000");

switch( f )
{
	case 22000: f = 23000;
}
if ( f != 23000 ) print("d3000");

switch( f )
{
	case 23000: f = 24000;
	case 22000:
}
if ( f != 24000 ) print("d4000");


switch( f )
{
	case 22000:
	case 24000: f = 25000; break;
}
if ( f != 25000 ) print("d5000");


function InAFunc()
{

	f1 = 1000;


	switch( f1 )
	{
		case 1000:
	}


	
	switch( f1 )
	{
		case 1000: break;
	}

	f1 = 0;
	switch( f1 )
	{
		default: break;
	}


	switch( f1 )
	{
		case 0: break;
	}

	f1 = 5;
	switch( f1 )
	{
	}

	switch( f1 )
	{
		default:
	}

	switch( f1 )
	{
		default: break;
	}

	switch( f1 )
	{
		case 0:
	}

	switch( f1 )
	{
		case 5:
	}

	switch( f1 )
	{
		case 5: break;
	}


	switch( f1 )
	{
		default: f1 = 2; break;
	}
	if ( f1 != 2 ) print("d0");

	switch( f1 )
	{
		case 1:
		default: f1 = 21; break;
	}
	if ( f1 != 21 ) print("d1");

	switch( f1 )
	{
		default: f1 = 201; break;
		case 1:
	}
	if ( f1 != 201 ) print("d01");

	f1 = 21;
	switch( f1 )
	{
		case 21:
		default: f1 = 22; break;
	}
	if ( f1 != 22 ) print("d2");

	switch( f1 )
	{
		case 22: f1 = 23;
	}
	if ( f1 != 23 ) print("d3");

	switch( f1 )
	{
		case 23: f1 = 24;
		case 22:
	}
	if ( f1 != 24 ) print("d4");

	switch( f1 )
	{
		case 22:
		case 24: f1 = 25; break;
	}
	if ( f1 != 25 ) print("d5");



	switch( f1 )
	{
		default: f1 = 2000; break;
	}
	if ( f1 != 2000 ) print("b0");

	switch( f1 )
	{
		case 1:
		default: f1 = 21000; break;
	}
	if ( f1 != 21000 ) print("b1");

	switch( f1 )
	{
		default: f1 = 201000; break;
		case 1:
	}
	if ( f1 != 201000 ) print("b01");


	f1 = 21000;
	switch( f1 )
	{
		case 21000:
		default: f1 = 22000; break;
	}
	if ( f1 != 22000 ) print("d2000");

	switch( f1 )
	{
		case 22000: f1 = 23000;
	}
	if ( f1 != 23000 ) print("d3000");

	switch( f1 )
	{
		case 23000: f1 = 24000;
		case 22000:
	}
	if ( f1 != 24000 ) print("d4000");

	switch( f1 )
	{
		case 22000:
		case 24000: f1 = 25000; break;
	}
	if ( f1 != 25000 ) print("d5000");

	v = 1;
	switch( v )
	{
		case 1:
			g = 2;
			switch( g )
			{
				case 2:
					d = 20;
					break;
			}
			break;
	}
	if ( d != 20 ) print("d20");
}


InAFunc();

 
enum
{
	n1,
	n2,
	n3
}
t = 0;

e = n1;
switch( e )
{
	case n1: t += 10; 
	case n2: t += 20; break;
}


e = n2;
switch( e )
{
	default:
	case n3: t += 15; break;
	case n1: t += 25; break;
}
if ( t != 45 ) print("e");




t = 0;
// some degenerate cases..
for( k = 0; k<3; ++k )
{
	switch( k )
	{
		default: t++; break;
		case 1: t += 10; break;
	}
}

// some degenerate cases..
for( k = 1000; k<1003; ++k )
{
	switch( k )
	{
		default: t++; break;
		case 1001: t += 10; break;
	}
}

if ( t != 24 ) print( "t1" );

for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1:(t += 2); break;
		default: ++t; ++t; ++t; break;
	}
}

for( k = 1000; k<1003; ++k )
{
	switch( k )
	{
		case 1001:(t += 2); break;
		default: ++t; ++t; ++t; break;
	}
}

if ( t != 40 ) print( "t2" );


t = 0;
for( k = 0; k<3; ++k )
{
	switch( k )
	{
		default: t += 1;
	}
}
if ( t != 3 ) print ("t3a");


t = 0;
for( k = 1000; k<1003; ++k )
{
	switch( k )
	{
		default: t += 1;
	}
}
if ( t != 3 ) print ("t3b");


for( k = 0; k<3; ++k )
{
	switch( k )
	{
	}
}


t = 0;
for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1: t++;
	}
}
if ( t != 1 ) print("t4");

t = 0;
for( k = 1000; k<1003; ++k )
{
	switch( k )
	{
		case 1001: t++;
	}
}
if ( t != 1 ) print("t4b");

t = 0;
for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1: t++; break;
	}
}
if ( t != 1 ) print("t5");

t = 0;
for( k = 1000; k<1003; ++k )
{
	switch( k )
	{
		case 1001: t++; break;
	}
}
if ( t != 1 ) print("t5b");


t = 0;
switch( 1 )
{
	case 1:
	{
		t += 1;
	}
	t += 2;
	{
		t += 3;
	}
	break;
}
if ( t != 6 ) print("t6");

t = 0;
switch( 1001 )
{
	case 1001:
	{
		t += 1;
	}
	t += 2;
	{
		t += 3;
	}
	break;
}
if ( t != 6 ) print("t6b");



switch( null )
{
	case 1: t = 10; break;
	case null: t = 20; break;
}
if ( t != 20 ) print("t6");

switch( 1 )
{
	case 1: t = 21; break;
	case null: t = 31; break;
}
if ( t != 21 ) print("t7");

switch( 0 )
{
	case 1: t = 22; break;
	case null: t = 32; break;
}
if ( t != 32 ) print("t8");

t = 0;
for( i=0; i<6; i++ )
{
	switch( i )
	{
		case 0: t += 0x1; break;
		case 1: t += 0x10; break;
		case 2: t += 0x100; break;
		case 3: t += 0x1000; break;
		case 4: t += 0x10000; break;
		default:t += 0x100000; break;
	}

	switch( i )
	{
		case 0: t += 0x1; break;
		case 1: t += 0x10; break;
		case 2: t += 0x100; break;
		default:t += 0x100000; break;
		case 3: t += 0x1000; break;
		case 4: t += 0x10000; break;
	}

	switch( i )
	{
		default:t += 0x100000; break;
		case 1: t += 0x10; break;
		case 0: t += 0x1; break;
		case 2: t += 0x100; break;
		case 4: t += 0x10000; break;
		case 3: t += 0x1000; break;
	}

	switch( i )
	{
		case 0:
		{
			switch( i )
			{
				case 0: t += 1;
				{
					switch( i )
					{
						case 0:
							switch( i )
							{
								case 0:
								{
									t += 0x1;
									break;
								}
							}
					}
				}
				
			}
			break;
		}
		break;
		
		case 1: t += 0x10; break;
		case 2: t += 0x100; break;
		default:t += 0x100000; break;
		case 3: t += 0x1000; break;
		case 4: t += 0x10000; break;
	}

}
if ( t != 0x444445 ) { str::sprintf( st, "%X", t); print(st); print( "t9" ); }






t = 0;
for( i=1000; i<1006; i++ )
{
	switch( i )
	{
		case 1000: t += 0x1; break;
		case 1001: t += 0x10; break;
		case 1002: t += 0x100; break;
		case 1003: t += 0x1000; break;
		case 1004: t += 0x10000; break;
		default:t += 0x100000; break;
	}

	switch( i )
	{
		case 1000: t += 0x1; break;
		case 1001: t += 0x10; break;
		case 1002: t += 0x100; break;
		default:t += 0x100000; break;
		case 1003: t += 0x1000; break;
		case 1004: t += 0x10000; break;
	}

	switch( i )
	{
		default:t += 0x100000; break;
		case 1001: t += 0x10; break;
		case 1000: t += 0x1; break;
		case 1002: t += 0x100; break;
		case 1004: t += 0x10000; break;
		case 1003: t += 0x1000; break;
	}

	switch( i )
	{
		case 1000:
		{
			switch( i )
			{
				case 1000: t += 1;
						{
							switch( i )
							{
								case 1000:
									switch( i )
									{
										case 1000:
										{
											t += 0x1;
											break;
										}
									}
							}
						}

			}
			break;
		}

		case 1001: t += 0x10; break;
		case 1002: t += 0x100; break;
		default:t += 0x100000; break;
		case 1003: t += 0x1000; break;
		case 1004: t += 0x10000; break;
	}

}
if ( t != 0x444445 ) print( "t9b" );

t = 0;
for( k = 4000000000 ; k < 4000000009; ++k )
{
	switch( k )
	{
		case 4000000000: t += 0x1; break;
		case 4000000001: t += 0x10; break;
		case 4000000002: t += 0x100; break;
		case 4000000003: t += 0x1000; break;
		case 4000000004: t += 0x10000; break;
		case 4000000006: t += 0x100000; break;
		case 4000000008: t += 0x1000000; break;
		default:         t += 0x10000000; break;
	}
}
if ( t != 0x21111111 ) print ("t10");

