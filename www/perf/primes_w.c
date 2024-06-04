function isprime(n)
{
	for (i = 2; i < n; ++i)
	{
		printl(n, " ", i);

		if (n % i == 0)
		{
			print( "FALSE\n" );
			return false;
		}
	}
	print( "TRUE\n" );
	return true;
}



function primes(n)
{
	count = 0;
	for (k = 2; k <= n; ++k)
	{
		if (isprime(k))
		{
			printl(k);
			++count;
		}
	}
	return count;
}


//for( j=0; j<20; ++j )
{
	isprime(2);
//	isprime(3);
//	isprime(4);
//	primes(3);
//	primes(14000);
}
