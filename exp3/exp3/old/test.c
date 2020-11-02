#include<stdio.h>
int main(void)
{
	FILE* source=fopen("read","r");
	FILE* destination=fopen("write.txt","w+");
	char tempchar;
	while((tempchar=fgetc(source))!=EOF)
	{
		fputc(tempchar,destination);
	}
	fclose(source);
	fclose(destination);

	return 0;
}