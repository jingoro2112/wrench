function isprime(n)
{
	for (i = 2; i < n; ++i)
		if (n % i == 0)
			return false;
	return true;
}



function primes(n)
{
	count = 0;
	for (i = 2; i < n; ++i)
	{
		if (isprime(i))
		{
			++count;
		}
	}
	return count;
}


for( i=0; i<20; ++i )
{
	primes(14000);
}
