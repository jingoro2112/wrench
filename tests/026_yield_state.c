/*~ ~*/

var sum = 0;

function step(v)
{
	sum += v;

	// Yield from inside a function frame and verify resume value.
	var resumed = yield(sum);
	if ( resumed != sum ) println("y1");

	return sum;
}

for( var i=1; i<=24; ++i )
{
	var got = step(i);
	var expect = (i * (i + 1)) / 2;
	if ( got != expect ) println("y2");
}

// Zero-arg yields are resumed without host-provided replacement.
if ( yield() != 0 ) println("y3");

if ( yield("abc") != "abc" ) println("y4");
if ( yield(12.5) != 12.5 ) println("y5");

var a[] = { 1, 2, 3 };
if ( yield(a[1]) != 2 ) println("y6");
