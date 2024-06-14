var gh;
var gt;
var tt = "tail";

gh = "head";
var h = gh + tt;
if ( h != "headtail" ) println( "g fail1" );

var g = "head" + tt;
if ( g != "headtail" ) println( "g fail2" );

h = "head";
h = h + tt;
if ( h != "headtail" ) println( "g fail3" );

h = "head";
h += tt;
if ( h != "headtail" ) println( "g fail4" );

gt = "tail";
h = "head";
h += gt;
if ( h != "headtail" ) println( "g fail5" );

gh = "head";
gh += tt;
if ( gh != "headtail" ) println( "g fail6" );


binaryAddTail( "tail" );

function binaryAddTail( tail )
{
	::gh = "head";
	var h = gh + tail;
	if ( h != "headtail" ) println( "fail1" );

	var g = "head" + tail;
	if ( g != "headtail" ) println( "fail2" );

	h = "head";
	h = h + tail;
	if ( h != "headtail" ) println( "fail3" );

	h = "head";
	h += tail;
	if ( h != "headtail" ) println( "fail4" );

	::gt = "tail";
	h = "head";
	h += ::gt;
	if ( h != "headtail" ) println( "fail5" );

	::gh = "head";
	::gh += tail;
	if ( ::gh != "headtail" ) println( "fail6" );
}
