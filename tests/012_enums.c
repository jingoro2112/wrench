/*~ ~*/

U();

function U()
{
	enum
	{
		value2 = 6,
		value3 = 6.5,
	};

	if ( value3 != 6.5 ) { println("f1"); }
	
	enum
	{
		a16 = 32000,
		a162 = -32000,
		a32 = 500000,
		a322 = -500000,
		f1 = 1.213,
		f2 = -2.345,
	}

	if ( value2 != 6 ) { println("f2 " + value2 ); }
	if ( a16 != 32000 ) { println("f3 " + a16 ); }
	if ( a162 != -32000 ) { println("f4 " + a162 ); }
	if ( a32 != 500000 ) { println("f5 " + a32 ); }
	if ( a322 != -500000 ) { println("f6 " + a322); }
	if ( f1 != 1.213 ) { println("fl0 " + f1); }
	if ( f2 != -2.345 ) { println("fl1 " + f2); }

}

enum
{
	name1
}

enum
{
	n1,
	n2,
	n3
}

enum
{
	name2 = 2
};

enum
{
	name3 = 3,
}

enum
{
	name4 = 4,
	name5
}

enum
{
	name6 = 6,
	name7 = 7
};

enum
{
	name8 = 8,
	name9 = 9,
}

enum
{
	name10 = 6,
	name11 = 6,
	name12,
	name13,
	name14 = -1,
	name15,
}

enum
{
	a16 = 32001,
	a162 = -32001,
	a32 = 500001,
	a322 = -500001,
};


if( name1 != 0 ) { println("bad 1"); }
if( n1 != 0 ) { println("bad 2"); }
if( n2 != 1 ) { println("bad 3"); }
if( n3 != 2) { println("bad 4"); }
if( name2 != 2 ) { println("bad 5"); }
if( name3 != 3 ) { println("bad 6"); }
if( name4 != 4 ) { println("bad 7"); }
if( name5 != 5 ) { println("bad 8"); }
if( name6 != 6 ) { println("bad 9"); }
if( name7 != 7 ) { println("bad a"); }
if( name8 != 8 ) { println("bad b"); }
if( name9 != 9 ) { println("bad c"); }
if( name10 != 6 ) { println("bad d"); }
if( name11 != 6 ) { println("bad e"); }
if( name12 != 7 ) { println("bad f"); }
if( name13 != 8 ) { println("bad g"); }
if( name14 != -1 ) { println("bad h"); }
if( name15 != 0 ) { println("bad i"); }
if ( a16 != 32001 ) { println("f7 " + a16 ); }
if ( a162 != -32001 ) { println("f8 " + a162 ); }
if ( a32 != 500001 ) { println("f9 " + a32 ); }
if ( a322 != -500001 ) { println("f10 " + a322); }

// named enums: basic auto-increment
enum Color { RED, GREEN, BLUE }

if ( Color::RED != 0 )   { println("ne1"); }
if ( Color::GREEN != 1 ) { println("ne2"); }
if ( Color::BLUE != 2 )  { println("ne3"); }

// named enum with explicit start value and continuation
enum Direction { NORTH = 1, SOUTH, EAST, WEST }

if ( Direction::NORTH != 1 ) { println("ne4"); }
if ( Direction::SOUTH != 2 ) { println("ne5"); }
if ( Direction::EAST != 3 )  { println("ne6"); }
if ( Direction::WEST != 4 )  { println("ne7"); }

// two namespaces with the same member name: no collision
enum Fruit { RED = 10, GREEN = 20, BLUE = 30 }

if ( Fruit::RED != 10 )   { println("ne8"); }
if ( Fruit::GREEN != 20 ) { println("ne9"); }
if ( Fruit::BLUE != 30 )  { println("nea"); }
if ( Color::RED != 0 )    { println("neb"); }  // unchanged by Fruit
if ( Color::GREEN != 1 )  { println("nec"); }

// named enum with explicit values and gaps, including negative
enum Status { IDLE = 0, RUNNING = 10, STOPPED = 20, ERR = -1 }

if ( Status::IDLE != 0 )     { println("ned"); }
if ( Status::RUNNING != 10 ) { println("nee"); }
if ( Status::STOPPED != 20 ) { println("nef"); }
if ( Status::ERR != -1 )     { println("neg"); }

// named enum with float values
enum Tolerance { TIGHT = 0.001, LOOSE = 0.1, NONE = 1.0 }

if ( Tolerance::TIGHT != 0.001 ) { println("neh"); }
if ( Tolerance::LOOSE != 0.1 )   { println("nei"); }
if ( Tolerance::NONE != 1.0 )    { println("nej"); }

// named enum used in expressions
if ( Color::RED + Color::BLUE != 2 )         { println("nek"); }
if ( Direction::SOUTH * Direction::NORTH != 2 ) { println("nel"); }

// named enum in switch/case
var s = Color::GREEN;
switch( s )
{
    case Color::RED:  { println("nem"); break; }
    case Color::GREEN: { break; }
    case Color::BLUE: { println("neo"); break; }
}

// named enum accessed from inside a function
function testNamedEnum()
{
    if ( Color::RED != 0 )       { println("nep"); }
    if ( Direction::WEST != 4 )  { println("neq"); }
    if ( Status::ERR != -1 )     { println("ner"); }

    // named enum declared inside a function (local scope)
    enum FuncLocal { AA = 100, BB = 200, CC }

    if ( FuncLocal::AA != 100 ) { println("nes"); }
    if ( FuncLocal::BB != 200 ) { println("net"); }
    if ( FuncLocal::CC != 201 ) { println("neu"); }
}

testNamedEnum();

// library constant / named enum interaction tests
//
// The test runner pre-registers:
//   TestLC::LIBVAL  = 100  (int, never claimed by any enum)
//   TestLC::BOTH    = 200  (int, same name as an enum member declared below)
//   TestMix::LIBONLY= 77   (int, only the lib side of a mixed-namespace enum)
//
// Rule: whichever the compiler sees first wins.
//   Before the enum declaration -> O_LoadLibConstant emitted -> runtime value
//   After  the enum declaration -> compile-time literal substituted -> enum value

// --- case 1: library constant lookup (no matching enum declared yet) ---
// TestLC::BOTH is registered as 200 and enum TestLC hasn't been declared yet,
// so the compiler emits O_LoadLibConstant -> resolves to 200 at runtime.
var lc_before = TestLC::BOTH;
if ( lc_before != 200 ) { println("lc1"); }

// TestLC::LIBVAL is registered as 100 and will never be claimed by an enum.
if ( TestLC::LIBVAL != 100 ) { println("lc2"); }

// --- case 2: enum declared; enum wins for its members from this point on ---
enum TestLC { BOTH = 999, ENUMONLY = 55 }

// TestLC::BOTH is now in constantValues -> tokenizer substitutes 999
if ( TestLC::BOTH != 999 )    { println("lc3"); }

// TestLC::LIBVAL is NOT in the enum -> still falls through to library lookup -> 100
if ( TestLC::LIBVAL != 100 )  { println("lc4"); }

// TestLC::ENUMONLY is only in the enum, never a library constant -> 55
if ( TestLC::ENUMONLY != 55 ) { println("lc5"); }

// --- case 3: mixed namespace - enum and library constants coexist ---
// TestMix::LIBONLY is registered as 77 (library constant).
// We declare enum TestMix with a different member; LIBONLY is not in the enum
// so it continues to resolve via library lookup.
enum TestMix { ENUMONLY = 33 }

if ( TestMix::ENUMONLY != 33 ) { println("lc6"); }  // enum literal
if ( TestMix::LIBONLY  != 77 ) { println("lc7"); }  // library constant


//println( RED ); // should emit an error
