#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <time.h>

char *argv0, *startdir, *filename;
int *N, dirn=0, filen=0;

void
usage(void)
{
	fprintf(stderr, "usage: %s <filename> <dirname>\n",
		argv0);
	exit(1);
}

int
isdir(char *p)
{
	struct stat st;
	lstat(p, &st);
	return S_ISDIR(st.st_mode) ;
}

int
sizeoffile(char *p)
{
	struct stat st;
	lstat(p, &st);
	return st.st_size ;
}

int
isskippable(char *p)
{
	return !strcmp("..", p) || !strcmp(".", p) ;
}

void
lookfile(char *p, char *name, int *n)
{
	struct stat st;
	lstat(p, &st) ;
	++*n;
	printf("%d %s %s %d %d\n",
		getpid(), p, name, st.st_size, *n);
}

void
findfile(char *p, char *name)
{
	char buf[256];
	struct stat st;
	lstat(p, &st);
	strcpy(buf, ctime(&st.st_mtime));
	buf[strlen(buf)-1] = 0 ;
	printf("%s %s %d %s %04o %d\n",
		p, name, st.st_size,
		buf, 0777 & st.st_mode,
		st.st_ino);
}

int
processdir(char *dirpath)
{
	char buf[512];
	char dp[sizeof(buf)];
	char *bigfilep=0;
	int i=0, n=0, siz, msiz=0;
	DIR *d;
	struct dirent *de;

	realpath(dirpath, dp);
	d = opendir(dp) ;
	while(de = readdir(d)){
		if(isskippable(de->d_name))
			continue;

		strcpy(buf, dp);
		strcat(buf, "/");
		strcat(buf, de->d_name);

		if(!strcmp(filename, de->d_name)){
			findfile(buf, de->d_name);
			continue;
		}

		if(isdir(buf)){
			++dirn;
			while(!*N)
				;
			if(!fork()){
				--*N;
				processdir(buf) ;
				++*N;
				exit(0);
			}
		} else {
			++filen;
			lookfile(buf, de->d_name, &n);
		}
	}
	closedir(d);

	return 0;
}

char *skipprefix(char *s, char *pref) {
	while(*s && *pref && *s == *pref)
		++s, ++pref;
	return s ;
}

void sigint(int signum) {
	exit(1);
}

int main(int argc, char *argv[]) {
	int n, dummy;

	argv0 = argv[0] ;
	if(argc != 3)
		usage();

	prctl(PR_SET_PDEATHSIG, SIGTERM);
	
	startdir = argv[1] ;
	filename = argv[2] ;

	signal(SIGINT, sigint);

	N = mmap(NULL, sizeof(*N),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	printf("Enter N: "); scanf("%d", N);

	puts("yep");
	processdir(startdir);

	while(wait(&dummy) > 0)
		;

	printf("%d %d\n", dirn, filen);
	
	return 0 ;
}
