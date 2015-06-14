use strict;
use warnings;
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
$sftpcmd=$sftpfile=$sshcmd=$configfile="";
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
my $logfile=log_p("$configfile") if ( $NO_SSH_NEED ==0 || $NO_SFTP_NEED==0 );
while (<CONFIG>) {
    chomp;
    if (/^port=/) {
        s/^port=([\w]+)/$1/;
        $port = $_;
        next;
    }
    if (/^user=/) {
        s/^user=([\w\d\.\-]+)/$1/;
        $user = $_;
        next;
    }
    if (/^passwd=/) {
        s/^passwd=([\w\W]+)/$1/;
        $passwd = $_;
        next;
    }
    if (/^rootpasswd=/) {
        s/^rootpasswd=([\w\W]+)/$1/;
        $rootpasswd = $_;
        next;
    }
    next if ( /^$/ || /^#/ );
    s/(^[\d]+\.[\d]+\.[\d]+\.[\d]+).*/$1/
      if (/^[\d]+\.[\d]+\.[\d]+\.[\d]+/)
      and $ip = $_;
    print
"\n\nip=\"$ip\" port=\"$port\" user=\"$user\" password=\"$passwd\" rootpasswd=\"$rootpasswd\"\n";
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
"expect -f sussh.txt $ip $port $user \"$passwd\" \"$sshcmd\" \"$rootpasswd\" $logfile"
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
            system("expect -f ssh.txt $ip $port $user \"$passwd\" \"$sshcmd\" $logfile");
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

sub usage {
    my $tmpcmdname = $0;
    $tmpcmdname =~ s/.*\\(.*)/$1/;
    print "\t$tmpcmdname sftpcmd sftpfile sshcmd configofhost";
    exit 1;
}
sub log_p {
  mkdir("log",0666) if ( ! -d "log" );
  ($_)=@_;
  s/.*\\(\w+)\.txt/$1/;
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
  $year=$year+1900;$mon=$mon+1;
  my $logfile="$_\.log\.txt";
  open(LOGFILE,">log\\$logfile") || die "Fail to open log\\$logfile: $!";
  print LOGFILE "Begin at $year $mon $mday $hour:$min:$sec ....\n";
  print LOGFILE "sftpcmd=\"$sftpcmd\", sftpfile=\"$sftpfile\", sshcmd=\"$sshcmd\", configfile=\"$configfile\"\n\n";
  close(LOGFILE) || die "Fail to close log\\$logfile: $!";
  return "log\\$logfile";
}
