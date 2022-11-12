/*~ ~*/

somethingH = { 1:10, 2:20 };
if ( somethingH._exists(1) != 1 ) print("x1");
somethingH._remove( 1 );
if ( somethingH._exists(1) != 0 ) print("x2");

something = { 1, 2, 3 };

on = 1;
for( value : something )
{
	if ( value != on++ )
	{
		print("f on 1");
	}
}
if ( on != 4 ) print( "f2");

something = { 2 };
on = 2;
for( value : something )
{
	if ( value != on++ )
	{
		print("f on 2");
	}
	break;
}
if ( on != 3 ) print( "f3");

something = { 1, 2, 3 };
somethingH = { 1:10, 2:20, "bill?":30, 500:"bob", 12:13 };

on = 0;
for( key,value2 : somethingH )
{
	on++;
	if ( somethingH[key] != value2 ) print("key err 1");
}
if ( on != 5 ) print( "wrong on" );

for( value2 : something )
{
	for( value2 : something )
	{
		for( value2 : something )
		{
			if ( value2 != 1 )
			{
				print("f4");
			}
			break;
		}
		if ( value2 != 1 )
		{
			print("f4");
		}
		break;
	}
	break;
}


