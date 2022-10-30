#!/usr/bin/perl
use strict;
use warnings;

sub fib {
    my $n = shift;

    if ($n == 1 or $n == 2) {
        return 1 
    }

    return (fib($n-1)+fib($n-2));            # recursive calling
}

fib(38);  