function update_particle(p)
{
	p.x+=p.vx;
	p.y+=p.vy;
	p.z+=p.vz;
}

function update(particles)
{
	for( p : particles )//for( p=0; p<50000; ++p )
	{
		update_particle(p);
	}
}

function update_several_times(particles, count)
{
	for ( i = 0; i < count; ++i)
	{
		update(particles);
	}
}

function updateI(particles)
{
	for( p : particles )//	for( p=0; p<50000; ++p )
	{
		p.x += p.vx;
		p.y += p.vy;
		p.z += p.vz;
	}
}

function update_several_timesI( particles, count )
{
	for ( i = 0; i < count; ++i )
	{
		updateI(particles);
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

particles[50000];
gc_pause( 50000 );
for ( i = 0; i < 50000; ++i )
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

update_several_times( particles, 500 );
update_several_timesI( particles, 500 );

