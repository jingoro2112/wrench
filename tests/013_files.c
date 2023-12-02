/*~ ~*/

j = "_test file_";
if ( !file::write( "_std_file_test.txt", j) ) println("bad file write");
k = file::read( "_std_file_test.txt" );
if ( k != j ) println( "bad readback" );