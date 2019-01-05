#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>

#define true 1
#define false 0

#define NONE 0
#define MUTEX 1
#define SPIN_LOCK 2
#define C_AND_S 3

volatile int s_exclusion = 0;

int opt_yield;
int mode;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_args
{
	long long *pointer;
	long long value;
	long it;
};

void lock()
{
	while (__sync_lock_test_and_set(&s_exclusion, 1)) {}
}

void unlock()
{
	__sync_lock_release(&s_exclusion);
}

void add(long long *pointer, long long value) 
{
    long long sum = *pointer + value;
    if (opt_yield)
    {
    	sched_yield();
    }
    *pointer = sum;
}

void add_m(long long *pointer, long long value)
{
	pthread_mutex_lock(&mutex);
	long long sum = *pointer + value;
	if (opt_yield)
	{
		sched_yield();
	}
	*pointer = sum;
	pthread_mutex_unlock(&mutex);
}

void add_s(long long *pointer, long long value) 
{
	lock();
    long long sum = *pointer + value;
    if (opt_yield)
    {
    	sched_yield();
    }
    *pointer = sum;
    unlock();
}

void add_c(long long *pointer, long long value) 
{
	long long old;
	long long compare;

    do {
    	old = *pointer;
    	long long sum = *pointer + value;
	    if (opt_yield)
	    {
	    	sched_yield();
	    }

	    compare = __sync_val_compare_and_swap(&old, *pointer, sum);
	} while (compare != *pointer);
}

void * thread_add(void *args)
{
	struct thread_args *t_argp = (struct thread_args *) args;

	long l;
	for (l = 0; l < t_argp->it; l++)
	{
		if (mode == MUTEX)
		{
			add_m(t_argp->pointer, t_argp->value);
		} 
		else if (mode == SPIN_LOCK)
		{
			add_s(t_argp->pointer, t_argp->value);
		}
		else if (mode == C_AND_S)
		{
			add_c(t_argp->pointer, t_argp->value);
		}
		else
		{
			add(t_argp->pointer, t_argp->value);
		}
	}

	//printf("count is: %ld\n", *(t_argp->pointer));
	pthread_exit(NULL);
}


int main(int argv, char** argc)
{
	char *none = "add-none";
	char *yield = "add-yield-none";
	char *m = "add-m";
	char *m_yield = "add-yield-m";
	char *s = "add-s";
	char *s_yield = "add-yield-s";
	char *c = "add-c";
	char *c_yield = "add-yield-c";

	struct timespec begin, end;
	static struct option long_opts[] =
	{
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", no_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int sync_flag = 0;
	long long count = 0;
	long num_threads = 0;
	long iterations = 0;
	char* sync_opt;
	char** field = NULL;
	opt_yield = 0;
	mode = NONE;

	clock_gettime(CLOCK_REALTIME, &begin);

	int o = 0;
	while (o != -1)
	{
		o = getopt_long(argv, argc, "tisy", long_opts, NULL);

		switch(o)
		{
			case 't':
				num_threads = (long) atoi(optarg);
				break;
			case 'i':
				iterations = (long) atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				sync_flag = 1;
				sync_opt = optarg;
				break;
			case '?':
				exit(1);
		}
	}

	field = opt_yield ? &yield : &none;

	if (sync_flag)
	{
		switch (sync_opt[0])
		{
			case 'm':
				mode = MUTEX;
				field = opt_yield ? &m_yield : &m;
				break;
			case 's':
				mode = SPIN_LOCK;
				field = opt_yield ? &s_yield : &s;
				break;
			case 'c':
				mode = C_AND_S;
				field = opt_yield ? &c_yield : &c;
				break;
			default:
				fprintf(stderr, "--sync must be passed m, s, or c\n");
				exit(2);
		}
	}

	//printf("mode: %d\n", mode);
	struct thread_args args1;
	args1.pointer = &count; args1.value = 1; args1.it = iterations;
	struct thread_args args2;
	args2.pointer = &count; args2.value = -1; args2.it = iterations;

	pthread_t *threads1 = malloc(num_threads*sizeof(pthread_t));
	pthread_t *threads2 = malloc(num_threads*sizeof(pthread_t)); 

	long i;
	for (i = 0; i < num_threads; i++)
	{
		pthread_create(&threads1[i], NULL, thread_add, (void *) &args1);
	}

	long ii;
	for (ii = 0; ii < num_threads; ii++)
	{
		pthread_create(&threads2[ii], NULL, thread_add, (void *) &args2);
	}
	
	long j;
	for (j = 0; j < num_threads; j++)
	{
		pthread_join(threads1[j], NULL);
	}

	long jj;
	for (jj = 0; jj < num_threads; jj++)
	{
		pthread_join(threads2[jj], NULL);
	}

	clock_gettime(CLOCK_REALTIME, &end);

	long runtime = end.tv_nsec - begin.tv_nsec;
	long totops = num_threads * iterations * 2;
	long avgops = runtime / totops;

	printf("%s,%ld,%ld,%ld,%ld,%ld,%lld\n",
		*field, num_threads, iterations, totops, runtime, avgops, count);

	free(threads1);
	free(threads2);
	pthread_exit(NULL);
}