#include "config.h"
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

/* mfds: monitor file descriptor  */
int mfds(int fds, fd_set * readfds, fd_set * writefds,
	 fd_set * exceptfds, struct timespec *timeout, sigset_t * sigmask)
{
    if (readfds != NULL) {
	FD_ZERO(readfds);
	FD_SET(fds, readfds);
    }
    if (writefds != NULL) {
	FD_ZERO(writefds);
	FD_SET(fds, writefds);
    }
    if (exceptfds != NULL) {
	FD_ZERO(exceptfds);
	FD_SET(fds, exceptfds);
    }
    return pselect((int) (fds + 1), readfds, writefds, exceptfds, timeout,
		   sigmask);
}

/* mfdsr: mfds for read operation */
int mfdsr(int fds, fd_set * readfds, struct timespec *timeout,
	  sigset_t * sigmask)
{
    return mfds(fds, readfds, NULL, NULL, timeout, sigmask);
}

/* mfdsw: mfds for write operation */
int mfdsw(int fds, fd_set * writefds, struct timespec *timeout,
	  sigset_t * sigmask)
{
    return mfds(fds, NULL, writefds, NULL, timeout, sigmask);
}

/* mfdse: mfds for except operation */
int mfdse(int fds, fd_set * exceptfds, struct timespec *timeout,
	  sigset_t * sigmask)
{
    return mfds(fds, NULL, NULL, exceptfds, timeout, sigmask);
}

void jointstr(char *strbuf1, char *strbuf2, char *pmptstr, char *buf)
{
    int len1, len2, size;
    if (strbuf1 != NULL)
	len1 = strlen(strbuf1);
    else
	len1 = 0;
    if (strbuf2 != NULL)
	len2 = strlen(strbuf2);
    else
	len2 = 0;
    if (pmptstr != NULL)
	size = strlen(pmptstr);
    else
	size = 0;
    memset(buf, 0, size + 1);
    strncpy(buf, strbuf2 + len2 - 1 - (size - len1 - 1), size - len1);
    strncpy(buf + (size - len1) - 1 + 1, strbuf1, len1);
}

/* chkp: check what prompt we get, then do something */
int chkp(char *strbuf1, char *strbuf2, struct struct_pmpt **pp,
	 int record[2])
{
    int len, len1, len2, size, fcount, scount, tcount;
    char *buf = NULL;
    char *strbuf = NULL;
    if (strbuf1 != "" && strbuf1 != NULL)
	len1 = strlen(strbuf1);
    else
	len1 = 0;
    if (strbuf2 != "" && strbuf2 != NULL)
	len2 = strlen(strbuf2);
    else
	len2 = 0;
    if (pp[0]->strptr[0] == "" || pp[0]->strptr[0] == NULL) {
	fprintf(stderr, "prompt can not be empty\n");
	return -1;
    }
    if (len1 + len2 == 0)
	return 0;
    strbuf = malloc((len1 + len2 + 1) * sizeof(*strbuf));
    if (strbuf == NULL)
	err_quit("malloc strbuf for chkp failed", 1);
    memset(strbuf, 0, len1 + len2 + 1);
    if (len1 > 0 && len2 > 0) {
	strncpy(strbuf, strbuf2, len2);
	strncpy(strbuf + len2, strbuf1, len1);
    } else {
	if (len1 > 0)
	    strncpy(strbuf, strbuf1, len1);
	if (len2 > 0)
	    strncpy(strbuf, strbuf2, len2);
    }
    len = strlen(strbuf);
    for (fcount = 0; pp[fcount]; fcount++) {
	for (scount = 0; pp[fcount]->strptr[scount]; scount++) {
	    size = strlen(pp[fcount]->strptr[scount]);
	    if (len < size)
		continue;
	    else
		for (tcount = len; tcount >= size; tcount--) {
		    if (strncmp
			(strbuf + tcount - 1 - (size - 1),
			 pp[fcount]->strptr[scount], size) == 0) {
			pp[fcount]->index = scount;
			if (record != NULL) {
			    record[0] = fcount;
			    record[1] = scount;
			}
			free(strbuf);
			strbuf = NULL;
#ifdef DEBUG
			printf("find prompt: \"%s\"\n",
			       pp[fcount]->strptr[scount]);
#endif
			return 1;
		    } else {
			continue;
		    }
		}
	}			/* scount */
    }				/* fcount  */
    free(strbuf);
    strbuf = NULL;
    return 0;
}

void echoall(char *strptr, int fds)
{
    int strsize = strlen(strptr);
    int wret, wnum;
    wret = wnum = 0;
    if (fds >= 0) {
	for (;;) {
	    if (wnum == strsize)
		break;
	    else {
		wret = write(fds, strptr + wnum, strsize - wnum);
		if (wret >= 0) {
		    wnum += wret;
		    sync();
		}
	    }
	}
    }
}

/* rdfpts: read from pts  */
int rdfpts(int fds, int output, struct struct_pmpt **pmptptr,
	   fd_set * readfds, struct timespec *timeout, sigset_t * sigmask,
	   int record[2])
{
    int i, strsize;
    char *str[2];
    i = strsize = 0;
#ifdef DEBUG
    int fcount, scount;
#endif
    str[0] = malloc(BUFSIZE * sizeof(str[0]));
    if (str[0] == NULL)
	return -1;
    memset(str[0], 0, BUFSIZE);
    str[1] = malloc(BUFSIZE * sizeof(str[1]));
    if (str[1] == NULL) {
	free(str[0]);
	str[0] = NULL;
	return -1;
    }
    memset(str[1], 0, BUFSIZE);
    for (;;) {
	if (mfdsr(fds, readfds, timeout, sigmask) > 0) {
	    if (strsize < BUFSIZE - 1) {
		if (read(fds, str[i] + strsize, BUFSIZE - 1 - strsize) <=
		    0) {
		    continue;
		}
		strsize = strlen(str[i]);
#ifdef DEBUG
		printf("str[%d]: \"%s\"\n", i, str[i]);
		printf("str[%d]: \"%s\"\n", 1 - i, str[1 - i]);
		printf("promptstr:\n");
		for (fcount = 0; pmptptr[fcount]; fcount++) {
		    for (scount = 0; pmptptr[fcount]->strptr[scount];
			 scount++) {
			printf("\"%s\"\n",
			       pmptptr[fcount]->strptr[scount]);
		    }
		}
		sync();
#endif
		if (chkp(str[i], str[1 - i], pmptptr, record)) {
		    echoall(str[i], output);
		    memset(str[i], 0, BUFSIZE);
		    memset(str[1 - i], 0, BUFSIZE);
		    break;
		}
	    } else {
		echoall(str[i], output);
		i = (++i) % 2;
		memset(str[i], 0, BUFSIZE);
		strsize = 0;
	    }			/* if (strsize < BUFSIZE - 1)  */
	} else {
	    continue;
	}			/* if (mfdsr(fds, readfds, timeout, sigmask) > 0)  */
    }				/* for(;;) */
    free(str[0]);
    str[0] = NULL;
    free(str[1]);
    str[1] = NULL;
    return 0;
}

/* getshpmpt: get shell prompt */
int getshpmpt(int fds, int output, char *pmptstrarr[], fd_set * readfds,
	      struct timespec *timeout, sigset_t * sigmask)
{
    struct struct_pmpt pp;
    struct struct_pmpt *pmptptr[2];
    int fcount;
    for (fcount = 0; pmptstrarr[fcount]; fcount++) {
	pp.strptr[fcount] = pmptstrarr[fcount];
    }
    pp.strptr[fcount] = NULL;
    pmptptr[0] = (&pp);
    pmptptr[1] = NULL;
    return rdfpts(fds, output, pmptptr, readfds, timeout, sigmask, NULL);
}

/* r4pts: read from pts  */
int r4pts(int fds, int output, char *promptstr, fd_set * readfds,
	  struct timespec *timeout, sigset_t * sigmask)
{
    char *tmpstrptr[2];
    tmpstrptr[0] = promptstr;
    tmpstrptr[1] = NULL;
    return getshpmpt(fds, output, tmpstrptr, readfds, timeout, sigmask);
}

int sshlogin(int fds, int output, char *passwd, fd_set * readfds,
	     fd_set * writefds, int record[2])
{
    struct struct_pmpt *pmpt_ptr[3];
    struct struct_pmpt pmpt[2];
    record[0] = record[1] = -1;
    pmpt[0].strptr[0] = "(yes/no)? ";
    pmpt[0].strptr[1] = "$ ";
    pmpt[0].strptr[2] = "# ";
    pmpt[0].strptr[3] = NULL;
    pmpt_ptr[0] = (&pmpt[0]);
    pmpt[1].strptr[0] = "assword: ";
    pmpt[1].strptr[1] = NULL;
    pmpt_ptr[1] = (&pmpt[1]);
    pmpt_ptr[2] = NULL;
    rdfpts(fds, output, pmpt_ptr, readfds, NULL, NULL, record);
    if (record[0] == 0) {
	if (record[1] == 0) {
	    w2pts(fds, writefds, NULL, NULL, "yes");
	    r4pts(fds, output, "assword: ", readfds, NULL, NULL);
	}
	if (record[1] == 1 || record[1] == 2) {
	    printf("get remote shell failed\n");
	    exit(-1);
	}
    }
    return w2pts(fds, writefds, NULL, NULL, passwd);
}

/* w2pts: write to pts */
int w2pts(int fds, fd_set * writefds, struct timespec *timeout,
	  sigset_t * sigmask, char *cmd)
{
    int wnum = 0;
    int wret = 0;
    int strsize = strlen(cmd);
    char *strbuf = malloc((strsize + 1) * sizeof(*strbuf));
    strcpy(strbuf, cmd);
    strbuf[strsize] = '\r';
    for (;;) {
	if (wnum == strsize + 1)
	    break;
	else {
	    if (mfdsw(fds, writefds, timeout, sigmask) > 0) {
		if ((wret =
		     write(fds, strbuf + wnum, strsize + 1 - wnum)) >= 0) {
		    wnum += wret;
		}
	    }
	}
    }
    sync();
    free(strbuf);
    strbuf = NULL;
    return 0;
}

/* wtopts: write to pts */
int wtopts(int fds, fd_set * writefds, struct timespec *timeout,
	   sigset_t * sigmask, char **cmdptr)
{
    char *buf;
    int index = 0;
    int totallen, len, fcount, val;
    totallen = len = fcount = 0;
    for (fcount = 0; cmdptr[fcount]; fcount++) {
	len = strlen(cmdptr[fcount]);
	if (len > 0)
	    totallen += len;
    }
    buf = malloc((totallen + 1) * sizeof(char));
    if (buf == NULL)
	err_quit("malloc failed", 1);
    memset(buf, 0, totallen + 1);
    for (fcount = 0; cmdptr[fcount]; fcount++) {
	len = strlen(cmdptr[fcount]);
	if (len > 0) {
	    strcpy(buf + index, cmdptr[fcount]);
	    index = strlen(buf);
	}
    }
    buf[totallen] = 0;
    val = w2pts(fds, writefds, timeout, sigmask, buf);
    free(buf);
    buf = NULL;
    return val;
}


void bsshusage(void)
{
    printf("NAME\n");
    printf("    bssh - batch ssh client\n\n");
    printf("SYNOPSIS\n");
    printf
	("    bssh [-l] -p passwd [-r rootpasswd] [-o options] [user@]hostname [cmd]\n\n");
    printf("DESCRIPTION\n");
    printf
	("    bssh (batch ssh client) is a program that let you to use ssh in the batch mode.\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf
	("\t-l\tcheck options and arguments and display them, then quit\n");
    printf("\t-p\tssh login password\n");
    printf("\t-r\troot password used in su command\n");
    printf
	("\t-s\tsilent operation; do not print the commands as they are executed\n");
    printf
	("\t-o\tall ssh -o options except here, please read ssh_config(5)\n");
    printf("\t-z\tsupplement more prompts used in execute commands\n\n");
    printf("AUTHOR\n");
    printf("Version 1.3, writen by NingXibo.\n\n");
    printf("REPORTING BUGS\n");
    printf("Report bugs to <ningxibo@gmail.com>.\n");
}

void bsftpusage(void)
{
    printf("NAME\n");
    printf("    bsftp - batch sftp client\n\n");
    printf("SYNOPSIS\n");
    printf
	("    bsftp [-h] [-l] -p passwd [-s] [-o options] [user@]hostname [cmd]\n\n");
    printf("DESCRIPTION\n");
    printf
	("    bsftp (batch ssh client) is a program that let you to use sftp in the batch mode.\n");
    printf("\t-h\tdisplay this help and exit\n");
    printf
	("\t-l\tcheck options and arguments and display them, then quit\n");
    printf("\t-p\tssh login password\n");
    printf
	("\t-s\tsilent operation; do not print the commands as they are executed\n");
    printf
	("\t-o\tall sftp -o options except here, please read ssh_config(5)\n\n");
    printf("AUTHOR\n");
    printf("Version 1.3, riten by NingXibo.\n\n");
    printf("REPORTING BUGS\n");
    printf("Report bugs to <ningxibo@gmail.com>.\n");
}
