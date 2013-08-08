#!/usr/bin/perl -w
#
#  Test that our lumail.lua file contains no trailing ";" chars.
#
# Steve
# --


use strict;
use File::Find;
use Test::More qw( no_plan );


#
#  Look for the file wherever it might be.
#
foreach my $file (qw! ./lumail.lua ../lumail.lua !)
{

    if ( -e $file )
    {
        is( 0, checkFile($file),
            "The file $file has zero trailing semicolons" );
    }
}

exit(0);


#
#  Check the named file for trailing ";", return the count.
#
sub checkFile
{
    my ($file) = (@_);
    my $count = 0;

    open( my $handle, "<", $file ) or
      die "Cannot open $file - $!";
    foreach my $line (<$handle>)
    {

        # We will count multiple tab characters in a single line.
        while ( $line =~ /(.*);\s+/ )
        {
            $count += 1;
            $line = $1 . $2;
        }
    }
    close($handle);

    return ($count);
}
