/*~ ~*/

var push[];
if ( list::count(push) != 0 ) println("list1");

list::push( push, 33 );
if ( list::count(push) != 1 ) println("list2");
if ( push[0] != 33 )println("list3");

list::push( push, 44 );
if ( list::count(push) != 2 ) println("list4");
if ( push[0] != 44 ) println("list5");
if ( push[1] != 33 ) println("list6");

var popb[] = { 10, 20, 30 };
list::pop_back( popb );
if ( list::count(popb) != 2 ) println("list7");
if ( popb[1] != 20 ) println("list8");
if ( popb[0] != 10 ) println("list9");

var pop[] = { 10, 20, 30 };
list::pop( pop );

if ( list::count(pop) != 2 ) println( "list10 " + list::count(pop) );
if ( pop[0] != 20 ) println("list11 " + pop[0] );
if ( pop[1] != 30 ) println("list12 " + pop[1] );


var i[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
array::insert( i, 6 );
if ( i[5] != 60 ) println("array0 " + i[5] );
if ( i[6] != 0 ) println("array1 " + i[6] );
if ( i[7] != 70 ) println("array2 " + i[7] );
if ( i[8] != 80 ) println("array3 " + i[8] );

array::insert( i, 2, 2 );
if ( i[1] != 20 ) println("array4 " + i[1] );
if ( i[2] != 0 ) println("array5 " + i[2] );
if ( i[3] != 0 ) println("array6 " + i[3] );
if ( i[4] != 30 ) println("array7 " + i[4] );
if ( i[5] != 40 ) println("array8 " + i[5] );
if ( i[6] != 50 ) println("array9 " + i[6] );

var t[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
array::truncate( t, 4 );
if ( t[3] != 40 ) println("array10" + t[3] );
if ( t._count != 4 ) println("array11" + t._count  );

var r[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
array::remove( r, 1 ); // remove the '20'
if ( r[0] != 10 ) println("array12" + r[0] );
if ( r[1] != 30 ) println("array13" + r[1] );
if ( r[2] != 40 ) println("array14" + r[2] );

array::remove( r, 0 ); // remove the '10'
if ( r[0] != 30 ) println("array12" + r[0] );
if ( r[1] != 40 ) println("array13" + r[1] );
if ( r[2] != 50 ) println("array14" + r[2] );
array::remove( r, 0, 100 ); // remove it all
if ( array::count(r) != 0 ) println("array15" + array::count(r) );

