#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <math.h>

struct ProcTree {
	int s;
	int sent;
	int got1, got2;
	int *c;
	int *cpids;
};
typedef struct ProcTree ProcTree;

int dummy;
char *argv0;
ProcTree **t;
int curtree;
int *started, *firstpid;
int lastnum = 8 ;

void
usage(void)
{
	fprintf(stderr, "usage: %s\n",
		argv0);
	exit(1);
}

void
nprintf(char *fmt, ...)
{
        va_list ap;
        char buf[256];
        int n;

        va_start(ap, fmt);
        n = vsprintf(buf, fmt, ap);
        va_end(ap);
        write(1, buf, n);
}

long
gettimeinms(void)
{
        long ms;
        struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        ms = round(spec.tv_nsec / 1.0e3);
        return ms ;
}

void
sighndl(int signum)
{
	ProcTree *ct = t[curtree] ;
	int *cpids = ct->cpids ;
	int *pi = cpids ;
	switch(signum){
	case SIGUSR1 :
		++ct->got1;
		break;
	case SIGUSR2 :
		++ct->got2;
		break;
	default:
		exit(1);
	}
		
	nprintf("%d %d %d got %d\n", curtree, getpid(), getppid(), signum);
	while(*pi){
		kill(*pi, t[curtree]->s);
		++ct->sent;
		nprintf("%d %d %d sent %d to %d at %d\n",
			curtree, getpid(), getppid(),
			signum, ct->c[pi - cpids], gettimeinms());
		if(ct->sent == 101){
			pi = cpids ;
			while(*pi){
				kill(*pi, SIGTERM);
				++pi;
			}
			while(wait(&dummy) > 0)
				;
			exit(0);
		}
		++pi;
	}
}

void
sigterm(int signum)
{
	ProcTree *ct = t[curtree] ;
	int *pi = ct->cpids ;
	while(*pi){
		kill(*pi, SIGTERM);
		++pi;
	}
	nprintf("%d %d exited after %d and %d\n",
		getpid(), getppid(), ct->got1, ct->got2);
	exit(0);
}

void
starttree(int n)
{
	int i, *p;
	int pid, cnum;
	ProcTree *ct = t[n] ;
	curtree = n ;
	ct->sent = 0 ;

	signal(SIGUSR1, sighndl);
	signal(SIGUSR2, sighndl);
	signal(SIGTERM, sigterm);

	p = ct->c ;
	while(*p) ++p ;
	cnum = p - ct->c ;

	ct->cpids = malloc(sizeof(int) * (cnum+1)) ;

	p = ct->c ;
	while(*p){
		if(*p == 1){ /* To to deal with infinite loop. */
			ct->cpids[p - ct->c] = *firstpid ;
			break;
		}

		pid = fork() ;
		if(!pid){
			starttree(*p);
			exit(0);
		}
		ct->cpids[p - ct->c] = pid ;
		++p;
	}
	ct->cpids[cnum] = 0 ;

	if(n == lastnum)
		*started = 1 ;

	while(1)
		;
}

int
main(int argc, char *argv[])
{
	argv0 = argv[0] ;
	if(argc != 1)
		usage();

	t = mmap(0, 10 * sizeof(*t),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	started = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	firstpid = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;

	for(int i = 0 ; i < lastnum + 2 ; ++i)
		t[i] = malloc(sizeof(t[0])) ;

	t[1]->c = (int[]){ 2, 3, 4, 0 } ;
	t[1]->s = SIGUSR1 ;

	t[2]->c = (int[]){ 5, 6, 0 } ;
	t[2]->s = SIGUSR2 ;

	t[6]->c = (int[]){ 7, 0 } ;
	t[6]->s = SIGUSR1;

	t[7]->c = (int[]){ 8, 0 } ;
	t[7]->s = SIGUSR1 ;

	t[8]->c = (int[]){ 1, 0 } ;
	t[8]->s = SIGUSR2 ;

	t[9] = 0 ;

	*started = 0 ;

	int pid = fork() ;
	if(!pid){
		starttree(1);
		exit(0);
	}

	*firstpid = pid ;
	while(!*started)
		;
	nprintf("%d %d\n", pid, *started);

	kill(pid, SIGUSR1);

	while(wait(&dummy) > 0)
		;

	return 0 ;
}
