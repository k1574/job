#ifndef __LL_H
#define __LL_H

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

#endif /* __LL_H */
