#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void read_write(int in_fd, int out_fd)
{
    char* buffer;
    buffer = (char *) malloc(sizeof(char));

    ssize_t status = read(in_fd, buffer, 1);
	ssize_t err = 0;

    while(status > 0) 
    {
        err = write(out_fd, buffer, 1);
    	if (err == -1)
        {
            fprintf(stderr, "write() failed: %s\n", strerror(errno));
        }
        status = read(in_fd, buffer, 1);
    }

	if (status == -1)
    {
        fprintf(stderr, "read() failed: %s\n", strerror(errno));
	}

    free(buffer);

}


void segf()
{
	char * temp = NULL;
	*temp = 'a';
}

void signal_handler(int signal_num)
{
	if (signal_num == SIGSEGV)
	{
		fprintf(stderr, "Segmentation fault caught. --catch passed as argument.\n");
		exit(4);
	}
}

int main(int argc, char **argv)
{

		int force_segfault = 0;

        char * input_file = NULL;
        char * output_file = NULL;


        int c = 0;

        while(c != -1)
        {
                static struct option long_options[] =
                {
                        {"input", required_argument, NULL, 'i'},
                        {"output", required_argument, NULL, 'o'},
                        {"segfault", no_argument, NULL, 's'},
                        {"catch", no_argument, NULL, 'c'},
                        {0, 0, 0, 0}
                };

                int option_index = 0;

                c = getopt_long(argc, argv, "i:o:sc", long_options, &option_index);

                if (c == -1)
                {
                  	break;
                }

                switch (c)
                {
                case 0:
                  	break;
                case 'i':
                  	input_file = optarg;
                  	break;
                case 'o':
                  	output_file = optarg;
                  	break;
                case 's':
                	force_segfault = 1;
                  	break;
                case 'c':
                  	signal(SIGSEGV, signal_handler);
                  	break;
             	case '?':
             		exit(1);
                }
        }

        int in_fd = 0, out_fd = 1;

        if (input_file)
        {
        	in_fd = open(input_file, O_RDONLY);

        	if (in_fd >= 0)
        	{
        		close(0);
        		dup(in_fd);
        		close(in_fd);
        	}


        	else
        	{
        		fprintf(stderr, "Opening failed on --input argument. %s could not be read", input_file);
        		exit(2);
        	}
        }

        if (output_file)
        {
        	out_fd = creat(output_file, 0666);

        	if (out_fd >= 0)
        	{
        		close(1);
        		dup(out_fd);
        		close(out_fd);
        	}

        	else
        	{
        		fprintf(stderr, "Creating output file failed on --output argument. %s could not be created\n", output_file);
        		exit(3);
        	}
        }

        if (force_segfault)
        {
        	segf();	
        }

        read_write(0, 1);
        exit(0);
}
