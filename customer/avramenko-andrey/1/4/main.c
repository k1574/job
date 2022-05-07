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

int *mutex, *p1s, *p2s;
int dummy, finish;
char *argv0;
int pid1, pid2;
int fd1[2], fd2[2];

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

int
ndgets(int fd, char *buf)
{
	char *s = buf ;
	int n = 0, rn ;
	while((rn = read(fd, s, 1)) && *s != '\n'){
		if(rn < 0)
			break;
		++s;
		n += rn ;
	}
	if(n)
		*++s = 0 ;
	return n ;
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
proc1(int signum)
{
	int i, j;
	for(i = 0 ; i<10 ; ++i){
		swait(mutex);
		for(j = 0 ; j<110 ; ++j){
			ndprintf(fd1[1], "%d %d %d\n", 1 + i*110 + j, getpid(), gettimeinms());
		}
		spost(mutex);
	}
	close(fd1[1]);
	close(fd1[0]);
	kill(getppid(), SIGUSR1);
	exit(0);
}

void
proc2(int signum)
{
	int i, j;
	for(i = 0 ; i<10 ; ++i){
		swait(mutex);
		for(j = 0 ; j<110 ; ++j){
			ndprintf(fd2[1], "%d %d %d\n", 1 + i*110 + j, getpid(), gettimeinms());
		}
		spost(mutex);
	}
	close(fd2[1]);
	close(fd2[0]);
	kill(getppid(), SIGUSR1);
	exit(0);
}

void
mainproc(int signum)
{
}

int
main(int argc, char *argv[])
{
	int i, cnt;
	char buf[BUFSIZ];

	mutex = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*mutex = 0 ;

	p1s = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*p1s = 0 ;

	p2s = mmap(0, sizeof(int),
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,
		-1, 0) ;
	*p2s = 0 ;
	

	argv0 = argv[0] ;
	if(argc != 1)
		usage();

	pipe(fd1);
	pipe(fd2);

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

	signal(SIGUSR1, mainproc);
	while(!*p1s || !*p2s)
		;


	kill(pid1, SIGUSR2);
	kill(pid2, SIGUSR2);

	while(1){
		for(i=0 ; i<75 ; ++i){
			ndgets(fd1[0], buf);
			ndprintf(1, "%s", buf);
		}
		for(i=0 ; i<75  ; ++i){
			ndgets(fd2[0], buf);
			ndprintf(1, "%s", buf);
		}
	}
	
	return 0 ;
}
