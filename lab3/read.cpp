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
	int shmid = shmget(key_t 1234, sizeof(struct shared_memory), 0666 | IPC_CREAT);
	if(shmid == -1)
	{
		printf("[READ]: shmget error!\n");
		exit(2);
	}


	int semid = semget(key_t 5678, 3, IPC_CREAT | 0666);
	if(semid == -1)
	{
		printf("[READ]: semget error!\n");
		exit(2);
	}


	struct shared_memory* write_addr = NULL;
	write_addr = (struct shared_memory*)shmat(shmid, 0, 0);
	
	FILE* fpr=NULL;
	fpr = fopen("./input.txt", "r");
	if(fpr == NULL)
	{
		printf("[READ]: open input.txt error!\n");
		exit(2);
	}

	char ch;
	int put = 0;
	while(!feof(fpr))
	{
		ch = fgetc(fpr);
		
		P(semid, 1);
		write_addr -> text[write_addr -> end] = ch;
		write_addr -> end = (write_addr -> end + 1) % N;
		V(semid, 2);
		put ++;

		if(put % 10 == 0)
			printf("[READ]: Buffer IN: %d bytes\n",put);
		if(ch == EOF)
			break;
	}
	fclose(fpr);
	printf("[READ]: DONE input.txt! \n");
	return 0;

}