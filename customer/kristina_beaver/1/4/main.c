#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <signal.h>

#include <unistd.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <semaphore.h>

#include <fcntl.h>

#define WROTE (*wrote)

enum{
	MustBeWrote = 1100,
	ParentReads = 3,
	ChildWrites = 3,
};

int *p1s, *p2s, *wrote;
int myid, mutexkey=1;
sem_t *mutex;
int dummy, *finish, *done;
char *argv0;
int pid1, pid2;
FILE *wfile, *rfile;

void
usage(void)
{
	fprintf(stderr, "usage: %s\n",
		argv0);
	exit(1);
}

gettimeinms(void)
{
	long ms;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	ms = round(spec.tv_nsec / 1.0e3);
	return ms;
}

void
swait(int *sem)
{
	while(*sem)
		;
	++*sem;
}

void
spost(int *sem)
{
	if(!*sem)
		return;
	--*sem;
}

void
proc(int signum)
{
	int i=0, j;
	while(WROTE < MustBeWrote){
		sem_wait(mutex);
		for(j = 0 ; j<ChildWrites && WROTE < MustBeWrote ; ++j, ++WROTE){
			fprintf(wfile, "%d %d %d\n", 1 + WROTE, getpid(), gettimeinms());
			fflush(wfile);
		}
		sem_post(mutex) ;
	}
	if(myid == 1) *p1s = 0 ;
	else if(myid == 2) *p2s = 0 ;
	kill(getppid(), SIGUSR1);
	exit(0);
}

void
mainproc(int signum)
{
	if(!*p2s && !*p1s){
		*finish = 1 ;
	}
}

int
main(int argc, char *argv[])
{
	int i, cnt;
	char buf[BUFSIZ];

	mutex = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	sem_init(mutex, 1, 1);

	p1s = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*p1s = 0 ;

	p2s = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*p2s = 0 ;

	finish = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*finish = 0 ;

	wrote = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*wrote = 0 ;


	argv0 = argv[0] ;
	if(argc != 1)
		usage();

	wfile = fopen("log", "w") ;

	myid = 1 ;
	if(!(pid1 = fork())){
		signal(SIGUSR2, proc);
		*p1s = 1 ;
		while(1)
			;
	}

	myid = 2 ;
	if(!(pid2 = fork())){
		signal(SIGUSR2, proc);
		*p2s = 1 ;
		while(1)
			;
	}

	myid=0;
	signal(SIGUSR1, mainproc);
	while(!*p1s || !*p2s)
		;

	kill(pid1, SIGUSR2);
	kill(pid2, SIGUSR2);

	int prev=0;
	int cur=0;

	rfile = fopen("log", "r") ;
	while(1){
		while(cur >= WROTE)
			;
		if(!fgets(buf, sizeof(buf), rfile))
			;
		printf("%d %s", getpid(), buf);
		++cur;
		if(cur >= MustBeWrote)
			goto exit;
	}

exit:
	sem_destroy(mutex);
	fclose(rfile);
	fclose(wfile);

	return 0 ;
}
