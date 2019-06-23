#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<time.h>

int exists = 0;
char fileSystem[20];

typedef struct superBlock
{
	int full;
	int size;
	int fileNum;
	int free;
	char name[20];
}superInstance;

typedef struct fcb
{
	int startAddr;
	int endAddr;
	int size;
	long timeCreated;
	char name[20];
	char free;
	char* remarks;
}fcbInstance;

typedef struct metaFile
{
	superInstance superBlock[1];
	fcbInstance fcb[28];
	int next[28];
}metaFileInstance;

metaFileInstance pfs;

int initialise(FILE *myfile, const char *pfs);
int loadFileSystem(FILE *myfile);
int openFileSystem(const char* pfs);
int put(const char* newfile);
int get(const char* newfile);
int rm(const char* newfile);
int putr(const char* newfile, char* remarks);
int kill(char* fileSystem);
int quit();
int dir();
int file_exists(const char* newfile);

int main()
{
	do{
		printf("PFS>");
		char buffer[1024];
		char *args[20] = {NULL};
		fgets(buffer,1024,stdin);
		buffer[strlen(buffer)-1] = 0;
		printf("%s",buffer);
		if(strcmp(buffer," ")==0 || strcmp(buffer,"\0")==0 || strcmp(buffer,"\n")==0)
			continue;
		if(strcmp(buffer,"quit")==0)
			break;
		if(strcmp(buffer,"dir")==0)
		{
			dir();
		}
		else
		{
			char* firstArgument;
			int i = 0;
			firstArgument = strtok(buffer," ");
			while(firstArgument != NULL)
			{
				args[i] = firstArgument;
				i++;
				firstArgument = strtok(NULL," ");
			}
			if(strcmp(args[0],"open")!=0 && strcmp(args[0],"kill")!=0 &&strcmp(args[0],"rm")!=0 && strcmp(args[0],"put")!=0 && strcmp(args[0],"get")!=0 && strcmp(args[0],"putr")!=0)
			{
				printf("Command error - type open/get/put/putr\n");
				continue;
			}
			else
			{
				if(i!=2)
				{
					printf("Arguments error\n");
					continue;
				}
			}

			if(strcmp(args[0],"open")==0)
			{
				openFileSystem(args[1]);
			}
			if(strcmp(args[0],"put")==0)
			{
				put(args[1]);
			}
			if(strcmp(args[0],"get")==0)
			{
				get(args[1]);
			}
			if(strcmp(args[0],"rm")==0)
			{
				rm(args[1]);
			}
			if(strcmp(args[0],"putr")==0)
			{
				putr(args[1],args[2]);
			}
			if(strcmp(args[0],"kill")==0)
			{
				kill(args[1]);
			}
		}
	}while(1);
		return 0;
}

int initialise(FILE *myfile, const char* fname)
{
	char* buff;
	buff = (char*)malloc(256);
	memset(buff,'\0',256);
	for(int i = 0; i < 40 ; i++)
		fwrite(buff,256,1,myfile);
	pfs.superBlock[0].full = -1;
	pfs.superBlock[0].size = 28;
	pfs.superBlock[0].fileNum = 0;
	pfs.superBlock[0].free = 28;
	memset(pfs.superBlock[0].name,'\0',20);
	strcpy(pfs.superBlock[0].name,fname);
	fseek(myfile,0,SEEK_SET);
	fwrite(pfs.superBlock,sizeof(pfs.superBlock[0]),1,myfile);
	free(buff);
	for(int i = 0; i<sizeof(pfs.fcb)/sizeof(pfs.fcb[0]);i++)
	{
		pfs.fcb[i].free = 0;
		memset(pfs.fcb[i].name, '\0', 20);
		pfs.fcb[i].startAddr = -1;
		pfs.fcb[i].endAddr = -1;
		pfs.fcb[i].size = 0;
		pfs.fcb[i].timeCreated = 0;
	}

	for(int i = 0; i < 28; i++)
	{
		pfs.next[i] = -2;
	}
	buff = (char*)malloc(sizeof(fcbInstance));
	memset(buff,'\0',sizeof(fcbInstance));
	fseek(myfile,256,SEEK_SET);
	for(int i = 0; i < 28; i++)
	{
		fwrite(buff,sizeof(fcbInstance),1,myfile);
	}
	free(buff);
	buff = (char*)malloc(sizeof(int));
	memset(buff,'\0',sizeof(int));
	for(int i = 0; i<28; i++)
	{
		fwrite(buff,sizeof(int),1,myfile);
	}
	free(buff);
	fclose(myfile);
	return 0;
}

int loadFileSystem(FILE *myfile)
{
	fread(pfs.superBlock,256,1,myfile);
	printf("\nNew file system created: \n");
	printf("SIZE:%dbytes--NAME:%s\n",pfs.superBlock[0].size,pfs.superBlock[0].name);
	if(pfs.superBlock[0].size!=28)
		return -1;
	fread(pfs.fcb,sizeof(fcbInstance),28,myfile);
	fseek(myfile,(1+8)*256,SEEK_SET);
	fread(pfs.next,sizeof(int),28,myfile);
	for(int i=0; i<28; i++)
		if(pfs.next[i]==0)
			pfs.next[i]=-2;
			//printf("--%d--\n",pfs.next[i]);
	return 0;
}

int openFileSystem(const char* newfile)
{
	strcpy(fileSystem,newfile);
	FILE *myfile;
	myfile = fopen(newfile,"r");
	if(myfile==NULL)
	{
		myfile = fopen(newfile,"w+");
		initialise(myfile,newfile);
	}
	else
	{
		if(-1==loadFileSystem(myfile))
		{
			printf("change file name\n");
			return -1;
		}
	}
	exists = 1;
	printf("New file system running\n");
	fclose(myfile);
	return 0;
}

int file_exists(const char* newfile)
{
	for(int i=0; i<28; i++)
	{
		if(strcmp(pfs.fcb[i].name,newfile)==0)
			return i;
	}
	return -1;
}

int findFreeFCB()
{
	for(int i=0; i<28; i++)
	{
		if(!pfs.fcb[i].free)
			return i;
	}
	return 1002;
}

int findFreeBlock()
{
	for(int i=0; i<28; i++)
	{
		if(pfs.next[i]==-2)
			return i;
	}
	return 1001;
}

fcbInstance *getFCB(int addr)
{
	return &pfs.fcb[addr];
}

int writeSuperBlock(superInstance *superBlock)
{
	FILE *myfile = fopen(fileSystem,"r+");
	fwrite(superBlock,1,sizeof(superInstance),myfile);
	fclose(myfile);
	return 0;
}

int writeFCB(fcbInstance *fcb,int addr)
{
	FILE *myfile = fopen(fileSystem,"r+");
	fseek(myfile,256+addr*sizeof(fcbInstance),SEEK_SET);
	fwrite(fcb,1,sizeof(fcbInstance),myfile);
	fclose(myfile);
	return 0;
}

int writeFreeBlockList(int addr)
{
	int *p;
	*p = pfs.next[addr];
	FILE *myfile = fopen(fileSystem,"r+");
	fseek(myfile,(1+8)*256+addr*sizeof(int),SEEK_SET);
	fwrite(p,sizeof(int),1,myfile);
	fclose(myfile);
	return 0;
}

int writeData(int blockStart, int* next, void* buff,int size)
{
	printf("New file added with StartingblockAddres--%d-and nextblockAddress-%d---\n",blockStart,*next);
	if(size==0)
		return 0;
	FILE *newf = fopen(fileSystem,"r+");
	fseek(newf,(1+3+8)*256+blockStart*256,SEEK_SET);
	fwrite(next,sizeof(int),1,newf);
	fwrite(buff,size,1,newf);
	fclose(newf);
	return 0;
}

int readData(int startAddr, int* nextBlockAddr,char** buff,int size)
{
	FILE *newf = fopen(fileSystem,"r+");
	fseek(newf,(1+3+8)*256+startAddr*256,SEEK_SET);
	fread(nextBlockAddr,sizeof(int),1,newf);
	fread(*buff,size,1,newf);
	return 0;
}
int kill(char* fileSystem)
{
	int check;
	check = remove(fileSystem);
	if(check==0)
	{
		printf("\n successfully deleted the file system\n");
	}
	else{
		printf("\nUnable to delete file system\n");
	}
	return 0;
}
	
int rm(const char* newfile)
{
	if(exists==0)
	{
		printf("\nOpen file system to delete file\n");
		return -1;
	}
	if(file_exists(newfile)==-1)
	{
		printf("\nthis file does not exist in this filesystem\n");
		return -1;
	}
	else{
		int status;
		status = remove(newfile);
		if(status==0)
		{
			printf("\nsuccessfuly deleted file \n");
		}
		else
		{
			printf("\nError deleting file\n");
		}
	}
	return 0;

}

int put(const char* newfile)
{
	if(exists==0)
	{
		printf("File system not open\n");
		return -1;
	}
	if(file_exists(newfile)!=-1)
	{
		printf("File already exists\n");
		return -1;
	}
	char* buff;
	buff = (char*)malloc(256-sizeof(int));
	FILE *newf;
	newf = fopen(newfile,"r");
	int size = 0;
	if(newf==NULL)
	{
		printf("FIle %s not found\n",newfile);
		return -1;
	}
	int fcbAddr = findFreeFCB();
	fcbInstance* fcb1 = getFCB(fcbAddr);
	pfs.fcb[fcbAddr].free = '1';
	strcpy(pfs.fcb[fcbAddr].name,newfile);
	time_t longTime;
	pfs.fcb[fcbAddr].timeCreated = longTime;
	int i = 0;
	while(!feof(newf))
	{
		int res = fread(buff,(256-sizeof(int)),1,newf);
		int startBlock = findFreeBlock();
		pfs.next[startBlock] = -1;
		int nextBlock = -1;
		if(i==0)
		{
			pfs.fcb[fcbAddr].startAddr = startBlock;
		}
		if(feof(newf))
		{
			size = ftell(newf);
			pfs.fcb[fcbAddr].endAddr = startBlock;
			pfs.fcb[fcbAddr].size = size;
		}
		else
		{
			nextBlock = findFreeBlock();
			if(nextBlock==1001)
			{
				printf("File System is full\n");
				pfs.superBlock[0].full = 1;
				return -1;
			}
		}
		writeData(startBlock,&nextBlock,buff,256-sizeof(int));
		pfs.superBlock[0].free--;
		free(buff);
		buff = (char*)malloc(256-sizeof(int));
		memset(buff,'\0',256-sizeof(int));
		writeFreeBlockList(startBlock);
		i++;
	}
	pfs.superBlock[0].fileNum++;
	writeFCB(&(pfs.fcb[fcbAddr]),fcbAddr);
	writeSuperBlock(&pfs.superBlock[0]);
	fclose(newf);
	return 0;
}

int putr(const char* newfile,char* remarks)
{
	printf("Remarks added to file");
}

int get(const char* newfile)
{
	if(exists==0)
	{
		printf("File System not open\n");
		return -1;
	}

	int fcbAddr = file_exists(newfile);
	if(fcbAddr==-1)
	{
		printf("File does not exist\n");
		return -1;
	}
	int startAddr = pfs.fcb[fcbAddr].startAddr;
	int endAddr = pfs.fcb[fcbAddr].endAddr;
	int size = pfs.fcb[fcbAddr].size;
	int nextBlock = -1;
	int* p = &nextBlock;
	char* buff;
	buff = (char*)malloc(256-sizeof(int));
	memset(buff,'\0',256-sizeof(int));
	FILE *newf;
	newf = fopen(newfile,"w");
	printf("File copied to the current directory\n");
	int j = size/(256-sizeof(int));
	int left = size % (256-sizeof(int));
	int readsize = 256-sizeof(int);
	do
	{
		if(j==0)
		{
			readsize = left;
			free(buff);
			buff = (char*)malloc(readsize);
			memset(buff,'\0',readsize);
		}
		readData(startAddr,p,&buff,readsize);
		startAddr = *p;
		fwrite(buff,readsize,1,newf);
		free(buff);
		buff = (char*)malloc(readsize);
		memset(buff,'\0',readsize);
		j--;
	}while(startAddr!=-1);
	free(buff);
	fclose(newf);
	return 0;
}

int getTime(char** startTime, time_t long_time)
{
	struct tm *newTime;
	time(&long_time);
	struct tm* nextTmNow = localtime(&long_time);
		*startTime = asctime(nextTmNow);
	return 0;
}

int dir()
{
	if(exists==0)
	{
		printf("File system not open\n");
		return -1;
	}
	for(int i=0; i<28; i++)
	{
		char* startTime;
		startTime = (char*)malloc(20);
		memset(startTime,'\0',20);
		getTime(&startTime,pfs.fcb[i].timeCreated);
		if(pfs.fcb[i].free)
		{
			printf("Name\t\tsize\t\tTimeCreated\n");
			printf("%s\t%dbytes\t%s\n",pfs.fcb[i].name,pfs.fcb[i].size,startTime);
		}
	}
	return 0;
}

