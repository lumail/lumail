#!/usr/bin/perl -w
#
#  Test that we don't use any "//" comments.
#
# Steve
# --


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

    # We only care about source-files.
    return unless ( $file =~ /\.(cc|h)$/ );

    #
    #  Count "//" coments; ensure the total is zero.
    #
    my $count = processFile($file);
    is( $count, 0, "Script has no single-line comments: $file" );
}



#
#  Count and return the number of "//" comments.
#
sub processFile
{
    my ($file) = (@_);
    my $count = 0;

    open( my $handle, "<", $file ) or
      die "Cannot open $file - $!";
    foreach my $line (<$handle>)
    {
        next if  ( $line =~ /lumail\.org/ );

        # We will count multiple tab characters in a single line.
        while ( $line =~ /(.*)\/\/(.*)/ )
        {
            $line = $1 . $2;
            $count += 1;
        }

    }
    close($handle);

    return ($count);
}
