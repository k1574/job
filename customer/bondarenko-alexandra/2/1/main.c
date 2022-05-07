#include <stdlib.h>
#include <stdio.h>

struct Matrice {
	struct Vector *zero_v;
	void *default_value ;
};

struct Vector {
	struct VectorEl *zero_el;
	void *default_value;
};

struct VectorEl {
	int idx;
	void *val;
	struct VectorEl *next;
};

typedef struct Matrice Matrice;
typedef struct Vector Vector;
typedef struct VectorEl VectorEl;

Matrice *
matrice_init(void *defval)
{
	Matrice *m = malloc(sizeof(Matrice)) ;
	m->zero_v = vector_init(defval) ;
	m->default_value = defval ;
	return m ;
}

void
matrice_set(Matrice *m, int idx1, int idx2, void *val)
{
	int isnil;
	Vector *v;

	vector_get(m->zero_v, &isnil, idx1);
	if(isnil)
		vector_set(m->zero_v, idx1, vector_init(m->default_value));

	v = vector_get(m->zero_v, 0, idx1) ;
	vector_set(v, idx2, val);
}

void
matrice_set_array(Matrice *m, void **arr, int w, int h)
{
	int i, j;
	for(i=0 ; i<h ; ++i){
		for(j=0 ; j<w ; ++j){
			matrice_set(m, j, i, arr[i*w + j]);
		}
	}
}

void *
matrice_get(Matrice *m, int *isnil, int idx1, int idx2)
{
	void *ret;
	Vector *v = vector_get(m->zero_v, isnil, idx1) ;
	if(isnil && *isnil){
		return v ;
	}

	ret = vector_get(v, isnil, idx2) ;
	return ret ;
}

VectorEl *
matrice_get_el(Matrice *m, int *isnil, int idx1, int idx2)
{
}

Vector *
vector_init(void *defval)
{
	Vector *v = malloc(sizeof(*v)) ;
	v->zero_el = malloc(sizeof(VectorEl)) ;
	v->default_value = defval ;
	v->zero_el->idx = -9999999 ; /* Must be the lowest index. */
	v->zero_el->val = 0 ;
	return v ;
}

void
vector_set(Vector *v, int idx, void *val)
{
	VectorEl *bel, *el = v->zero_el ;
	while(el->next && (idx > el->next->idx))
		el = el->next ;

	if(idx == el->idx){
		/* Nothing. */
	}else if(el->next){
		bel = el->next ;
		el->next = malloc(sizeof(VectorEl)) ;	
		el->next->next = bel ;
		el = el->next ;
	}else{
		el->next = malloc(sizeof(VectorEl)) ;
		el = el->next ;
	}

	el->idx = idx ;
	el->val = val ;
}

VectorEl *
vector_get_el(Vector *v, int *isnil, int idx)
{
	VectorEl *el = v->zero_el ;
	while(el->next && (idx > el->next->idx))
		el = el->next;

	if(!el->next){
		goto def_exit;
	} else if(idx == el->next->idx){
		if(isnil) *isnil = 0 ;
		return el->next ;
	}

def_exit:
	if(isnil) *isnil = 1 ;
	return 0;
}

void *
vector_get(Vector *v, int *isnil, int idx)
{
	VectorEl *el = vector_get_el(v, isnil, idx) ;
	if(el)
		return el->val ;
	return v->default_value ;
}

void
vector_debug_print(Vector *v)
{
	VectorEl *el = v->zero_el ;
	while(el->next){
		el=el->next ;
		printf("%d %x\n", el->idx, (int)el->val);
	}
}

void
matrice_debug_print(Matrice *m, int w, int h)
{
	int i, j, isnil;
	void *vd;
	for(i=0 ; i<h ; ++i){
		for(j=0 ; j<w ; ++j){
			vd = matrice_get(m, &isnil, j, i);
			if(!isnil)
				printf("%x ", (int)vd);
			else
				printf("nil ");
		}
		puts("");
	}
}


int
main(int argc, char *argv[])
{
	int isnil;
	Matrice *m = matrice_init(0) ;
	void *arr[] = {
		1, 2, 3, 4, 5,
		6, 7, 8, 9, 10,
		11, 12, 13, 14, 15
	} ;
	matrice_set_array(m, arr, 5, 3);
	matrice_set(m, 6, 9, 256);
	matrice_set(m, 7, 10, 85);
	matrice_set(m, 8, 11, 256);
	matrice_debug_print(m, 10, 15);

	return 0 ;
}

