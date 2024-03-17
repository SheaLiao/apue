#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>

#define FIFO_FILE1 ".fifo_chat1"
#define FIFO_FILE2 ".fifo_chat2"

int g_stop = 0;

void sig_pipe(int signum);

int main(int argc, char *argv[])
{
	int	fdr_fifo;
	int	fdw_fifo;
	int	rv;
	fd_set	rdset;
	char	buf[1024];
	int	mode = 0;

	if( argc != 2 )
	{
		printf("Usage: %s [0/1]\n", basename(argv[0]));
 		printf("This chat program need run twice, 1st time run with [0] and 2nd time with [1]\n");
 		return -1;
	}

	mode = atoi(argv[1]);

	if( access(FIFO_FILE1, F_OK) )
	{
		printf("FIFO file \"%s\" not exist and create it noe\n", FIFO_FILE1);
		mkfifo(FIFO_FILE1, 0666);
	}

	if( access(FIFO_FILE2, F_OK) )
	{
		printf("FIFO file \"%s\" not exist and create it noe\n", FIFO_FILE2);
		mkfifo(FIFO_FILE2, 0666);
	}

	signal(SIGPIPE, sig_pipe);

	if( 0 == mode )
	{
		printf("start open '%s' for read and it will blocked untill write endpoint opened...\n", FIFO_FILE1);
		fdr_fifo = open(FIFO_FILE1, O_RDONLY);
		if( fdr_fifo < 0 )
		{
			printf("Open fifo[%s] for chat read endpoint failure:%s\n", FIFO_FILE1,strerror(errno));
			return -2;
		}	

		printf("start open '%s' for write\n", FIFO_FILE2);
		fdw_fifo = open(FIFO_FILE2, O_WRONLY);
		if( fdw_fifo < 0 )
		{
			printf("Open fifo[%s] for chat write endpoint failure:%s\n", FIFO_FILE2,strerror(errno));
			return -3;
		}	
	}
	else
	{
		printf("start open '%s' for write and it will blocked untill write endpoint opened...\n", FIFO_FILE1);
		fdw_fifo = open(FIFO_FILE1, O_WRONLY);
		if( fdw_fifo < 0 )
		{
			printf("Open fifo[%s] for chat write endpoint failure:%s\n", FIFO_FILE1,strerror(errno));
			return -4;
		}	

		printf("start open '%s' for read\n", FIFO_FILE2);
		fdr_fifo = open(FIFO_FILE2, O_RDONLY);
		if( fdr_fifo < 0 )
		{
			printf("Open fifo[%s] for chat read endpoint failure:%s\n", FIFO_FILE2,strerror(errno));
			return -5;
		}	
	}

	printf("Start chating with another program now,please input message now:\n");
	while( !g_stop )
	{
		FD_ZERO(&rdset);
		FD_SET(STDIN_FILENO, &rdset);
		FD_SET(fdr_fifo, &rdset);

		rv = select(fdr_fifo+1, &rdset, NULL, NULL, NULL);
		if( rv <= 0 )
		{
			printf("Select get time or error: %s\n",strerror(errno));
			continue;
		}

		if( FD_ISSET(fdr_fifo, &rdset) )
		{
			memset(buf, 0, sizeof(buf));
			rv = read(fdr_fifo, buf, sizeof(buf));
			if( rv < 0 )
			{
				printf("Read data from FIFO get error: %s\n",strerror(errno));
				break;

			}
			else if( 0 == rv )
			{
				printf("Another side of FIFO get closed and program will exit now\n");
				break;
			}

			printf("<--%s", buf);
		}

		if( FD_ISSET(STDIN_FILENO, &rdset) )
		{
			memset(buf, 0, sizeof(buf));
			fgets(buf, sizeof(buf), stdin);
			write(fdw_fifo, buf, strlen(buf));
		}
	}
}

void sig_pipe(int signum)
{
	if( SIGPIPE == signum )
	{
		printf("Get pipe broken signal and let programe exit\n");
		g_stop = 1;
	}
}
