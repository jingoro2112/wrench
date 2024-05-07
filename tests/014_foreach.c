/*~ ~*/

somethingH = { 1:10, 2:20 };
if ( somethingH._exists(1) != 1 ) println("x1");
somethingH._remove( 1 );
if ( somethingH._exists(1) != 0 ) println("x2");

something = { 1, 2, 3 };

on = 1;
for( value : something )
{
	if ( value != on++ )
	{
		println("f on 1");
	}
}
if ( on != 4 ) println( "f2");

something = { 2 };
on = 2;
for( value : something )
{
	if ( value != on++ )
	{
		println("f on 2");
	}
	break;
}
if ( on != 3 ) println( "f3");

something = { 1, 2, 3 };
somethingH = { 1:10, 2:20, "bill?":30, 500:"bob", 12:13 };
on = 0;
for( key,value2 : somethingH )
{
	on++;
	if ( somethingH[key] != value2 ) println("key err 1");
}
if ( on != 5 ) println( "wrong on" );

somethingH = { -1:10, 0:20, 1:30 };
on2 = 0;
for( key,value2 : somethingH )
{
	on2++;
}
if ( on2 != 3 ) println( "wrong on2" );


for( value2 : something )
{
	for( value2 : something )
	{
		for( value2 : something )
		{
			if ( value2 != 1 )
			{
				println("f4");
			}
			break;
		}
		if ( value2 != 1 )
		{
			println("f4");
		}
		break;
	}
	break;
}


