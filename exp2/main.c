#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pthread.h>

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

int sem_odd_exist,sem_even_exist,sem_empty;
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

void *subpcalculator(void* temp)
{
	for(int i=1;i<=1000;i++)
	{
		a+=i;
		if(a%2)
			V(sem_odd_exist,0);	//by this order,the subpoddprinter only prints after the adding process is done
		else 
			V(sem_even_exist,0);
		P(sem_empty,0);	//the subpcalculator only loops until subpoddprinter already prints recent a
	}
	type=1;	//to inform the subpoddprinter/3 to exit
	V(sem_odd_exist,0);	//in case that though subpcalculator finishes,subpoddprinter uses P function and goes into a deadlock
	V(sem_even_exist,0);
	return NULL;
}

void *subpoddprinter(void* temp)
//the original shape of the thread's function is strictecd
{
	for(;;)
	{
		P(sem_odd_exist,0);
		if(type==1)
			break;	//before printing,confirm if the subpcalculator process has finished
		printf("The value of a is %d.Odd.Printed by subpoddprinter.\n",a);
		V(sem_empty,0);
	}
	return NULL;
}


void *subpevenprinter(void* temp)
{
	for(;;)
	{
		P(sem_even_exist,0);
		if(type==1)
			break;	//before printing,confirm if the subpcalculator process has finished
		printf("The value of a is %d.Even.Printed by subpevenprinter.\n",a);
		V(sem_empty,0);
	}
	return NULL;
}

int main(void)
{
	a=0;
	type=0;
	sem_odd_exist=semget(IPC_PRIVATE,1,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a new system V IPC object of signal,the third parameter contians management and authority
	sem_even_exist=semget(IPC_PRIVATE,1,IPC_CREAT|0666);	//the second part of the third parameter is the same as the former line
	sem_empty=semget(IPC_PRIVATE,1,IPC_CREAT|0666);	//the second part of the third parameter is the same as the former line
	
	union semun arg;
	arg.val=0;	//the fourth parameter is restricted by the standard,no need to specify its details
	semctl(sem_odd_exist,1,SETVAL,arg);	//set the initial number of the signal equal to arg.val
	semctl(sem_even_exist,1,SETVAL,arg);
	semctl(sem_empty,1,SETVAL,arg); 

	pthread_t pthread1,pthread3,pthread2;
	pthread_create(&pthread1,NULL,subpoddprinter,NULL);	//set a new thread apart from this 'main thread',its procession is defined as subpoddprinter
	pthread_create(&pthread3,NULL,subpcalculator,NULL);	//the first parameter is used to point to the id of this thread,the second is used to set the thread's properties 
	pthread_create(&pthread2,NULL,subpevenprinter,NULL);
	
	pthread_join(pthread1,NULL);	//the main thread suspends until other three processes are done
	pthread_join(pthread3,NULL);
	pthread_join(pthread2,NULL);

	semctl(sem_odd_exist,1,IPC_RMID);	//delete the signal,no need of the fourth parameter
	semctl(sem_even_exist,1,IPC_RMID);
	semctl(sem_empty,1,IPC_RMID);

	return 0;
}
