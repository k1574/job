#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <fcntl.h>

enum{
	StrSiz = 50,
	StrNum = 75,
	MustBeWrote = 1000,
	Proc1 = 1,
	Proc2 = 2,
	MainProc = 3,
	Finish = 4,
};

int *s, *p1s, *p2s;
int dummy, finish;
char *argv0;
int pid1, pid2;
int *canread;
int *wrote, *wroteonce;
int *done;

char *genbufs[StrNum];

void proc(int signum, int pn);

void
usage(void)
{
	fprintf(stderr, "usage: %s\n",
		argv0);
	exit(1);
}

void
ndprintf(int fd, char *fmt, ...)
{
        va_list ap;
        char buf[256];
        int n;

        va_start(ap, fmt);
        n = vsprintf(buf, fmt, ap);
        va_end(ap);
        write(fd, buf, n);
}

long
gettimeinms(void)
{
	long ms;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	ms = round(spec.tv_nsec / 1.0e3);
	return ms;
}

void
swait(int *sem, int val)
{
	ndprintf(1, "");
	while(*sem)
		;
	*sem = val ;
}

void
stake(int *sem, int val)
{
	while(*sem != val)
		;
}

void
slet(int *sem, int val)
{
	int prevval = *sem ;
	*sem = val ;
	swait(sem, prevval);
}

void
spost(int *sem)
{
	*sem = 0 ;
}

void
proc1(int signum)
{
	proc(signum, Proc1);
}

void
proc2(int signum)
{
	proc(signum, Proc2);
}

int
min(int a, int b)
{
	return b<a ? b : a ;
}

void
proc(int signum, int pn)
{
	int i, border;
	while(*wrote < MustBeWrote){
		swait(s, pn);
		border = min(*wrote+StrNum, MustBeWrote) ;

		for(i = 0 ; *wrote+i < border ; ++i ){
			sprintf(genbufs[i], "%d\t%d\t%d",
				*wrote + i + 1, getpid(), gettimeinms());
		}

		*wroteonce = i ;
		*wrote += *wroteonce ;

		if(*wroteonce)
			slet(s, MainProc);

		spost(s);
	}
	kill(getppid(), SIGUSR1);
	exit(0);
}

void
procmain(int signum)
{
}

void *
shrmalloc(int size)
{
	return mmap(0, size,
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
}

int
main(int argc, char *argv[])
{
	int i, cnt, prevwrote, border;
	char buf[BUFSIZ];

	s = shrmalloc(sizeof(int)) ;
	*s = 0 ;

	p1s = shrmalloc(sizeof(int)) ;
	*p1s = 0 ;

	p2s = shrmalloc(sizeof(int)) ;
	*p2s = 0 ;

	wrote = shrmalloc(sizeof(int)) ;
	*wrote = 0 ;

	wroteonce = shrmalloc(sizeof(int)) ;
	*wroteonce = 0 ;

	done = shrmalloc(sizeof(int)) ;
	done = 0 ;
	

	for(i = 0 ; i < StrNum ; ++i)
		genbufs[i] = shrmalloc(StrSiz) ;

	argv0 = argv[0] ;
	if(argc != 1)
		usage();


	if(!(pid1 = fork())){
		signal(SIGUSR2, proc1);
		*p1s = 1 ;
		while(1)
			;
	}

	if(!(pid2 = fork())){
		signal(SIGUSR2, proc2);
		*p2s = 1 ;
		while(1)
			;
	}

	signal(SIGUSR1, procmain);
	while(!*p1s || !*p2s)
		;

	kill(pid1, SIGUSR2);
	kill(pid2, SIGUSR2);

	while(*wrote < MustBeWrote){
		stake(s, MainProc);
		border = *wroteonce ;
		for(i = 0 ; i < border ; ++i){
			ndprintf(1, "%s\n", genbufs[i]);
		}
		spost(s);
	}

	while(wait(&dummy) > 0)
		;

	return 0 ;
}
