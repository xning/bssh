use strict;
use warnings;
use Win32::Process qw(:DEFAULT STILL_ACTIVE);
use Net::OpenSSH;
use Cwd;
use File::Basename;
use IO::Handle;
STDOUT->autoflush(1);
sub usage();
sub getopt();
my ($configfile,$active_num,@cmd,$command);
$active_num=6;
my ($ip,$port,$user,$passwd,$rootpasswd);
my %opts;
$ip="127.0.0.1";
$port=22;
my ($qs_name,$qs_count,$qs_srv_count);
my ($bad_srv_count,$bad_srv_ip);
my ($ind,$w,@obj,$exitcode);
getopt();
$w=$active_num;
$ind=0;
my $workdir = getcwd();
$workdir =~ s/\//\\\\/;
open( CONFIG, "$configfile" )
  or die "Failed to open $configfile: $!\n";
while (<CONFIG>) {
    while ($ind >= $w) {
	for my $i (0 .. $w-1) {
	    eval {
		$obj[$i]->GetExitCode($exitcode);
		if ( $exitcode != STILL_ACTIVE ) {
		    $ind--;
		    last;
		}
	    };
	    if ($@) {
		$ind--;
		last;
	    }
	}
	sleep 5;
    }
    chomp;
    next if ( /^$/ || /^\#/ );
    if (/^port=/) {
        s/^port=([\w]+)/$1/;
        $port = $_;
	$opts{port}=$_;
        next;
    }
    if (/^user=/) {
        s/^user=([\w\d\.\-]+)/$1/;
        $user = $_;
	$opts{user}=$_;
        next;
    }
    if (/^passwd=/) {
        s/^passwd=([\w\W]+)/$1/;
        $passwd = $_;
	$opts{passwd}=$_;
        next;
    }
    if (/^rootpasswd=/) {
        s/^rootpasswd=([\w\W]+)/$1/;
        $rootpasswd = $_;
        next;
    }
    if (/^qs=/) {
	s/qs=(.*)/$1/;
	$qs_name=$_;
	($ip,$port,$user,$passwd,$rootpasswd)=qw("127.0.0.1" 22 "root" "" "");
	$w=$active_num;$ind=0;
	next;
    }
    if (/^[\d]+\.[\d]+\.[\d]+\.[\d]+/) {
	s/(^[\d]+\.[\d]+\.[\d]+\.[\d]+).*/$1/;
	$ip = $_;
    }
#    Win32::Process::Create(
#	($obj[$ind]),
#	"$ENV{SYSTEMROOT}\\system32\\$command",
#	"$command"
#	. " -P $port -pw \"$passwd\" -batch"
#	. " \"$user\@$ip\""
#	. " @cmd",
#	0,
#	CREATE_NEW_CONSOLE,
#	"$workdir"
#	) || die "[+] 给$qs_name服务器$ip:$port建立进程失败: $!\n";
    print "[+] 处理$qs_name的服务器$ip:$port ....\n";
    my $ssh=Net::OpenSSH->new($ip, %opts) or
	die "[+] Failed to create ssh process:$!\n";
    my ($input,@input);
    my $output = $ssh->system("echo hello; sleep 20; echo bye");
    print "$output\n";
#    system("$command"," -P $port -pw \"$passwd\" -batch "," \"$user\@$ip\""," @cmd") and
#	die "[+] 给$qs_name服务器$ip:$port建立进程失败: $!\n";
    print "[+] $qs_name的服务器$ip:$port处理完毕\n";
    $ind++;
}

sub usage() {
print <<EOF
auto_sftp.pl: atuomate sftp uploading or downloading.
usage:auto_sftp sftp_cmd configfile
if any bug, please contact ningxibo\@gmail.com.
EOF
;
exit(1);
}
sub getopt() {
    my $i=0;
    my $is_arg=0;
    $command="psftp.exe";
    foreach (@ARGV) {
	$i++;
	if ($is_arg) {
	    $is_arg=0;
	    next;
	}
	if (/^-h$|^--help$/) {
	    usage();
	} elsif (/^-c$/) {
	    $active_num=$ARGV[$i];
	    $is_arg=1;
	    next;
	} elsif (/^-s$/)  {
	    $command="plink.exe";
	    next;
	} else {
	    push @cmd,"\"$_\"";
	    next;
	}
    }
    pop @cmd;
    if ($i>=1) {
	$configfile=$ARGV[-1];
    } else {
	usage();
    }
    if ($#cmd==-1) {
	push @cmd,"exit";
    }
}
