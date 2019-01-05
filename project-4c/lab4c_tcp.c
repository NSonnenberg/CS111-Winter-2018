#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <mraa/gpio.h>
#include <mraa/aio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>

#define STDIN 0
#define STDOUT 1

static int BUFFER_LIMIT = 256;

sig_atomic_t volatile run_flag = 1;

const int B = 4275;
const int R0 = 100000;

void do_when_interrupted()
{
	run_flag = 0;
}

int main(int argv, char** argc)
{
	time_t curr_time;
	struct tm *info;
	int socketfd;

	uint16_t value;

	int log_fd;

	int report_flag = 1;
	int log_flag = 0;
	int f_flag = 1;
	int period = 1;

	int id = 0;
	int port = 0;

	char log_str[BUFFER_LIMIT];
	char r_buffer[BUFFER_LIMIT];
	char buffer[BUFFER_LIMIT];
	char* log_file = NULL;
	char* scale = NULL;
	char* host_name = NULL;

	float temperature;

	struct sockaddr_in client;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	int i;
	for (i = 0; i < argv; i++)
	{

		int j;
		for (j = 0; argc[i][j] != '\0'; j++)
		{
			if (!isdigit(argc[i][j]))
			{
				break;
			}

			else
			{
				port = atoi(argc[i]);
			}
		}
	}

	if (port <= 0)
	{
		fprintf(stderr, "Error: no port specified\n");
		exit(1);
	}

	//printf("port=%d\n", port);

	static struct option long_opts[] =
	{
		{"scale", required_argument, 0, 's'},
		{"period", required_argument, 0, 'p'},
		{"log", required_argument, 0, 'l'},
		{"id", required_argument, 0, 'i'},
		{"host", required_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	int c = 0;
	while (c != -1)
	{
		c = getopt_long(argv, argc, "splih", long_opts, NULL);

		switch (c)
		{
			case 's':
				scale = optarg;
				break;
			case 'p':
				period = atoi(optarg);
				break;
			case 'l':
				log_flag = 1;
				log_file = optarg;
				break;
			case 'h':
				host_name = optarg;
				break;
			case 'i':
				id = atoi(optarg);
				break;
			case '?':
				exit(1);
		}
	}

	if (scale)
	{
		if (scale[0] == 'C')
		{
			f_flag = 0;
		} 

		else if (scale[0] == 'F')
		{
			f_flag = 1;
		}

		else
		{
			fprintf(stderr, "Error: --scale must have arguments C or F\n");
			exit(1);
		}
	}

	if (log_flag && log_file)
	{
		log_fd = creat(log_file, 0666);
		//printf("log_fd: %d\n", log_fd);

		if (log_fd < 0)
		{
			fprintf(stderr, "Error: failed to open file: %s\n", strerror(errno));
			exit(1);
		}
	}

	else 
	{
		fprintf(stderr, "Error: log argument required\n");
		exit(1);
	}

	//printf("log=%s\n", log_file);

	if (!host_name)
	{
		fprintf(stderr, "Error: no host name specified\n");
		exit(1);
	}

	//printf("hostname=%s\n", host_name);

	if (id <= 0)
	{
		fprintf(stderr, "Error: no id specified\n");
		exit(1);
	}

	struct hostent *server = gethostbyname(host_name);
	if (!server)
	{
		fprintf(stderr, "Error retrieving server%s\n", strerror(errno));
		exit(2);
	}

	client.sin_addr.s_addr = *(long *) server->h_addr_list[0];
	client.sin_family = AF_INET;
	client.sin_port = htons(port);

	int conn = connect(socketfd, (struct sockaddr *) &client, sizeof(client));

	if (conn < 0)
	{
		fprintf(stderr, "Error connecting to socket: %s\n", strerror(errno));
		exit(2);
	}

	//printf("id=%d,socketfd=%d\n", id, socketfd);

	dprintf(socketfd, "ID=%d\n", id);
	dprintf(log_fd, "ID=%d\n", id);

	//printf("printed to socket\n");

	struct pollfd sock = {socketfd, POLLIN | POLLHUP | POLLERR, 0};

	struct pollfd fds[1] = {sock};

	buffer[0] = '\0';

//	mraa_gpio_context button;
//	button = mraa_gpio_init(62);
//	mraa_gpio_dir(button, MRAA_GPIO_IN);

	//printf("made button\n");

	//mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);

	mraa_aio_context sensor;
	sensor = mraa_aio_init(1);

	//printf("made sensor\n");

	while (run_flag)
	{
		//printf("entered run_flag loop\n");
		poll(fds, 1, 0);

		if (fds[0].revents == POLLIN)
		{
			int num_bytes = read(socketfd, r_buffer, BUFFER_LIMIT);

			int i;
			for (i = 0; i < num_bytes; i++)
			{
				int curr_len = strlen(buffer);
				if (curr_len < (BUFFER_LIMIT - 2))
				{
					buffer[curr_len] = r_buffer[i];
					buffer[curr_len + 1] = '\0';
				}

				if (r_buffer[i] == '\n')
				{
					if (log_flag)
					{
						if (write(log_fd, buffer, strlen(buffer) + 1) < 0)
						{
							fprintf(stderr, "Error: write to log failed: %s\n", strerror(errno));
							//mraa_gpio_close(button);
							mraa_aio_close(sensor);
							exit(1);
						}
					}
								
					if (strncmp(buffer, "SCALE=", 6) == 0)
					{
						if (buffer[6] == 'F')
						{
							f_flag = 1;
						} 

						else if (buffer[6] == 'C')
						{
							f_flag = 0;
						}
					}

					else if (strncmp(buffer, "PERIOD=", 7) == 0)
					{
						period = atoi(&buffer[7]);
					}

					else if (strncmp(buffer, "STOP", 4) == 0)
					{
						report_flag = 0;
					}

					else if (strncmp(buffer, "START", 5) == 0)
					{
						report_flag = 1;
					}

					else if (strncmp(buffer, "LOG", 3) == 0)
					{
					}

					else if (strncmp(buffer, "OFF", 3) == 0)
					{
						run_flag = 0;
					}
					
					buffer[0] = '\0';
				}
			}

			//printf("POLLIN: buffer=%s\n", buffer);
		}

		value = mraa_aio_read(sensor);

		time(&curr_time);

		info = localtime(&curr_time);

		float R = 1023.0/value-1.0;
		R = R0 * R;
		temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;

		if (f_flag)
		{
			temperature = ((temperature * 9)/5) + 32;
		}

		if (report_flag)
		{
			dprintf(socketfd, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temperature);
		}
		
		if (log_flag)
		{
			//printf("made it to log_flag in while loop\n");
			sprintf(log_str, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temperature);
			//printf("log_str: %s\n", log_str);
			if (write(log_fd, log_str, strlen(log_str) + 1) < 0)
			{
				fprintf(stderr, "Error: write to log failed: %s\n", strerror(errno));
				//mraa_gpio_close(button);
				mraa_aio_close(sensor);
				exit(1);
			}
		}

		sleep(period);
	}

	time(&curr_time);

	info = localtime(&curr_time);

	if (log_flag)
	{
		sprintf(log_str, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
		if (write(log_fd, log_str, strlen(log_str) + 1) < 0)
		{
			fprintf(stderr, "Error: write to log failed: %s\n", strerror(errno));
			//mraa_gpio_close(button);
			mraa_aio_close(sensor);
			exit(1);
		}
	}

	dprintf(socketfd, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);

	//mraa_gpio_close(button);
	mraa_aio_close(sensor);

	return 0;
}