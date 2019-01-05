default: 
	gcc -o lab4c_tcp lab4c_tcp.c -lm -lmraa -Wall -Wextra
	gcc -o lab4c_tls lab4c_tls.c -lm -lmraa -lssl -lcrypto -Wall -Wextra

dist: 
	tar -czf lab4c-204748664.tar.gz lab4c_tls.c lab4c_tcp.c README Makefile

clean:
	rm -rf *.tar.gz lab4c_tcp lab4c_tls