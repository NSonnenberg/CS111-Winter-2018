#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "SortedList.h"

volatile int s_exclusion = 0;

int s_flag = 0;
int m_flag = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_args
{
	SortedList_t* head;
	SortedListElement_t **element_list;
	int iterations;
	int start_index;
};

void signal_handler(int signal_num)
{
	if (signal_num == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault caught. --catch passed as argument.\n");
		exit(4);
	}
}

void lock()
{
	while (__sync_lock_test_and_set(&s_exclusion, 1)) {}
}

void unlock()
{
	__sync_lock_release(&s_exclusion);
}

char *rand_string(char *str, size_t size)
{
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size) {
        --size;
        size_t n;
        for (n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

char* rand_string_alloc(size_t size)
{
     char *s = malloc(size + 1);
     if (s) {
         rand_string(s, size);
     }
     return s;
}

SortedList_t* initialize_list()
{
	SortedList_t *head = malloc(sizeof(SortedList_t));
	head->next = head;
	head->prev = head;
	head->key = NULL;

	return head;
}

/*
SortedListElement_t** populateList(SortedList_t* head, int num_elements)
{	
	SortedListElement_t **element_list = malloc(sizeof(SortedListElement_t) * num_elements);

	int i;
	for (i = 0; i < num_elements; i++)
	{
		SortedListElement_t* element = malloc(sizeof(SortedListElement_t));
		element->key = rand_string_alloc((rand() % 20) + 1); 
		element_list[i] = element;
	}

	return element_list;
}*/

void free_list(SortedListElement_t **element_list, int num_elements)
{
	int i;
	for (i = 0; i < num_elements; i++)
	{
		char *key = (char *) element_list[i]->key;
		free(key);
		free(element_list[i]);
	}

	free(element_list);
}

void* insert_delete_t(void *args_t)
{
	struct thread_args *args = (struct thread_args *) args_t;
	
	//printf("entered insert_delete_t()\n");
	SortedList_t *head = args->head;
	SortedListElement_t **element_list = args->element_list;
	int iterations = args->iterations;
	int start_index = args->start_index;

	//printf("element_list is %p\n", (void *) element_list);

	//printf("start_index=%d, running until index: %d\n", start_index, (iterations + start_index));

	int i;
	for (i = start_index; i < (iterations + (start_index)); i++)
	{
		if (m_flag)
		{
			pthread_mutex_lock(&mutex);
			SortedList_insert(head, element_list[i]);
			pthread_mutex_unlock(&mutex);
		}

		else if (s_flag)
		{
			lock();
			SortedList_insert(head, element_list[i]);
			unlock();
		}

		else 
		{
			SortedList_insert(head, element_list[i]);
		}
	}

	//printf("##start_index=%d finished inserting\n", start_index);

	int length = SortedList_length(head);
	if (length == -1) 
	{
		fprintf(stderr, "SortedList_length() returned -1: head was null\n");
		exit(2);
	}
	//fprintf(stderr, "length=%d\n", length);

	int j = 0;
	for (j = start_index; j < (iterations + start_index); j++)
	{
		if (m_flag)
		{
			pthread_mutex_lock(&mutex);
			if (SortedList_delete(element_list[j]) != 0)
			{
				fprintf(stderr, "SortedList_delete() returned 1\n");
				exit(2);
			}
			pthread_mutex_unlock(&mutex);
		}

		else if (s_flag)
		{
			lock();
			if (SortedList_delete(element_list[j]) != 0)
			{
				fprintf(stderr, "SortedList_delete() returned 1\n");
				exit(2);
			}
			unlock();
		}

		else
		{
			if (SortedList_delete(element_list[j]) != 0)
			{
				fprintf(stderr, "SortedList_delete() returned 1\n");
				exit(2);
			}
		}
		//printf("SortedList_delete() returned: %d\n", t);
	}

	//length = SortedList_length(head);
	//printf("length=%d\n", length);

	pthread_exit(NULL);
}

int main(int argv, char** argc)
{
	int opt_yield;
	int num_threads = 1;
	int iterations = 1;
	int num_elements = 0;

	char mode[20];
	char *y_flags = NULL;
	char *sync_opt = NULL;

	struct timespec begin, end;
	static struct option long_opts[] =
	{
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	opt_yield = 0;
	strcpy(mode, "list-");

	// initialize empty list
	SortedList_t *head = initialize_list();

	//printf("Running getopt_long()\n");
	int o = 0;
	while (o != -1)
	{
		//printf("entered getopt_long() loop\n");
		o = getopt_long(argv, argc, "tiy", long_opts, NULL);

		switch(o)
		{
			case 't':
				num_threads = (int) atoi(optarg);
				break;
			case 'i':
				iterations = (int) atoi(optarg);
				break;
			case 'y':
				y_flags = optarg;
				break;
			case 's':
				sync_opt = optarg;
				break;
			case '?':
				exit(1);
		}
	}

	signal(SIGSEGV, signal_handler);

	if (y_flags != NULL)
	{
		//printf("entered y_flags\n");
		if (strlen(y_flags) == 0 || strlen(y_flags) > 3)
		{
			fprintf(stderr, "--yield argument invalid\n");
			exit(1);
		}

		unsigned int a;
		for (a = 0; a < strlen(y_flags); a++)
		{
			switch(y_flags[a])
			{
				case 'i':
					opt_yield |= INSERT_YIELD;
					strcat(mode, "i");
					break ;
				case 'd':
					opt_yield |= DELETE_YIELD;
					strcat(mode, "d");
					break;
				case 'l':
					opt_yield |= LOOKUP_YIELD;
					strcat(mode, "l");
					break;
				default:
					fprintf(stderr, "invalid letter passed to --yield\n");
					exit(1);
			}
		}

		strcat(mode, "-");
	}

	else
	{
		strcat(mode, "none-");
	}

	if (sync_opt != NULL)
	{
		switch(sync_opt[0])
		{
			case 'm':
				m_flag = 1;
				strcat(mode, "m");
				break;
			case 's':
				s_flag = 1;
				strcat(mode, "s");
				break;
			default:
				fprintf(stderr, "invalid letter passed to --sync\n");
				exit(1);
		}
	}

	else
	{
		strcat(mode, "none");
	}

	//strcat(mode, "none");

	//printf("got through y_flags\n");

	num_elements = num_threads * iterations;

	SortedListElement_t **element_list = malloc(sizeof(SortedListElement_t) * num_elements);

	//printf("element_list in main=%p num_elements=%d\n", (void *) element_list, num_elements);

	int i;
	for (i = 0; i < num_elements; i++)
	{
		SortedListElement_t* element = malloc(sizeof(SortedListElement_t));
		element->key = rand_string_alloc((rand() % 20) + 1); 
		element_list[i] = element;
	}

	//printf("got through list population\n");

	clock_gettime(CLOCK_REALTIME, &begin);

	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);

	struct thread_args *args = malloc(sizeof(struct thread_args) * num_threads);

	int ii;
	for (ii = 0; ii < num_threads; ii++)
	{
		//printf("executing thread %d\n", ii);
		args[ii].head=head; args[ii].element_list=element_list; args[ii].iterations=iterations;	
		args[ii].start_index = ii * iterations;
		pthread_create(&threads[ii], NULL, insert_delete_t, (void *) &args[ii]);
	}

	int j;
	for (j = 0; j < num_threads; j++)
	{
		pthread_join(threads[j], NULL);
	}

	clock_gettime(CLOCK_REALTIME, &end);

	free_list(element_list, num_elements);

	int length = SortedList_length(head);
	int total_ops = num_threads * iterations * 3;
	long runtime = end.tv_nsec - begin.tv_nsec;
	long avg_ops = runtime / ((long) total_ops);

	fprintf(stderr, "length=%d\n", length);
	printf("%s,%d,%d,1,%d,%ld,%ld\n", 
		mode, num_threads, iterations, total_ops, runtime, avg_ops);

	pthread_exit(NULL);
}