# Notes Project 2A

```c
int main() 
{
	struct timespec begin, end;
	volatile int count;
	count = 0;
	int i;
	clock_gettime(CLOACK_REALTIME, begin);
	for (i = 0; i < 10000000; i++)
	{
		count++;
	}


}
```

```
volatile int x = 0;
int main() 
{
	int y = __sync_fetch_and_add(&x, 1);
	printf("X's value is %d\n", x);
	printf("Y is ")
}
```

```
void *PrintHello(void *id)
{
	long threadid;
	threadid = (long) id;
	printf("I'm thread #%ld\n", threadid);
	pthread_exit(NULL);
}

int main() 
{
	pthread_t thread[5];
	long i;
	for (i = 0; i < 5; i++)
	{
		pthread_create(&thread[i], NULL, PrintHello, (void * ) i);
	}

	pthread_exit(NULL); // NECESSARY otherwise it won't work
}
```

COMPILE WITH -lpthread IN GCC OTHERWISE IT WON'T WORK

