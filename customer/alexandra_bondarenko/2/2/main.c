#include <stdio.h>

char spaces[] = "\t\r\n ";


int
stringlen(char *s)
{
	char *ps = s ;
	while(*s)
		++s;
	return s - ps ;
}

int
eq_any_of_chrs(char c, char *chrs)
{
	int i, len = stringlen(chrs) ;
	for(i=0 ; i<len ; ++i)
		if(c == chrs[i])
			return 1 ;
	return 0 ;
}

char *
skipchrs(char *s, char *chrs)
{
	while(eq_any_of_chrs(*s, chrs))
		++s;
	return s ;
}

char *
skipnotchrs(char *s, char *chrs)
{
	while(!eq_any_of_chrs(*s, chrs))
		++s;
	return s ;
}

int
Words(char *s)
{
	int cnt = 0 ;
	while(*s){
		if(eq_any_of_chrs(*s, spaces))
			s = skipchrs(s, spaces);
		if(*s){
			s = skipnotchrs(s, spaces);
			++cnt;
		}
	}
	return cnt ;
}

int
main(int argc, char *argv[])
{
	char buf[1024];
	int n=0;

	while(fgets(buf, sizeof(buf), stdin)){
		n += Words(buf) ;
	}

	printf("%d\n", n);

	return 0 ;
}

