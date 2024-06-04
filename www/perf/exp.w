function expLoop(n)
{
    sum = 0;
    for( i=0; i<n; ++i )
	{
		sum = sum + math::exp(1./(1.+i));
	}
    return sum;
}

for( i=0; i<20; ++i )
{
	expLoop(10000000);
}
