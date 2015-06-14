use strict;
use warnings;
sub usage();
$| = 1;
$ENV{"LANG"} = "C";
usage() if ($#ARGV==-1);
my $ip           = "127.0.0.1";
my $port         = 22;
my $user         = "";
my $passwd       = "";
my $cmd          = "";
my $rcmd         = "";
$ip=$ARGV[0];
$port=$ARGV[1];
$user=$ARGV[2];
$passwd=$ARGV[3];
$cmd=$ARGV[4];
$rcmd=$ARGV[5];
my $logfile = "log/$ip-tun.log";
while (1) {
   system("tclsh85 tclssh_tun.tcl $ip $port $user \"$passwd\"  \"$cmd\" $logfile $rcmd");
   system ("sleep.cmd 10");
}
sub usage() {
   print "\n\ttunel.pl ip port user passwd cmd rcmd\n";
   exit 1;
}