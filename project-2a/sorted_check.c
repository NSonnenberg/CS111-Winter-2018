#include <stdio.h>
#include <stdlib.h>
#include "SortedList.h"

static int NUM_ELEMENTS = 10;

int main()
{
	SortedList_t *head = malloc(sizeof(SortedList_t));
	head->next = head; head->prev = head; head->key = NULL;

	printf("initialized head\n");
	SortedListElement_t **element_list = malloc(sizeof(SortedListElement_t) * NUM_ELEMENTS);
	printf("element_list allocated\n");
	if (element_list == NULL)
	{
		exit(1);
	}

	int i;
	for (i = 0; i < NUM_ELEMENTS; i++)
	{
		SortedListElement_t *element = malloc(sizeof(SortedListElement_t));
		//printf("allocated list element %d\n", i);
		element->key = "ab";
		//printf("set key\n");
		element_list[i] = element;
		//printf("placed element in list\n");
	}

	printf("initialized list\n");

	int n;
	for (n = 0; n < NUM_ELEMENTS; n++)
	{
		SortedList_insert(head, element_list[n]);
	}

	int length =  SortedList_length(head);
	printf("length = %d\n", length);

	int j;
	for (j = 0; j < NUM_ELEMENTS; j++)
	{
		int delete_ret = SortedList_delete(element_list[j]);
		printf("delete returned: %d\n", delete_ret);
	}

	int length_end = SortedList_length(head);
	printf("end length = %d\n", length_end);

	int k;
	for (k = 0; k < NUM_ELEMENTS; k++)
	{
		free(element_list[i]);
	}

	free(element_list);
	free(head);

	return 0;
}