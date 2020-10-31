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

int ticketreq;
int sold,remain;	//a is the common variable;type indicates whether the process is done
int seller1,seller2,seller3;

void P(int semid,int index)
//the function itself creates a sembuf struct to define the demanded operation of the signal,and uses semop to operate the signal ofsemid
//so the operation of the signal is wrapped into the call of function P or V
{
	struct sembuf sem;
	sem.sem_num=index;	    //in this program,cause there is only one signal,so the suffix(index) is 0
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

void *ticketselling1(void* temp)
{
	while(1)
	{
        P(ticketreq,0);
        if(sold<remain)
        {
            printf("Ticket booked successfully!From booker 1.\n");
            sold++;
            seller1++;
            V(ticketreq,0);
        }
        else
        {
            V(ticketreq,0);
            break;
        }
	}
	return NULL;
}

void *ticketselling2(void* temp)
{
	while(1)
	{
        P(ticketreq,0);
        if(sold<remain)
        {
            printf("Ticket booked successfully!From booker 2.\n");
            sold++;
            seller2++;
            V(ticketreq,0);
        }
        else
        {
            V(ticketreq,0);
            break;
        }
	}
	return NULL;
}

void *ticketselling3(void* temp)
{
	while(1)
	{
        P(ticketreq,0);
        if(sold<remain)
        {
            printf("Ticket booked successfully!From booker 3.\n");
            sold++;
            seller3++;
            V(ticketreq,0);
        }
        else
        {
            V(ticketreq,0);
            break;
        }
	}
	return NULL;
}

int main(void)
{
	sold=0;
	remain=10000;
    seller1=seller2=seller3=0;
	ticketreq=semget(IPC_PRIVATE,1,IPC_CREAT|0666);	//the second part of the third parameter is the same as the former line
	union semun arg;
	arg.val=1;	//the fourth parameter is restricted by the standard,no need to specify its details
	
    semctl(ticketreq,0,SETVAL,arg);	//set the initial number of the signal equal to arg.val

    pthread_t pthread1,pthread2,pthread3;
	pthread_create(&pthread1,NULL,ticketselling1,NULL);	//set a new thread apart from this 'main thread',its procession is defined as subpoddprinter
	pthread_create(&pthread2,NULL,ticketselling2,NULL);	//the first parameter is used to point to the id of this thread,the second is used to set the thread's properties 
	pthread_create(&pthread3,NULL,ticketselling3,NULL);
	
    pthread_join(pthread1,NULL);	//the main thread suspends until other three processes are done
	pthread_join(pthread2,NULL);
	pthread_join(pthread3,NULL);

	semctl(ticketreq,1,IPC_RMID);	//delete the signal,no need of the fourth parameter

    printf("Tickets have already been sold %d from seller1.\n",seller1);
    printf("Tickets have already been sold %d from seller2.\n",seller2);
    printf("Tickets have already been sold %d from seller3.\n",seller3);
	return 0;
}
