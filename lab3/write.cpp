#include<sys/types.h>
#include<iostream>
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<stdio.h>
#include<unistd.h>

using namespace std;

#define N 10

union semun{
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

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


int main()
{
	int semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
	if(semid == -1)
	{
		printf("[WRITE]: semget error! \n");
		exit(2);
	}

	int shmid = shmget(IPC_PRIVATE, sizeof(struct shared_memory), IPC_CREAT | 0666);
	if(shmid == -1)
	{
		printf("shmget error!\n");
		exit(2);
	}

	struct shared_memory* read_addr = NULL;
	read_addr = (struct shared_memory*) shmat(shmid, 0, 0);
	
	FILE* fpw = NULL;
	fpw = fopen("./output.txt", "w");
	if(fpw == NULL)
	{
		printf("[WRITE]: open output.txt error! \n");
		exit(2);
	}

	int get = 0;
	char ch;
	while(1)
	{
		P(semid, 2);
		ch = read_addr -> text[read_addr -> start];
		read_addr -> start = (read_addr -> start + 1) % N;
		V(semid, 1);
		get ++;

		if((get % 10 == 0))
			printf("[WRITE]: Buffer OUT: %d bytes\n", get);
		if(ch == EOF)
			break;

		fputc(ch, fpw);
	}
	fclose(fpw);
	printf("WRITEN output.txt!\n");
	return 0;

}