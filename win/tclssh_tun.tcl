#!/bin/sh
# \
exec tclsh85 "$0" ${1+"$@"}
package require Expect
# exp_internal 1
set host    [lindex $argv 0]
set port    [lindex $argv 1]
set user    [lindex $argv 2]
set passwd  [lindex $argv 3]
set execcmd [lindex $argv 4]
set logfile [lindex $argv 5]
set rmcd    [lindex $argv 6]
log_file $logfile
log_user 1
spawn -noecho ssh -oPreferredAuthentications=password -oStrictHostKeyChecking=no -oConnectTimeout=15 -oPort=$port -$rmcd $user@$host
sleep 3
expect {
"Connection timed out"     {exit 1;}
"Connection reset by peer" {exit 1;}
"Permission denied"        {exit 1;}
"Connection closed"        {exit 1;}
}
expect "assword:"
send "$passwd\r";
set timeout -1
expect -re "$|#"
send "$execcmd\r"
expect {
"going down for reboot NOW!" {exit 1;}
}
expect -re "$|#"
send "exit\r"
# sleep 1
expect logout
exit 0

