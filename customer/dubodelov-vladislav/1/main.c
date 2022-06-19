#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

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

struct LinkedList {
	/* First element is always empty one. */
	struct LinkedListElement *first;
	unsigned int len;
} ;

typedef struct LinkedList LinkedList;

struct LinkedListElement {
	void *data;
	struct LinkedListElement *next;
};

typedef struct LinkedListElement LinkedListEl;

LinkedList *ll_create(void);
void ll_append(LinkedList *l, void *data);
void ll_push(LinkedList *l, void *data);
void *ll_at(LinkedList *l, unsigned int n);
int ll_insert(LinkedList *l, unsigned int n, void *data);
int ll_remove(LinkedList *l, unsigned int n);
int ll_swap(LinkedList *l, unsigned int i1, unsigned int i2);

int f_quit(void);
int f_print(void);
int f_input(void);
int f_add(void);
int f_remove(void);
int f_change(void);
int f_sort(void);
int f_save(void);
int f_restore(void);

void printSorts(void);
int hasNotDigits(char *s);

int byNothing(void *, void *);
int byClockFreq(void *, void *);
int byClockFreqRev(void *, void *);
int byMemory(void *, void *);
int byMemoryRev(void *, void *);
int byCPUType(void *, void *);
int byCPUTypeRev(void *, void *);
int byHDDCapacity(void *, void *);
int byHDDCapacityRev(void *, void *);
int byManufacture(void *, void *);
int byManufactureRev(void *, void *);

char *chomp(char *s, char c);

LinkedList *db;

/* Команды. */
HndlFuncStruct handlers[] = {
	{f_quit, "Quit"},
	{f_print, "Print"},
	{f_input, "Input"},
	{f_add, "Add"},
	{f_remove, "Remove"},
	{f_change, "Change"},
	{f_sort, "Sort"},
	{f_save, "Save"},
	{f_restore, "Restore"},
};

/* Функции сортировки. */
struct {
	int (*Fn)(void *, void *);
	char *Desc;
} sorts[] = {
    {byNothing, "Exit"},
	{byManufacture, "By manufacture"},
	{byManufactureRev, "By manufacture (reverse)"},
	{byCPUType, "By CPU type"},
	{byCPUTypeRev, "By CPU type (reverse)"},
	{byClockFreq, "By clock frequency"},
	{byClockFreqRev, "By clock frequency (reverse)"},
	{byMemory, "By memory"},
	{byMemoryRev, "By memory (reverse)"},
	{byHDDCapacity, "By HDD capacity"},
	{byHDDCapacityRev, "By HDD capacity (reverse)"},
};

/* Создание связаного списка. */
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

/* Добавить в конец списка. */
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

/* Добавить в начало списка. */
void
ll_push(LinkedList *l, void *data)
{
	LinkedListEl *el = l->first->next ;
	l->first->next = malloc(sizeof(LinkedListEl)) ;
	l->first->next->data = data ;
	l->first->next->next = el ;
	++l->len;
}

/* Получение элемента списка по индексу. */
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

/* Удаление элемента списка по индексу. */
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

/* Вставить элемент после элемента с указанным индексом. */
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

/* Поменять местами 2 элемента по их индексам. */
int
ll_swap(LinkedList *l, unsigned int i1, unsigned int i2)
{
        int i;

	LinkedListEl *el1, *el2;
	void *v;

	if(i1 == i2)
		return 0 ;

	if(i1 >= l->len || i2 >= l->len )
		return 1 ;


	el1 = l->first ;
	for(i=0 ; i<=i1 ; ++i)
		el1 = el1->next ;

	el2 = l->first ;
	for(i=0 ; i<=i2 ; ++i)
		el2 = el2->next ;

	v = el1->data ;
	el1->data = el2->data ;
	el2->data = v ;

	return 0 ;
}

/* Сортировка пузырьком по произвольной переданной функции. */
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

/* Не сортировать. */
int
byNothing(void *v1, void *v2)
{
    return 0 ;
}

/* Сортировать по частоте. */
int
byClockFreq(void *v1, void *v2)
{
	Computer 
		*c1 = (Computer *)v1 ,
		*c2 = (Computer *)v2 ;
	return (c1->ClockFreq >= c2->ClockFreq) ? 1 : -1 ;
}

int
byClockFreqRev(void *v1, void *v2)
{
	Computer 
		*c1 = (Computer *)v1 ,
		*c2 = (Computer *)v2 ;
	return (c1->ClockFreq <= c2->ClockFreq) ? 1 : -1 ;
}

/* Сортировать по памяти. */
int
byMemory(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->Memory > c2->Memory ? 1 : -1 ;
}

int
byMemoryRev(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->Memory < c2->Memory ? 1 : -1 ;
}

/* Сортировать по производителю. */
int
byManufacture(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return strcmp(c1->Manufacture, c2->Manufacture) ;
}

int
byManufactureRev(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return -strcmp(c1->Manufacture, c2->Manufacture) ;
}

/* Сортировать по типу ЦПУ. */
int
byCPUType(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return strcmp(c1->CPUType, c2->CPUType) ;
}

int
byCPUTypeRev(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	   return -strcmp(c1->CPUType, c2->CPUType) ;
}

/* Сортировать по вместимости жёсткого диска. */
int
byHDDCapacity(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->HDDCapacity > c2->HDDCapacity ? 1 : -1 ;
}

int
byHDDCapacityRev(void *v1, void *v2)
{
    Computer *c1 = (Computer *)v1,
    	*c2 = (Computer *)v2 ;
	return c1->HDDCapacity < c2->HDDCapacity ? 1 : -1 ;
}

/* Вывести в терминал данные о компьютере.  */
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
f_print()
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
f_input()
{
	while(f_add())
		;
	return 0 ;
}

/* Прочитать строку из стандартного ввода без символа новой строки. */
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
f_add()
{
	Computer *c = malloc(sizeof(*c)) ;
	if(!readComputer(c)){
	   return 0 ;
	}
	
	ll_push(db, c);

	return 1 ;
}

int
readIndex(char *prompt, int i1, int i2)
{
	int i;
	char buf[512] = "0";
	
	do{
		i = atoi(readString(prompt, buf, sizeof(buf))) ;
		if(i < i1 || i2 <= i){
			puts("Wrong index");
			continue;
		}
	}while(hasNotDigits(buf) || i < i1 || i2 <= i);
	
	return i ;
}

int
f_remove()
{
	int i = readIndex("> ", 0, db->len);
	ll_remove(db, i);
	return 0 ;
}

int
f_change()
{
	int i = readIndex("> ", 0, db->len) ;
	readComputer(ll_at(db, i));
	return 0 ;
}

int
f_sort()
{
	int in;
	char buf[512];
	printSorts();
	in = atoi(readString("> ", buf, sizeof(buf))) ;
	ll_bubbleSort(db, sorts[in].Fn);
	return 0 ;
}

int
f_save(void)
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
f_restore()
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
f_quit(void)
{
	exit(0);
}

/* Напечатать список сортировок. */
void
printSorts(void)
{
	int i;
	for(i = 0 ; i<LENGTH(sorts) ; ++i){
		printf("%d. %s\n", i, sorts[i].Desc);
	}
}

/* Напечатать команды. */
void
printHndls(void)
{
	int i;
	for(i = 0 ; i<LENGTH(handlers) ; ++i){
		printf("%d. %s\n", i, handlers[i].Desc);
	}
}

/* Строка имеет в себе не цифру. */
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

/* Обнулить последний символ в строке. */
char *
chomp(char *s, char c)
{
	int len = strlen(s) ;
	if(s[len-1] == c)
		s[len-1] = 0 ;
	return s ;
}

int 
main(void)
{
	db = ll_create() ;
	int in, ret;
	char buf[512];
	while(1){
		printHndls();
		in = readIndex("> ", 0, LENGTH(handlers)) ;
		handlers[in].Hndl();
	}
}

