#define  _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <regex.h>
#include <getopt.h>
#include "parser.h"
#define LOGINCONF 512
#define OPTNUM 32
#define EXECCMD_NUM 1024
#define USERATHOST 512
#define OPTOFPORTLEN 16
#define CONFIG_FILE_LEN 512
#define ZSTRING_NUM 128
#define REG_ERR_BUF 128
void err_quit(char *pstr, int quit_val);
int err_return(char *pstr, int return_val);
void ana(char *str1, char *str2, char *str3);
int create(struct login_record *re, char *str);
int usage();
char *userathost(char *str1, char *str2, char *str3);
int main(int argc, char *argv[])
{
    int usfds[2];
    int pid;
    int fpid;
    char usfdsstr[128];
    int len = 0;
    int indicator = 0;
    char *parser = "parser";
    char *parserstr;
    char *config_file_name = "/.bssh_config";
    char *homedir;
    char config_file[CONFIG_FILE_LEN];
    char *bsshconfigstr;
    char *cmdstr_default = "bssh";
    char *cmdstr;
    int sftpindex = 0;
    int execindex = 0;
    int fcount;
    unsigned int waittime=7;
    char *execcmd[EXECCMD_NUM];
    char userathost_array[USERATHOST];
    char portopt[OPTOFPORTLEN];
    char *parserstrptr[4];
    char *zstrptr[ZSTRING_NUM];
    /* variables for regexes support */
    int longindex = 0;
    struct option long_opt_ptr[] = {
	{"user", required_argument, NULL, 'U'},
	{"passwd", required_argument, NULL, 'P'},
	{"rootpasswd", required_argument, NULL, 'R'},
	{"ip", required_argument, NULL, 'I'},
	{"port", required_argument, NULL, 'O'},
	{"waittime",required_argument,NULL,'T'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
    };
    char *user_str_ptr = NULL;
    regex_t user_reg;
    char *passwd_str_ptr = NULL;
    regex_t passwd_reg;
    char *rootpasswd_str_ptr = NULL;
    regex_t rootpasswd_reg;
    char *ip_str_ptr = NULL;
    regex_t ip_reg;
    char *port_str_ptr = NULL;
    regex_t port_reg;
    int reg_return_code = 0;
    int cflags = 0;
    char reg_err_buf[REG_ERR_BUF];
/*
 * c: bssh or bsftp?
 * f: configure file
 * n: set HISTFILESIZE to zero
 * p: where is the parser
 * l: list the arguments
 * z: zstrings for bssh
 */
    char *optstr = "+c:ef:hln::p:rz:";
    char opt;
    int cindex, findex, lindex, pindex, zindex, optc;
    char *optstrptr[OPTNUM];
    struct login_record login_struc;
    char *loginconf = malloc(LOGINCONF * sizeof(*loginconf));
    if (loginconf == NULL)
	err_quit("malloc failed", 1);
    memset(loginconf, 0, LOGINCONF);
    memset(&login_struc, 0, sizeof(login_struc));
    memset(portopt, 0, OPTOFPORTLEN);
    memset(config_file, 0, CONFIG_FILE_LEN);
    cindex = findex = pindex = -1;
    lindex = zindex = 0;
    optc = 0;
    int reverse = 0;
    int histsize = 0;
    if (setenv("PATH", ".:~/bin:/bin:/sbin:/usr/bin:/usr/sbin", 1) == -1)
	err_quit("setenv PATH failed", 1);
    homedir = getenv("HOME");
    if (homedir == NULL)
	err_quit("get home dir failed", 1);
    if ((strlen(homedir) + strlen(config_file_name)) >= CONFIG_FILE_LEN)
	err_quit("path of config file is too long", 1);
    strcat(config_file, homedir);
    strcat(config_file, config_file_name);

    while ((opt =
	    getopt_long(argc, argv, optstr, long_opt_ptr,
			&longindex)) != -1) {
	switch (opt) {
	case 'c':
	    cindex = optc;
	    optstrptr[cindex] = optarg;
	    optc = (optc + 1) % OPTNUM;
	    break;
	case 'e':
	    cflags = REG_EXTENDED;
	    break;
	case 'f':
	    findex = optc;
	    optstrptr[findex] = optarg;
	    optc = (optc + 1) % OPTNUM;
	    break;
	case 'h':
	    usage();
	    break;
	case 'l':
	    lindex = 1;
	    break;
	case 'n':
	    if (optarg == NULL) {
		if (setenv("HISTFILESIZE", "500", 1) == -1)
		    err_quit("setenv HISTFILESIZE failed", 1);
	    } else {
		if (setenv("HISTFILESIZE", optarg, 1) == -1)
		    err_quit("setenv HISTFILESIZE failed", 1);
	    }
	    histsize = 1;
	    break;
	case 'p':
	    pindex = optc;
	    optstrptr[optc] = optarg;
	    optc = (optc + 1) % OPTNUM;
	    break;
	case 'r':
	    reverse = 1;
	    break;
	case 'z':
	    zindex = (zindex + 1) % ZSTRING_NUM;
	    zstrptr[zindex - 1] = optarg;
	    break;
	    /* long options */
	case 'U':
	    user_str_ptr = optarg;
	    break;
	case 'P':
	    passwd_str_ptr = optarg;
	    break;
	case 'R':
	    rootpasswd_str_ptr = optarg;
	    break;
	case 'I':
	    ip_str_ptr = optarg;
	    break;
	case 'O':
	    port_str_ptr = optarg;
	    break;
	case 'T':
	  waittime = strtoul(optarg, NULL, 10);
	  break;
	}
    }
    if (pindex != -1) {
	parserstr = optstrptr[pindex];
	if (access(parserstr, F_OK) == -1) {
	    fprintf(stderr, "%s is not exist\n", parserstr);
	    exit(1);
	}
	if (access(parserstr, X_OK) == -1) {
	    fprintf(stderr, "%s is not executable\n", parserstr);
	    exit(1);
	}
    } else
	parserstr = parser;

    if (findex != -1)
	bsshconfigstr = optstrptr[findex];
    else
	bsshconfigstr = config_file;
    if (access(bsshconfigstr, F_OK) == -1) {
	fprintf(stderr, "%s is not exist\n\n", bsshconfigstr);
	usage();
    }
    if (access(parserstr, R_OK) == -1) {
	fprintf(stderr, "%s is not readable\n", bsshconfigstr);
	exit(1);
    }


    if (cindex != -1)
	cmdstr = optstrptr[cindex];
    else
	cmdstr = cmdstr_default;
    if (strlen(cmdstr) > 4)
	if (strncmp(cmdstr + strlen(cmdstr) - 5, "bsftp", 5) == 0)
	    sftpindex = 1;
    if (!histsize)
	if (setenv("HISTFILESIZE", "0", 1) == -1)
	    err_quit("setenv PATH failed", 1);
    /* process regexes */
    if (user_str_ptr) {
	reg_return_code = regcomp(&user_reg, user_str_ptr, cflags);
	if (reg_return_code != 0) {
	    memset(reg_err_buf, 0, REG_ERR_BUF);
	    regerror(reg_return_code, &user_reg, reg_err_buf, REG_ERR_BUF);
	    fprintf(stderr, "%s: %s\n", user_str_ptr, reg_err_buf);
	    exit(1);
	}
    }
    if (passwd_str_ptr) {
	reg_return_code = regcomp(&passwd_reg, passwd_str_ptr, cflags);
	if (reg_return_code != 0) {
	    memset(reg_err_buf, 0, REG_ERR_BUF);
	    regerror(reg_return_code, &passwd_reg, reg_err_buf,
		     REG_ERR_BUF);
	    fprintf(stderr, "%s: %s\n", passwd_str_ptr, reg_err_buf);
	    exit(1);
	}
    }
    if (rootpasswd_str_ptr) {
	reg_return_code =
	    regcomp(&rootpasswd_reg, rootpasswd_str_ptr, cflags);
	if (reg_return_code != 0) {
	    memset(reg_err_buf, 0, REG_ERR_BUF);
	    regerror(reg_return_code, &rootpasswd_reg, reg_err_buf,
		     REG_ERR_BUF);
	    fprintf(stderr, "%s: %s\n", rootpasswd_str_ptr, reg_err_buf);
	    exit(1);
	}
    }
    if (ip_str_ptr) {
	reg_return_code = regcomp(&ip_reg, ip_str_ptr, cflags);
	if (reg_return_code != 0) {
	    memset(reg_err_buf, 0, REG_ERR_BUF);
	    regerror(reg_return_code, &ip_reg, reg_err_buf, REG_ERR_BUF);
	    fprintf(stderr, "%s: %s\n", ip_str_ptr, reg_err_buf);
	    exit(1);
	}
    }
    if (port_str_ptr) {
	reg_return_code = regcomp(&port_reg, port_str_ptr, cflags);
	if (reg_return_code != 0) {
	    memset(reg_err_buf, 0, REG_ERR_BUF);
	    regerror(reg_return_code, &port_reg, reg_err_buf, REG_ERR_BUF);
	    fprintf(stderr, "%s: %s\n", port_str_ptr, reg_err_buf);
	    exit(1);
	}
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, usfds) != 0)
	err_quit("sockepair failed", 1);
    pid = fork();
    if (pid == -1)
	err_quit("fork failed", 1);
    else if (!pid) {
	close(usfds[0]);
	sprintf(usfdsstr, "%d", usfds[1]);
	parserstrptr[0] = "parser";
	parserstrptr[1] = bsshconfigstr;
	parserstrptr[2] = usfdsstr;
	parserstrptr[3] = NULL;
	if (execv(parserstr, parserstrptr)
	    == -1)
	    err_quit("exec parser failed", 1);
    } else {
	close(usfds[1]);
	if (lindex) {
	    printf("cmd: %s\tconfigfile: %s\n", cmdstr, bsshconfigstr);
	    if (zindex) {
		printf("zstring:\n");
		for (fcount = 0; fcount < zindex; fcount++) {
		    printf("\t%s\n", zstrptr[fcount]);
		}
	    }
	}
	while (1) {
	    len = strlen(loginconf);
	    if (strcmp(loginconf + len - strlen(okstr_ptr), okstr_ptr) !=
		0) {
		if (read(usfds[0], loginconf + len, LOGINCONF - len) < 0)
		    continue;
		continue;
	    }


	    indicator = create(&login_struc, loginconf);
	    if (indicator == 0) {
		all_write_out(usfds[0], nextstr_ptr);
		memset(loginconf, 0, LOGINCONF);
		continue;

	    }
	    if (indicator == 2) {
		if (lindex)
		    goto EXIT;
		break;
	    }
	    /* process regexes */
	    if (user_str_ptr)
		if (!reverse) {
		    if (regexec(&user_reg, login_struc.user, 0, NULL, 0) !=
			0)
			goto CONTINUE;
		} else {
		    if (regexec(&user_reg, login_struc.user, 0, NULL, 0) ==
			0)
			goto CONTINUE;
		}

	    if (passwd_str_ptr)
		if (!reverse) {
		    if (regexec
			(&passwd_reg, login_struc.passwd, 0, NULL, 0) != 0)
			goto CONTINUE;
		} else {
		    if (regexec
			(&passwd_reg, login_struc.passwd, 0, NULL, 0) == 0)
			goto CONTINUE;
		}
	    if (rootpasswd_str_ptr)
		if (!reverse) {
		    if (regexec
			(&rootpasswd_reg, login_struc.rootpasswd, 0,
			 NULL, 0) != 0)
			goto CONTINUE;
		} else {
		    if (regexec
			(&rootpasswd_reg, login_struc.rootpasswd, 0,
			 NULL, 0) == 0)
			goto CONTINUE;
		}
	    if (ip_str_ptr)
		if (!reverse) {
		    if (regexec(&ip_reg, login_struc.ip, 0, NULL, 0) != 0)
			goto CONTINUE;
		} else {
		    if (regexec(&ip_reg, login_struc.ip, 0, NULL, 0) == 0)
			goto CONTINUE;
		}
	    if (port_str_ptr)
		if (!reverse) {
		    if (regexec(&port_reg, login_struc.port, 0, NULL, 0) !=
			0)
			goto CONTINUE;
		} else {
		    if (regexec(&port_reg, login_struc.port, 0, NULL, 0) ==
			0)
			goto CONTINUE;
		}
	    if (indicator == 1) {
		if (lindex) {
		    printf("user: \'%s\' ", login_struc.user);
		    printf("passwd: \'%s\' ", login_struc.passwd);
		    printf("rootpasswd: \'%s\' ", login_struc.rootpasswd);
		    printf("ip: %s ", login_struc.ip);
		    printf("port: %s\n", login_struc.port);
		    all_write_out(usfds[0], nextstr_ptr);
		    memset(loginconf, 0, LOGINCONF);
		    continue;
		} else {
		    execindex = 0;
		    if (sftpindex == 1) {
			execcmd[execindex] = "bsftp";
			execindex = (execindex + 1) % EXECCMD_NUM;
		    } else {
			execcmd[execindex] = "bssh";
			execindex = (execindex + 1) % EXECCMD_NUM;
			if (strlen(login_struc.rootpasswd) != 0) {
			    execcmd[execindex] = "-r";
			    execindex = (execindex + 1) % EXECCMD_NUM;
			    execcmd[execindex] = login_struc.rootpasswd;
			    execindex = (execindex + 1) % EXECCMD_NUM;
			}
		    }
		    execcmd[execindex] = "-p";
		    execindex = (execindex + 1) % EXECCMD_NUM;
		    execcmd[execindex] = login_struc.passwd;
		    execindex = (execindex + 1) % EXECCMD_NUM;
		    if (strlen(login_struc.port) != 0) {
			memset(portopt, 0, OPTOFPORTLEN);
			strcat(portopt, "-oPort=");
			strncat(portopt, login_struc.port,
				OPTOFPORTLEN - strlen("-oPort="));
			execcmd[execindex] = portopt;
			execindex = (execindex + 1) % EXECCMD_NUM;
		    }
		    if (zindex) {
			for (fcount = 0; fcount < zindex; fcount++) {
			    execcmd[execindex] = "-z";
			    execcmd[execindex + 1] = zstrptr[fcount];
			    execindex = (execindex + 2) % EXECCMD_NUM;
			}
		    }
		    execcmd[execindex] =
			userathost(login_struc.user, login_struc.ip,
				   userathost_array);
		    execindex = (execindex + 1) % EXECCMD_NUM;
		    for (fcount = optind; argv[fcount]; fcount++) {
			execcmd[execindex] = argv[fcount];
			execindex = (execindex + 1) % EXECCMD_NUM;
		    }
		    execcmd[execindex] = NULL;

		    fpid = fork();
		    if (fpid == -1)
			err_quit("fork failed", 1);
		    else if (!fpid)
			execv(cmdstr, execcmd);

		    waitpid(fpid, NULL, 0);
		  CONTINUE:
		    all_write_out(usfds[0], nextstr_ptr);
		    memset(loginconf, 0, LOGINCONF);
		    sleep(waittime);
		    continue;
		}
	    }
	}
	free(loginconf);
      EXIT:
	if (user_str_ptr)
	    regfree(&user_reg);
	if (passwd_str_ptr)
	    regfree(&passwd_reg);
	if (rootpasswd_str_ptr)
	    regfree(&rootpasswd_reg);
	if (ip_str_ptr)
	    regfree(&ip_reg);
	if (port_str_ptr)
	    regfree(&port_reg);
	exit(0);
    }
    return 0;
}

void err_quit(char *pstr, int quit_val)
{
    if (pstr != NULL)
	fprintf(stderr, "%s: %s\n", pstr, strerror(errno));
    exit(quit_val);
}

int err_return(char *pstr, int return_val)
{
    if (pstr != NULL)
	fprintf(stderr, "%s: %s\n", pstr, strerror(errno));
    return return_val;
}

int usage(void)
{
    printf("NAME\n");
    printf
	("    tt2 - one to two or more management platform for linux to linuxes\n\n");
    printf("SYNOPSIS\n");
    printf("    tt2 [ options ] argments\n\n");
    printf("DESCRIPTION\n");
    printf
	("\tArguments are commands that will be transfered to bssh or bsftp.\n");
    printf
	("\tNow tt2 support regular expressions to select hosts to excute commands.\n");
    printf("\t -c\tbssh or bsftp? default cmd is bssh\n");
    printf
	("\t -f\tpath of the config file. the default file is ~/.bssh_config\n");
    printf("\t -h\tdisplay this help and exit\n");
    printf
	("\t -l\tdisplay the contents of the configure file and the -z strings\n");
    printf("\t -p\tpath of the parser\n");
    printf("\t -z\tthe same option of bssh\n\n");
    printf("\t --waittime\twait how many seconds to work on the next server\n");
    printf("\t -e\tuse extended regular expressions\n");
    printf("\t -r\treverse tall regexes\' match results\n");
    printf
	("\t --ip\tis the host\'s addr matchs? All other such options include:\n");
    printf("\t     \t--port,--user,--passwd,--rootpasswd\n");
    printf("AUTHOR\n");
    printf("Writen by NingXibo.\n\n");
    printf("REPORTING BUGS\n");
    printf("Report bugs to <ningxibo@gmail.com>.\n");
    exit(1);
}

void ana(char *str1, char *str2, char *str3)
{
    int i, j;
    for (i = 0; str1[i] != '\n'; i++)
	str2[i] = str1[i];
    str2[i] = 0;
    j = 0;
    for (i++; str1[i] != '\n'; i++) {
	str3[j] = str1[i];
	j++;
    }
    str3[j] = 0;
}

int create(struct login_record *re, char *str)
{
    char in[32];
    char data[255];
    int i = 0;
    int j = 0;
    ana(str, in, data);
    i = atoi(in);
    j = strlen(data);
    switch (i) {
    case USER_RET:
	strncpy(re->user, data, j);
	re->user[j] = 0;
	return 0;
    case PASSWD_RET:
	strncpy(re->passwd, data, j);
	re->passwd[j] = 0;
	return 0;
    case ROOTPASSWD_RET:
	strncpy(re->rootpasswd, data, j);
	re->rootpasswd[j] = 0;
	return 0;
    case IP_RET:
	strncpy(re->ip, data, strlen(data));
	re->ip[j] = 0;
	return 1;
    case PORT_RET:
	strncpy(re->port, data, j);
	re->port[j] = 0;
	return 0;
    case END_RET:
	return 2;
    }
}

char *userathost(char *str1, char *str2, char *str3)
{
    int i, j;
    i = strlen(str1);
    j = strlen(str2);
    strncpy(str3, str1, i);
    strncpy(str3 + i, "@", 1);
    strncpy(str3 + i + 1, str2, j);
    str3[i + j + 1] = 0;
    return str3;
}

int all_write_out(int fds, char *str)
{
    int i = 0;
    i = write(fds, str, strlen(str));
    if (i < strlen(str))
	if (i < 0)
	    return all_write_out(fds, str);
	else
	    return all_write_out(fds, str + i);
    return i;
}
