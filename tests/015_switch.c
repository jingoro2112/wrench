/*~ ~*/

/*
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
		}
		
		case 1: t += 0x10; break;
		case 2: t += 0x100; break;
		default:t += 0x100000; break;
		case 3: t += 0x1000; break;
		case 4: t += 0x10000; break;
	}

}
if ( t != 0x333333 ) print( "t9" );









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
		}

		case 1001: t += 0x10; break;
		case 1002: t += 0x100; break;
		default:t += 0x100000; break;
		case 1003: t += 0x1000; break;
		case 1004: t += 0x10000; break;
	}

}
if ( t != 0x333333 ) print( "t9b" );
*/

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
