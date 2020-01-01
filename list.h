#ifndef LIST_H
#define LIST_H

typedef struct node
{
	void *data;
	struct node *next;
	struct node *previous;
} Node;

// These functions all return the first node in the modified list
Node *list_create(void *data);
Node *list_createafter(void *data, Node *list);
Node *list_createbefore(void *data, Node *list);
Node *list_remove(Node *n);

Node *list_getlast(Node *list);
Node *list_getfirst(Node *list);

void list_free(Node *list);

#endif // LIST_H

