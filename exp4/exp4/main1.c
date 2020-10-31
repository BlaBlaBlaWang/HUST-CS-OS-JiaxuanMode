#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>

int judge(char * d_name){	//to filter the '.' and '..'items,which has many invisible chars in the name string 
	if(d_name[0]=='.'&&d_name[1]==0)
		return 1;
	else if(d_name[0]=='.'&&d_name[1]=='.'&&d_name[2]==0)
		return 1;
	else 
		return 0;
}

void printdir(char *dir,int depth){
	DIR *dp;
	struct dirent*entry;
	struct stat statbuf;
	
	if((dp=opendir(dir))==NULL){	//open the direction and get the dirent flow
		printf("opendirection fault!");
		return;
	}

	chdir(dir);	//change the process's relative path to the same effect as "cd (dir)" in shell

	struct dirent* tempdirent;
	while((tempdirent=readdir(dp))!=NULL){
		if(judge(tempdirent->d_name))
			continue;
		printf("%s\n",tempdirent->d_name);
	}

	return;
}

int main(void){
	printdir("test",0);

	return 0;
}
