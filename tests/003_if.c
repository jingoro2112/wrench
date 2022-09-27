/*~
a
b
c
d
e
f
yes
no
no
yes
yes2
10
made it
here too
yay
yay2
yay3
~*/


if ( 9 < 10 ) log("a");
if ( 11 < 10 ) log("a fail");
if ( 9 > 10 ) log("b fail");
if ( 11 > 10 ) log("b");
if ( 10 <= 10 ) log("c");
if ( 11 <= 10 ) log("c fail");
if ( 10 >= 10 ) log("d");
if ( 9 >= 10 ) log("d fail");
if ( 10 == 10 ) log("e");
if ( 9 == 10 ) log("e fail");
if ( 9 != 10 ) log("f");
if ( 10 != 10 ) log("f faile");


a = 10;


if ( a == 10 )
	log( "yes" );
else
	log( "no" );


if ( a != 10 )
	log( "yes" );
else
{
	log( "no" );
}

if ( a != 10 )
{
	log( "yes" );
}
else
	log( "no" );

if ( a == 10 )
	log( "yes" );

log("yes2");

if ( n != n )
{
	log( "null NOT okay" );
}

c = 4;
d = 6;

log(8 + 3 + c++ - --d) ;

			
c = 4;
d = 6;
a = 10;
b = 20;

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 10) )
	if ( b = 5 )
		if ( c = 5 )
		{
			if ( b + c == 10 )
				log( "made it" );

			log( "here too" );
		}
	else
		log( "no" );

if ( a == 10 && b == 20 && ((8 + 3 + c++ - --d) == 1) )
{
	if ( b = 5 )
		
		if ( c = 5 )
		{
			if ( b + c == 10 )
				log( "made it" );
			log( "here too" );
		}
		else
			log( "no" );
}
else
{
	log( "yay" );
}

a = 10;
b = 2.3;
if ( !a || !b )
{
	log( "boo" );
}

a = 0;
if ( !a || !b )
{
	log( "yay2" );
}

if ( !a && !b )
{
	log( "boo" );
}

b = 0;
if ( !a && !b )
{
	log( "yay3" );
}
