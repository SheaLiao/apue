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
#include <dirent.h>
#include <fcntl.h>


int get_temperature(float *temp)
{
        int     fd = -1;
        int     found = 0;
        int     rv = -1;
        char    buf[128];
        char   *ptr = NULL;
        char    w1_path[64] = "/sys/bus/w1/devices/";
        char    chip[32];
        DIR    *dirp = NULL;
        struct  dirent *direntp = NULL;

        if( NULL ==(dirp = opendir(w1_path)))
        {
                printf("open %s failure: %s\n",w1_path,strerror(errno));
                return -1;
        }

        while( NULL != (direntp = readdir(dirp)) )
        {
                if( strstr(direntp->d_name,"28-") )
                {
                        strncpy(chip,direntp->d_name,sizeof(chip));
                        found = 1;
                }
        }
        closedir(dirp);

        if( !found )
        {
                printf("cannot find ds18b20 path\n");
                return -2;
        }

        strncat(w1_path,chip,sizeof(w1_path)-strlen(w1_path));
        strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));

//      printf("w1_path: %s\n",w1_path);

        fd = open(w1_path,O_RDONLY);
        if( fd < 0 )
        {
                printf("open file failure:%s\n",strerror(errno));
                return -3;
        }

//      printf("fd=%d\n",fd);

        memset(buf,0,sizeof(buf));

        if( read(fd,buf,sizeof(buf)) < 0 )
        {
                printf("read data failure\n");
                return -4;
                close(fd);
        }

//      printf("buf= %s\n",buf);

        ptr = strstr(buf,"t=");
        if( !ptr )
        {
                printf("cannot find t=\n");
                return -5;
                close(fd);
        }

        ptr += 2;
        *temp = atof(ptr)/1000;
//      printf("temp= %f\n",*temp);


        return 0;
}


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
        float                   temp;
	char			t_buf[1024];
        int                     conn_fd = -1;
        int                     rv = -1;
        char                    buf[1024];
        struct sockaddr_in      serv_addr;
        char                    *serv_ip =  NULL;
        int                     port = 0;
        int                     opt = -1;
        const char              *optstring = "i:p:h";
        struct option           opts[] = {
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

	if( get_temperature(&temp) < 0 )
        {
                printf("get temperature failure\n");
                return -1;
        }
        printf("temperature:%f\n",temp);

	memset(t_buf,0,sizeof(t_buf));
	snprintf(t_buf,sizeof(t_buf),"Temperature:%.2f",temp);

        while(1)
        {
                rv = write(conn_fd,t_buf,strlen(t_buf));
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

