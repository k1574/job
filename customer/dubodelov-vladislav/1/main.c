#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "defs.h"

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

struct Computer {
	char Manufacture[26], CPUType[26];
	float ClockFreq;
	int Memory, HDDCapacity;
};

typedef struct Computer Computer;

/* Handler to make the program extendable. */
typedef int (*HndlFunc)(void);

struct HndlFuncStruct {
	HndlFunc Hndl;
	char *Desc;
} ;
typedef struct HndlFuncStruct HndlFuncStruct;

int quit(void);
int print(void);
int input(void);
int add(void);
int localRemove(void);
int change(void);
int sort(void);
int numSort(void);
int save(void);
int restore(void);

void printSorts(void);
int hasNotDigits(char *s);

int byNothing(void *, void *);
int byClockFreq(void *, void *);
int byMemory(void *, void *);
int byCPUType(void *, void *);
int byHDDCapacity(void *, void *);
int byManufacture(void *, void *);

char *chomp(char *s, char c);

LinkedList *db;

/* Handlers themselves. */
HndlFuncStruct handlers[] = {
	{quit, "Quit"},
	{print, "Print"},
	{input, "Input"},
	{add, "Add"},
	{localRemove, "Remove"},
	{change, "Change"},
	{sort, "Sort"},
	{save, "Save"},
	{restore, "Restore"},
};

struct {
	int (*Fn)(void *, void *);
	char *Desc;
} sorts[] = {
    {byNothing, "Exit"},
	{byManufacture, "By manufacture"},
	{byCPUType, "By CPU type"},
	{byClockFreq, "By clock frequency"},
	{byMemory, "By memory"},
	{byHDDCapacity, "By HDD capacity"},
};

LinkedList *
ll_create(void)
{
	LinkedList *l = malloc(sizeof(LinkedList)) ;
	l->first = malloc(sizeof(LinkedListEl)) ;
	l->first->next = 0 ;
	l->first->data = 0 ;
	l->len = 0 ;
	return l ;
}

void
ll_append(LinkedList *l, void *data)
{
	int i, len;
	len = l->len ;
	LinkedListEl *el = l->first ;
	for( i=0 ; i<len ; ++i)
		el = el->next ;
	el->next = malloc(sizeof(LinkedListEl)) ;
	el->next->data = data ;
	++l->len;
}

void
ll_push(LinkedList *l, void *data)
{
	LinkedListEl *el = l->first->next ;
	l->first->next = malloc(sizeof(LinkedListEl)) ;
	l->first->next->data = data ;
	l->first->next->next = el ;
	++l->len;
}

void *
ll_at(LinkedList *l, unsigned int n)
{
	unsigned int i;
	LinkedListEl *el;

	if(n >= l->len) return 0 ;

	el = l->first->next;
	for( i=0 ; i<n ; ++i )
		el = el->next ;

	return el->data ;
}

int
ll_remove(LinkedList *l, unsigned int n)
{
	int i;
	LinkedListEl *el, *buf;

	if(n>=l->len) return -1 ;

	el = l->first ;
	for( i=0 ; i<n ; ++i)
		el = el->next ;	

	buf = el->next ;
	el->next = el->next->next;
	free(buf);

	--l->len;
	return 0 ;
}

int
ll_insert(LinkedList *l, unsigned int n, void *data)
{
	unsigned int i;
	LinkedListEl *el, *buf;

	if(n > l->len) return -1 ;

	el = l->first ;
	for( i=0 ; i<n ; ++i)
		el = el->next ;

	buf = el->next ;
	el->next = malloc(sizeof(LinkedListEl)) ;
	el->next->data = data ;
	el->next->next = buf ;

	++l->len;
	return 0 ;
}

int
ll_swap(LinkedList *l, unsigned int i1, unsigned int i2)
{
	int i;
	int min;
	LinkedListEl *el,
		*el1, *el2,
		*els1, *els2,
		*elss1, *elss2;

	if(i1 == i2)
		return 0 ;

	if(i1 >= l->len || i2 >= l->len )
		return 1 ;


	el1 = l->first ;
	for(i=0 ; i<i1 ; ++i)
		el1 = el1->next ;

	el2 = l->first ;
	for(i=0 ; i<i2 ; ++i)
		el2 = el2->next ;

	els1 = el1->next ;
	els2 = el2->next ;

	elss1 = els1 ? els1->next : 0 ;
	elss2 = els2 ? els2->next : 0 ;

	if(abs(i1-i2) == 1){
		if(i1<i2){
			el1->next = els2 ;
			els2->next = els1 ;
			els1->next = elss2 ;
		} else {
			el2->next = els1 ;
			els1->next = els2 ;
			els2->next = elss1 ;
		}
	} else {
		el1->next = els2 ;
		el2->next = els1 ;

		els1->next = elss2 ;
		els2->next = elss1 ;
	}

	return 0 ;
}

void
ll_bubbleSort(LinkedList *l, int (*fn)(void *, void *))
{
	int i, j;
	for(i=0 ; i<l->len - 1 ; ++i){
		for(j=0 ; j<l->len - 1 - i ; ++j){
			if(fn(ll_at(l, j), ll_at(l, j+1)) > 0){
			   ll_swap(l, j, j+1);
			}
		}
	}
}

int
byNothing(void *v1, void *v2)
{
    return 0 ;
}

int
byClockFreq(void *v1, void *v2)
{
	Computer 
		*c1 = (Computer *)v1 ,
		*c2 = (Computer *)v2 ;
	return (c1->ClockFreq >= c2->ClockFreq) ? 1 : -1 ;
}

int
byMemory(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->Memory > c2->Memory ? 1 : -1 ;
}

int
byManufacture(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return strcmp(c1->Manufacture, c2->Manufacture) ;
}

int
byCPUType(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return strcmp(c1->CPUType, c2->CPUType) ;
}

int
byHDDCapacity(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->HDDCapacity > c2->HDDCapacity ? 1 : -1 ;
}

void
printComputer(Computer *c)
{
	printf(
		"Manufacture: %s\n"
		"CPU type: %s\n" 
		"Clock frequency: %f\n"
		"Memory: %d\n"
		"HDD capacity: %d\n",
		c->Manufacture, c->CPUType,
		c->ClockFreq, c->Memory, c->HDDCapacity
	);
}

int
print()
{
	int i;
	for(i=0 ; i<db->len ; ++i){
		puts("");
		printComputer((Computer *)ll_at(db, i));
	}
	puts("");
	return 0 ;
}

int
input()
{
	while(add())
		;
	return 0 ;
}

char *
readString(char *prompt, char *buf, int siz)
{
	printf("%s", prompt);
	return chomp(fgets(buf, siz, stdin), '\n') ;
}

Computer *
readComputer(Computer *c){
	char buf[512];
	readString("Manufacture: ", c->Manufacture, sizeof(c->Manufacture));
	if(!strcmp(c->Manufacture, "*")){
		return 0 ;
	}

	readString("CPU type: ", c->CPUType, sizeof(c->CPUType));

	c->ClockFreq =
		atof(readString("Clock frequency: ", buf, sizeof(buf))) ;

	c->Memory =
		atoi(readString("Memory: ", buf, sizeof(buf))) ;

	c->HDDCapacity =
		atoi(readString("HDD capacity: ", buf, sizeof(buf))) ;


	return c ;
}


int
add()
{
	Computer *c = malloc(sizeof(*c)) ;
	if(!readComputer(c)){
	   return 0 ;
	}
	
	ll_push(db, c);

	return 1 ;
}

int
readIndex()
{
	int i;
	char buf[512] = "0";
	
	do{
		i = atoi(readString("Enter index: ", buf, sizeof(buf))) ;
		if(i < 0 || db->len <= i){
			puts("Wrong index");
			continue;
		}
	}while(hasNotDigits(buf) || i < 0 || db->len <= i);
	
	return i ;
}

int
localRemove()
{
	int i = readIndex();
	ll_remove(db, i);
	return 0 ;
}

int
change()
{
	int i = readIndex() ;
	readComputer(ll_at(db, i));
	return 0 ;
}

int
sort()
{
	int in;
	char buf[512];
	printSorts();
	in = atoi(readString("> ", buf, sizeof(buf))) ;
	ll_bubbleSort(db, sorts[in].Fn);
	return 0 ;
}

int
save(void)
{
	int i;
	char buf[512];
	FILE *f = fopen(readString("File name: ", buf, sizeof(buf)), "w+b") ;
	if(!f){
		perror("fopen");
		return 1 ;
	}

	for(i=0 ; i<db->len ; ++i)
		fwrite(ll_at(db, i), sizeof(Computer), 1, f);
	fclose(f);
	return 0 ;
}

int
restore()
{
	int i, n;
	char buf[512];
	FILE *f = fopen(readString("File name: ", buf, sizeof(buf)), "r+b") ;
	if(!f){
		perror("fopen");
		return 1;
	}

	do{
		Computer *c = malloc(sizeof(*c)) ;
		n = fread(c, sizeof(*c), 1, f);
		if(n != 1){
			free(c);
		}else{
			ll_append(db, c);
		}
	}while(!feof(f));

	fclose(f);
	return 0 ;
}

int
quit(void)
{
	exit(0);
}

int
ckIn(int in)
{
	if(in < 0 || in > LENGTH(handlers))
		return 1 ;
	return 0 ;
}

void
printSorts(void)
{
	int i;
	for(i = 0 ; i<LENGTH(sorts) ; ++i){
		printf("%d. %s\n", i, sorts[i].Desc);
	}
}

void
printHndls(void)
{
	int i;
	for(i = 0 ; i<LENGTH(handlers) ; ++i){
		printf("%d. %s\n", i, handlers[i].Desc);
	}
}

int
hasNotDigits(char *s)
{
	if(!strlen(s))
		return 1 ;
	while(*s){
		if( !isdigit((int)*s) )
			return 1 ;
		++s;
	}
	return 0 ;
}

char *
chomp(char *s, char c)
{
	int len = strlen(s) ;
	if(s[len-1] == c)
		s[len-1] = 0 ;
	return s ;
}

void
mainLoop(void)
{
	int in, ret;
	char buf[512];
	while(1){
		printHndls();
		readString("> ", buf, sizeof(buf));
		in = atoi(buf) ;

		if(ckIn(in)){
			puts("No such function");
			continue;
		}
		handlers[in].Hndl();
	}
}

int 
main(int argc, char *argv[])
{
	db = ll_create() ;
	mainLoop();
}

