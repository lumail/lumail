#!/usr/bin/perl -w
#
#  We want braces to be on their own lines.
#
#  For example this is bad:
#
#    if ( foo ) {
#        bar();
#        baz();
#    }
#
#
#  This is good:
#
#   if ( foo )
#   {
#       bar();
#       baz();
#   }
#
#

use strict;
use File::Find;
use Test::More qw( no_plan );


#
#  Find all the files beneath the current directory,
# and call 'checkFile' with the name.
#
find( { wanted => \&checkFile, no_chdir => 1 }, '.' );


#
#  Check a file.
#
#
sub checkFile
{

    # The file.
    my $file = $File::Find::name;

    # We don't care about directories
    return if ( !-f $file );

    # We only care about .cc + .h
    return unless ( $file =~ /\.(cc|h)$/ );

    #
    #  Count hanging braces characters
    #
    my $count = countBraces($file);

    is( $count, 0, "Script has no bogus braces: $file" );
}




sub countBraces
{
    my ($file) = (@_);
    my $count = 0;

    open( my $handle, "<", $file ) or
      die "Cannot open $file - $!";
    foreach my $line (<$handle>)
    {
        chomp($line);

        # Look for lines with trailing "{".
        if ( $line =~ /\{$/ )
        {
            my $orig = $line;

            $line =~ s/[ \t]//g;

            if ( length($line) != 1 )
            {
                print "Bogus Line: $orig\n";
            }

            $count += ( length($line) - 1 );

        }
    }
    close($handle);

    return ($count);
}
