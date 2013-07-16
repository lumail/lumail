#!/usr/bin/perl -w
#
#  Test that none of our scripts contain any literal TAB characters.
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
    #  Count TAB characters; ensure the total is zero.
    #
    my $count = countTabCharacters($file);
    is( $count, 0, "Script has no tab characters: $file" );
}



#
#  Count and return the number of literal TAB characters contained
# in the specified file.
#
sub countTabCharacters
{
    my ($file) = (@_);
    my $count = 0;

    open( my $handle, "<", $file ) or
      die "Cannot open $file - $!";
    foreach my $line (<$handle>)
    {

        # We will count multiple tab characters in a single line.
        while ( $line =~ /(.*)\t(.*)/ )
        {
            $count += 1;
            $line = $1 . $2;
        }
    }
    close($handle);

    return ($count);
}
