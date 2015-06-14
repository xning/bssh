%option 8bit
%{
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#define RET_VAL_STR_LEN 64
#define STRBUF_LEN 256
char strbuf[STRBUF_LEN];
void pre_str(char *,char *,char *);
int usage(void);
%}
IP [0-9]+"."[0-9]+"."[0-9]+"."[0-9]+ 
%%
"user=".* {
    memset(strbuf,0,STRBUF_LEN);
    pre_str("user=",yytext,strbuf);
    return USER_RET;
}
"passwd=".* {
    memset(strbuf,0,STRBUF_LEN);
    pre_str("passwd=",yytext,strbuf);
    return PASSWD_RET;
}
"rootpasswd=".* {
    memset(strbuf,0,STRBUF_LEN);
    pre_str("rootpasswd=",yytext,strbuf);
    return ROOTPASSWD_RET;
}
{IP} {
    pre_str("",yytext,strbuf);
    return IP_RET;
}
"port=".* {
	memset(strbuf,0,STRBUF_LEN);
	pre_str("port=",yytext,strbuf);
	return PORT_RET;
}
<<EOF>> {
	return END_RET;
}
[ \t\n]+
[#].*    {
	return COMMIT_RET;
}
%%
int main(int argc,char **argv)
{
    int in=-1;
    int fds=-1;
    int len=0;
    char ret_val_str[RET_VAL_STR_LEN];
    ++argv, --argc;
    if (argc == 2 ) {
	yyin = fopen(argv[0], "r");
	fds=atoi(argv[1]);
    }
    else if (argc ==1 ) {
	yyin = fopen(argv[0], "r");
	fds=1;
    }
    else 
	    usage();
    while (1) {
	    in=yylex();
	    if (in == END_RET) {
		    sprintf(ret_val_str,"%d",in);
		    all_write_out(fds,ret_val_str);
		    all_write_out(fds,okstr_ptr);
		    break;
	    }
	    if (in == COMMIT_RET)
		    continue;
	    sprintf(ret_val_str,"%d",in);
	    all_write_out(fds,ret_val_str);
	    all_write_out(fds,"\n");
	    all_write_out(fds,strbuf);
	    all_write_out(fds,okstr_ptr);
	    
	    memset(ret_val_str,0,RET_VAL_STR_LEN);
	    while (1) {
		    len=strlen(ret_val_str);
		    if (strncmp(ret_val_str+len-strlen(nextstr_ptr),nextstr_ptr,strlen(nextstr_ptr))==0)
			    break;
		    try_read_in(fds,ret_val_str+len,RET_VAL_STR_LEN-len);
	    }
    }
    return 0;
}

void pre_str(char *strbuf1,char *strbuf2, char *strbuf3)
{
	int len = strlen(strbuf1);
	strcpy(strbuf3,strbuf2+len);
	strbuf2[len]=0;
}

int usage(void)
{
	fprintf(stderr,"Need at least on argment.\n");
	return 1;
}

int all_write_out(int fds,char *str)
{
	int i=0;
	i=write(fds,str,strlen(str));
	if (i<strlen(str))
		if (i<0)
			return all_write_out(fds,str);
		else
			return all_write_out(fds,str+i);
	return i;
}
int try_read_in(int fds,char *buf,int buf_len)
{
	int i=0;
	i=read(fds,buf,buf_len);
	if (i<=0)
		return try_read_in(fds,buf,buf_len);
	return i;
}
