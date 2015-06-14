use strict;
use warnings;
use Win32::Process qw(:DEFAULT STILL_ACTIVE);
use Cwd;
use File::Basename;
use IO::Handle;
STDOUT->autoflush(1);
my ( $fork_count, $fork_num, $host_num, $host_sum ) = qw(6 0 0 0);
my ( $sftp_cmd, $ssh_cmd, $sftp_file )=("","","");
my ( $workdir, $perldir ) = ("","");
my ( $USABLE,  $SLEEP )   = ("",2);
my ($exitcode);
my $p_alive;
my @obj;
my @str;
$workdir = getcwd();
$workdir =~ s/\//\\\\/;
( -e "$workdir" . "\\\\tt.pl" ) or print "[+] tt.pl not found\n" and exit 1;

foreach my $env_var ( "PATH", "Path" ) {
    foreach ( split /;/, $ENV{$env_var} ) {
        s/\//\\\\/;
        if ( -e "$_" . "\\\\perl.exe" ) {
            $perldir = "$_";
            last;
        }
    }
}
if ( $perldir eq "" ) {
    print "[+] Perl interpreter not found\n";
    exit 1;
}
my $opt_is_ok = 0;
if ( $#ARGV == 3 ) {
    $opt_is_ok  = 1;
    $fork_count = $ARGV[0];
    $sftp_cmd   = $ARGV[1];
    $sftp_file  = $ARGV[2];
    $ssh_cmd    = $ARGV[3];
}
elsif ( $#ARGV == 2 ) {
    if ( $ARGV[0] =~ /\d+/ ) {
        if ( $ARGV[1] eq "get" || $ARGV[1] eq "put" ) {
            $opt_is_ok = 1;
	    $fork_count=$ARGV[0];
            $sftp_cmd  = $ARGV[1];
            $sftp_file = $ARGV[2];
        }
    }
    elsif ( $ARGV[0] eq "get" || $ARGV[0] eq "put" ) {
        $opt_is_ok = 1;
        $sftp_cmd  = $ARGV[0];
        $sftp_file = $ARGV[1];
        $ssh_cmd   = $ARGV[2];
    }
}
elsif ( $#ARGV == 1 ) {
    if ( $ARGV[0] =~ /\d+/ ) {
        $opt_is_ok  = 1;
        $fork_count = $ARGV[0];
        $ssh_cmd    = $ARGV[1];
    }
    elsif ( $ARGV[0] eq "get" || $ARGV[0] eq "put" ) {
        $opt_is_ok = 1;
        $sftp_cmd  = $ARGV[0];
        $sftp_file = $ARGV[1];
    }
} elsif ( $#ARGV == 0 ) {
    $opt_is_ok=1;
    $ssh_cmd=$ARGV[0];
}
if ( !$opt_is_ok ) {
    my $basename=basename($0);
    print "usage:\n";
    print "\t$basename fork_num sftp_cmd sftp_file ssh_cmd\n";
    print "\t$basename sftp_cmd sftp_file ssh_cmd\n";
    print "\t$basename fork_num sftp_cmd sftp_file\n";
    print "\t$basename fork_num ssh_cmd\n";
    print "\t$basename sftp_cmd sftp_file\n";
    print "\nAny bug, please mail to ningxibo\@gmail.com\n";
    exit 1;
}
$obj[$_] = $USABLE foreach ( 0 .. $fork_count - 1 );
foreach ( glob( "$workdir" . "\\\\kh\\\\*.txt" ) ) {
  START_PROCESS:
    foreach my $fork_ind ( 0 .. $fork_count - 1 ) {
        if ( $obj[$fork_ind] eq $USABLE ) {
            $fork_num = $fork_ind;
            print "[+] Create process $fork_num now ....\n";
            goto OK;
        }
    }
    sleep $SLEEP;
    goto FIND_USABLE_PROCESS;
  OK:
    if ( $sftp_cmd ne "" && $ssh_cmd ne "" ) {
	$str[$fork_num]="$sftp_cmd \"$sftp_file\" \"$ssh_cmd\" \"$_\"";
	print "[+] tt2.pl $str[$fork_num]\n";
	Win32::Process::Create(
	    ( $obj[$fork_num] ),
	    "$perldir\\perl.exe",
	    "perl.exe"
	    . " tt2.pl"
	    . " $sftp_cmd"
	    . " \"$sftp_file\""
	    . " \"$ssh_cmd\""
	    . " \"$_\"",
	    0,
	    CREATE_NEW_CONSOLE,
	    "$workdir"
	    ) || die "[+] Failed to create process:$!";
    } elsif ( $sftp_cmd eq "" && $ssh_cmd ne "" ) {
	$str[$fork_num]="\"$ssh_cmd\" \"$_\"";
	print "[+] tt2.pl $str[$fork_num]\n";
	Win32::Process::Create(
	    ( $obj[$fork_num] ),
	    "$perldir\\perl.exe",
	    "perl.exe"
	    . " tt2.pl"
	    . " \"$ssh_cmd\""
	    . " \"$_\"",
	    0,
	    CREATE_NEW_CONSOLE,
	    "$workdir"
	    ) || die "[+] Failed to create process:$!";
    } elsif ( $sftp_cmd ne "" && $ssh_cmd eq "") {
	$str[$fork_num]="$sftp_cmd $sftp_file $_";
	print "[+] tt2.pl $str[$fork_num]\n";
	Win32::Process::Create(
	    ( $obj[$fork_num] ),
	    "$perldir\\perl.exe",
	    "perl.exe"
	    . " tt2.pl"
	    . " $sftp_cmd"
	    . " \"$sftp_file\""
	    . " \"$_\"",
	    0,
	    CREATE_NEW_CONSOLE,
	    "$workdir"
	    ) || die "[+] Failed to create process:$!";
    }
    next;
  FIND_USABLE_PROCESS:
    foreach my $fork_ind ( 0 .. $fork_count - 1 ) {
        $obj[$fork_ind]->GetExitCode($exitcode);
        if ( $exitcode != STILL_ACTIVE ) {
            $obj[$fork_ind] = $USABLE;
	    print "[+] Process $fork_ind could be used again ....\n";
        }
    }
    goto START_PROCESS;
}
STILL_ALIVE:
$p_alive=0;
foreach my $fork_ind ( 0 .. $fork_count - 1 ) {
    $obj[$fork_ind]->GetExitCode($exitcode);
    if ( $exitcode == STILL_ACTIVE ) {
	$p_alive=1;
	print "[+] Process $fork_ind is still alive ....\n";
	print "[+] tt2.pl $str[$fork_ind]\n";
    }
}
if ($p_alive) {
    sleep $SLEEP;
    goto STILL_ALIVE;
} else {
    goto ALL_DIE;
}
ALL_DIE:
    print "[+] Procesing finished";
STDOUT->autoflush(0);
