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

long num_lock_ops = 0;
long lock_acq_time = 0;

struct thread_args
{
	SortedList_t** lists;
	SortedListElement_t **element_list;
	int iterations;
	int start_index;
	int num_lists;
};

void signal_handler(int signal_num)
{
	if (signal_num == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault caught. --catch passed as argument.\n");
		exit(4);
	}
}

int getHash(const char* source)
{
	int hash = 0;
	int i = 0;
	while (source[i])
	{
		int a = source[i] - '0';
		hash = (hash * 10) + a;
		i++;
	}

	if (hash < 0) { hash *= -1; }

	return hash;
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

SortedList_t** initialize_list(int num_lists)
{
	SortedList_t **heads = malloc(sizeof(SortedList_t) * num_lists);

	int i;
	for (i = 0; i < num_lists; i++)
	{
		heads[i] = malloc(sizeof(SortedList_t));
		heads[i]->next = heads[i];
		heads[i]->prev = heads[i];
		heads[i]->key = NULL;
	}

	return heads;
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
	struct timespec t_begin_i, t_begin_d, t_end_i, t_end_d;
	struct thread_args *args = (struct thread_args *) args_t;
	
	//printf("entered insert_delete_t()\n");
	SortedList_t **lists = args->lists;
	SortedListElement_t **element_list = args->element_list;
	int iterations = args->iterations;
	int start_index = args->start_index;
	int num_lists = args->num_lists;

	//printf("element_list is %p\n", (void *) element_list);

	//printf("start_index=%d, running until index: %d\n", start_index, (iterations + start_index));

	int i;
	for (i = start_index; i < (iterations + (start_index)); i++)
	{
		//printf("creating hash with %s num_lists=%d\n", element_list[i]->key, num_lists);
		int hash = getHash(element_list[i]->key);
		int insert_index = hash % num_lists;
		//printf("made hash index=%d, hash=%d\n", insert_index, hash);
		if (m_flag)
		{
			clock_gettime(CLOCK_REALTIME, &t_begin_i);
			pthread_mutex_lock(&mutex);
			clock_gettime(CLOCK_REALTIME, &t_end_i);
			lock_acq_time += ((1000000000 * (t_end_i.tv_sec - t_begin_i.tv_sec)) + (t_end_i.tv_nsec - t_begin_i.tv_nsec));
			num_lock_ops++;

			SortedList_insert(lists[insert_index], element_list[i]);

			pthread_mutex_unlock(&mutex);
		}

		else if (s_flag)
		{
			clock_gettime(CLOCK_REALTIME, &t_begin_i);
			lock();
			clock_gettime(CLOCK_REALTIME, &t_end_i);
			lock_acq_time += ((1000000000 * (t_end_i.tv_sec - t_begin_i.tv_sec)) + (t_end_i.tv_nsec - t_begin_i.tv_nsec));
			num_lock_ops++;

			SortedList_insert(lists[insert_index], element_list[i]);

			unlock();
		}

		else 
		{
			SortedList_insert(lists[insert_index], element_list[i]);
		}
	}

	//printf("##start_index=%d finished inserting\n", start_index);

	int k;
	for (k = 0; k < num_lists; k++)
	{
		int length = SortedList_length(lists[k]);
		if (length == -1) 
		{
			fprintf(stderr, "SortedList_length() returned -1: head was null\n");
			exit(2);
		}
		//fprintf(stderr, "length=%d\n", length);
	}
	
	int j = 0;
	for (j = start_index; j < (iterations + start_index); j++)
	{
		if (m_flag)
		{
			clock_gettime(CLOCK_REALTIME, &t_begin_d);
			pthread_mutex_lock(&mutex);
			clock_gettime(CLOCK_REALTIME, &t_end_d);
			lock_acq_time += ((1000000000 * (t_end_d.tv_sec - t_begin_d.tv_sec)) + (t_end_d.tv_nsec - t_begin_d.tv_nsec));
			num_lock_ops++;

			if (SortedList_delete(element_list[j]) != 0)
			{
				fprintf(stderr, "SortedList_delete() returned 1\n");
				exit(2);
			}
			pthread_mutex_unlock(&mutex);
		}

		else if (s_flag)
		{
			clock_gettime(CLOCK_REALTIME, &t_begin_d);
			lock();
			clock_gettime(CLOCK_REALTIME, &t_end_d);
			lock_acq_time += ((1000000000 * (t_end_d.tv_sec - t_begin_d.tv_sec)) + (t_end_d.tv_nsec - t_begin_d.tv_nsec));
			num_lock_ops++;

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

	//length = SortedList_length(lists[0]);
	//printf("length=%d\n", length);

	pthread_exit(NULL);
}

int main(int argv, char** argc)
{
	int opt_yield;
	int num_threads = 1;
	int iterations = 1;
	int num_elements = 0;
	int num_lists = 1;
	int length = 0;

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
		{"lists", required_argument, 0, 'l'},
		{0, 0, 0, 0}
	};

	opt_yield = 0;
	strcpy(mode, "list-");

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
			case 'l':
				num_lists = (int) atoi(optarg);
				break;
			case '?':
				exit(1);
		}
	}


	// initialize empty list
	SortedList_t **lists = initialize_list(num_lists);

	//signal(SIGSEGV, signal_handler);

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
		args[ii].lists=lists; args[ii].element_list=element_list; args[ii].iterations=iterations;	
		args[ii].start_index = ii * iterations;
		args[ii].num_lists = num_lists;
		pthread_create(&threads[ii], NULL, insert_delete_t, (void *) &args[ii]);
	}

	int j;
	for (j = 0; j < num_threads; j++)
	{
		pthread_join(threads[j], NULL);
	}

	//printf("joined threads\n");
	clock_gettime(CLOCK_REALTIME, &end);

	free_list(element_list, num_elements);

	//printf("getting %d list length(s)\n", num_lists);
	int k;
	for (k = 0; k < num_lists; k++)
	{
		length += SortedList_length(lists[k]);
	}

	int total_ops = num_threads * iterations * 3;
	long runtime = (1000000000 * (end.tv_sec - begin.tv_sec)) +  (end.tv_nsec - begin.tv_nsec);
	long avg_ops = runtime / ((long) total_ops);
	long wait_for_lock = 1;
	if (s_flag || m_flag) {wait_for_lock = lock_acq_time / num_lock_ops;}

	fprintf(stderr, "length=%d\n", length);
	printf("%s,%d,%d,%d,%d,%ld,%ld,%ld\n", 
		mode, num_threads, iterations, num_lists, total_ops, runtime, avg_ops, wait_for_lock);

	pthread_exit(NULL);
}