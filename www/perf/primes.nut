local function isprime(n)
{
    local i;
    for (i = 2; i < n; ++i)
        if (n % i == 0)
            return false;
    return true;
}



local function primes(n)
{
    local count = 0;
    local i;
    for (i = 2; i <= n; ++i)
        if (isprime(i))
            ++count;
    return count;
}

for( local i=0; i<20; ++i )
{
	primes(14000);
}
