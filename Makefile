# gcc options for gcov: -fprofile-acrs -ftest-coverage
ssh:lib
	@gcc -ggdb -L. -Wall -Wstrict-prototypes -fomit-frame-pointer\
 -o bssh bssh.c -lbssh
debugssh:lib
	@gcc -ggdb -DDEBUG -L. -o bssh bssh.c -lbssh
sftp:lib
	@gcc -ggdb -L. -Wall -Wstrict-prototypes -fomit-frame-pointer\
 -o bsftp bsftp.c -lbssh
debugsftp:lib
	@gcc -ggdb -DDEBUG -L. -o bsftp bsftp.c -lbssh
final:lib parser
	@gcc -O3 -L. -o bssh bssh.c -lbssh
	@gcc -O3 -L. -o bsftp bsftp.c -lbssh
	@gcc -O3 -o tt2 tt2.c
clean:
	@rm -f *~ *.o *.out *.a
Makefile:;
lib: lib.o ;
lib.o:lib.c
	@gcc -c $^
	@ar -cr libbssh.a $@
sshpg:libpg
	@gcc -pg -ggdb -L. -o bssh bssh.c -lbssh
sftppg:libpg
	@@gcc -pg -ggdb -L. -o bsftp bsftp.c -lbssh
libpg:
	@@gcc -pg -c lib.c
	@ar -cr libbssh.a lib.o
parser:parser.h parser.lex
	@flex parser.lex
	@gcc -O3 -o parser lex.yy.c -lfl
	@rm -f lex.yy.c
tt2:tt2.c
	@gcc -o tt2 tt2.c
pack:
	@tar cfj expect.tar.bz2 *.c *.h *.lex Makefile README
