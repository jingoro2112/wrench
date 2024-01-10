/*~ ~*/

j = "_test file_";
if ( !io::writeFile( "_std_file_test.txt", j) ) println("bad file write");
k = io::readFile( "_std_file_test.txt" );
if ( k != j ) println( "bad readback" );
