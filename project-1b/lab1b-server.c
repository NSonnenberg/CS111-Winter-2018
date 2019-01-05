#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "zlib.h"

#define STDIN 0
#define STDOUT 1
#define true 1
#define false 0

static int BUFFER_LIMIT = 256;

void create_pipe(int fd[2])
{
	if (pipe(fd) == -1)
	{
		fprintf(stderr, "Error creating pipe\n");
		exit(1);
	}
}


int main(int argc, char** argv)
{
	//char in[BUFFER_LIMIT];
	//char out[BUFFER_LIMIT];
	int port = -1;
	int pipe1_fd[2];
	int pipe2_fd[2];

	int compress_flag = false;
	
	unsigned char c_in[BUFFER_LIMIT];
	unsigned char c_out[BUFFER_LIMIT];

	int listenfd, commfd;

	struct sockaddr_in server;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	static struct option long_opts[] = 
	{
		{"port", required_argument, 0, 'p'},
		{"compress", no_argument, 0, 'c'},
		{0, 0, 0, 0}
	};

	int c = 0;
	while (c != -1)
	{
		int option_index = 0;

		c = getopt_long(argc, argv, "pc", long_opts, &option_index);
		switch(c)
		{
			case 'p':
				port = atoi(optarg);
				break;
			case 'c':
				compress_flag = true;
				break;
			case '?':
				exit(1);
		}
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	if (port == -1)
	{
		fprintf(stderr, "Error: port option was not specified\n");
		exit(1);
	}
	server.sin_port = port;

	bind(listenfd, (struct sockaddr *) &server, sizeof(server));

	listen(listenfd, 3);

	printf("Waiting for client...\n");
	commfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
	printf("Client found\n");

	// pipe1_fd = parent to child
	// pipe2_fd = child to parent
	create_pipe(pipe1_fd);
	create_pipe(pipe2_fd);

	char * buf1 = (char *) malloc(BUFFER_LIMIT * sizeof(char));
	char * buf2 = (char *) malloc(BUFFER_LIMIT * sizeof(char));

	int pid = fork();

	if (pid == -1)
	{
		fprintf(stderr, "Error forking processes\n");
		exit(1);
	}
	if (pid == 0)
	{
		close(pipe1_fd[1]);
		close(pipe2_fd[0]);
		dup2(pipe1_fd[0], STDIN);
		dup2(pipe2_fd[1], STDOUT);
		dup2(pipe2_fd[1], 2);
		close(pipe1_fd[0]);
		close(pipe2_fd[1]);

		if (execl("/bin/bash", "bash", NULL) == -1)
		{
			fprintf(stderr, "Error creating shell\n");
			exit(1);
		}
	}
	else 
	{ 
		close(pipe1_fd[0]);
		close(pipe2_fd[1]);

		struct pollfd pin = {commfd, POLLIN | POLLHUP | POLLERR, 0};
		struct pollfd pout = {pipe2_fd[0], POLLIN | POLLHUP | POLLERR, 0};

		struct pollfd fds[2] = {pin, pout};
		int num_bytes1, num_bytes2;
		while(true)
		{
			poll(fds, 2, 0);
			
			if (fds[0].revents == POLLIN)
			{
				num_bytes1 = read(commfd, buf1, BUFFER_LIMIT);

				if (num_bytes1 < 0)
				{
					fprintf(stderr, "Error reading bytes: %s\n", strerror(errno));
					exit(1);
				}

				if (compress_flag)
				{
					z_stream stdin2shell;

					stdin2shell.zalloc = Z_NULL;
					stdin2shell.zfree = Z_NULL;
					stdin2shell.opaque = Z_NULL;

					inflateInit(&stdin2shell);


					//fprintf(stderr, "left loop with k\n");					
					stdin2shell.avail_in = num_bytes1;
					stdin2shell.next_in = (unsigned char *) buf1;
					stdin2shell.avail_out = BUFFER_LIMIT;
					stdin2shell.next_out = c_in;

					do {
						//fprintf(stderr, "time to die %d\n", stdin2shell.avail_in);
						inflate(&stdin2shell, Z_SYNC_FLUSH);
					} while (stdin2shell.avail_in > 0);

					int diff = BUFFER_LIMIT - stdin2shell.avail_out;

					int k = 0;
					while (k < diff)
					{
						//fprintf(stderr, "in for loop iteration k=%d\n", k);
						if (c_in[k] == '\r' || c_in[k] == '\n')
						{
							c_in[k] = '\n';
						}

						k++;
					}

					write(pipe1_fd[1], c_in, diff);

					fprintf(stderr, "Sent %d compressed bytes to shell: %s\n", diff, c_in);
					inflateEnd(&stdin2shell);
				} 

				else 
				{
					int i = 0;
					while (i < num_bytes1)
					{
						if (buf1[i] == '\r' || buf1[i] == '\n')
						{
							//char temp[2] = "\r\n";
							char lf = '\n';
							//write(STDOUT, temp, 2);
							write(pipe1_fd[1], &lf, 1);
							i++;
							continue;
						}

						if (buf1[i] == 3)
						{
							//char temp[4] = "\n^C\n";
							//write(STDOUT, temp, 4);
							kill(pid, SIGINT);
						}

						if (buf1[i] == 4)
						{
							//char temp[4] = "\n^D\n";
							//write(STDOUT, temp, 4);
							close(pipe1_fd[1]);

						}

						//write(STDOUT, buf1 + i, 1);
						write(pipe1_fd[1], buf1 + i, 1);
						i++;
					}
				}
			}

			if (fds[1].revents == POLLIN)
			{
				//fprintf(stderr, "whats up\n");
				num_bytes2 = read(pipe2_fd[0], buf2, BUFFER_LIMIT);

				if (num_bytes2 < 0)
				{
					fprintf(stderr, "Error reading bytes: %s\n", strerror(errno));
					exit(1);
				}

				if (compress_flag)
				{
					z_stream shell2stdout;

					shell2stdout.zalloc = Z_NULL;
					shell2stdout.zfree = Z_NULL;
					shell2stdout.opaque = Z_NULL;

					deflateInit(&shell2stdout, Z_DEFAULT_COMPRESSION);

					//fprintf(stderr, "compress read from shell\n");
					shell2stdout.avail_in = num_bytes2;
					shell2stdout.next_in = (unsigned char *) buf2;
					shell2stdout.avail_out = BUFFER_LIMIT;
					shell2stdout.next_out = c_out;

					do {
						deflate(&shell2stdout, Z_SYNC_FLUSH);
					} while (shell2stdout.avail_in > 0);

					fprintf(stderr, "Sent %d compressed bytes to client\n", BUFFER_LIMIT - shell2stdout.avail_out);
					write(commfd, c_out, BUFFER_LIMIT - shell2stdout.avail_out);

					deflateEnd(&shell2stdout);
				}

				else 
				{
					int j = 0;
					while (j < num_bytes2)
					{
						if (buf2[j] == '\n')
						{
							char temp[2] = "\r\n";
							write(commfd, temp, 2);
							j++;
							continue;
						}

						write(commfd, buf2 + j, 1);
						j++;
					}
				}
			}

			if (fds[1].revents == POLLHUP || fds[1].revents == POLLERR)
			{
				close(pipe2_fd[0]);
				int status = 0;
				waitpid(pid, &status, 0);

				fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", status&0x007f, ((status&0xff00) >> 8));
				break;
			}
		}
	}

	
	free(buf1);
	free(buf2);
	exit(0);
}

