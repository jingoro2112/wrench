function fibR(n)
{
    if (n < 2) return n;
    return (fibR(n-2) + fibR(n-1));
}

for( local i=0; i<39; ++i )
{
	fibR(i);
}