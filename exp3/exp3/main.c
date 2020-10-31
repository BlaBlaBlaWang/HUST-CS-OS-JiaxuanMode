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
short semid1num[10]={0,0,0,0,0,0,0,0,0,0};
short semid2num[10]={0,0,0,0,0,0,0,0,0,0};

void P(int semid,int index)
//the function itself creates a sembuf struct to define the demanded operation of the signal,and uses semop to operate the signal ofsemid
//so the operation of the signal is wrapped into the call of function P or V
{
	struct sembuf sem;
	sem.sem_num=index;	//in this program,cause there is only one signal,so the suffix(index) is 0
	sem.sem_op=-1;
	sem.sem_flg=SEM_UNDO;	//default parameter
	semop(semid,&sem,10);
	return;
}
void V(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num=index;
	sem.sem_op=1;
	sem.sem_flg=SEM_UNDO;
	semop(semid,&sem,10);
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
		shmid[i]=shmget(IPC_PRIVATE,10,IPC_CREAT|0666);	//allocate 10 byte for each shared memory segment
		sharedmemory[i]=(char*)shmat(shmid[i],NULL,0);		//this function is used to assosiate shared memory with process virtual memory address,the third parameter is set as default	
	}	

	semid1=semget(IPC_PRIVATE,10,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a set of signal values
	semid2=semget(IPC_PRIVATE,10,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
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
		//procession of process 1,it is responsible for reading from the source file 
		FILE* source=fopen("./read.txt","r");
		if(source==NULL)
			printf("OPEN ERROR!");
		//printf("OPEN SUCCEDDED!");
		//file opening

		int i=0,temppoint=0;	//temppoint is used to locate the location of pointer,used along with shared memory
		char tempchar;

		P(semid1,i%10);	//use system V signal to check whether this segment has been emptied

		do{
			tempchar=fgetc(source);
			
			//read from the file,write to the shared memory
			if(temppoint!=10)
			{
				*(sharedmemory[i%10]+temppoint)=tempchar;	//save the read char into the shared memory segment
				temppoint++;
			}
			else 
			{
				V(semid2,(i++)%10);	//reveal the signal value and move i to point at the next shared memory segment
				temppoint=0;	//reveal in-seg pointer for next shared memory segment
				P(semid1,i%10);
				*(sharedmemory[i%10]+temppoint)=tempchar;
				temppoint++;
			}
		}while(tempchar!=EOF&&i<=1000);

		if(temppoint==1)
			V(semid2,i%10);	//in case that child 1 ends at the start of the next seg and cause child 2 to deadlock

		fclose(source);
		printf("Reading done!i equals to %d\n",i);

		return 0;	//after reading child 1 will return
	}
	else
	{
		pid2=fork();
		if(pid2==0)
		{
			//procession of process 2,it is responsible for writing to the destination file
			//file creating
			FILE* destination=fopen("./write","w+");

			int i=0,temppoint=0;	//temppoint is used to locate the location of pointer,used along with shared memory
			char tempchar;

			P(semid2,i%10);	//use system V signal to check whether this segment has been written

			do{
				//read from the shared memory,write to the file
				if(temppoint!=10)
				{
					tempchar=*(sharedmemory[i%10]+temppoint);	//save the read char into the destination file 
					fputc(tempchar,destination);
					temppoint++;
				}
				else 
				{
					V(semid1,(i++)%10);	//reveal the signal value and move i to point at the next shared memory segment
					temppoint=0;	//reveal in-seg pointer for next shared memory segment
					P(semid2,i%10);
					tempchar=*(sharedmemory[i%10]+temppoint);
					fputc(tempchar,destination);
					temppoint++;
				}
			}while(tempchar!=EOF&&i<=1000);

			fclose(destination);

			return 0;
		}
	}
	printf("hai");

	waitpid(0,&status,WUNTRACED);	//wait for all children processes to end

	printf("hai");

	semctl(semid1,1,IPC_RMID);	//delete the signal set,no need of the fourth parameter,the second parameter is ignored 
	semctl(semid2,1,IPC_RMID);

	for(int i=0;i<10;i++)
		shmctl(shmid[i],IPC_RMID,0);	//delete the shared memory segment after use
	return 0;
}
