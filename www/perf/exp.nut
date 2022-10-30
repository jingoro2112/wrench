local function expLoop(n)
{
    local sum = 0
    local i;
    for (i = 0; i < n; ++i)
      sum += exp(1./(1.+i))
    return sum
}

for( local i=0; i<20; ++i )
{
	expLoop(10000000);
}
