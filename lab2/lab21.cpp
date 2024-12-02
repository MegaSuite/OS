#include<sys/types.h>
#include<iostream>
#include<pthread.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdio.h>

using namespace std;

#define MAX 1000

union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};
union semun arg;
struct sembuf sem;


int num = MAX, semid;
pthread_t subp1, subp2, subp3;


void P(int semid, int index)
{	  
	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op = -1;	
    sem.sem_flg = 0;	//操作标记：0或IPC_NOWAIT等	
    semop(semid, &sem, 1);	//1:表示执行命令的个数
    return;
}

void V(int semid, int index)
{	 
	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op =  1;
    sem.sem_flg = 0;	
    semop(semid, &sem, 1);	
    return;
}

void *subpf1(void*)
{
	int x1 = 0;
	while(num > 0)
    {
		P(semid, 0);
		if(!num)
        {
			V(semid, 0);
			break;
		}
		cout << "subp1 sell, remaining: " << num -- << endl;
		x1 ++;
		V(semid, 0);
	}
	cout << "pthread 1 sold a total of:" << x1 << endl;
    return nullptr;
}

void *subpf2(void*)
{
	int x2 = 0;
	while(num > 0)
    {
		P(semid, 0);
		if(!num)
        {
			V(semid, 0);
			break;
		}
		cout << "subp2 sell, remaining: " << num -- << endl;
		x2++;
		V(semid, 0);
	}
	cout << "pthread 2 sold a total of:" << x2 << endl;
    return nullptr;
}

void *subpf3(void*)
{
	int x3 = 0;
	while(num > 0)
    {
		P(semid, 0);
		if(!num)
        {
			V(semid, 0);
			break;
		}
		cout << "subp3 sell, remaining: " << num -- << endl;
		x3++;
		V(semid, 0);
	}
	cout << "pthread 3 sold a total of:" << x3 << endl;
    return nullptr;
}

int main()
{
    //创建信号量集
	semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if(semid == -1)
    {
		cout << "semget error!";
		return 1;
	}

    //信号灯赋值
	arg.val = 1;
	if(semctl(semid, 0, SETVAL, arg) < 0)
    {
		cout << "semctl error!";
		return 1;
	}
	
	//创建线程
	pthread_create(&subp1, NULL, subpf1, NULL);
	pthread_create(&subp2, NULL, subpf2, NULL);
	pthread_create(&subp3, NULL, subpf3, NULL);

    //等待结束
	pthread_join(subp1, NULL);
	pthread_join(subp2, NULL);
	pthread_join(subp3, NULL);

    //删除信号灯
	semctl(semid, 0, IPC_RMID, arg);
	return 0;

}