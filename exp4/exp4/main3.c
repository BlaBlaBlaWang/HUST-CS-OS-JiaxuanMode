#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<ctype.h>
#include<time.h>

char* userNameFromId(uid_t uid){
	struct passwd *pwd;

	pwd=getpwuid(uid);
	return (pwd==NULL)?NULL:pwd->pw_name;
}

char* groupNameFromId(uid_t gid){
	struct group *grp;

	grp=getgrgid(gid);
	return (grp==NULL)?NULL:grp->gr_name;
}

int judge(char * d_name){	//to filter the '.' and '..'items,which has many invisible chars in the name string 
	if(d_name[0]=='.'&&d_name[1]==0)
		return 1;
	else if(d_name[0]=='.'&&d_name[1]=='.'&&d_name[2]==0)
		return 1;
	else 
		return 0;
}

void printauth(unsigned short mask,unsigned short st_mode,char yes){	//print the authority of the file for diffrent users
	if(mask&st_mode)	//masks such as S_IRUSR working on struct stat's st_mode member
		putchar(yes);
	else 
		putchar('-');
}

void printauth_full(unsigned short st_mode){	//use printauth and print whole authorities
	printauth(S_IRUSR,st_mode,'r');
	printauth(S_IWUSR,st_mode,'w');
	printauth(S_IXUSR,st_mode,'x');

	printauth(S_IRGRP,st_mode,'r');
	printauth(S_IWGRP,st_mode,'w');
	printauth(S_IXGRP,st_mode,'x');

	printauth(S_IROTH,st_mode,'r');
	printauth(S_IWOTH,st_mode,'w');
	printauth(S_IXOTH,st_mode,'x');

	putchar(' ');

	return;
}

void commonprint(struct stat tempstate,struct dirent*tempdirent){	//print the common items for all files
	printauth_full(tempstate.st_mode);
	printf("%d ",tempstate.st_nlink);	//print the hard links of the file,which is equal to the number of its files including itself
	printf("%s ",groupNameFromId(tempstate.st_gid));	//print group name of the file's possession
	printf("%s ",userNameFromId(tempstate.st_uid));	//print user name of the file's possession
	printf("%6lu ",tempstate.st_size);	//print size of the file
	char * time=ctime((time_t*)&(tempstate.st_atime));	//get the time for the specified format
	int i=0;
	while(*time!='\n')
		if(i++>=4)	//skip week information like ls -l
			putchar(*(time++));
		else time++;
	putchar(' ');	
	printf("%s\n",tempdirent->d_name);	//print the name of the file
}

void printdir(char *dir,int depth,char *fulldir){
	DIR *dp;
	struct dirent*tempdirent;
	struct stat tempstate;

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

	while((tempdirent=readdir(dp))!=NULL){
		
		lstat(tempdirent->d_name,&tempstate);
		if(S_ISDIR(tempstate.st_mode)){	
			if(judge(tempdirent->d_name))
				continue;
			putchar('d');	//indicating that this dirent is a folder
			commonprint(tempstate,tempdirent);
			printdir(tempdirent->d_name,depth+4,relpath);
		}else{
			putchar('-');	//indicating that this dirent is a file
			commonprint(tempstate,tempdirent);
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
