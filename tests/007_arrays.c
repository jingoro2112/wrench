/*~
good
good
~*/


aa[10];
a[500] = { aa };

if ( a._count != 501 ) print("bad size");
if ( a[0]._count != 11 ) print("bad size2");

for( i=0; i<500; ++i )
{
	allocArray();
}
for( i=0; i<500; ++i )
{
	allocArray();
}


b[40000];
b[1] = 10;
b[35000] = 20;
if ( b[1] != 10 || b[35000] != 20 )
{
	print("bad");
}


a[2] = 3;
a[400] = 4;
if ( a[2] != 3 )
{
	print("bad");
}
else
{
	print("good");
}
if ( a[400] != 4 )
{
	print("bad");
}
else
{
	print("good");
}

gi = 0;

function allocArray()
{
	ab[500];
	i = 2;
	ab[2] = 3;
	ab[401] = 41;
	if ( ab[2] != 3 )
	{
		print("bad");
	}
	if ( ab[401] != 41 )
	{
		print("bad");
	}

	ac[50];

	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			print( "oops" );
		}
	}

	for( i=0; i<50; ++i )
	{
		ac[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		::a[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( ac[i] != i+1 )
		{
			print( "oops_LL" );
		}
	}

	for( i=0; i<50; ++i )
	{
		if ( ::a[i] != i+1 )
		{
			print( "oops_GL" );
		}
	}
	
	for( ::gi=0; ::gi<50; ++::gi )
	{
		if ( ac[::gi] != ::gi+1 )
		{
			print( "oops_LG" );
		}
	}

	for( ::gi=0; ::gi<50; ++::gi )
	{
		if ( ::a[::gi] != ::gi+1 )
		{
			print( "oops_GG" );
		}
	}
}


grow[1] = 10;
grow[2] = 11;
grow[3] = 12;
grow[4] = 13;
grow[5] = 14;
grow[6] = 15;
if ( grow[6] != 15 ) print("badref0");
if ( grow[5] != 14 ) print("badref1");
if ( grow[4] != 13 ) print("badref2");
if ( grow[3] != 12 ) print("badref3");
if ( grow[2] != 11 ) print("badref4");


a[10] = { 1, 2, 3 };

leakCheck( a[0] );

function leakCheck( value )
{
	::a = 10;
	b[20]; // cause GC to run

	value = 20;
	if ( value != 20 ) print("B1");
	if ( 20 != value ) print("B2");
}
