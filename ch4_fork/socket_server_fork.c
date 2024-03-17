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
#include <ctype.h>


#define MSG_STR "Hello World!"
#define MAX_CLIENT 5

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
        int                     client_fd = -1;
        int                     rv = -1;
        char                    buf[1024];
        struct sockaddr_in      serv_addr;
        struct sockaddr_in      cli_addr;
        socklen_t               cliaddr_len;
        int                     port = 0;
        int                     on = 1;
	pid_t			pid;
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

        memset(&cli_addr,0,sizeof(cli_addr));
        while(1)
        {
                printf("Wating for client connect...\n");
                client_fd = accept(sock_fd,(struct sockaddr*)&cli_addr,&cliaddr_len);

		pid = fork();
		if( pid < 0 )
		{
			printf("fork() creat child process failure:%s\n",strerror(errno));
			close(client_fd);
			continue;
		}
		else if( pid > 0 )
		{
			close(client_fd);
			continue;
		}
		else if( 0 == pid )
		{
			int	i = 0;

			printf("Child process start to commuicate with socket client...\n");

			close(sock_fd);

			while(1)
			{
				memset(buf,0,sizeof(buf));
                		rv = read( client_fd,buf,sizeof(buf) );
                		if(rv < 0)
                		{
                        		printf("Read data from server by sockfd[%d] faliure: %s\n",client_fd,strerror(errno));
                        		close(client_fd);
                        		exit(0);
                		}
                		else if( 0 == rv )
                		{
                        		printf("Sockfd[%d] get disconnected\n",client_fd);
                        		close(client_fd);
                        		exit(0);
                		}
                		else if( rv > 0 )
                		{
                        		printf("Read %d bytes data from server: '%s'\n",rv,buf);
                		}

				for(i=0;i<rv;i++)
				{
					buf[i] = toupper(buf[i]);
				}

                		rv = write(client_fd,MSG_STR,strlen(MSG_STR));
                		if( rv < 0 )
                		{
                        		printf("write to client by sockfd[%d] failure: %s\n",sock_fd,strerror(errno));
                        		close(client_fd);
                        		exit(0);
                		}

			}
		}

        }

        close(sock_fd);

        return 0;
}

