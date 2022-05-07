#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

char *argv0, *dirpath;
int min, max;

void
usage(void)
{
	fprintf(stderr, "usage: %s <dir> <min> <max>\n",
		argv0);
	exit(1);
}

void
processfile(const char *p)
{
	int e, siz;
	struct stat st;
	if(e=lstat(p, &st)){
		switch(e){
		case EACCES :
			fprintf(stderr, "%s: Access denied\n",
				p);
		break;
		default:
			fprintf(stderr, "%s: Unknown (not handled) error\n",
				p);
		}
		return;
	}

	siz = st.st_size ;
	if(min<=siz  && siz<=max)
		printf("%s\t%d\n", p, siz);
}

void
fperror(char *fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap) ;
	va_end(ap);
	perror(buf);
}

int
main(int argc, char *argv[])
{
	char buf[512];
	DIR *d;
	struct dirent *de;

	argv0 = argv[0] ;
	if(argc != 4)
		usage();

	dirpath = argv[1] ;
	
	if(!strcmp(argv[2], "0"))
		min = 0 ;
	else if( !(min = atoi(argv[2])) )
		usage();

	if(!strcmp(argv[3], "0"))
		max = 0 ;
	else if( !(max = atoi(argv[3])) )
		usage();
			
	if(max < min)
		usage();

	realpath(dirpath, buf);
	if(!(d = opendir(buf))){
		perror(dirpath);
		exit(1);
	}

	while(de = readdir(d)){
		realpath(dirpath, buf);
		strcat(buf, "/");
		strcat(buf, de->d_name);
		processfile(buf);
	}

	closedir(d);
	
	return 0 ;
}
