use strict;
use warnings;
use IO::Handle;
STDOUT->autoflush(1);
sub usage;
sub log_p;
$| = 1;
$ENV{"LANG"} = "C";
my $ip           = "127.0.0.1";
my $port         = 22;
my $user         = "";
my $passwd       = "";
my $rootpasswd   = "";
my $NO_SFTP_NEED = 0;
my $NO_SSH_NEED  = 0;
my $WHOFIRST     = 0;
my ( $sftpcmd, $sftpfile, $sshcmd, $configfile );
$sftpcmd = $sftpfile = $sshcmd = $configfile = "";
my $skip = 0;
my ( $user_re, $passwd_re, $rootpasswd_re, $port_re, $ip_re );
my (
    $user_re_index, $passwd_re_index, $rootpasswd_re_index,
    $port_re_index, $ip_re_index
);
$user_re_index = $passwd_re_index = $rootpasswd_re_index = $port_re_index =
  $ip_re_index = 0;
my $regex_reverse = 0;
my (
    $user_re_match, $passwd_re_match, $rootpasswd_re_match,
    $port_re_match, $ip_re_match
);
$user_re_match = $passwd_re_match = $rootpasswd_re_match = $port_re_match =
  $ip_re_match = 1;
my ( $chgpasswd, $passwd_cmd, $chgpasswd_index );
$chgpasswd_index = 0;
my ($logfile,$kn);
$kn="knonwhosts";
mkdir( "knownhosts", 0666 ) if ( !-d "knownhosts" );
REGEX_OPT:

foreach (@ARGV) {
    if (/^--user$/) {
        $user_re = eval { qr/$ARGV[1]/ };
        die "Check you regular expression for user: \'$ARGV[1]\'\n" if $@;
        $user_re_index = 1;
        shift @ARGV;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^--passwd$/) {
        $passwd_re = eval { qr/$ARGV[1]/ };
        die "Check you regular expression for passwd: \'$ARGV[1]\'\n" if $@;
        $passwd_re_index = 1;
        shift @ARGV;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^--rootpasswd$/) {
        $rootpasswd_re = eval { qr/$ARGV[1]/ };
        die "Check you regular expression for rootpasswd: \'$ARGV[1]\'\n" if $@;
        $rootpasswd_re_index = 1;
        shift @ARGV;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^--port$/) {
        $port_re = eval { qr/$ARGV[1]/ };
        die "Check you regular expression for port: \'$ARGV[1]\'\n" if $@;
        $port_re_index = 1;
        shift @ARGV;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^--ip$/) {
        $ip_re = eval { qr/$ARGV[1]/ };
        die "Check you regular expression for ip: \'$ARGV[1]\'\n" if $@;
        $ip_re_index = 1;
        shift @ARGV;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^-r$/) {
        $regex_reverse = 1;
        shift @ARGV;
        goto REGEX_OPT;
    }
    if (/^--chgpasswd$/) {
        $chgpasswd_index = 1;
        shift @ARGV;
        goto REGEX_OPT;
    }
    last;
}
goto CHGPASSWD if ($chgpasswd_index);
if ( $#ARGV == 0 ) {
    ($configfile) = @ARGV;
    $NO_SFTP_NEED = 1;
    $NO_SSH_NEED  = 1;
}
elsif ( $#ARGV == 1 ) {
    ( $sshcmd, $configfile ) = @ARGV;
    $NO_SFTP_NEED = 1;
}
elsif ( $#ARGV == 2 ) {
    ( $sftpcmd, $sftpfile, $configfile ) = @ARGV;
    $NO_SSH_NEED = 1;
}
elsif ( $#ARGV == 3 ) {
    if ( "$ARGV[0]" eq "put" || "$ARGV[0]" eq "get" ) {
        $WHOFIRST = 0;
        ( $sftpcmd, $sftpfile, $sshcmd, $configfile ) = @ARGV;
    }
    else {
        $WHOFIRST = 1;
        ( $sshcmd, $sftpcmd, $sftpfile, $configfile ) = @ARGV;
    }
}
else {
    usage;
}
open( CONFIG, "$configfile" )
  or die "Fail: open $configfile";
$logfile = log_p("$configfile")
  if ( $NO_SSH_NEED == 0 || $NO_SFTP_NEED == 0 );
while (<CONFIG>) {
    chomp;
    if (/^port=/) {
        s/^port=([\w]+)/$1/;
        $port = $_;
        if ($port_re_index) {
            if (/$port_re/) {
                $port_re_match = ( 1 - $regex_reverse );
            }
            else {
                $port_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^user=/) {
        s/^user=([\w\d\.\-]+)/$1/;
        $user = $_;
        if ($user_re_index) {
            if (/$user_re/) {
                $user_re_match = ( 1 - $regex_reverse );
            }
            else {
                $user_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^passwd=/) {
        s/^passwd=([\w\W]+)/$1/;
        $passwd = $_;
        if ($passwd_re_index) {
            if (/$passwd_re/) {
                $passwd_re_match = ( 1 - $regex_reverse );
            }
            else {
                $passwd_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^rootpasswd=/) {
        s/^rootpasswd=([\w\W]+)/$1/;
        $rootpasswd = $_;
        if ($rootpasswd_re_index) {
            if (/$rootpasswd_re/) {
                $rootpasswd_re_match = ( 1 - $regex_reverse );
            }
            else {
                $rootpasswd_re_match = $regex_reverse;
            }
        }
        next;
    }
    next if ( /^$/ || /^#/ );
    s/(^[\d]+\.[\d]+\.[\d]+\.[\d]+).*/$1/
      if (/^[\d]+\.[\d]+\.[\d]+\.[\d]+/)
      and $ip = $_;
    if ($ip_re_index) {
        if (/$ip_re/) {
            $ip_re_match = ( 1 - $regex_reverse );
        }
        else {
            $ip_re_match = $regex_reverse;
        }
    }
    $skip =
      $user_re_match *
      $passwd_re_match *
      $rootpasswd_re_match *
      $port_re_match *
      $ip_re_match;
    if ( !$skip ) {
        next;
    }
    print "\n\nip=\"$ip\" port=\"$port\" user=\"$user\" password=\"$passwd\" rootpasswd=\"$rootpasswd\"\n";
    if ( "$user" ne "" && "$passwd" ne "" && "$rootpasswd" ne "" ) {
        goto lab2 if ( $WHOFIRST == 1 );
      lab1:
        if ( $NO_SFTP_NEED == 0 ) {
            print "\nsftp -oPort=$port $user\@$ip\n";
            system(
"expect -f sftp.txt $ip $port $user \"$passwd\" \"$sftpcmd\" $sftpfile $logfile"
            );
        }
        goto lab5 if ( $WHOFIRST == 1 );
      lab2:
        if ( $NO_SSH_NEED == 0 ) {
            print "\nssh -oPort=$port $user\@$ip\n";
            system(
"tclsh85 tclsussh.tcl $ip $port $user \"$passwd\" \"$sshcmd\" \"$rootpasswd\" $logfile"
            );
        }
        goto lab1 if ( $WHOFIRST == 1 );
    }
    elsif ( "$user" ne "" && "$passwd" ne "" && "$rootpasswd" eq "" ) {
        goto lab4 if ( $WHOFIRST == 1 );
      lab3:
        if ( $NO_SFTP_NEED == 0 ) {
            print "\nsftp -oPort=$port $user\@$ip\n";
            system(
"expect -f sftp.txt $ip $port $user \"$passwd\" \"$sftpcmd\" $sftpfile $logfile"
            );
        }
        goto lab5 if ( $WHOFIRST == 1 );
      lab4:
        if ( $NO_SSH_NEED == 0 ) {
            print "\nssh -oPort=$port $user\@$ip\n";
            system(
"tclsh85 tclssh.tcl $ip $port $user \"$passwd\" \"$sshcmd\" $logfile"
            );
        }
        goto lab3 if ( $WHOFIRST == 1 );
    }
    else {
        print "Failed to process $configfile,please check up it\n";
        exit 1;
    }
  lab5:
}
close(CONFIG)
  or die "Fail: close $configfile";
exit 0;

CHGPASSWD:
if ( $#ARGV == 2 ) {
    ( $passwd_cmd, $chgpasswd, $configfile ) = @ARGV;
}
else {
    usage();
}
open( CONFIG, "$configfile" )
  or die "Fail: open $configfile";
$logfile = log_p("$configfile");
while (<CONFIG>) {
    chomp;
    if (/^port=/) {
        s/^port=([\w]+)/$1/;
        $port = $_;
        if ($port_re_index) {
            if (/$port_re/) {
                $port_re_match = ( 1 - $regex_reverse );
            }
            else {
                $port_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^user=/) {
        s/^user=([\w\d\.\-]+)/$1/;
        $user = $_;
        if ($user_re_index) {
            if (/$user_re/) {
                $user_re_match = ( 1 - $regex_reverse );
            }
            else {
                $user_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^passwd=/) {
        s/^passwd=([\w\W]+)/$1/;
        $passwd = $_;
        if ($passwd_re_index) {
            if (/$passwd_re/) {
                $passwd_re_match = ( 1 - $regex_reverse );
            }
            else {
                $passwd_re_match = $regex_reverse;
            }
        }
        next;
    }
    if (/^rootpasswd=/) {
        s/^rootpasswd=([\w\W]+)/$1/;
        $rootpasswd = $_;
        if ($rootpasswd_re_index) {
            if (/$rootpasswd_re/) {
                $rootpasswd_re_match = ( 1 - $regex_reverse );
            }
            else {
                $rootpasswd_re_match = $regex_reverse;
            }
        }
        next;
    }
    next if ( /^$/ || /^#/ );
    s/(^[\d]+\.[\d]+\.[\d]+\.[\d]+).*/$1/
      if (/^[\d]+\.[\d]+\.[\d]+\.[\d]+/)
      and $ip = $_;
    if ($ip_re_index) {
        if (/$ip_re/) {
            $ip_re_match = ( 1 - $regex_reverse );
        }
        else {
            $ip_re_match = $regex_reverse;
        }
    }
    $skip =
      $user_re_match *
      $passwd_re_match *
      $rootpasswd_re_match *
      $port_re_match *
      $ip_re_match;
    if ( !$skip ) {
        next;
    }
    print "\n\nip=\"$ip\" port=\"$port\" user=\"$user\" password=\"$passwd\" rootpasswd=\"$rootpasswd\"\n";
    if ( "$user" ne "" && "$passwd" ne "" && "$rootpasswd" ne "" ) {
        print "\nssh -oPort=$port $user\@$ip\n";
        system(
"tclsh85 tclsussh_passwd.tcl $ip $port $user \"$passwd\" \"$rootpasswd\" \"$passwd_cmd\" \"$chgpasswd\" \"$logfile\""
        );
    }
    elsif ( "$user" ne "" && "$passwd" ne "" && "$rootpasswd" eq "" ) {
        print "\nssh -oPort=$port $user\@$ip\n";
        system(
"tclsh85 tclssh_passwd.tcl $ip $port $user \"$passwd\" \"$passwd_cmd\" \"$chgpasswd\" \"$logfile\""
        );
    }
    else {
        usage();
    }
}
close(CONFIG)
  or die "Fail: close $configfile";
exit 0;

sub usage {
    my $tmpcmdname = $0;
    $tmpcmdname =~ s/.*\\(.*)/$1/;
    print "\t$tmpcmdname [options] sftpcmd sftpfile sshcmd configfile\n";
    print
"\t$tmpcmdname --chgpasswd [options] passwd_cmd new_password configfile\n";
    exit 1;
}

sub log_p {
    mkdir( "log", 0666 ) if ( !-d "log" );
    ($_) = @_;
    s/.*\\(\w+)\.txt/$1/;
    my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) =
      localtime(time);
    $year = $year + 1900;
    $mon  = $mon + 1;
    my $logfile = "$_-$year-$mon-$mday-$hour-$min-$sec\.log\.txt";
    open( LOGFILE, ">log\\$logfile" ) || die "Fail to open log\\$logfile: $!";
    print LOGFILE "Begin at $year $mon $mday $hour:$min:$sec ....\n";
    print LOGFILE
      "sftpcmd=\"$sftpcmd\", sftpfile=\"$sftpfile\", sshcmd=\"$sshcmd\",
    configfile=\"$configfile\"\n\n";
    close(LOGFILE) || die "Fail to close log\\$logfile: $!";
    return "log\\$logfile";
}
STDOUT->autoflush(0);
