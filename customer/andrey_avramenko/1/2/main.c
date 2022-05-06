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

char *argv0, *srcdirpath, *dstdirpath;
char **filepaths;
int *N, cmptype;
int (*cmpfn)(const void *, const void *);

void
usage(void)
{
	fprintf(stderr, "usage: %s <src_dir> <sort_type> <dst_dir>\n",
		argv0);
	exit(1);
}

int
cmpbyname(const void *v1, const void *v2)
{
	const char *s1 = *(char **)v1 ;
	const char *s2 = *(char **)v2 ;
	return strcmp(s1, s2) ;
}

int
cmpbysize(const void *v1, const void *v2)
{
	int e;
	struct stat st1, st2;
	const char *s1 = *(char **)v1 ;
	const char *s2 = *(char **)v2 ;

	if(e=lstat(s1, &st1)){
		perror("lstat");
		return 0 ;
	}

	if(lstat(s2, &st2)){
		perror("lstat");
		return 0 ;
	}

	return st1.st_size - st2.st_size ;
}

void
mkdirp(char *p, int mode)
{
	char buf[BUFSIZ];
	char *pp = p ;
	while(*p){
		if(*p == '/'){
			memcpy(buf, pp, p - pp + 1);
			buf[p - pp + 1] = 0 ;
			mkdir(buf, mode);
		}
		++p;
	}
	mkdir(pp, mode);
}

void
copyfile(char *src, char *dst)
{
	int nr, nw, srcfd, dstfd;
	char buf[BUFSIZ];
	char *dn, *d;

	srcfd = open(src, O_RDONLY) ;

	d = strdup(dst) ;
	dn = dirname(d) ;
	mkdirp(dn, 0755);

	dstfd = open(dst, O_WRONLY | O_CREAT) ;
	if(dstfd < 0)
		perror("open");
	fchmod(dstfd, 0644);

	do{
		nr = read(srcfd, buf, sizeof(buf)) ;
		nw = write(dstfd, buf, nr);
		printf("%d %s %s %d\n", getpid(),src, dst, nw);
	}while(nr == sizeof(buf));

	close(srcfd);
	close(dstfd);
}

int
isdir(char *p)
{
	struct stat st;
	lstat(p, &st);
	return S_ISDIR(st.st_mode) ;
}

int
isskippable(char *p)
{
	return !strcmp("..", p) || !strcmp(".", p) ;
}

int
countfiles(char *dirpath)
{
	char buf[512];
	int i;
	DIR *d;
	struct dirent *de;

	realpath(dirpath, buf);
	d = opendir(buf) ;
	i = 0 ;
	while(de = readdir(d)){
	
		if(isskippable(de->d_name))
			continue;

		realpath(dirpath, buf);
		strcat(buf, "/");
		strcat(buf, de->d_name);
		if(isdir(buf))
			i += countfiles(buf) ;
		else
			++i;
	}
	closedir(d);

	return i ;
}

int
getallfiles(char *dirpath, char **dst)
{
	char buf[512];
	int i, n;
	DIR *d;
	struct dirent *de;

	realpath(dirpath, buf);
	d = opendir(buf) ;
	i = 0 ;
	while(de = readdir(d)){
		if(isskippable(de->d_name))
			continue;

		realpath(dirpath, buf);
		strcat(buf, "/");
		strcat(buf, de->d_name);

		if(isdir(buf)){
			i += getallfiles(buf, dst+i) ;
		} else {
			dst[i] = strdup(buf) ;
			++i;
		}
	}
	closedir(d);

	return i;
}

char *
skipprefix(char *s, char *pref)
{
	while(*s && *pref && *s == *pref)
		++s, ++pref;
	return s ;
}

void
sigint(int signum)
{
	exit(1);
}

int
main(int argc, char *argv[])
{
	char pref1[BUFSIZ], pref2[BUFSIZ], buf[BUFSIZ];
	int n, dummy;

	argv0 = argv[0] ;
	if(argc != 4)
		usage();

	signal(SIGINT, sigint);

	srcdirpath = argv[1] ;
	
	if(!strcmp(argv[2], "0"))
		cmptype = 0 ;
	else if( !(cmptype = atoi(argv[2])) )
		usage();

	dstdirpath = argv[3] ;
	
	switch(cmptype){
	case 1 :
		cmpfn = cmpbysize ;
	break;
	case 2 :
		cmpfn = cmpbyname ;
	break;
	default:
		usage();
	}
	realpath(srcdirpath, pref1);
	realpath(dstdirpath, pref2);

	printf("Enter N-thread value: ");
	
	N = mmap(NULL, sizeof(*N),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	scanf("%d", N);

	n = countfiles(srcdirpath);

	filepaths = malloc(n * sizeof(*filepaths)) ;

	n = getallfiles(srcdirpath, filepaths);

	qsort(filepaths, n, sizeof(*filepaths), cmpfn);

	if(mkdir(dstdirpath, 0755) < 0){
		switch(errno){
		case EEXIST :
			/* Nothing. Just ignore it. */
			break;
		case EACCES :
			fprintf(stderr, "%s: Access denied\n", argv0);
			break;
		default:
			fprintf(stderr, "%s: Unknown (unhandled) error\n", argv0);
			exit(1);
		}
	}

	for(int i=0 ; i<n ; ++i){
		strcpy(buf, pref2);
		strcat(buf, skipprefix(filepaths[i], pref1));
		--*N;
		if(!fork()){
			prctl(PR_SET_PDEATHSIG, SIGTERM);
			copyfile(filepaths[i], buf);
			++*N;
			exit(0);
		}
		if(!*N)
			wait(&dummy);
	}

	while(wait(&dummy) > 0)
		;
	
	return 0 ;
}
