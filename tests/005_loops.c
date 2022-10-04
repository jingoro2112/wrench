/*~
0
0
1
-1
out 1
11
out 2
12
out 2
13
loop
10
loop
13
loop
16
loop
19
1
c17
b17
c14
c14
b14
c10
c10
c10
c10
out
5
6
7
8
9
10
11
12
10
5
yup
yup
yup
yup
13
0
1
2
3
4
5
6
7
8
9
10
1
lt6!
2
lt6!
3
lt6!
4
lt6!
5
lt6!
6
lt6!
7
10
110
120
131
1
2
3
2
3
5
3
4
7
4
5
9
5
6
11
6
7
13
7
8
15
8
9
17
9
10
19
0
1
2
3
4
5
6
7
8
9
10
10
1
1
~*/

tle();
function tle()
{
}

prime(0);
prime(10);
prime(30);
prime(3000);

function prime2()
{
	return true;
}

function prime(n)
{
	count = 0;
	for (i = 2; i <= n; ++i)
		if (prime2())
			++count;
	return count;
}

for( i=0; i<10; ++i)
{
	
}

for( i=0,j=0; i<2; ++i,j-- )
{
	log(i);
	log(j);
}

a = 10;
while( ++a < 14)
{
	if ( a < 12 )
	{
		log( "out 1" );
		log( a );
		continue;
	}

	log( "out 2" );
	log( a );
}

a = 10;
while( a < 20)
{
	log( "loop" );
	log( a );
	++a;
	a += 1;
	a = a + 1;
}

log(f1());

unit f1()
{
	while( 1 )
		while( 2 )
		{
			while( 3 )
				return 1;
		}
}


a = 20;
while( --a > 5 )
{
	while( --a > 10 )
	{
		while ( --a > 15 )
		{
			if ( a < 17 )
			{
				log( "b17" );
				break;
			}
			else
			{
				log( "c17" );
				continue;
			}

			log( "better not" );
		}

		if ( a < 14 )
		{
			log( "b14" );
			break;
		}

		log( "c14" );
	}

	log( "c10" );
}

log( "out" );

i = 5;
do
{
	log( i );
	++i;
} while ( i < 13 );

do
	--i;
while( i >10 );
log(i);

i = 5;
do
{
	break;
	++i;
} while ( i < 13 );
log(i);

i = 5;
do
{
	++i;
	do
	{
		i++;
		break;
		log( "nope" );
	} while( i < 20 );
	
	log("yup");
	continue;
	log("nope2");
} while ( i < 13 );
log(i);

for( i=0; i<10; ++i )
	log(i);
log(i);


for( tristan = 1; tristan <= 7; tristan++ )
{
	log( tristan );
	if ( tristan <= 6 )
	{
		log( "lt6!");
	}
}


i = 0;
for(;;)
{
	if ( ++i == 10 )
	{
		break;
	}
}
log(i);

for( i=100;;)
{
	if ( ++i == 110 )
	{
		break;
	}
}
log(i);

for(;i<120;)
{
	++i;
}
log(i);

for(;;++i)
{
	if ( i > 130 )
	{
		break;
	}
}
log(i);

for( i=1, j=2, k=3; i<10; ++i, ++j, k = j + i )
{
	log(i);
	log(j);
	log(k);
}

for( i=0; i<10; ++i )
	log(i);
log(i);
	
log(i);

log(f2());
log(f3());

unit f2()
{
	for(;;)
		for(;;)
		{
			for(;;)
				return 1;
		}
}

unit f3()
{
	do {
		do 
			do {
				return 1;
			} while(1);
		while(1);
	} while(1);
}
