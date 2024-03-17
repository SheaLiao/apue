#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	pid_t	pid;

	printf("Parent process PID[%d] start rnning...\n",getpid());

	pid = fork();
	if( pid < 0 )
	{
		printf("fork() creat child process failure:%s\n",strerror(errno));
		return -1;
	}
	else if( 0 == pid )
	{
		printf("Child process PID[%d] start running, parent PID is[%d]\n",getpid(),getppid());
		return 0;
	}
	else
	{
		printf("parent process PID[%d] continue running, and child process PID is [%d]\n",getpid(),pid);
		return 0;
	}
}
