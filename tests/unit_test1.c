sys::importCompile(io::readFile( "tests/unit_test_lib.c"));


var q = {"a" : 1, "b" : 2, "c" : 3};
var r = {"a" : 1, "b" : 2, "c" : 4};
assertNotEqual(q, r, "{a: 1, b: 2, c: 3} and {a: 1, b: 2, c: 4} Hash Tables are NOT equal");


var a = 1;
var b = 1;
assertEqual(a, b, "1 and 1 are equal");

var c = 1;
var d = 2;
assertNotEqual(c, d, "1 and 2 are NOT equal");

var e = 2;
var f = 4;
assertNotEqual(e, f, "2 and 4 are NOT equal");

var af = 1.0;
var bf = 1.0;
assertEqual(af, bf, "1.0 and 1.0 are equal");

var cf = 1.0;
var df = 2.0;
assertNotEqual(cf, df, "1.0 and 2.0 are NOT equal");

var ef = 2.0;
var ff = 4.0;
assertNotEqual(ef, ff, "2.0 and 4.0 are NOT equal");

var aff = 1.0;
var bff = 1;
assertEqual(aff, bff, "1.0 and 1 are NOT equal");

var cff = 1.0;
var dff = 2;
assertNotEqual(cff, dff, "1.0 and 2 are NOT equal");

var eff = 2.0;
var fff = 4;
assertNotEqual(eff, fff, "2.0 and 4 are NOT equal");

var ag = 1;
var bg = 1.1;
assertNotEqual(ag, bg, "1 and 1.1 are NOT equal");

var cg = 1;
var dg = 2.001;
assertNotEqual(cg, dg, "1 and 2.001 are NOT equal");

var eg = 2;
var fg = 4.000001;
assertNotEqual(eg, fg, "2 and 4.000001 are NOT equal");

var g = 3;
var h = "o";
assertNotEqual(g, h, "3 and o are NOT equal");

var i = "Word";
var j = "Word";
assertEqual(i, j, "Word and Word are equal");

var k[] = {1, 2, 3, 4};
var l[] = {1, 2, 3, 4};
assertEqual(k, l, "[1, 2, 3, 4] and [1, 2, 3, 4] Arrays are equal");

var m[] = {1, 2, 3, 4};
var n[] = {1, 2, 3, 5};
assertNotEqual(m, n, "[1, 2, 3, 4] and [1, 2, 3, 5] Arrays are NOT equal");

var o = {"a" : 1, "b" : 2, "c" : 3};
var p = {"a" : 1, "b" : 2, "c" : 3};
assertEqual(o, p, "{a: 1, b: 2, c: 3} and {a: 1, b: 2, c: 3} Hash Tables are equal");

var s = {"a" : 1, "b" : 2, "c" : 3};
var t = {"a" : 1, "b" : 2, "c" : 3, "d" : 4};
assertNotEqual(s, t, "{a: 1, b: 2, c: 3} and {a: 1, b: 2, c: 3, d: 4} Hash Tables are NOT equal");

var u = {"a" : 1, "b" : 2, "c" : 3};
var v = {"a" : 1, "b" : "two", "c" : 3};
assertNotEqual(u, v, "{a: 1, b: 2, c: 3} and {a: 1, b: two, c: 4} Hash Tables are NOT equal");

var string1 = "Hello";
var string2 = "Hello";
assertEqual(string1, string2, "Hello and Hello are equal");

var string3 = "Hello";
var string4 = "World";
assertNotEqual(string3, string4, "Hello and World are NOT equal");

var string5 = "Hello";
var string6 = "Hello World";
assertNotEqual(string5, string6, "Hello and Hello World are NOT equal");


struct S {
	var a;
	var b;
	var c;
};

struct V {
	var a;
	var b;
	var c;
};


var s3 = new S { 1, 2, 3 };
var s4 = new S { 1, 2, 4 }; 
assertNotEqual(s3, s4, "S(1, 2, 3) and S(1, 2, 4) Structs are equal");

var s1 = new S { 1, 2, 3 };
var s2 = new S { 1, 2, 3 };

assertEqual(s1, s2, "S(1, 2, 3) and S(1, 2, 3) Structs are equal");

var v1 = new V { 1, 2, 3 };

assertNotEqual(s1, v1, "S(1, 2, 3) and V(1, 2, 3) Structs are equal");

var y1 = {"a" : 1, "b" : 2, "c" : 3};
var array = {1, 2, 3};
assertNotEqual(v1, y1, "V(1, 2, 3) and {a: 1, b: 2, c: {d, e}} Struct and HashTable are equal");
assertNotEqual(y1, v1, "{a: 1, b: 2, c: 3} and V(1, 2, 3) HashTable and Struct are equal");
assertNotEqual(array, v1, "{1, 2, 3} and V(1, 2, 3) Array and Struct are equal");
