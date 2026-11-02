/*~ ~*/

var j = "_test file_";
if ( !io::writeFile( "_std_file_test.txt", j) ) println("bad file write");
var k = io::readFile( "_std_file_test.txt" );
if ( k != j ) println( "bad readback" );

io::writeFile( "_std_delete_test.txt", "delete me" );
io::deleteFile( "_std_delete_test.txt" );
if ( io::readFile("_std_delete_test.txt") ) println("bad file delete");
