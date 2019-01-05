#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define false 0
#define true 1
#define STDIN 0
#define STDOUT 1

static int BUFFER_LIMIT = 256;

void read_write(int read_fd, int write_fd, char* buffer)
{
	int num_bytes;

	while(true)
	{
		num_bytes = read(read_fd, buffer, BUFFER_LIMIT);
		int exit_flag = false;

		if (num_bytes < 0)
		{
			fprintf(stderr, "Error reading bytes: %s\n", strerror(errno));
			exit(1);
		}

		int i = 0;
		while (i < num_bytes)
		{
			if (buffer[i] == '\r' || buffer[i] == '\n')
			{
				char temp[2] = "\r\n";
				write(write_fd, temp, 2);
				i++;
				continue;
			}

			if (buffer[i] == 4)
			{
				exit_flag = true;
				break;	
			}

			write(write_fd, buffer + i, 1);
			i++;
		}

		if (exit_flag)
		{
			break;
		}
	}
}

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
	struct termios oldtio, newtio;
	int shell_flag = false;
	int pipe1_fd[2];
	int pipe2_fd[2];

	

	char* buffer = (char*) malloc(BUFFER_LIMIT * sizeof(char));
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

	if (shell_flag)
	{
		printf("in shell argument check\n");
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

			struct pollfd pin = {STDIN, POLLIN | POLLHUP | POLLERR, 0};
			struct pollfd pout = {pipe2_fd[0], POLLIN | POLLHUP | POLLERR, 0};

			struct pollfd fds[2] = {pin, pout};

			int num_bytes1, num_bytes2;

			while(true)
			{
				poll(fds, 2, 0);
				
				if (fds[0].revents == POLLIN)
				{
					num_bytes1 = read(STDIN, buf1, BUFFER_LIMIT);

					if (num_bytes1 < 0)
					{
						fprintf(stderr, "Error reading bytes: %s\n", strerror(errno));
						exit(1);
					}

					int i = 0;
					while (i < num_bytes1)
					{
						if (buf1[i] == '\r' || buf1[i] == '\n')
						{
							char temp[2] = "\r\n";
							char lf = '\n';
							write(STDOUT, temp, 2);
							write(pipe1_fd[1], &lf, 1);
							i++;
							continue;
						}

						if (buf1[i] == 3)
						{
							char temp[4] = "\n^C\n";
							write(STDOUT, temp, 4);
							kill(pid, SIGINT);
						}

						if (buf1[i] == 4)
						{
							char temp[4] = "\n^D\n";
							write(STDOUT, temp, 4);
							close(pipe1_fd[1]);

						}

						write(STDOUT, buf1 + i, 1);
						write(piputtpe1_fd[1], buf1 + i, 1);
						i++;
					}
				}

				if (fds[1].revents == POLLIN)
				{
					num_bytes2 = read(pipe2_fd[0], buf2, BUFFER_LIMIT);

					if (num_bytes2 < 0)
					{
						fprintf(stderr, "Error reading bytes: %s\n", strerror(errno));
						exit(1);
					}

					int j = 0;
					while (j < num_bytes2)
					{
						if (buf2[j] == '\n')
						{
							char temp[2] = "\r\n";
							write(STDOUT, temp, 2);
							j++;
							continue;
						}

						write(STDOUT, buf2 + j, 1);
						j++;
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

			free(buf1);
			free(buf2);

		}


	}

	else
	{
		read_write(STDIN, STDOUT, buffer);
	}

	tcsetattr(STDIN, TCSANOW, &oldtio);
	free(buffer);

	exit(0);
}