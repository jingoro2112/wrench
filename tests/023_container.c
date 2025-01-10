/*~ ~*/

var push[];
list::push( push, 33 );
return;
println( push._count + " 0:" + push[0] );
list::push( push, 44 );
println( push._count + " 0:" + push[0] + " 1:" + push[1] );

var popb[] = { 10, 20, 30 };
println( list::pop_back(popb) );
println( popb._count, popb[1] );

var pop[] = { 10, 20, 30 };
println( list::pop(pop) );
println( pop._count, pop[0] );

var i[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
array::insert( i, 6 );
println( i[5] + " " + i[6] + " " + i[7] + " " + i[8] );

array::insert( i, 2, 2 );
println( "1:" + i[1] + " 2:" + i[2] + " 3:" + i[3] + " 4:" + i[4] + " 5:" + i[5] + " 6:" + i[6] );

var t[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
array::truncate( t, 4 );
println( t[3], t._count );

var r[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
println( r[0], r[1], r[2], r._count );
array::remove( r, 1 ); // remove the '20'
println( r[0], r[1], r[2], r._count );
array::remove( r, 0 ); // remove the '10'
println( r[0], r[1], r[2], r._count );
array::remove( r, 0, 100 ); // remove it all
println( r._count );

var a;
array::clear( a );

a[0] = "hi";
println( a[0] );
a[1] = "there";
println( list::peek(a) );
