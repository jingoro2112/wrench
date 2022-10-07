/*~
~*/

// callisto has switch and so will wrench.. but not today

// some degenerate cases..
for( k = 0; k<3; ++k )
{
	switch( k )
	{
		default: print( "c" ); break;
		case 1: print( "1c" ); break;
	}
}

for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1: print( "1b" ); break;
		default: print( "d" ); break;
	}
}

for( k = 0; k<3; ++k )
{
	switch( k )
	{
		default: print( "d" );
	}
}

for( k = 0; k<3; ++k )
{
	switch( k )
	{
	}
}

for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1: print( "1a" );
	}
}

for( k = 0; k<3; ++k )
{
	switch( k )
	{
		case 1: print( "1" ); break;
	}
}

switch( null )
{
	case 0: print("zero"); break;
	case null: print("null"); break;
}

switch( 0 )
{
	case 0: print("zero"); break;
	case null: print("null"); break;
}

for( i=0; i<6; i++ )
{
	switch( i )
	{
		case 0: print("zero"); break;
		case 1: print("one"); break;
		case 2: print("two"); break;
		case 3: print("three"); break;
		case 4: print("four"); break;
		default: print("default"); break;
	}

	switch( i )
	{
		case 0: { print("zero"); break; }
		case 1: print("one"); break;
		case 2: { print("two"); break; }
		default: print("default"); break;
		case 3: print("three"); break;
		case 4: { print("four"); break; }
	}

	switch( i )
	{
		case 0:
		{
			switch( i )
			{
				case 0:
				{
					switch( i )
					{
						case 0:
							switch( i )
							{
								case 0:
								{
									print( "zero!" );
									break;
								}
							}
					}
				}
			}
		}
		
		case 1: print("one"); break;
		case 2: { print("two"); break; }
		default: print("default"); break;
		case 3: print("three"); break;
		case 4: { print("four"); break; }
	}

}


for( k = 5000000000 ; k < 5000000009; ++k )
{
	switch( k )
	{
		case 5000000000: print( "0" ); break;
		case 5000000001: print( "1" ); break;
		case 5000000002: print( "2" ); break;
		case 5000000003: print( "3" ); break;
		case 5000000004: print( "4" ); break;
		case 5000000006: print( "6" ); break;
		case 5000000008: print( "8" ); break;
		case 5000000009: print( "9" ); break;
		default: print( "N" ); break;
	}
}
