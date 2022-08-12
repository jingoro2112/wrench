/*~
good
100
100
~*/

a[50];
a[2] = 3;
if ( a[2] != 3 )
{
	log("bad");
}
else
{
	log("good");
}

function allocArray()
{
	a[50];
	i = 2;
	a[2] = 3;
	if ( a[2] != 3 )
	{
		log("bad");
	}

	a[50];
	for( i=0; i<50; ++i )
	{
		a[i] = i+1;
	}

	for( i=0; i<50; ++i )
	{
		if ( a[i] != i+1 )
		{
			log( "oops" );
		}
	}

	
}

for( i=0; i<100; ++i )
{
	allocArray();
}
log(100);
for( i=0; i<100; ++i )
{
	allocArray();
}
log(100);
