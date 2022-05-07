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

char *argv0, *startdir, *outfilepath;
int *N;
FILE *outfile;

void
usage(void)
{
	fprintf(stderr, "usage: %s <src_dir> <dst_dir>\n",
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

		if(isdir(buf)){
			while(!*N)
				;
			if(!fork()){
				--*N;
				processdir(buf) ;
				++*N;
				exit(0);
			}
		} else {
			siz = sizeoffile(buf) ;
			if(siz > msiz){
				msiz = siz ;
				if(bigfilep)
					free(bigfilep);
				bigfilep = strdup(de->d_name) ;
			}

			n += siz ;
			++i;
			printf("%d\t%s\t%s\t%d\t%d\n", getpid(), buf, de->d_name, siz, i);
		}
	}
	closedir(d);

	printf("%s\t%d\t%d\t%s\n",
		dp, i, n, bigfilep);
	fprintf(outfile,
		"%s\t%d\t%d\t%s\n",
		dp, i, n, bigfilep);

	return n;
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
	outfilepath = argv[2] ;

	signal(SIGINT, sigint);

	N = mmap(NULL, sizeof(*N),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	printf("Enter N: "); scanf("%d", N);

	if(!( outfile = fopen(outfilepath, "w") )){
		perror("fopen");
		exit(1);
	}

	processdir(startdir);

	while(wait(&dummy) > 0)
		;
	
	return 0 ;
}
