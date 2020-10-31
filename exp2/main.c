#include<pthread.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>

#ifndef SEMUN_H
#define SEMUN_H
#include<sys/types.h>
union semun{
	int 				val;
	struct semid_ds*	buf;
	unsigned short *	array;
#if defined(_linux_)
	struct seminfo *	_buf;
#endif
};
#endif

int semid1,semid2;
pthread_t p1,p2;
int a,type;	//a is the common variable;type indicates whether the process is done

void P(int semid,int index)
//the function itself creates a sembuf struct to define the demanded operation of the signal,and uses semop to operate the signal ofsemid
//so the operation of the signal is wrapped into the call of function P or V
{
	struct sembuf sem;
	sem.sem_num=index;	//in this program,cause there is only one signal,so the suffix(index) is 0
	sem.sem_op=-1;
	sem.sem_flg=SEM_UNDO;	//default parameter
	semop(semid,&sem,1);
	return;
}
void V(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num=index;
	sem.sem_op=1;
	sem.sem_flg=SEM_UNDO;
	semop(semid,&sem,1);
	return;
}

void *subp1(void* temp)
//the original shape of the thread's function is strictecd
{
	for(;;)
	{
		P(semid1,0);
		if(type==1)
			break;	//before printing,confirm if the subp2 process has finished
		printf("The value of a is %d.\n",a);
		V(semid2,0);
	}
	return NULL;
}

void *subp2(void* temp)
{
	for(int i=1;i<=1000;i++)
	{
		a+=i;
		V(semid1,0);	//by this order,the subp1 only prints after the adding process is done
		P(semid2,0);	//the subp2 only loops until subp1 already prints recent a
	}
	type=1;	//to inform the subp1 to exit
	V(semid1,0);	//in case that though subp2 finishes,subp1 uses P function and goes into a deadlock
	return NULL;
}

int main(void)
{
	a=0;
	type=0;
	semid1=semget(IPC_PRIVATE,1,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a new system V IPC object of signal,the third parameter contians management and authority
	semid2=semget(IPC_PRIVATE,1,IPC_CREAT|0666);	//the second part of the third parameter is the same as the former line
	
	union semun arg;
	arg.val=0;	//the fourth parameter is restricted by the standard,no need to specify its details
	semctl(semid1,1,SETVAL,arg);	//set the initial number of the signal equal to arg.val
	semctl(semid2,1,SETVAL,arg);

	pthread_t pthread1,pthread2;
	pthread_create(&pthread1,NULL,subp1,NULL);	//set a new thread apart from this 'main thread',its procession is defined as subp1
	pthread_create(&pthread2,NULL,subp2,NULL);	//the first parameter is used to point to the id of this thread,the second is used to set the thread's properties 

	pthread_join(pthread1,NULL);	//the main thread suspends until other two processes are done
	pthread_join(pthread2,NULL);

	semctl(semid1,1,IPC_RMID);	//delete the signal,no need of the fourth parameter
	semctl(semid2,1,IPC_RMID);

	return 0;
}
