/*~
good
good
~*/

a[500];
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
	log("bad");
}


a[2] = 3;
a[400] = 4;
if ( a[2] != 3 )
{
	log("bad");
}
else
{
	log("good");
}
if ( a[400] != 4 )
{
	log("bad");
}
else
{
	log("good");
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
		log("bad");
	}
	if ( ab[401] != 41 )
	{
		log("bad");
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
			log( "oops" );
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
			log( "oops_LL" );
		}
	}

	for( i=0; i<50; ++i )
	{
		if ( ::a[i] != i+1 )
		{
			log( "oops_GL" );
		}
	}
	
	for( ::gi=0; ::gi<50; ++::gi )
	{
		if ( ac[::gi] != ::gi+1 )
		{
			log( "oops_LG" );
		}
	}

	for( ::gi=0; ::gi<50; ++::gi )
	{
		if ( ::a[::gi] != ::gi+1 )
		{
			log( "oops_GG" );
		}
	}
}
