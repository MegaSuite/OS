#include<sys/types.h>
#include<iostream>
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdio.h>

using namespace std;

const int MAX = 100;

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};
union semun arg;
struct sembuf sem;

int sum = 0, semid, x = 1;
pthread_t calculator, even, odd;

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

void *calculator_t(void*)
{
	while(x <= MAX)
    {
		P(semid,0);
		sum += x;
		x ++;
		if(sum % 2 == 0)
			V(semid,1);
        else
			V(semid,2);
	}	
    return nullptr;
}

void *even_t(void*)
{
	int x1 = 0;
	while(x <= MAX)
    {
		if(x1 == 50)
			break;

		P(semid, 1);
		cout << "phread1 print even num: " << sum << endl;
		x1 ++;
		V(semid, 0);
	}
	cout << "pthread 1 print even nums: "<< x1 <<" times" << endl;

    return nullptr;
}

void *odd_t(void*)
{
	int x2 = 0;
	while(x <= MAX)
    {
		if(x2 == 50)
			break;
		
		P(semid, 2);
		cout << "phread2 print odd num: "<< sum << endl;
		x2 ++;
		V(semid, 0);
	}
	cout << "pthread 2 print odd nums: " << x2 << " times" << endl;

    return nullptr;
}

int main()
{
	//创建信号量集
	semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
	if(semid == -1)
    {
		cout << "semget error!";
		return 1;
	}

    //计算资源：1
	arg.val = 1;
	if(semctl(semid, 0, SETVAL, arg) < 0)
    {
		cout << "semctl error!";
		return 1;
	}

	arg.val = 0;
    //偶数资源：0
	if(semctl(semid, 1, SETVAL, arg) < 0)
    {
		cout << "semctl error!";
		return 1;
	}
    //奇数资源：0
	if(semctl(semid, 2, SETVAL, arg) < 0)
    {
		cout << "semctl error!";
		return 1;
	}

    //创建线程
	pthread_create(&calculator, NULL, calculator_t, NULL);
	pthread_create(&even, NULL, even_t, NULL);
	pthread_create(&odd, NULL, odd_t, NULL);
    
    //等待结束
	pthread_join(calculator, NULL);
	pthread_join(even, NULL);
	pthread_join(odd, NULL);

    //删除信号集
	semctl(semid, 3, IPC_RMID, arg);
	return 0;
}