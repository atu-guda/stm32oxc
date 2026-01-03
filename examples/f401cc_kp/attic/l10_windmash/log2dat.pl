#!/usr/bin/perl
#===============================================================================
#
#         FILE: log2dat.pl
#
#        USAGE: ./log2dat.pl  
#
#  DESCRIPTION: 
#
#      OPTIONS: ---
# REQUIREMENTS: ---
#       AUTHOR: Anton Guda (atu), 
# ORGANIZATION: 
#      VERSION: 1.0
#      CREATED: 11/09/22 17:48:15
#===============================================================================

use strict;
use warnings;
use utf8;

while (<>) {
    if( $_ =~ /\s*#/ ) {
        next;
    }
    my ($fl_r, $S_r, $fl_m, $S_m, $fl_a, $fl_b, $t ) = split;
    $S_r = hex( $S_r );
    $S_m = hex( $S_m );
    $fl_a = hex( $fl_a );
    $fl_b = hex( $fl_b );
    $t = 1.0e-3 * $t;
    print $t, ' ', $S_r,' ',  $S_m,' ',  $fl_r,' ',  $fl_m,' ',  $fl_a,' ',  $fl_b, "\n";
}

