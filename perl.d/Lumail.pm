use strict;
use warnings;

package Lumail;

use Net::IMAP::Client;


=begin doc

Connect to the IMAP server the user has specified.

We do this by reading from the environment the values:

=over 8

=item imap_server

The URL of the IMAP server to connect to, for example C<imap://imap.example.com/>, or C<imaps://imap.gmail.com/>.

=item imap_username

The username to connect as.

=item imap_password

The password to authenticate with.

=back

=cut

sub imap_connect
{
    my $user = $ENV{ 'imap_username' } || "";
    my $pass = $ENV{ 'imap_password' } || "";
    my $imap = $ENV{ 'imap_server' }   || "";

    my $port = 0;
    my $ssl  = 0;
    my $host = "";
    if ( $imap =~ /^imaps:\/\/([^\/]+)\/?$/i )
    {
        $port = 993;
        $host = $1;
        $ssl  = 1;
    }
    elsif ( $imap =~ /^imap:\/\/([^\/]+)\/?$/i )
    {
        $port = 143;
        $host = $1;
        $ssl  = 0;
    }

    my $handle = Net::IMAP::Client->new(
        server          => $host,
        user            => $user,
        pass            => $pass,
        ssl             => $ssl,
        ssl_verify_peer => 0,
        port            => $port

                                       );
    if ( !$handle )
    {
        return undef;
    }
    if ($handle)
    {
        $handle->login();
    }
    return ($handle);
}

1;
