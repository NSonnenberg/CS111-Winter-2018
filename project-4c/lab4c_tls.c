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
#include <ctype.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <resolv.h>

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

SSL_CTX* initialize_SSL()
{   
	const SSL_METHOD *method;
    SSL_CTX *ctx;
 
 	SSL_library_init();
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = SSLv23_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        exit(2);
    }
    return ctx;
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        //printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        //printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        //printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
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

	char w_buffer[BUFFER_LIMIT];
	char log_str[BUFFER_LIMIT];
	char r_buffer[BUFFER_LIMIT];
	char buffer[BUFFER_LIMIT];
	char* log_file = NULL;
	char* scale = NULL;
	char* host_name = NULL;

	float temperature;

	SSL_CTX *ctx = initialize_SSL();

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

	if (!host_name)
	{
		fprintf(stderr, "Error: no host name specified\n");
		exit(1);
	}

// Leon Hornwood

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

	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, socketfd);
	if (SSL_connect(ssl) == -1)
	{
		ERR_print_errors_fp(stderr);
		exit(2);
	}

	ShowCerts(ssl);

	sprintf(w_buffer, "ID=%d\n", id);
	SSL_write(ssl, w_buffer, strlen(w_buffer));
	dprintf(log_fd, "ID=%d\n", id);


	struct pollfd sock = {socketfd, POLLIN | POLLHUP | POLLERR, 0};

	struct pollfd fds[1] = {sock};

	buffer[0] = '\0';

	mraa_aio_context sensor;
	sensor = mraa_aio_init(1);

	while (run_flag)
	{
		poll(fds, 1, 0);

		if (fds[0].revents == POLLIN)
		{
			int num_bytes = SSL_read(ssl, r_buffer, BUFFER_LIMIT);

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
			sprintf(w_buffer, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temperature);
			SSL_write(ssl, w_buffer, strlen(w_buffer));
		}
		
		if (log_flag)
		{
			sprintf(log_str, "%02d:%02d:%02d %.1f\n", info->tm_hour, info->tm_min, info->tm_sec, temperature);
			
			if (write(log_fd, log_str, strlen(log_str) + 1) < 0)
			{
				fprintf(stderr, "Error: write to log failed: %s\n", strerror(errno));
				//mraa_gpio_close(button);
				mraa_aio_close(sensor);
				exit(2);
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
			exit(2);
		}
	}

	sprintf(w_buffer, "%02d:%02d:%02d SHUTDOWN\n", info->tm_hour, info->tm_min, info->tm_sec);
	SSL_write(ssl, w_buffer, strlen(w_buffer));

	//mraa_gpio_close(button);
	mraa_aio_close(sensor);
	close(socketfd);
	SSL_CTX_free(ctx);

	return 0;
}