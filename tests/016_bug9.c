// Segmentation fault at the end of the function execution. #9

function faulty_func()
{
	t = 1.0;
	x = t * 4.0;  // <-- any arithmetic operation involving a literal
	// can be x = 1.0 + 2.0;
	z = x * t; // <-- any binary operation on two variables
	return z;
}

log( faulty_func() );
