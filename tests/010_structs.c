/*~
123
bob
23
0
321
2
hello
0
hi2
<<>>
4
20
ok2
1.5
hello
1.5
1.5
long string
long string
2
0
8
9
10
30
45
60
8
321
2
23
321
2
122.3
713.398
~*/



instance2 = new a
{
	123,
	42,
	"bob",
	in.a,
	4 + 5,
};

log( instance2.p );

log( instance2.a );

instance3 = new a()
{
};

log( instance3.p );
log( instance3.a );


instance4 = new a
{
	321
};

log( instance4.p );
log( instance4.h );




instance0 = new a();
instance0 = new a();
instance0 = new a();
instance0 = new a();
instance0 = new a();


log( instance0.b );
log( instance0.v );

g = "hi2";
instance0.p = g;
log( instance0.p );
instance0.p = "1";
instance0.p = "2";
log( instance0.p = "<<>>" );
instance0.p = "4";
log( instance0.p );

instance0.p = 20;
log( instance0.p );
instance0.p = "ok2";
log( instance0.p );
instance0.p = 1.5;
log( instance0.p );
log( instance0.b );
log( instance0.b = instance0.p );
log( instance0.b );
instance0.p = "long string";
log( instance0.b = instance0.p );
log( instance0.b );

log( 0b010 );
log( 0b );


j[30] = { 8, 9 };
log( j[0] );
log( j[1] );


a = 10;
b = 20;
j[10] = { a, b + a, 45, 50, 77, 60 };
log( j[0] );
log( j[1] );
log( j[2] );
log( j[5] );


in = new a;

in.a = 8;
in.b = 7;
if ( in.a == in.b && 1 ) log("G1");
if ( in.a != in.a && 1 ) log("G2");
if ( in.a < in.b && 1 )  log("G3");
if ( in.b > in.a && 1 )  log("G4");
if ( in.b >= a && 1 ) log("G5");
if ( in.a <= in.b && 1 ) log("G6");

localIf();
function localIf()
{
	::in.a = 8;
	::in.b = 7;
	if ( ::in.a == ::in.b && 1 ) log("L1");
	if ( ::in.a != ::in.a && 1 ) log("L2");

}

if ( n != n )
{
	log( "null NOT okay" );
}

in.a = 2;
log( in.a + in.a + in.a + in.a );


struct Bob
{
};

function a()
{
	p = 23;
	h = 2;
	b = "hello";
	a;
	b;
}

instance2 = new a
{
	321,
	42,
	"bob",
	in.a,
	4 + 5,
};

log( instance2.p );
log( instance2.a );

instance3 = new a
{
};

log( instance3.p );


instance4 = new a
{
	321
};

log( instance4.p );
log( instance4.h );


function update_particle(p)
{
	p.x+=p.vx;
	p.y+=p.vy;
	p.z+=p.vz;
}

function update(particles)
{
	for( p=0; p<500; ++p )
	{
		update_particle(particles[p]);
	}
}

function update_several_times(particles, count)
{
	for ( i = 0; i < count; ++i)
	{
		update(particles);
	}
}

function update_several_timesI( particles, count )
{
	for ( i = 0; i < count; ++i )
	{
		for( p=0; p<500; ++p )
		{
			particles[p].x += particles[p].vx;
			particles[p].y += particles[p].vy;
			particles[p].z += particles[p].vz;
		}
	}
}

struct particle
{
	x;
	y;
	z;
	vx;
	vy;
	vz;
};

particles[500];
for ( i = 0; i < 500; ++i )
{
	particles[i] = new particle
	{
		x = i + 0.1,
		y = i + 0.2,
		z = i + 0.3,
		vx = 1.1,
		vy = 2.1,
		vz = 3.1
	};
}


update_several_timesI( particles, 50 );
update_several_times( particles, 50 );

log( particles[10].x );
log( particles[499].y );


