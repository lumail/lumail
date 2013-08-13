#!/usr/bin/perl -w
#
#  Test that no source file includes the same header more than once.
#
# Steve
# --


use strict;
use File::Find;
use Test::More qw( no_plan );


#
#  Look for the source files wherever they might be.
#
foreach my $dir (qw! ../src/ ./src/ !)
{
    if ( -d $dir )
    {
        foreach my $file ( sort( glob("$dir/*.cc $dir/*.h") ) )
        {
            checkFile($file);
        }
    }
}

exit(0);


#
#  Check there are no multiple-included files.
#
sub checkFile
{
    my ($file) = (@_);

    my %seen;

    open( my $handle, "<", $file ) or
      die "Cannot open $file - $!";
    foreach my $line (<$handle>)
    {
        if ( $line =~ /^#include\s+(["<])([^"<]+)[">]/ )
        {
            my $include = $2;
            $seen{ $include } += 1;
        }
    }
    close($handle);

    foreach my $inc ( keys %seen )
    {
        my $count = $seen{ $inc };

        is( $count, 1, "$file: Include file only included once: $inc" );
    }
}
