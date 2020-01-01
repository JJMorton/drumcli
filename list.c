#include <stdlib.h>
#include <stdio.h>

#include "list.h"

Node *list_create(void *data)
// Creates a node with no connections
{
	Node *n = malloc(sizeof(Node));
	if (n == NULL)
	{
		printf("Failed to allocate memory for list\n");
		return NULL;
	}
	n->data = data;

	n->next = NULL;
	n->previous = NULL;

	return n;
}

Node *list_createafter(void *data, Node *list)
// Inserts a node into a linked list after the node 'list'
{
	Node *n = list_create(data);
	if (n == NULL) return NULL;

	/*
	 * list -----n---->
	 *      <----n----- next
	 *           n
	 */

	n->previous = list;

	/*
	 * list -----n---->
	 *      <----n----- next
	 *      <=== n
	 */

	if (n->previous != NULL)
		n->next = n->previous->next;

	/*
	 * list -----n---->
	 *      <----n----- next
	 *      <=== n ===>
	 */

	if (n->previous != NULL)
		n->previous->next = n;

	/*
	 * list ===> n
	 *      <----n----- next
	 *      <=== n ===>
	 */

	if (n->next != NULL)
		n->next->previous = n;

	/*
	 * list ===> n
	 *           n <=== next
	 *      <=== n ===>
	 */

	return list_getfirst(n);
}

Node *list_createbefore(void *data, Node *list)
{
	// If there is a previous node, create the new one after that
	if (list != NULL && list->previous != NULL) return list_createafter(data, list->previous);

	Node *n = list_create(data);
	if (n == NULL) return NULL;
	if (list == NULL) return n;

	// Create the node at the beginning of the list

	/*
	 * previous -----n---->
	 *          <----n----- list
	 *               n
	 */

	n->next = list;

	/*
	 * previous -----n---->
	 *          <----n----- list
	 *               n ===>
	 */

	n->previous = list->previous;

	/*
	 * previous -----n---->
	 *          <----n----- list
	 *          <=== n ===>
	 */

	if (n->previous != NULL)
		n->previous->next = n;

	/*
	 * previous ===> n
	 *          <----n----- list
	 *          <=== n ===>
	 */

	if (n->next != NULL)
		n->next->previous = n;

	/*
	 * previous ===> n
	 *               n <=== list
	 *          <=== n ===>
	 */

	return list_getfirst(n);

}

Node *list_getlast(Node *list)
{
	if (list == NULL) return NULL;
	while (list->next != NULL) list = list->next;
	return list;
}

Node *list_getfirst(Node *list)
{
	if (list == NULL) return NULL;
	while (list->previous != NULL) list = list->previous;
	return list;
}

Node *list_remove(Node *n)
// Removes a node from a list and frees it (and its data)
{
	Node *previous = n->previous;
	Node *next = n->next;

	if (previous != NULL) previous->next = next;
	if (next != NULL) next->previous = previous;

	free(n->data);
	free(n);

	// Returns the first node in the new list, or NULL if it is empty
	return list_getfirst(previous == NULL ? next : previous);
}

void list_free(Node *list)
{
	if (list == NULL) return;
	while (list != NULL)
	{
		Node *next = list->next;
		list_remove(list);
		list = next;
	}
}

