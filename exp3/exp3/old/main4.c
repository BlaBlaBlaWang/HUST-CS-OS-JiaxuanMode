#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/shm.h>
#include<wait.h>

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
int status;
int a,type;	//a is the common variable;type indicates whether the process is done
char *sharedmemory[10];
short semid1num[10]={1,1,1,1,1,1,1,1,1,1};
short semid2num[10]={0,0,0,0,0,0,0,0,0,0};

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

int main(void)
{
	a=0;
	type=0;

	int shmid[10];
	for(int i=0;i<10;i++)
	{
		shmid[i]=shmget(IPC_PRIVATE,10,IPC_CREAT|0666);	//allocate 10 byte for each shared memory segment
		sharedmemory[i]=(char*)shmat(shmid[i],NULL,0);		//this function is used to assosiate shared memory with process virtual memory address,the third parameter is set as default	
	}	

	semid1=semget(IPC_PRIVATE,1,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a new system V IPC object of signal,the third parameter contians management and authority
	semid2=semget(IPC_PRIVATE,1,IPC_CREAT|0666);	//the second part of the third parameter is the same as the former line
	
	union semun arg;
	arg.val=0;	//the fourth parameter is restricted by the standard,no need to specify its details
	semctl(semid1,1,SETVAL,arg);	//set the initial number of the signal equal to arg.val
	semctl(semid2,1,SETVAL,arg);

	int child1,child2;
	child1=fork();
	if(child1==0)
	{
		FILE * destination=fopen("./write","w+");
		int i=0;
		int tempseg=0;
		char tempchar=1;

		for(;tempchar!=EOF;)
		{
			P(semid1,0);
			
			for(i=0;i<10;i++)
			{
				if((tempchar=*(sharedmemory[tempseg%10]+i))==EOF)
					break;
				fputc(tempchar,destination);
			}

			tempseg++;

			V(semid2,0);
		}
		fclose(destination);
		return 0;
	}
	else
	{
		child2=fork();
		if(child2==0)
		{
			FILE * source=fopen("./read","r");
			char tempchar=0;
			int tempseg=0;

			do
			{
				for(int i=0;i<10&&tempchar!=EOF;i++)
				{
					tempchar=fgetc(source);
					*(sharedmemory[tempseg%10]+i)=tempchar;
				}
				tempseg++;
				V(semid1,0);	//by this order,the subp1 only prints after the adding process is done
				P(semid2,0);	//the subp2 only loops until subp1 already prints recent a
			}while(tempchar!=EOF);

			fclose(source);
						
			return 0;
		}
	}
	waitpid(0,&status,WUNTRACED);	//wait for all children processes to end


	semctl(semid1,1,IPC_RMID);	//delete the signal,no need of the fourth parameter
	semctl(semid2,1,IPC_RMID);

	return 0;
}
