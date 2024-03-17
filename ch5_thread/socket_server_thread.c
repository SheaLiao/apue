#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <pthread.h>
#include <ctype.h>


#define MSG_STR "Hello World!"
#define MAX_CLIENT 13

typedef void *(THREAD_BODY) (void *thread_arg);

void *thread_worker(void *ctx);
int thread_start(pthread_t *thread_id, THREAD_BODY *thread_workerbody, void *thread_arg);

void print_usage(char *progname)
{
        printf("%s usage: \n",progname);
        printf("-p(--port): sepcify server port.\n");
        printf("-h(--help): print this help inforation.\n");

        return ;
}

int main(int argc,char *argv[])
{
        int                     sock_fd = -1;
	int			client_fd = -1;
        int                     rv = -1;
        struct sockaddr_in      serv_addr;
	struct sockaddr_in	cli_addr;
	socklen_t		cliaddr_len;
        int                     port = 0;
	pthread_t		tid;
	int 			on = 1;
        int                     opt = -1;
        const char              *optstring = "p:h";

        struct option           opts[] = {
                {"prot", required_argument, NULL, 'p'},
                {"help", no_argument, NULL, 'h'},
                {NULL, 0, NULL, 0}
        };

        while ( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
        {
                switch (opt)
                {
                        case 'p':
                                port = atoi(optarg);
                                break;
                        case 'h':
                                print_usage(argv[0]);
                                return 0;
                }
        }

        if( !port )
        {
                print_usage(argv[0]);
                return 0;
        }

        sock_fd = socket(AF_INET,SOCK_STREAM,0);
        if( sock_fd < 0 )
        {
                printf("Creat socket failure: %s\n",strerror(errno));
                return -1;
        }
        printf("Soket creat fd[%d] successfully\n",sock_fd);

	setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        rv = bind( sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) );
        if(rv < 0)
        {
                printf("Socket[%d] bind on port [%d] failure: %s\n",sock_fd,port,strerror(errno));
                return -2;
        }
        printf("Socket[%d] bind on port [%d]successfully!\n",sock_fd,port);

	listen(sock_fd,MAX_CLIENT);
	printf("Start to listen on port [%d]\n",port);

	memset(&cli_addr,0,sizeof(cli_addr));
        while(1)
        {
		printf("Wating for client connect...\n");
		client_fd = accept(sock_fd,(struct sockaddr*)&cli_addr,&cliaddr_len);
                if( client_fd < 0 )
                {
                        printf("Accept client connect failure: %s\n",strerror(errno));
                        break;
                }
		printf("Accept client connect from [%s:%d]\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port) );

		thread_start(&tid,thread_worker,(void *)client_fd);

        }


        close(sock_fd);

        return 0;
}


int thread_start(pthread_t *thread_id, THREAD_BODY *thread_workbody, void *thread_arg)
{
	int		rv = -1;
	pthread_attr_t	thread_attr;

	if( pthread_attr_init(&thread_attr) )
	{
		printf("pthread_attr_init() failure: %s\n",strerror(errno));
		goto CleanUp;
	}

	if( pthread_attr_setstacksize(&thread_attr, 120*1024) )
        {
                printf("pthread_attr_setstacksize() faliure: %s\n",strerror(errno));
                goto CleanUp;
        }

        if( pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) )
        {
                printf("pthread_attr_setdetachstate() faliure: %s\n",strerror(errno));
                goto CleanUp;
        }

	if( pthread_create(thread_id, &thread_attr, thread_workbody, thread_arg) )
	{
 		printf("Create thread faliure: %s\n",strerror(errno));
                goto CleanUp;
	}

        rv = 0;

CleanUp:
	pthread_attr_destroy(&thread_attr);
	return rv;
}

void *thread_worker(void *ctx)
{
	int	client_fd = -1;
	int	rv = -1;
	char	buf[1024];
	int	i;
	
	if( !ctx )
	{
		printf("Invalid input arguments in %s()\n",__FUNCTION__);
		pthread_exit(NULL);
	}

	client_fd = (int)ctx;

	printf("Child thread start to communicate with socket client...\n");

	while(1)
	{
		 memset(buf,0,sizeof(buf));
                rv = read( client_fd,buf,sizeof(buf) );
                if(rv < 0)
                {
                        printf("Read data from server by sockfd[%d] faliure: %s and thread will exit\n",client_fd,strerror(errno));
                        close(client_fd);
			pthread_exit(NULL);
                }
                else if( 0 == rv )
                {
                        printf("Sockfd[%d] get disconnected and thread will exit\n",client_fd);
                        close(client_fd);
			pthread_exit(NULL);
                }
                else if( rv > 0 )
                {
                        printf("Read %d bytes data from server: '%s'\n",rv,buf);
                }

		for(i=0;i<rv;i++)
		{
			buf[i] = toupper(buf[i]);
		}

		rv = write(client_fd, buf, rv);
		if( rv < 0 )
		{
			printf("write to client by sockfd[%d] failure: %s and thread will exit\n",client_fd,strerror(errno));
			close(client_fd);
			pthread_exit(NULL);
		}

	}
}
