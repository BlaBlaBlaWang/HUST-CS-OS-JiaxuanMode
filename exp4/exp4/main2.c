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

void printdir(char *dir,int depth,char *fulldir){
	DIR *dp;
	struct dirent*entry;
	struct stat statbuf;

	//used for stack version
	char relpath[80];
	strcpy(relpath,fulldir);
	strcat(relpath,"/");
	strcat(relpath,dir);
	
	if((dp=opendir(dir))==NULL){	//open the direction and get the dirent flow
		printf("opendirection fault!\n");
		
		return;
	}

	chdir(dir);	//change the process's relative path to the same effect as "cd (dir)" in shell

	struct dirent* tempdirent;
	struct stat tempstate;

	while((tempdirent=readdir(dp))!=NULL){
		
		lstat(tempdirent->d_name,&tempstate);
		if(S_ISDIR(tempstate.st_mode)){	
			if(judge(tempdirent->d_name))
				continue;
		
			printf("THIS IS A DIRENT:%s\n",tempdirent->d_name);
			



			//more information printing would soon be enriched
			printdir(tempdirent->d_name,depth+4,relpath);
		}else{
			printf("%s\n",tempdirent->d_name);
			




			//more information printing would soon be enriched
		}

	}

	chdir("..");	//back to the parent direction
	closedir(dp);	//close the opened dirent flow

	return;
}

int main(void){
	printdir("test",0,".");

	return 0;
}
