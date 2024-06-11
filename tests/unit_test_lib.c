function assertEqual(actual, expected, comment)
{
	var message = "";
	if (actual == expected) {
		str::sprintf(message, "PASS" );
	} else {
		str::sprintf(message, "FAIL: %s", comment);
	}

	println(message);
}

function assertNotEqual(actual, expected, comment)
{
	var message = "";
	if (actual != expected) {
		str::sprintf(message, "PASS" );
	} else {
		str::sprintf(message, "FAIL: %s", comment);
	}

	println(message);
}

