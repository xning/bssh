#ifndef _EXPECT_TT_CONFIG_H
#define _EXPECT_TT_CONFIG_H 1
#define _XOPEN_SOURCE 600
#include <termios.h>
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pty.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
/* #include <getopt.h> */

#define CMDNUM 1024
#define BUFSIZE 1024
#define MAX_PMPTSTR_NUM 16
#define OOPTSTRPTRNUM 64
#define OPTSTEPTRNUM 64
#define RDSTDINBUFSIZE 64
#define SHPMPTSTRPTRNUM 512
#define CMDPMPTSTRPTRNUM 256
#define WINROW 24
#define WINCOL 80
#define bashstr  "/bin/bash"
#define SHPMPT0 "$ "
#define SHPMPT1 "# "
#define TRY_ALLOC_MEM 20
/* #define DEBUG 1 */
char *shpmptstrptr[SHPMPTSTRPTRNUM];
char *cmdpmptstrptr[CMDPMPTSTRPTRNUM];
typedef void *(*fp) (void *);
struct struct_pmpt {
    int index;
    char *strptr[MAX_PMPTSTR_NUM];
};

void err_quit(char *str, int quit_val);
int err_return(char *pstr, int return_val);
int mfds(int fds, fd_set * readfds, fd_set * writefds,
	 fd_set * exceptfds, struct timespec *timeout, sigset_t * sigmask);
int mfdsr(int fds, fd_set * readfds, struct timespec *timeout,
	  sigset_t * sigmask);
int mfdsw(int fds, fd_set * writefds, struct timespec *timeout,
	  sigset_t * sigmask);
int mfdse(int fds, fd_set * exceptfds, struct timespec *timeout,
	  sigset_t * sigmask);
int chkp(char *strbuf1, char *strbuf2, struct struct_pmpt **pp,
	 int record[2]);
int rdfpts(int fds, int output, struct struct_pmpt **pmptptr,
	   fd_set * readfds, struct timespec *timeout, sigset_t * sigmask,
	   int record[2]);
/* int isp(char *strbuf1, char *strbuf2, char *pp); */

int r4pts(int fds, int output, char *pmpt_str, fd_set * readfds,
	  struct timespec *timeout, sigset_t * sigmask);
int wtopts(int fds, fd_set * writefds, struct timespec *timeout,
	   sigset_t * sigmask, char **cmdptr);
int w2pts(int fds, fd_set * writefds, struct timespec *timeout,
	  sigset_t * sigmask, char *cmd);
int getshpmpt(int fds, int output, char *pmptstrarr[], fd_set * readfds,
	      struct timespec *timeout, sigset_t * sigmask);
int sshlogin(int fds, int output, char *passwd, fd_set * readfds,
	     fd_set * writefds, int record[2]);

void bsshusage(void);
void bsftpusage(void);
#endif				/* _EXPECT_TT_CONFIG_H */
