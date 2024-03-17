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

#define MSG_STR "Hello World!"

void print_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("-i(--ipaddr): sepcify server IP address.\n");
	printf("-p(--port): sepcify server port.\n");
	printf("-h(--help): print this help information.\n");

	return ;
}

int main(int argc,char *argv[])
{
	float			temp;
	int                     conn_fd = -1;
        int                     rv = -1;
        char                    buf[1024];
        struct sockaddr_in      serv_addr;
        char                    *serv_ip =  NULL;
	int                     port = 0;
	int			opt = -1;
	const char 		*optstring = "i:p:h";
	struct option		opts[] = {
       		{"ipaddr", required_argument, NULL, 'i'},
        	{"prot", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
        	{NULL, 0, NULL, 0}
	};	       

	while ( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
   	{
        	switch (opt)
        	{
            		case 'i':
                		serv_ip = optarg;
                		break;
            		case 'p':
                		port = atoi(optarg);
                		break;
            		case 'h':
                		print_usage(argv[0]);
                		return 0;
        	}	
    	}


	if( !serv_ip || !port )
	{
		print_usage(argv[0]);
		return 0;
	}

        conn_fd = socket(AF_INET,SOCK_STREAM,0);
        if( conn_fd < 0 )
        {
                printf("Creat socket failure: %s\n",strerror(errno));
                return -2;
        }
        printf("Soket creat fd[%d] successfully\n",conn_fd);

        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_aton(serv_ip,&serv_addr.sin_addr);

	rv = connect( conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) );
        if(rv < 0)
        {
                printf("Connect to server [%s:%d] failure: %s\n",serv_ip,port,strerror(errno));
                return -3;
        }
        printf("Connect to server [%s:%d] successfully!\n",serv_ip,port);

        while(1)
        {
		rv = write(conn_fd,MSG_STR,strlen(MSG_STR));
		if( rv < 0 )
		{
                	printf("Write data to server [%s:%d] failure: %s\n",serv_ip,port,strerror(errno));
			break;
		}

	        memset(buf,0,sizeof(buf));
	        rv = read( conn_fd,buf,sizeof(buf) );
	        if(rv < 0)
	        {
        	        printf("Read data from server by sockfd[%d] faliure: %s\n",conn_fd,strerror(errno));
               		break;
        	}
        	else if( 0 == rv )
        	{
                	printf("Sockfd[%d] get disconnected\n",conn_fd);
                	break;
        	}
        	else if( rv > 0 )
        	{
                	printf("Read %d bytes data from server: '%s'\n",rv,buf);
        	}
	}

        close(conn_fd);

        return 0;
}
