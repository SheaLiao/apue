#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>

int get_temperature(float *temp,char *devid);

int main(int argc, char *argv[])
{
	float 	temp;
	char	devid[32];
	int   	rv;
	
	rv = get_temperature(&temp,devid);
	if( rv < 0)
	{
		printf("get temperature failure\n");
		return -1;
	}
	printf("temperature:%f %s\n",temp,devid);

	return 0;
}

int get_temperature(float *temp,char *devid)
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

	memcpy(devid,chip,sizeof(chip));
	printf("w1_path: %s\n",w1_path);

	fd = open(w1_path,O_RDONLY);
	if( fd < 0 )
	{
		printf("open file failure:%s\n",strerror(errno));
		return -3;
	}

//	printf("fd=%d\n",fd);

	memset(buf,0,sizeof(buf));

	if( read(fd,buf,sizeof(buf)) < 0 )
	{
		printf("read data failure\n");
		return -4;
		close(fd);
	}

//	printf("buf= %s\n",buf);

	ptr = strstr(buf,"t=");
	if( !ptr )
	{
		printf("cannot find t=\n");
		return -5;
		close(fd);
	}

	ptr += 2;
	*temp = atof(ptr)/1000;
//	printf("temp= %f\n",*temp);


	return 0;
}

