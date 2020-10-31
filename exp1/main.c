#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<sys/types.h>
#include<wait.h>

int statepara,PID1,PID2;	//statepara is used for waitpid
int pipefd[2];
char usrbuf[23];	//used for sprintf's formated string translation

void sigintfunc(int signo)
{
	//kill children using kill to send user-defined signal
	kill(PID1,SIGUSR1);
	waitpid(PID1,&statepara,WUNTRACED);
	//WUNTRACED表示不光返回终止子进程的信息，还返回因信号而停止的子进程的信息
	kill(PID2,SIGUSR2);
	waitpid(PID2,&statepara,WUNTRACED);
	//wait for children to exit and recycle their resources
	close(pipefd[0]);
	close(pipefd[1]);
	//close pipe for parent
	printf("Parent Process is Killed!\n");
	exit(0);
	//parent exit
}

void child1kill(int sig_no)
{
	close(pipefd[1]);	//when child exits,first close its opening pipe file
	printf("\nChild Process 1 is Killed by Parent!\n");	
	exit(0);
}

void child2kill(int sig_no)
{
	close(pipefd[0]);
	printf("Child Process 2 is Killed by Parent!\n");
	exit(0);
}

void child1(int pipefdtemp)
{
	signal(SIGINT,SIG_IGN);	//used to ignore the keyborad interruption,or ctrl+c will end three processes
	signal(SIGUSR1,child1kill);	//used to define the child1's exiting process
	//process of child 1
	for(int i=1;1;i++)
	{
		sprintf(usrbuf,"I send you %4d times.",i);	//firstly translate formated string
		write(pipefdtemp,usrbuf,23);	//secondly write the string to the pipe
		sleep(1);	//pause for 1s
	}
}

void child2(int pipefdtemp)
{
	signal(SIGINT,SIG_IGN);
	signal(SIGUSR2,child2kill);
	
	char strings[100];
	//process of child 2
	while(1)
	{
		read(pipefdtemp,strings,23);	//read chokely from the pipe
		printf("%s\n",strings);
	}
}

int main(void)
{
	char choice=0;
	do{
		printf("Set upper limit of the information number?(y/n)");
		choice=getchar();
		getchar();
	}while(choice!='y'&&choice!='n');
	printf("The choice is:%c\n",choice);

	int upperlimit=0;
	if(choice=='y'){
		printf("Please type in the limit:");
		scanf("%d",&upperlimit);
	}

	if(pipe(pipefd)<0)	//create 
	{
		printf("pipe creation error!\n");	//print the error of pipe creation
		exit(1);
	}

	PID1=fork();	//create child 1
	if(PID1==0)
	{
		close(pipefd[0]);	//child 1 doesn't need the read end of the pipe
		child1(pipefd[1]);	//child 1 writes to the pipe
	}
	else
	{
		PID2=fork();	//create child 2 only in parent's process
		if(PID2==0)
		{
			close(pipefd[1]);	//child 2 doesn't need the write end of the pipe
			child2(pipefd[0]);	//child 2 reads from the pipe
		}	
	}

	signal(SIGINT,sigintfunc);	//the parent process should change its signal handler after children are set

	if(choice=='n')
		while(1){
			//打印相关的消息
			sleep(1);
			printf("1\n");
		}//parent loops until signal is received
	else{
		while(upperlimit-->0){
			//打印相关的消息
			sleep(1);
			printf("1\n");
		}
		sigintfunc(0);	//切腹自尽	
	}

	return 0;
}