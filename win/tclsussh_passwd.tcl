#!/bin/sh
# \
exec tclsh85 "$0" ${1+"$@"}
package require Expect
set host       [lindex $argv 0]
set port       [lindex $argv 1]
set user       [lindex $argv 2]
set passwd     [lindex $argv 3]
set rootpasswd [lindex $argv 4]
set passwd_cmd    [lindex $argv 5]
set chgpasswd [lindex $argv 6]
set logfile    [lindex $argv 7]
log_file $logfile
log_user 1
spawn -noecho ssh -oPreferredAuthentications=password -oStrictHostKeyChecking=no -oCheckHostIP=no -oUserKnownHostsFile=knownhosts\\$host-$port.txt -oConnectTimeout=15 -oPort=$port $user@$host
sleep 3
expect {
"Connection timed out"     {exit 1;}
"Connection reset by peer" {exit 1;}
"Permission denied"        {exit 1;}
"Connection closed"        {exit 1;}
}
expect "assword:"
send "$passwd\r";
# set timeout -1
expect $
send "su\r"
expect assword:
send "$rootpasswd\r"
expect "#"
send "$passwd_cmd\r"
expect "assword: "
send "$chgpasswd\r"
expect "assword: "
send "$chgpasswd\r"
expect -re "$|#"
send "exit\r"
expect "$"
send "exit\r"
expect logout
exit 0

