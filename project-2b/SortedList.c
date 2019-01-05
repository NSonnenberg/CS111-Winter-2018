#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "SortedList.h"

int opt_yield;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	//printf("entered insert()\n");
	if (list == NULL || element == NULL || element->key == NULL)
	{
		return;
	}

	//printf("about to initialize curr\n");

	SortedListElement_t *curr = list->next;

	//printf("initialized curr\n");

	while (curr != list)
	{
		//printf("entered while loop\n");
		if (strcmp(element->key, curr->key) > 0)
		{
			break;
		}

		curr = curr->next;
	}

	if (opt_yield & INSERT_YIELD) sched_yield();

	//printf("starting pointer redirection\n");

	element->prev = curr->prev;
	element->next = curr;
	curr->prev->next = element;
	curr->prev = element;

	//printf("finished pointer redirection\n");
}

int SortedList_delete(SortedListElement_t *element)
{
	if (element == NULL || element->next->prev != element || element->prev->next != element)
	{
		return 1;
	}

	if (element->prev != NULL && element->next != NULL)
	{
		if (opt_yield & DELETE_YIELD) sched_yield();

		element->prev->next = element->next;
		element->next->prev = element->prev;

		return 0;
	}

	return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	if (list == NULL || key == NULL)
	{
		return NULL;
	}

	SortedListElement_t *curr = list;

	while (curr->next != list)
	{
		if (curr->key != NULL && strcmp(curr->key, key) == 0)
		{
			return curr;
		}

		if (opt_yield & LOOKUP_YIELD) sched_yield();

		curr = curr->next;
	}

	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	if (list == NULL)
	{
		return -1;
	}

	int count = 0;
	SortedListElement_t *curr = list->next;

	while (curr != list)
	{
		count++;

		if (opt_yield & LOOKUP_YIELD) sched_yield();

		curr = curr->next;
	}

	return count;	
}