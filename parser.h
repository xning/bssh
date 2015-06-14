#ifndef BSSH_PARSE_H
#define BSSH_PARSE_H
#define COMMIT_RET -1
#define IP_RET 3
#define END_RET 5
#define PASSWD_RET 1
#define PORT_RET 4
#define ROOTPASSWD_RET 2
#define USER_RET 0
char nextstr_ptr[]="next\n";
char okstr_ptr[]="\nok\n";
struct login_record {
	char user[64];
	char passwd[128];
	char rootpasswd[128];
	char ip[64];
	char port[16];
};
int all_write_out(int fds,char *str);
int try_read_in(int fds,char *buf,int buf_len);
#endif
