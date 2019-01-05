# NAME: Jessie Liu,Neil Sonnenberg
# EMAIL: jessie.qy.liu@gmail.com,sonnenbergneil@gmail.com
# ID: 204791822,204748664

CC=gcc
CFLAGS= -Wall -Wextra

build:
	$(CC) $(CFLAGS) -o lab3a lab3a.c

.PHONY: clean dist


dist:
	tar -czf lab3a-204791822.tar.gz lab3a.c Makefile README ext2_fs.h


clean:
	rm -rf lab3a lab3a-204791822.tar.gz
