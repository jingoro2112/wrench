/*~ ~*/

j = "_test file_";
if ( !file::write( "_std_file_test.txt", j) ) print("bad file write");
k = file::read( "_std_file_test.txt" );
if ( k != j ) print( "bad readback" );