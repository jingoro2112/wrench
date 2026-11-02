/*~ ~*/

if ( math::sizeofFloat() == 4 )
{
	return;
}

var gTiny = 0.00000000000000000000000000000000000000000000000001;
var gHuge = 1000000000000000000000000000000000000000.0;
var gNear = 1.000000000001;
var gMantissaA = 16777216.0;
var gMantissaB = 16777217.0;
var gA;
var gB;
var gStore;

function checkTrue( label, condition )
{
	if ( !condition )
	{
		println( label );
	}
}

function checkFalse( label, condition )
{
	if ( condition )
	{
		println( label );
	}
}

function checkClose( label, actual, expected, tolerance )
{
	var diff = actual - expected;
	if ( diff < 0.0 )
	{
		diff = -diff;
	}

	if ( diff > tolerance )
	{
		println( label );
	}
}

function testLiteralParsing()
{
	checkTrue( "sizeof", math::sizeofFloat() == 8 );
	checkTrue( "lit tiny positive", gTiny > 0.0 );
	checkTrue( "lit tiny bounded", gTiny < 0.000000000000000000000000000000000000000000000001 );
	checkTrue( "lit huge positive", gHuge > 100000000000000000000000000000000000000.0 );
	checkTrue( "lit near greater", gNear > 1.0 );
	checkTrue( "lit near distinct", gNear != 1.0 );
	checkClose( "lit near delta", gNear - 1.0, 0.000000000001, 0.000000000000001 );
	checkTrue( "mantissa add", gMantissaA + 1.0 == gMantissaB );
	checkTrue( "mantissa distinct", gMantissaB != gMantissaA );
	checkClose( "lit scale", gTiny * gHuge, 0.00000000001, 0.0000000000001 );

	var localTiny = 0.00000000000000000000000000000000000000000000000001;
	var localHuge = 1000000000000000000000000000000000000000.0;
	var localNear = 1.000000000001;
	checkTrue( "local tiny", localTiny > 0.0 );
	checkTrue( "local huge", localHuge > 100000000000000000000000000000000000000.0 );
	checkTrue( "local near", localNear != 1.0 );
	checkClose( "direct literal", (0.125 + 0.375) * 8.0, 4.0, 0.0 );
}

function testArithmetic()
{
	checkClose( "add ff pos", 1.25 + 2.5, 3.75, 0.0 );
	checkClose( "add ff neg", -1.25 + -2.5, -3.75, 0.0 );
	checkClose( "add fi", 1.25 + 2, 3.25, 0.0 );
	checkClose( "add if", 2 + 1.25, 3.25, 0.0 );

	checkClose( "sub ff pos", 5.5 - 2.25, 3.25, 0.0 );
	checkClose( "sub ff neg", -5.5 - -2.25, -3.25, 0.0 );
	checkClose( "sub fi", 5.5 - 2, 3.5, 0.0 );
	checkClose( "sub if", 5 - 2.5, 2.5, 0.0 );

	checkClose( "mul ff pos", 1.5 * 2.5, 3.75, 0.0 );
	checkClose( "mul ff neg", -1.5 * 2.5, -3.75, 0.0 );
	checkClose( "mul fi", 1.5 * 3, 4.5, 0.0 );
	checkClose( "mul if", 3 * 1.5, 4.5, 0.0 );

	checkClose( "div ff pos", 7.5 / 2.5, 3.0, 0.0 );
	checkClose( "div ff neg", -7.5 / 2.5, -3.0, 0.0 );
	checkClose( "div fi", 7.5 / 3, 2.5, 0.0 );
	checkClose( "div if", 7 / 2.0, 3.5, 0.0 );

	if ( 1.0 / 0.0 ) println( "div zero ff" );
	if ( 1 / 0.0 ) println( "div zero if" );
	if ( 1.0 / 0 ) println( "div zero fi" );
}

function testOptimizedLoadsAndStores()
{
	::gA = 5.5;
	::gB = -1.5;
	var lA = 6.5;
	var lB = -2.0;

	checkClose( "ll add", lA + lB, 4.5, 0.0 );
	checkClose( "gg add", ::gA + ::gB, 4.0, 0.0 );
	checkClose( "gl add", ::gA + lB, 3.5, 0.0 );
	checkClose( "lg add", lA + ::gB, 5.0, 0.0 );

	checkClose( "ll sub", lA - lB, 8.5, 0.0 );
	checkClose( "gg sub", ::gA - ::gB, 7.0, 0.0 );
	checkClose( "gl sub", ::gA - lB, 7.5, 0.0 );
	checkClose( "lg sub", lA - ::gA, 1.0, 0.0 );

	checkClose( "ll mul", lA * lB, -13.0, 0.0 );
	checkClose( "gg mul", ::gA * ::gB, -8.25, 0.0 );
	checkClose( "gl mul", ::gA * lB, -11.0, 0.0 );
	checkClose( "lg mul", lA * ::gB, -9.75, 0.0 );

	checkClose( "ll div", lA / lB, -3.25, 0.0 );
	checkClose( "gg div", ::gA / ::gB, -3.666666666666667, 0.00000000000001 );
	checkClose( "gl div", ::gA / lB, -2.75, 0.0 );
	checkClose( "lg div", lA / ::gA, 1.1818181818181819, 0.00000000000001 );

	::gStore = 1.5;
	::gStore = ::gStore + 2.25;
	::gStore = ::gStore - 1.0;
	::gStore = ::gStore * -2.0;
	::gStore = ::gStore / -5.5;
	checkClose( "global store", ::gStore, 1.0, 0.0 );

	var lStore = 1.5;
	lStore = lStore + 2.25;
	lStore = lStore - 1.0;
	lStore = lStore * -2.0;
	lStore = lStore / -5.5;
	checkClose( "local store", lStore, 1.0, 0.0 );
}

function testCompoundAssignments()
{
	var f = 1.25;
	f += 2;
	checkClose( "f add assign int", f, 3.25, 0.0 );
	checkClose( "f sub assign ret", f -= 0.5, 2.75, 0.0 );
	checkClose( "f sub assign", f, 2.75, 0.0 );
	f *= -4;
	checkClose( "f mul assign int", f, -11.0, 0.0 );
	checkClose( "f div assign ret", f /= -2.0, 5.5, 0.0 );
	checkClose( "f div assign", f, 5.5, 0.0 );

	var i = 2;
	i += 0.25;
	checkClose( "i add assign float", i, 2.25, 0.0 );
	i = 2;
	i -= 0.25;
	checkClose( "i sub assign float", i, 1.75, 0.0 );
	i = 2;
	i *= 0.25;
	checkClose( "i mul assign float", i, 0.5, 0.0 );
	i = 2;
	i /= 0.25;
	checkClose( "i div assign float", i, 8.0, 0.0 );

	var zero = 0.0;
	f = 10.0;
	f /= zero;
	if ( f ) println( "f div assign zero" );

	i = 10;
	i /= zero;
	if ( i ) println( "i div assign zero" );
}

function testComparisonsAndLogic()
{
	checkTrue( "cmp lt true", 1.25 < 1.5 );
	checkFalse( "cmp lt false", 1.5 < 1.25 );
	checkTrue( "cmp le true", 1.25 <= 1.25 );
	checkFalse( "cmp le false", 1.5 <= 1.25 );
	checkTrue( "cmp gt true", 1.5 > 1.25 );
	checkFalse( "cmp gt false", 1.25 > 1.5 );
	checkTrue( "cmp ge true", 1.25 >= 1.25 );
	checkFalse( "cmp ge false", 1.25 >= 1.5 );
	checkTrue( "cmp eq true", 1.25 == 1.25 );
	checkFalse( "cmp eq false", gNear == 1.0 );
	checkTrue( "cmp ne true", gNear != 1.0 );
	checkFalse( "cmp ne false", 1.25 != 1.25 );

	checkTrue( "cmp if lt", 1 < 1.25 );
	checkTrue( "cmp fi gt", 1.25 > 1 );
	checkTrue( "cmp neg lt", -2.5 < -1.25 );
	checkFalse( "cmp neg gt", -2.5 > -1.25 );

	var lA = 1.000000000001;
	var lB = 1.0;
	::gA = 1.000000000001;
	::gB = 1.0;
	checkTrue( "ll cmp", lA > lB );
	checkTrue( "gg cmp", ::gA > ::gB );
	checkTrue( "gl cmp", ::gA > lB );
	checkTrue( "lg cmp", lA > ::gB );
	checkFalse( "ll cmp false", lB > lA );
	checkFalse( "gg cmp false", ::gB > ::gA );

	checkTrue( "logic and tt", gTiny && 2.0 );
	checkFalse( "logic and tf", gTiny && 0.0 );
	checkFalse( "logic and ft", 0.0 && gTiny );
	checkTrue( "logic or tf", gTiny || 0.0 );
	checkTrue( "logic or ft", 0.0 || gTiny );
	checkFalse( "logic or ff", 0.0 || 0.0 );
}

function testCastsUnaryAndIntegerOps()
{
	var castPos;
	var castNeg;
	var castSrcPos = 1.999999999;
	var castSrcNeg = -1.999999999;
	castPos = (int)castSrcPos;
	castNeg = (int)castSrcNeg;
	checkTrue( "cast int pos", castPos == 1 );
	checkTrue( "cast int neg", castNeg == -1 );

	var p = -0.5;
	var postInc;
	var preInc;
	var postDec;
	var preDec;
	postInc = p++;
	checkClose( "post inc ret", postInc, -0.5, 0.0 );
	checkClose( "post inc value", p, 0.5, 0.0 );
	preInc = ++p;
	checkClose( "pre inc ret", preInc, 1.5, 0.0 );
	checkClose( "pre inc value", p, 1.5, 0.0 );
	postDec = p--;
	checkClose( "post dec ret", postDec, 1.5, 0.0 );
	checkClose( "post dec value", p, 0.5, 0.0 );
	preDec = --p;
	checkClose( "pre dec ret", preDec, -0.5, 0.0 );
	checkClose( "pre dec value", p, -0.5, 0.0 );

	checkTrue( "mod pos", 13 % 5 == 3 );
	checkTrue( "mod neg", -13 % 5 == -3 );
	checkTrue( "shift left", 3 << 4 == 48 );
	checkTrue( "shift right", 48 >> 4 == 3 );
	checkTrue( "bit and", (0x33 & 0x0F) == 3 );
	checkTrue( "bit or", (0x30 | 0x03) == 0x33 );
	checkTrue( "bit xor", (0x33 ^ 0x11) == 0x22 );
	checkTrue( "bit not", ~0x0000FFFF == 0xFFFF0000 );
	checkTrue( "bit neg and", (-16 & 0x0F) == 0 );

	var b = 0x0F;
	b &= 0x03;
	checkTrue( "and assign", b == 3 );
	b |= 0x10;
	checkTrue( "or assign", b == 0x13 );
	b ^= 0x11;
	checkTrue( "xor assign", b == 2 );
	b = b << 4;
	checkTrue( "left assign", b == 32 );
	b = b >> 3;
	checkTrue( "right assign", b == 4 );
	b %= 3;
	checkTrue( "mod assign", b == 1 );
}

function testContainers()
{
	var arr[] = { 1.25, -2.5, 0.0 };
	arr[2] = gTiny;
	checkTrue( "array count", arr._count == 3 );
	checkClose( "arr binary ee", arr[0] + arr[1], -1.25, 0.0 );
	arr[0] += arr[1];
	checkClose( "arr add assign ee", arr[0], -1.25, 0.0 );
	arr[0] -= -0.75;
	checkClose( "arr sub assign ef", arr[0], -0.5, 0.0 );
	arr[1] *= -2;
	checkClose( "arr mul assign ei", arr[1], 5.0, 0.0 );
	arr[1] /= 2.0;
	checkClose( "arr div assign ef", arr[1], 2.5, 0.0 );
	arr[2] *= gHuge;
	checkClose( "arr tiny huge", arr[2], 0.00000000001, 0.0000000000001 );

	var idx = 1;
	arr[idx] = 16777216.0;
	arr[idx] += 1.0;
	checkTrue( "arr mantissa", arr[idx] == 16777217.0 );
	checkClose( "arr post inc ret", arr[0]++, -0.5, 0.0 );
	checkClose( "arr post inc value", arr[0], 0.5, 0.0 );
	checkClose( "arr pre inc ret", ++arr[0], 1.5, 0.0 );
	checkClose( "arr pre inc value", arr[0], 1.5, 0.0 );

	var h = { "a": 1.5, "b": -2.25, "tiny": 0.0 };
	h["tiny"] = gTiny;
	checkTrue( "hash tiny", h["tiny"] > 0.0 );
	h["a"] += h["b"];
	checkClose( "hash add ee", h["a"], -0.75, 0.0 );
	checkClose( "hash b after add", h["b"], -2.25, 0.0 );
	h["a"] *= -4.0;
	checkClose( "hash mul ef", h["a"], 3.0, 0.0 );
	checkClose( "hash b before div", h["b"], -2.25, 0.0 );
	h["a"] /= h["b"];
	checkClose( "hash div ee", h["a"], -1.3333333333333333, 0.00000000000001 );
	h["b"] -= 0.75;
	checkClose( "hash sub ef", h["b"], -3.0, 0.0 );
}

function testMath()
{
	var pi = 3.141592653589793;
	checkClose( "sin", math::sin( pi / 2.0 ), 1.0, 0.000000000001 );
	checkClose( "cos", math::cos( pi ), -1.0, 0.000000000001 );
	checkClose( "tan", math::tan( pi / 4.0 ), 1.0, 0.000000000001 );
	checkClose( "sinh", math::sinh( 0.0 ), 0.0, 0.0 );
	checkClose( "cosh", math::cosh( 0.0 ), 1.0, 0.0 );
	checkClose( "tanh", math::tanh( 0.0 ), 0.0, 0.0 );
	checkClose( "asin", math::asin( 0.5 ), 0.5235987755982989, 0.000000000001 );
	checkClose( "acos", math::acos( 0.5 ), 1.0471975511965979, 0.000000000001 );
	checkClose( "atan", math::atan( 1.0 ), 0.7853981633974483, 0.000000000001 );
	checkClose( "log exp", math::log( math::exp( 20.0 ) ), 20.0, 0.000000000001 );
	checkClose( "log10", math::log10( 1000.0 ), 3.0, 0.000000000001 );
	checkTrue( "exp huge", math::exp( 700.0 ) > gHuge );
	checkClose( "sqrt", math::sqrt( 2.0 ) * math::sqrt( 2.0 ), 2.0, 0.000000000001 );
	checkClose( "floor", math::floor( -1.25 ), -2.0, 0.0 );
	checkClose( "ceil", math::ceil( -1.25 ), -1.0, 0.0 );
	checkClose( "trunc", math::trunc( -1.25 ), -1.0, 0.0 );
	checkClose( "abs", math::abs( -gTiny ), gTiny, 0.0 );
	checkClose( "deg2rad", math::deg2rad( 180.0 ), pi, 0.000000000001 );
	checkClose( "rad2deg", math::rad2deg( pi ), 180.0, 0.000000000001 );
}

function testTernaryAndBranches()
{
	var t = gTiny ? 1.25 : -1.25;
	checkClose( "tern true", t, 1.25, 0.0 );
	t = 0.0 ? 1.25 : -1.25;
	checkClose( "tern false", t, -1.25, 0.0 );

	if ( gNear > 1.0 )
	{
		t = 2.0;
	}
	else
	{
		println( "branch true" );
	}

	if ( gNear < 1.0 )
	{
		println( "branch false" );
	}
}

testLiteralParsing();
testArithmetic();
testOptimizedLoadsAndStores();
testCompoundAssignments();
testComparisonsAndLogic();
testCastsUnaryAndIntegerOps();
testContainers();
testMath();
//testTernaryAndBranches();

