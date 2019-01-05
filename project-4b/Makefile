default: build

build: 
	gcc -lmraa -lm -o lab4b lab4b.c -Wall -Wextra

check: build
	@{ echo "LOG"; sleep 2; echo "OFF"; sleep 2; } | ./lab4b --log=temp
	@grep -q LOG temp || [ $$? -eq 0 ]
	rm -f temp
	@echo "Test Passed"	

dist:
	tar -czf lab4b-204748664.tar.gz lab4b.c Makefile README

clean:
	rm -f *.tar.gz lab4b 	

