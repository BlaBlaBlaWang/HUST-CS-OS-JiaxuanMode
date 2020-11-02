#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/shm.h>
#include<wait.h>
#include<string.h>
#define UNIT_SIZE 100

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
char inputfilename[100];
char outputfilename[100];
int status;

/*
int type=0,type2=0;
int endpoint=-1;
int endseg=-1;
int iequalstozero=0;
*/

char *sharedmemory[11];	//point at the shraed memory segments for use
short semid1num[10]={0,0,0,0,0,0,0,0,0,0};	//semid1 is used for writing process
short semid2num[10]={1,1,1,1,1,1,1,1,1,1};	//semid2 is used for reading process,by setting signals to 1,the first time will complete very fast,then it would wait for writing process to digest

void P(int semid,int index)
//the function itself creates a sembuf struct to define the demanded operation of the signal,and uses semop to operate the signal ofsemid
//so the operation of the signal is wrapped into the call of function P or V
{
	struct sembuf sem;
	sem.sem_num=index;	//in this program,cause there would be a signal set,so the suffix(index) is index
	sem.sem_op=-1;
	sem.sem_flg=SEM_UNDO;	//default parameter
	semop(semid,&sem,1);	//notice the third parameter standing for size of the set
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
	int shmid[11];	//shared memory IDs
	for(int i=0;i<11;i++)
	{
		shmid[i]=shmget(IPC_PRIVATE,UNIT_SIZE,IPC_CREAT|0666);	//allocate 10 byte for each shared memory segment
		sharedmemory[i]=(char*)shmat(shmid[i],NULL,0);		//this function is used to assosiate shared memory with process virtual memory address,the invital parameters are set as default	
	}	

	semid1=semget(IPC_PRIVATE,10,IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);	//create a new system V signal set,the second parameter indicates the size of the set
	semid2=semget(IPC_PRIVATE,10,IPC_CREAT|0666);
	
	union semun arg;
	arg.array=semid1num;
	semctl(semid1,0,SETALL,arg);	//set the initial number of the signal set equal to arg.array,the second parameter would be ignored
	arg.array=semid2num;
	semctl(semid2,0,SETALL,arg);

	int* type=(int *)(sharedmemory[10]+0);
	int* type2=(int *)(sharedmemory[10]+4);
	int* endpoint=(int *)(sharedmemory[10]+8);
	int* endseg=(int *)(sharedmemory[10]+12);
	int* iequalstozero=(int *)(sharedmemory[10]+16);
	
	int writebuf,readbuf;	//writebuf and readbuf
	writebuf=fork();
	if(writebuf==0)
	{		
		
		FILE * destination=fopen(outputfilename,"wb+");

        int i=0,tempseg=0;	//tempseg indicates the seg of shared memory being used,and i indicates the pointer's position inside the 10-byte seg
        char tempchar=1;
        int temppoint=0;

        for(;*type2!=1;)
        {
            P(semid1,tempseg%10);
            
            for(i=0;i<UNIT_SIZE&&!(*type==1&&tempseg%10==*endseg&&(i==*endpoint||(i==0&&*iequalstozero==1)));i++)
            {
                tempchar=*(sharedmemory[tempseg%10]+i);
                fputc(tempchar,destination);
            }
            if(*type==1&&tempseg%10==*endseg&&(i==*endpoint||(i==0&&*iequalstozero==1)))
                *type2=1;
            V(semid2,(tempseg++)%10);	//after processing a seg,the signal and shm also points at the next,by modding it makes a loop
        }
        printf("endseg=%d,endpoint=%d",*endseg,*endpoint);

        fclose(destination);

        
        return 0;
		
	}
	else
	{
		readbuf=fork();
		if(readbuf==0)
		{
			FILE * source=fopen(inputfilename,"rb");

            char tempchar=0;
            int tempseg=0;
            int temppoint=0;

            printf("%d",1);
			do
            {
                P(semid2,(tempseg)%10);	//the readbuf only continues until writebuf already writes recent data,but the first time is special,it can write independently for speed

                for(int i=0;i<UNIT_SIZE;i++)
                {
					if(feof(source))
                    {   
                        if(i==0)
                            *iequalstozero=1;
                        *endpoint=i-1;
                        *endseg=(tempseg%10);
                        printf("endseg=%d!",*endseg);
                        break;
                    }
                    tempchar=fgetc(source);
                    *(sharedmemory[tempseg%10]+i)=tempchar;
                }

                V(semid1,(tempseg++)%10);	//by this order,the writebuf only reads after the reading process is done
                
            }while(!feof(source));	//the EOF is also written into the shm to inform the writebuf

            *type=1;
            fclose(source);
			printf("%d %d",*endpoint,*endseg);
                        
            return 0;
		}
	}
	waitpid(writebuf,&status,WUNTRACED);
	waitpid(readbuf,&status,WUNTRACED);

	
	semctl(semid1,1,IPC_RMID);	//delete the signal,no need of the fourth parameter
	semctl(semid2,1,IPC_RMID);

	for(int i=0;i<10;i++)
		shmctl(shmid[i],IPC_RMID,0);	//delete the shared memory segment after use

	printf("haha");
	
	return 0;
}
