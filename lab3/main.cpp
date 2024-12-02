#include<sys/types.h>
#include<iostream>
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

using namespace std;

#define N 10  //缓冲区大小

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

//环形缓冲区
struct shared_memory{
	int start;  
	int end;   
	char text[N];  
};


void P(int semid,int index)
{	  
	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op = -1;	
    sem.sem_flg = 0;	//操作标记：0或IPC_NOWAIT等	
    semop(semid,&sem,1);	//1:表示执行命令的个数
    return;
}

void V(int semid,int index)
{	 
	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op =  1;
    sem.sem_flg = 0;	
    semop(semid,&sem,1);	
    return;
}

union semun arg;
struct sembuf sem;
char c;

int main()
{
	pid_t read_buffer, write_buffer;

	//创建共享内存
	struct shmid_ds shmids;
	int shmid = shmget(IPC_PRIVATE, sizeof(struct shared_memory), IPC_CREAT | 0666);  
	if(shmid == -1)
    {
		printf("shmget error!\n");
		exit(1);
	}

	// 访问共享内存
	struct shared_memory* main_attach_addr = NULL;
	main_attach_addr = (struct shared_memory*) shmat(shmid, 0, 0); 
	if(main_attach_addr == (void*) -1)
    {
		printf("main.cpp shmat error!\n");
		exit(1);
	}
	main_attach_addr->end = main_attach_addr -> start = 0;


	// 创建信号灯
	int semid = semget(IPC_PRIVATE, 3 , IPC_CREAT | 0666);
	//0号：init=1,缓冲区读写锁
	//1号：init=N,空闲位置数
	//2号：init=0,缓冲区已有数据数
	arg.val = 1;
	if(semctl(semid, 0, SETVAL, arg) < 0)
    {
		printf("semctl 0 error!\n");
		exit(0);
	}
	arg.val = N;
	if(semctl(semid, 1, SETVAL, arg) < 0)
    {
		printf("semctl 1 error!\n");
		exit(0);
	}
	arg.val = 0;
	if(semctl(semid, 2, SETVAL, arg) < 0)
    {
		printf("semctl 2 error!\n");
		exit(0);
	}

	//进程创建
	if((read_buffer = fork()) == 0)
    {
		printf("READ_BUFFER CREATED! \n");
		execl("./read", "read", NULL);
		exit(0);
	}
    else if((write_buffer = fork()) == 0)
    {
		printf("WRITE_BUFFER CREATED! \n");
		execl("./write", "write", NULL);
		exit(0);
	}
    else
    {
		wait(NULL);
		wait(NULL);

		semctl(semid, 3, IPC_RMID, arg);
		shmctl(shmid, IPC_RMID, NULL);
	}



}