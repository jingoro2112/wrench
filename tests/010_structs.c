/*~ ~*/

// these should both be legal, null list length but legal

struct args( a, b )
{
	var c = b;
	var d = a;
}

struct Frame
{
	var Channel;
	var Id;
	var Length;
}
var frame = new Frame ();

frame.Channel = 10;
frame.Id = 20;
frame.Length = 30;

checkStruct( frame );

z = 1;

struct EMPTY
{
};
var e = new EMPTY;
e = new EMPTY[]{};
e = new EMPTY[]{{}};


struct KWT
{
	s;
	tok;
};

kwds2 = new KWT[]{};
kwds3 = new KWT[]{{}};

abs = "and";

really = { 10, 20 };

kwds = new KWT[]
{
	{"abs", 0},
	{abs, z},
	{"asc", 2,},
	{"bye", really},
};


if ( kwds[0].s != "abs" || kwds[0].tok != 0 ) println("kwd 0");

if ( kwds[1].s != "and" || kwds[1].tok != 1 ) println("kwd 1");
if ( kwds[2].s != "asc" || kwds[2].tok != 2 ) println("kwd 2");
if ( kwds[3].s != "bye" || kwds[3].tok[1] != 20 ) println("kwd 3");




instance2 = new a
{
	123,
	42,
	0,
	"bob",
	4 + 5,
};

if ( instance2.a != "bob" ) println("err 2");
if ( instance2.p != 123 ) println("err 1");

instance3 = new a()
{
};

if ( instance3.p != 23 ) println( "err 3" );

instance4 = new a
{
	321
};

if ( instance4.p != 321 ) println ("err 4" );
if ( instance4.h != 2 ) println( "err 5" );


instance0 = new a();
instance0 = new a();
instance0 = new a();
instance0 = new a();
instance0 = new a();


if ( instance0.b != "hello" ) println( "err 6" );

g = "hi2";
instance0.p = g;
if ( instance0.p != "hi2" ) println("err 7");
instance0.p = "1";
instance0.p = "2";
if ( (instance0.p = "<<>>") != "<<>>" )  println("err 8");
instance0.p = "4";
if ( instance0.p != "4" )  println("err 9");


instance0.p = 20;
if ( instance0.p != 20 )  println("err 10");
instance0.p = "ok2";
if ( instance0.p != "ok2" )  println("err 11");
instance0.p = 1.5;

if ( instance0.p != 1.5 ) println("err 12");
if ( instance0.b != "hello" ) println("err 13");

if ((instance0.b = instance0.p) != 1.5) println("err 14");
if ( instance0.b != 1.5)  println("err 15");

instance0.p = "long string";
if ( (instance0.b = instance0.p) != "long string" ) println("err 15");
if ( instance0.b != "long string") println("err 16");



j[30] = { 8, 9 };
if ( j[0] != 8 ) println("err 17");
if ( j[1] != 9 ) println("err 18");


aa = 10;
bb = 20;
j[10] = { aa, bb + aa, 45, 50, 77, 60 };
if ( j[0] != 10 )println("err 19");
if ( j[1] != 30 )println("err 20");
if ( j[2] != 45 )println("err 21");
if ( j[5] != 60 )println("err 22");

in = new a;

in.a = 8;
in.b = 7;
if ( in.a == in.b && 1 ) println("G1");
if ( in.a != in.a && 1 ) println("G2");
if ( in.a < in.b && 1 )  println("G3");
if ( in.b > in.a && 1 )  println("G4");
if ( in.b >= aa && 1 ) println("G5");
if ( in.a <= in.b && 1 ) println("G6");

localIf();
function localIf()
{
	::in.a = 8;
	::in.b = 7;
	if ( ::in.a == ::in.b && 1 ) println("L1");
	if ( ::in.a != ::in.a && 1 ) println("L2");

}

if ( n != n )
{
	println( "null NOT okay" );
}

in.a = 2;

if ( in.a + in.a + in.a + in.a != 8 ) println("err 23");

// play with the parser:
struct Bob
{//{
/*{*/};

function/**/a()//{
{
	var p = 23;
	var h = 2;
	b = "hello";
	var a;
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

if ( instance2.p != 321 ) println("err 24");
if ( instance2.a != 2 ) println("err 25");

instance3 = new a
{
};

if ( instance3.p != 23 ) println("err 26");


instance4 = new a
{
	321
};

if ( instance4.p != 321) println("err 27");
if ( instance4.h != 2 ) println("err 28");

struct/* */particle
{//
	x;
	y;
	z; // specifically does NOT get the 'global' z because this is a struct
	vx;
	vy;
	vz;
};

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

//if ( particles[10].x != 122.299881 ) { println(particles[10].x); println("err 29"); }
//if ( particles[499].y != 713.397705 ) { println(particles[499].y); println("err 30"); }



var t1 = new T1()
{
	55,66
};
if ( t1.a != 55 ) println("t1 1");
if ( t1.b != 66 ) println("t1 2");
if ( t1.c != 0 ) println("t1 3");

var t2 = new T1()
{
	0,555,666
};
if ( t2.a != 0 ) println("t2 1");
if ( t2.b != 555 ) println("t2 2");
if ( t2.c != 666) println("t2 3");

a = 10;
var t3 = new T1(a);
if ( t3.a != 11 ) println("t3 1");
if ( t3.b != 15 ) println("t3 2");
if ( t3.c != 10 ) println("t3 3");

a = 16;
var t4 = new T1(a) { b = 201, a = 101 };
if ( t4.a != 101 ) println("t4 1");
if ( t4.b != 201 ) println("t4 2");
if ( t4.c != 16 ) println("t4 3");

var t4 = new T1(a) { c = 601, b = 501, a = 401  };
if ( t4.a != 401 ) println("t4 1b");
if ( t4.b != 501 ) println("t4 2b");
if ( t4.c != 601 ) println("t4 3b");

struct T1( n ) { var a = 11; var b = 15; var c = n; };
