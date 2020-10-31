#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<wait.h>

#ifndef SEMUN_H
#define SEMUN_H
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
int status;
short semid1num[10]={0};
short semid2num[10]={0};
int type=0;

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

char *sharedmemory[10];

int main(void)
{	

	int shmid[10];
	int semid1;	//ten system V signals are used to prove if the shared memory segment is being read or written
	int semid2;	//two set is used for two processes

	for(int i=0;i<10;i++)
	{
		shmid[i]=shmget(IPC_PRIVATE,1,IPC_CREAT|0666);	//allocate 10 byte for each shared memory segment
		sharedmemory[i]=(char*)shmat(shmid[i],NULL,0);		//this function is used to assosiate shared memory with process virtual memory address,the third parameter is set as default	
	}	

	semid1=semget(IPC_PRIVATE,1,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a set of signal values
	semid2=semget(IPC_PRIVATE,1,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
	union semun arg;
	arg.array=semid1num;	//for reading process,the signal's initial value is 1 to let it read and save only once before consumed
	semctl(semid1,0,SETALL,arg);
	arg.array=semid2num;	//for writing process,it can only write after reading process is done
	semctl(semid2,0,SETALL,arg);
	
	//semctl(semid1,0,GETALL,arg);
	//semctl(semid2,0,GETALL,arg);
	//the original value set is corrected

	int pid1,pid2;
	
	pid1=fork();
	if(pid1==0)
	{
		FILE* destination=fopen("./write","w+");
		for(;;)
		{
			P(semid1,0);
			if(type==1)
				break;	//before printing,confirm if the subp2 process has finished
			fputc(*sharedmemory[0],destination);
			V(semid2,0);
		}
		return 0;
	}
	else
	{
		pid2=fork();
		if(pid2==0)
		{
			char tempchar='1';
			FILE* source=fopen("./read","r");
			if(source==NULL)
				printf("OPEN ERROR!");

			for(int i=1;tempchar!=EOF&&i<=1000;i++)
			{
				tempchar=fgetc(source);
				*sharedmemory[0]=tempchar;
				V(semid1,0);	//by this order,the subp1 only prints after the adding process is done
				P(semid2,0);	//the subp2 only loops until subp1 already prints recent a
			}
			type=1;	//to inform the subp1 to exit
			V(semid1,0);	//in case that though subp2 finishes,subp1 uses P function and goes into a deadlock
			return 0;
		}
	}
	waitpid(0,&status,WUNTRACED);	//wait for all children processes to end

	semctl(semid1,1,IPC_RMID);	//delete the signal set,no need of the fourth parameter,the second parameter is ignored 
	semctl(semid2,1,IPC_RMID);

	for(int i=0;i<10;i++)
		shmctl(shmid[i],IPC_RMID,0);	//delete the shared memory segment after use
	return 0;
}
