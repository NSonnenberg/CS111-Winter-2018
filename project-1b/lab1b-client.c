#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "zlib.h"

#define STDIN 0
#define STDOUT 1
#define true 1
#define false 0

static int BUFFER_LIMIT = 256;

int main(int argc, char** argv)
{
	struct termios oldtio, newtio;
	int socketfd;
	char in[BUFFER_LIMIT];
	char out[BUFFER_LIMIT];
	int port = -1;

	char* log_file = NULL;
	int log_flag = false;
	int log_fd;

	int compress_flag = false;
	
	unsigned char c_in[BUFFER_LIMIT];
	unsigned char c_out[BUFFER_LIMIT];

	struct sockaddr_in client;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	static struct option long_opts[] = 
	{
		{"port", required_argument, 0, 'p'},
		{"log", required_argument, 0 , 'l'},
		{"compress", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	int c = 0;
	while (c != -1)
	{
		int option_index = 0;

		c = getopt_long(argc, argv, "plc", long_opts, &option_index);
		switch(c)
		{
			case 'p':
				port = atoi(optarg);
				break;
			case 'l':
				log_flag = true;
				log_file = optarg;
				break;
			case 'c':
				compress_flag = true;
				break;
			case '?':
				exit(1);
		}
	}

	if (log_flag)
	{	
		log_fd = creat(log_file, 0666);
		if (log_fd < 0)
		{
			fprintf(stderr, "Error: Failed to create log file: %s\n", strerror(errno));
			//tcsetattr(STDIN, TCSANOW, &oldtio);
			exit(1);
		}
	}

	tcgetattr(STDIN, &oldtio);
	newtio = oldtio;

	newtio.c_iflag = ISTRIP;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	if (tcsetattr(STDIN, TCSANOW, &newtio) < 0)
	{
		fprintf(stderr, "Error setting terminal settings\n");
		exit(1);
	}

	client.sin_addr.s_addr = inet_addr("127.0.0.1");
	client.sin_family = AF_INET;
	if (port == -1)
	{
		fprintf(stderr, "Error: port option was not specified\n");
		tcsetattr(STDIN, TCSANOW, &oldtio);
		exit(1);
	}
	client.sin_port = port;

	int conn = connect(socketfd, (struct sockaddr *) &client, sizeof(client));
	if (conn == -1)
	{
		fprintf(stderr, "Error connecting to the server: %s\n", strerror(errno));
		tcsetattr(STDIN, TCSANOW, &oldtio);
		exit(1);
	}

	struct pollfd pin = {STDIN, POLLIN | POLLHUP | POLLERR, 0};
	struct pollfd pout = {socketfd, POLLIN | POLLHUP | POLLERR, 0};

	struct pollfd fds[2] = {pin, pout};	

	int numbytes1, numbytes2;
	while(true)
	{
		poll(fds, 2, 0);

		if (fds[0].revents == POLLIN)
		{
			// to send to server
			numbytes1 = read(STDIN, out, BUFFER_LIMIT);
			int diff;

			if (compress_flag)
			{
				z_stream stdin2shell;

				stdin2shell.zalloc = Z_NULL;
				stdin2shell.zfree = Z_NULL;
				stdin2shell.opaque = Z_NULL;

				deflateInit(&stdin2shell, Z_DEFAULT_COMPRESSION);

				stdin2shell.avail_in = numbytes1;
				stdin2shell.next_in = (unsigned char *) out;
				stdin2shell.avail_out = BUFFER_LIMIT;
				stdin2shell.next_out = c_out;

				do {
					deflate(&stdin2shell, Z_SYNC_FLUSH);
				} while (stdin2shell.avail_in > 0);

				diff = BUFFER_LIMIT - stdin2shell.avail_out;

				if (write(socketfd, c_out, BUFFER_LIMIT - stdin2shell.avail_out) < 0)
				{
					fprintf(stderr, "Failed to write to server: %s\n", strerror(errno));
					tcsetattr(STDIN, TCSANOW, &oldtio);
					exit(1);
				}

				deflateEnd(&stdin2shell);
				//fprintf(stderr, "\r\nWrote %d compressed bytes to server\r\n", BUFFER_LIMIT - stdin2shell.avail_out);				
			}

			int j = 0;
			while (j < numbytes1)
			{
				if (out[j] == '\r' || out[j] == '\n')
				{
					char temp[2] = "\r\n";
					write(STDOUT, temp, 2);
					if (!compress_flag) { write(socketfd, temp, 2); }
					j++;
					continue;
				}
				write(STDOUT, out + j, 1);
				if (!compress_flag) { write(socketfd, out + j, 1); }
				j++;
			}

			if (log_flag && numbytes1 > 0)
			{
				char log[BUFFER_LIMIT + 15];
				int numdigits = 0;
				int t = numbytes1;
				while (t != 0)
				{
					numdigits++;
					t /= 10;
				}
				if (compress_flag) {
					sprintf(log, "SENT %d bytes: %s \n", diff, c_out);
				} else {
					sprintf(log, "SENT %d bytes: %s \n", numbytes1, out);
				}
				if (write(log_fd, log, 14 + numdigits + numbytes1) < 0)
				{
					fprintf(stderr, "Failed to write to log file: %s\n", strerror(errno));
					tcsetattr(STDIN, TCSANOW, &oldtio);
					exit(1);
				}
			}
		}


		if (fds[1].revents == POLLIN)
		{
			// receive from server
			numbytes2 = read(socketfd, in, BUFFER_LIMIT);
			//int diff;

			if (numbytes2 == 0)
			{
				close(socketfd);
				break;
			}

			if (compress_flag)
			{
				z_stream shell2stdout;

				shell2stdout.zalloc = Z_NULL;
				shell2stdout.zfree = Z_NULL;
				shell2stdout.opaque = Z_NULL;

				inflateInit(&shell2stdout);

				shell2stdout.avail_in = numbytes2;
				shell2stdout.next_in = (unsigned char *) in;
				shell2stdout.avail_out = BUFFER_LIMIT;
				shell2stdout.next_out = c_in;

				do {
					inflate(&shell2stdout, Z_SYNC_FLUSH);
				} while (shell2stdout.avail_in > 0);

				//diff = BUFFER_LIMIT - shell2stdout.avail_out;

				write(STDOUT, c_in, BUFFER_LIMIT - shell2stdout.avail_out);
				inflateEnd(&shell2stdout);
			}

			else 
			{
				int i = 0;
				while (i < numbytes2)
				{
					write(STDOUT, in + i, 1);
					i++;
				}
			}

			if (log_flag && numbytes2 > 0)
			{
				fprintf(stderr, "entered receive log\r\n");
				char log[BUFFER_LIMIT + 20];
				int n;
				if (compress_flag) {
					n = sprintf(log, "RECEIVED %d bytes: %s \n", numbytes2, in);
				} else {
					n = sprintf(log, "RECEIVED %d bytes: %s \n", numbytes2, in);
				}
				if (write(log_fd, log, n) < 0)
				{
					fprintf(stderr, "Error writing to log file: %s\n", strerror(errno));
					tcsetattr(STDIN, TCSANOW, &oldtio);
					exit(1);
				}
			}
		}
		
	}

	if (log_flag)
	{
		close(log_fd);
	}

	
	

	tcsetattr(STDIN, TCSANOW, &oldtio);
	exit(0);	
}