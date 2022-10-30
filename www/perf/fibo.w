function fibo(n)
{
	if ( n <= 1) return n;
	return fibo( n - 2 ) + fibo(n - 1);
}

for( i=0; i<39; ++i )
{
	fibo(i);
}
