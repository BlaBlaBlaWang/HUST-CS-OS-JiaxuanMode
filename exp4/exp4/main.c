#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>

void printdir(char *dir,int depth){
	DIR *dp;
	struct dirent*entry;
	struct stat statbuf;
	
	if((dp=opendir(dir))==NULL){	//open the direction and get the dirent flow
		printf("opendirection fault!");
		return;
	}

	chdir(dir);	//change the process's relative path to the 

	struct dirent* tempdirent;
	while((tempdirent=readdir(dp))!=NULL){
		printf("%s\n",tempdirent->d_name);
	}

	return;
}

int main(void){
	printdir("test",0);

	return 0;
}
