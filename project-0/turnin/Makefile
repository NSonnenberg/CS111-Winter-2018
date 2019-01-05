# NAME: Neil Sonnenberg
# EMAIL: sonnenbergneil@gmail.com
# ID: 204748664

default: build

build: lab0.c
	gcc -o lab0 lab0.c -g -Wall -Wextra

clean: lab0
	rm lab0 *.tar.gz

check: build all_tests

dist: build
	tar -czf lab0-204748664.tar.gz lab0.c Makefile README *.png

default_test:
	@echo "Testing default case";
	echo "Hello World" > testin.txt;
	cat testin.txt | ./lab0 > testout.txt;
	cmp testin.txt testout.txt; \
	if [[ $$? != 0 ]]; then \
	echo "Test Failed: Error information in log.txt"; \
	echo "Error: STDOUT differed from STDIN" >> log.txt; \
	else \
	echo "Test Passed"; \
	fi; \
	rm -f testin.txt testout.txt

invalid_arg_test:
	@echo "Testing invalid argument";
	./lab0 --apple &> /dev/null; \
	if [[ $$? != 1 ]]; then \
	echo "Test Failed: Error information in log.txt"; \
	echo "Error: Program did not exit with correct error code" >> log.txt; \
	else \
	echo "Test Passed"; \
	fi

open_test:
	@echo "Testing --input error code";
	echo "Hello World" > testin.txt; \
	chmod u-r testin.txt; \
	./lab0 --input=testin.txt &> /dev/null; \
	if [[ $$? != 2 ]]; then \
	echo "Test Failed: Error information in log.txt"; \
	echo "Error: File is not readable. Program did not catch." >> log.txt; \
	else \
	echo "Test Passed"; \
	fi; \
	rm -f testin.txt

creat_test:
	@echo "Testing --output error code";
	touch testout.txt;
	chmod u-w testout.txt;
	./lab0 --output=testout.txt &>/dev/null; \
	if [[ $$? != 3 ]]; then \
	echo "Test Failed: Error information in log.txt"; \
	echo "Error: File is not writeable. Program did not catch." >> log.txt; \
	else \
	echo "Test Passed"; \
	fi; \
	rm -f testout.txt

input_output_test:
	@echo "Testing --input and --output";
	echo "Hello World" > testin.txt; \
	./lab0 --input=testin.txt --output=testout.txt; \
	cmp testin.txt testout.txt; \
	if [[ $$? != 0 ]]; then \
	echo "Test Failed: Error information in log.txt\n"; \
	echo "Error: output file differs from input file" >> log.txt; \
	else \
	echo "Test Passed"; \
	fi; \
	rm -f testin.txt testout.txt

segfault_catch_test:
	@echo "Testing --segfault and --catch";
	./lab0 --segfault --catch; \
	if [[ $$? != 4 ]]; then \
	echo "Test Failed: Error information in log.txt"; \
	echo "Error: SIGSEGV not caught" >> log.txt; \
	else \
	echo "Test Passed"; \
	fi

all_tests: default_test invalid_arg_test open_test creat_test input_output_test segfault_catch_test
