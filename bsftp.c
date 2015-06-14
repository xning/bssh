#include "config.h"
int main(int argc, char **argv)
{
    int pid, master, count, fcount, record[2];
    char *slavestr;
    int i;
/* for options */
    int opt;
/*
 * h for help, o for ssh -o option, p for non-super-user passwod
 * r for root password used in su command, l for list options, cmd and
 * auguments, etc.  s for slient operation.
 */
    char *optstr = "+:ho:p:ls";
    char *ooptstrptr[OOPTSTRPTRNUM];
    int ooptc = 1;
    char *cmd[CMDNUM];
    char *optstrptr[OPTSTEPTRNUM];
    int optc = 0;
    int rdstdin = 0;
    char *rdstdinbuf;
    FILE *stdinfp;
    int oindex, pindex, sindex, lindex;
    int output = 1;
/* child process's varibles */
    int slave;
    fd_set rfds, wfds;
    struct winsize size;
    struct termios term;
    struct termios *termptr = NULL;
/* end of child process's varibles */
    pindex = sindex = lindex = -1;
    oindex = 0;
    memset(&term, 0, sizeof(struct termios));
    if (isatty(1)) {
	if (tcgetattr(1, &term) == 0)
	    termptr = (&term);
	else
	    termptr = NULL;
    }
    if ((master = posix_openpt(O_RDWR | O_NOCTTY)) == -1
	|| grantpt(master) == -1 || unlockpt(master) == -1)
	err_quit("open pty failed", 1);
    slavestr = ptsname(master);
    if (slavestr == NULL)
	err_quit("ptsname failed", 1);
    if ((pid = fork()) == -1) {
	err_quit("fork failed", 1);
/* child process */
    } else if (!pid) {
	if (setsid() == -1)
	    err_quit(NULL, 1);
	slave = open(slavestr, O_RDWR);
	if (slave == -1)
	    err_quit(NULL, 1);
	if (termptr != NULL) {
	    term.c_cflag &= ~CSIZE;
	    term.c_cflag |= CS8;
	    term.c_oflag |= OCRNL;
	    if (tcsetattr(slave, TCSANOW, termptr) < 0)
		err_quit(NULL, 1);
	} else {
	    if (tcgetattr(slave, &term) != 0)
		exit(1);
	    term.c_cflag &= ~CSIZE;
	    term.c_cflag |= CS8;
	    term.c_oflag |= OCRNL;
	    if (tcsetattr(slave, TCSANOW, &term) < 0)
		err_quit(NULL, 1);
	}
	size.ws_row = WINROW;
	size.ws_col = WINCOL;
	if (ioctl(slave, TIOCSWINSZ, &size) < 0)
	    err_quit(NULL, 1);
	if (dup2(slave, 0) == -1 || dup2(slave, 1) == -1
	    || dup2(slave, 2) == -1 || close(slave))
	    err_quit(NULL, 1);
	if (execl(bashstr, "-bash", "--login", "-i", NULL) == -1)
	    err_quit("exec bash failed", 1);
	return 0;

/* parent process */
    } else {
/* process options */
	for (opt = 0; argv[opt]; opt++) {
	    if (strcmp(argv[opt], "-") == 0) {
		rdstdin = 1;
		for (count = opt; count + 1 < argc; count++) {
		    argv[count] = argv[count + 1];
		}
		argc--;
		break;
	    }
	}
	while ((opt = getopt(argc, argv, optstr)) != -1) {
	    switch (opt) {
	    case 'h':
		bsftpusage();
		exit(-1);
	    case 'o':
		ooptstrptr[2 * oindex + 0] = "-o";
		ooptstrptr[2 * oindex + 1] = optarg;
		oindex = (oindex + 1) % (OOPTSTRPTRNUM / 2);
		ooptc = 2 * oindex;
		break;
	    case 'p':
		pindex = optc;
		optstrptr[optc] = optarg;
		optc = (optc + 1) % OPTSTEPTRNUM;
		break;
	    case 'l':
		lindex = 1;
		optc = (optc + 1) % OPTSTEPTRNUM;
		break;
	    case 's':
		sindex = 1;
		output = -1;
		break;
	    case ':':
		exit(1);
	    case '?':
		exit(1);
	    }
	}
	if (optind > argc - 1 || argc <= 3) {
	    printf("\n\tplease try -h option\n\n");
	    exit(1);
	}
	if (pindex == -1) {
	    printf("no password\n");
	    exit(1);
	}

/* prepare ssh login cmd */
	fcount = 0;
	cmd[0] = "sftp -o ConnectionAttempts=2 -o ConnectTimeout=15 \
-o ServerAliveInterval=5  -o ServerAliveCountMax=30000 \
-o PreferredAuthentications=password -oCheckHostIP=no";
	if (ooptc != 1) {
	    for (fcount = 0; fcount < oindex; fcount++) {
		cmd[4 * fcount + 1] = " ";
		cmd[4 * fcount + 2] = ooptstrptr[2 * fcount];
		cmd[4 * fcount + 3] = " ";
		cmd[4 * fcount + 4] = ooptstrptr[2 * fcount + 1];
	    }
	}
	cmd[4 * fcount + 1] = " ";
	cmd[4 * fcount + 2] = argv[optind];
	cmd[4 * fcount + 3] = NULL;
/* process -l options */
	if (lindex == 1) {
	    printf("password: \"%s\"\n", optstrptr[pindex]);
	    if (sindex == 1)
		printf("silent operation: enable\n");
	    for (count = 0; cmd[count]; count++)
		printf("%s", cmd[count]);
	    printf("\n");
	    if (optind < argc - 1) {
		printf
		    ("Execute the following commands in remote shell:\n");
		for (count = optind + 1; count < argc; count++) {
		    printf("\t\"%s\"\n", argv[count]);
		}
	    }
	    if (rdstdin)
		printf("Find - option, so get command from stdin also\n");
	    exit(1);
	}

/* setup shell prompt */
	shpmptstrptr[0] = "]$ ";
	shpmptstrptr[1] = "]# ";
	shpmptstrptr[2] = NULL;
/* get local shell and setup enviroment */
        for (i=0;i<TRY_ALLOC_MEM;i++) {
          if (getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL) == 0)
            break;
          if (i==(TRY_ALLOC_MEM-1)) {
            fprintf(stderr, "[+] Alloct memories fail, exit now.");
            exit(1);
          }
        }
	w2pts(master, &wfds, NULL, NULL,
	      "export LANG=en_US.UTF-8;PATH=$PATH:/sbin:/usr/sbin");
/* try to get remote shell */
        for (i=0;i<TRY_ALLOC_MEM;i++) {
          if (getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL) == 0)
            break;
          if (i==(TRY_ALLOC_MEM-1)) {
            fprintf(stderr, "[+] Alloct memories fail, exit now.");
            exit(1);
          }
        }
        /*	getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL); */
	wtopts(master, &wfds, NULL, NULL, cmd);
	sshlogin(master, output, optstrptr[pindex], &rfds, &wfds, record);
	shpmptstrptr[0] = "sftp> ";
	shpmptstrptr[1] = NULL;

/* setup remomote enviroment */
        for (i=0;i<TRY_ALLOC_MEM;i++) {
          if (getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL) == 0)
            break;
          if (i==(TRY_ALLOC_MEM-1)) {
            fprintf(stderr, "[+] Alloct memories fail, exit now.");
            exit(1);
          }
        }
/*	getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL); */
/* begin to exec commands */
	for (count = optind + 1; count < argc; count++) {
	    w2pts(master, &wfds, NULL, NULL, argv[count]);
            for (i=0;i<TRY_ALLOC_MEM;i++) {
              if (getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL) == 0)
                break;
              if (i==(TRY_ALLOC_MEM-1)) {
                fprintf(stderr, "[+] Alloct memories fail, exit now.");
                exit(1);
              }
            }
        /*	    getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL); */
	}

	if (rdstdin) {
	    stdinfp = fdopen(0, "rb");
	    if (stdinfp == NULL)
		err_quit("get FILE structure failed", 1);
	    rdstdinbuf = malloc(RDSTDINBUFSIZE * sizeof(*rdstdinbuf));
	    if (rdstdinbuf == NULL)
		err_quit("malloc buffer for read stdin failed", 1);
	    while (fgets(rdstdinbuf, RDSTDINBUFSIZE, stdinfp) != NULL) {
		count = strlen(rdstdinbuf);
		rdstdinbuf[count - 1] = 0;
		w2pts(master, &wfds, NULL, NULL, rdstdinbuf);
                for (i=0;i<TRY_ALLOC_MEM;i++) {
                  if (getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL) == 0)
                    break;
                  if (i==(TRY_ALLOC_MEM-1)) {
                    fprintf(stderr, "[+] Alloct memories fail, exit now.");
                    exit(1);
                  }
                }
                /*		getshpmpt(master, output, shpmptstrptr, &rfds, NULL, NULL); */
		fflush(stdinfp);
	    }
	    free(rdstdinbuf);
	}
/* resetup shell prompt */
	shpmptstrptr[0] = "$ ";
	shpmptstrptr[1] = "# ";
	shpmptstrptr[2] = NULL;
/* exit and clean */
	w2pts(master, &wfds, NULL, NULL, "exit");
	printf("\n");

	sync();
	if (close(master) == -1)
	    err_quit("close or free failed", 1);
	return 0;
    }
    return 0;
}

