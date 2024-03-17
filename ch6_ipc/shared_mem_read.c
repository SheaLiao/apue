#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define FTOK_PATH "/dev/zero"
#define FTOK_PROJID 0x22

typedef struct st_student
{
 	char name[64];
 	int age;
} t_student;

union semun
{
        int val;
        struct semid_ds *buf;
        unsigned short *arry;
};

int semaphore_init(void);
int semaphore_p(int semid);
int semaphore_v(int semid);
void semaphore_term(int semid);


int main(int argc, char *argv[])
{
	key_t		shm_key;
	int		shmid;
	int		semid;
	int		i;
	t_student	*student;

	semid = semaphore_init();
        if( semid < 0 )
        {
                printf("semaphore initial failure: %s\n", strerror(errno));
                return -1;
        }


	shm_key = ftok(FTOK_PATH, FTOK_PROJID);
	if( shm_key < 0 )
	{
		printf("ftok() get IPC token failure: %s\n", strerror(errno));
 		return -1;
	}

	shmid = shmget(shm_key, sizeof(t_student), IPC_CREAT|0666);
	if( shmid < 0 )
	{
		printf("shmget() create shared memroy failure: %s\n", strerror(errno));
 		return -2;
	}

	student = shmat(shmid, NULL, 0);
	if( (void *)-1 == student )
	{
		printf("shmat() alloc shared memroy failure: %s\n", strerror(errno));
 		return -2;
	}

	semaphore_p(semid);

	for(i=0; i<4; i++)
	{
		printf("Student '%s' age [%d]\n", student->name, student->age);
 		sleep(1);
	}

	semaphore_v(semid);

	shmctl(shmid, IPC_RMID, NULL);

	semaphore_term(semid);

	return 0;
}


int semaphore_init(void)
{
        key_t           key;
        int             semid;
        union semun     sem_union;

        key = ftok(FTOK_PATH, FTOK_PROJID); //获取到关键字，路径，标号（随便给的）
        if( key < 0 )
        {
                printf("ftok() get IPC token failure: %s\n", strerror(errno));
                return -1;
        }

        semid = semget(key, 1, IPC_CREAT|0644);//semget创建一个信号量id， 关键字，创建几个信号量，IPC_CREAT如果信号量不存在则创建，权限为0644。
        if( semid < 0 )
        {
                printf("semget() get semid failure: %s\n", strerror(errno));
                return -2;
        }

    	/*设置信号量的值*/
        sem_union.val = 0;
        if( semctl(semid, 0, SETVAL, sem_union) < 0 )//semid,要设置第几个信号，SETVAL指要设置这个信号的值，设置的值为sem_union
        {
                printf("semctl() set initial value failure: %s\n", strerror(errno));
                return -3;
        }

        printf("Semaphore get key_t[0x%x] and semid[%d]\n", key,semid);

        return semid;
}


/*移除一个信号*/
void semaphore_term(int semid)
{
        union semun     sem_union;

        if( semctl(semid, 0, IPC_RMID, sem_union) , 0 )//IPC_RMID 移除
        {
                printf("semctl() delete semaphore ID failure: %s\n",strerror(errno));
        }

        return;
}


/*P操作*/
int semaphore_p(int semid)
{
        struct sembuf   _sembuf;

        _sembuf.sem_num = 0;
        _sembuf.sem_op = -1;	//P操作信号量值减一
        _sembuf.sem_flg = SEM_UNDO;

        if( semop(semid, &_sembuf, 1) < 0 )//如果信号量为0，则在这里阻塞
        {
                printf("semop p operator failure:%s\n", strerror(errno));
                return -1;
        }

        return 0;
}

/*V操作*/
int semaphore_v(int semid)
{
        struct sembuf   _sembuf;

        _sembuf.sem_num = 0;
        _sembuf.sem_op = 1;  //V操作信号量值加一
        _sembuf.sem_flg = SEM_UNDO;

        if( semop(semid, &_sembuf, 1) < 0 )
        {
                printf("semop V operator failure: %s\n", strerror(errno));
                return -1;
        }

        return 0;
}

