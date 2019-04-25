#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include "coordinates.h"


char *inputString(FILE* fp, size_t size){
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

char **allShapeArguments(char *shape,int *argCounter){
	char *arg;
	arg=strtok(shape," ");
	size_t size=5;
	//size_t argCounter=0;
	char **allargs = realloc(NULL, sizeof(char*)*size);
	if (!allargs)
		{
			printf("Error on memory allocation.\n");
			return allargs;
		}
	while(arg != NULL){
		allargs[(*argCounter)++]=arg;
		if ((*argCounter) == size)
		{
			allargs = realloc(allargs, sizeof(char*)*(size+=16));
			if (!allargs)
			{
				printf("Error on memory allocation.\n");
				return allargs;
			}
		}
		arg=strtok(NULL," ");
	}
	return realloc(allargs, sizeof(char*)*(*argCounter));
}


int main(int argc, char const *argv[])
{
	if (argc != 7)
	{
		printf("Wrong number of arguments.\n");
		printf("./shapes -i InputBinaryFile -w WorkersCount -d TempDir\n");

		return 1;
	}

	FILE *filepointerIn, *fd;
	coordinates record;
	int recordCounter, numworkers, recPerWorker, tmprc, pos, nbytes, nbytesr, workersPos, inputPos, outputPos, endFlag;
	size_t size, shCounter;
	int *workers, *offsets;
	char *buf, *buf1, *shape, *arg, **allshapes;
	long fileSize;
	endFlag=1;

	int i;
	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i],"-i"))
		{
			filepointerIn = fopen (argv[i + 1],"rb");
			inputPos=i + 1;
			if (filepointerIn == NULL)
			{
				printf("Unable to open input file.\n");
				return 1;
			}
			fseek(filepointerIn, 0 , SEEK_END);
			fileSize = ftell(filepointerIn);
			rewind(filepointerIn);
			recordCounter = (int) fileSize/sizeof(coordinates);		/*store in recordCounter how many coordinates ther are*/
		}else if (!strcmp(argv[i],"-d"))
		{
			mkdir(argv[i + 1], 0777);
			outputPos= i + 1;
		}else if (!strcmp(argv[i],"-w"))
		{
			numworkers=atoi(argv[i + 1]);
			workersPos=i +1;
			workers=realloc(NULL, sizeof(int)*numworkers);
			offsets=realloc(NULL, sizeof(int)*numworkers);
		}
	}
	tmprc=recordCounter;
	pos=0;
	for (i = 0; i < numworkers; ++i)
	{
		workers[i]=0;
	}
	while(tmprc != 0){		/*divide equally the coordinates to each worker and save the number of coordinates of each*/
		workers[pos]=workers[pos] + 1;		/*worker to his cell*/
		pos++;
		if (pos == numworkers)
		{
			pos=0;
		}
		tmprc--;
	}
	offsets[0]=0;
	for (i = 1; i < numworkers; ++i)
	{
		offsets[i]=offsets[i - 1] + workers[i - 1]*sizeof(coordinates);		/*compute each workers offset*/
	}
	int cliCounter=0;
	while(endFlag){		/*while th user doesnt type exit*/
		printf("Give a shape arg1 arg2 arg3,shape2 arg21 arg 22 ..; or exit;\n");
		buf1= inputString(stdin, 10);		/*read the cli line*/
		char *buf2;
		buf2=realloc(NULL, sizeof(char)*strlen(buf1) + 1);
		strcpy(buf2,buf1);
		buf2[strlen(buf1)]='\0';
		buf=strtok(buf1,";");
		if (!strcmp(buf,buf2))		/*check if ; was given as a termination line character*/
		{
			printf("; as termination symbol must be used.\n");
			return 1;
		}
		shape=strtok(buf,",");
		size=5;
		shCounter=0;
		allshapes = realloc(NULL, sizeof(char*)*size);
		if (!allshapes)
			{
				printf("Error on memory allocation.\n");
				return 1;
			}
		while(shape != NULL){		/*have a shape and its arguments in each cell of array allshapes*/
			allshapes[shCounter++]=shape;
			if (shCounter == size)
			{
				allshapes = realloc(allshapes, sizeof(char*)*(size+=16));
				if (!allshapes)
				{
					printf("Error on memory allocation.\n");
					return 1;
				}
			}
			shape=strtok(NULL,",");
		}
		free(buf2);
		allshapes=realloc(allshapes, sizeof(char*)*shCounter);
		char **allshapesSep[shCounter];
		pid_t pid;
		int argCounter[shCounter];
		for (i = 0; i < shCounter; ++i)
		{
			argCounter[i]=0;
		}
		for (i = 0; i < shCounter; ++i)
		{
			allshapesSep[i]=allShapeArguments(allshapes[i],&argCounter[i]);	/*have an argument in each cell of array allshapesSep*/
		}
		int handlerIDs[shCounter];
		char **colors=realloc(NULL,sizeof(char*)*shCounter);
		cliCounter++;
		for (i = 0; i < shCounter; ++i)
		{
			if (!strcmp(allshapesSep[i][0],"circle"))
			{
				pid = fork();
				if(pid <0){
					perror("fork");
					return 1;
				}else if(pid == 0){
					pid_t parentID = getpid();
	                int *workersIDs = realloc(NULL,sizeof(int)*numworkers);
	                char handlerOut[50];
	                sprintf(handlerOut,"%s/%d.out", argv[outputPos], parentID);		/*create the handler output file name*/
	                FILE* hfilepointerOut = fopen(handlerOut, "w");		/*open handler output file*/
	                int j;
					for (j = 0; j < numworkers; ++j)
					{
						struct pollfd fdarray [1];
						int rc, bytes_in;
						char *in;
						pid_t wid;
						int filepointerOut;
						in=malloc(1000);
						char workerOut[100];
						sprintf(workerOut,"%s/%d_w%d.fifo",argv[outputPos], getpid(),j);	/*create the worker output file name*/
						int retval=mkfifo(workerOut, 0777);	/*create a pipe for each worker*/
						if (retval == -1)
						{
							perror("mkfifo fail/n");
							exit(-1);
						}
						wid=fork();
						if (wid<0){
							perror("fork");
						}else if (wid == 0)
						{
							char s1[20],s2[20];
							sprintf(s1,"%d",offsets[j]);
							sprintf(s2,"%d",workers[j]);
							if (execlp("./circle","circle","-i",argv[inputPos],"-o",workerOut,"-a",/*exec the shape asked by user*/
								allshapesSep[i][1],allshapesSep[i][2],allshapesSep[i][3],"-f",s1,
								"-n",s2, (char*)NULL) == -1)
							{
									perror("execlp failed");
									exit(-1);
							}
						}else if (wid > 0)
						{
							filepointerOut=open(workerOut, O_RDWR,0);
							if (filepointerOut == -1)
							{
								perror("Read fifo fail");
								exit(-1);
							}
							for (;;){
								/*  initialize  poll  parameters  */
								fdarray [0].fd = filepointerOut;
								fdarray [0]. events = POLLIN;
								/*  wait  for  incomign  data  or  poll  timeout  */
								rc = poll(fdarray , 1, 300);
								if (rc == 0) {
									//printf("Poll timed -out.\n");
									close (filepointerOut);
									break;
								}
								else if ( (rc == 1) && (fdarray [0]. revents  ==  POLLIN)   ){
									if ( fdarray [0].fd == filepointerOut ){
										bytes_in = read(filepointerOut, in , 1000 );
										in[bytes_in ]='\0';
										fwrite(in,sizeof(char),strlen(in),hfilepointerOut);
										close(filepointerOut);
										break;
									}
								}else if (rc == -1)
								{
									close (filepointerOut);
								}
							}
						}
						remove(workerOut);		/*remove each workers pipe*/
					}
					fclose(hfilepointerOut);
					return 1;
				}else if (pid > 0)
				{
					handlerIDs[i]=pid;
					colors[i]=realloc(NULL,sizeof(char)*strlen(allshapesSep[i][4]) + 1);
					strcpy(colors[i],allshapesSep[i][4]);		/*keep each shapes color*/
					colors[i][strlen(allshapesSep[i][4])]='\0';
				}
			}else if (!strcmp(allshapesSep[i][0],"semicircle"))
			{
				pid = fork();
				if(pid <0){
					perror("fork");
					return 1;
				}else if(pid == 0){
					pid_t parentID = getpid();
	                int *workersIDs = realloc(NULL,sizeof(int)*numworkers);
	                char handlerOut[50];
	                sprintf(handlerOut,"%s/%d.out", argv[outputPos], parentID);
	                FILE* hfilepointerOut = fopen(handlerOut, "w");
	                int j;
					for (j = 0; j < numworkers; ++j)
					{
						struct pollfd fdarray [1];
						int rc, bytes_in;
						char *in;
						pid_t wid;
						int filepointerOut;
						in=malloc(1000);
						char workerOut[100];
						sprintf(workerOut,"%s/%d_w%d.fifo",argv[outputPos], getpid(),j);
						int retval=mkfifo(workerOut, 0777);
						if (retval == -1)
						{
							perror("mkfifo fail/n");
							exit(-1);
						}
						wid=fork();
						if (wid<0){
							perror("fork");
						}else if (wid == 0)
						{
							char s1[20],s2[20];
							sprintf(s1,"%d",offsets[j]);
							sprintf(s2,"%d",workers[j]);
							if (execlp("./semicircle","semicircle","-i",argv[inputPos],"-o",workerOut,"-a",
								allshapesSep[i][1],allshapesSep[i][2],allshapesSep[i][3],allshapesSep[i][4],"-f",s1,
								"-n",s2, (char*)NULL) == -1)
							{
									perror("execlp failed");
									exit(-1);
							}
						}else if (wid > 0)
						{
							filepointerOut=open(workerOut, O_RDWR,0);
							if (filepointerOut == -1)
							{
								perror("Read fifo fail");
								exit(-1);
							}
							for (;;){
								/*  initialize  poll  parameters  */
								fdarray [0].fd = filepointerOut;
								fdarray [0]. events = POLLIN;
								/*  wait  for  incomign  data  or  poll  timeout  */
								rc = poll(fdarray , 1, 300);
								if (rc == 0) {
									//printf("Poll timed -out.\n");
									close (filepointerOut);
									break;
								}
								else if ( (rc == 1) && (fdarray [0]. revents  ==  POLLIN)   ){
									if ( fdarray [0].fd == filepointerOut ){
										bytes_in = read(filepointerOut, in , 1000 );
										in[bytes_in ]='\0';
										fwrite(in,sizeof(char),strlen(in),hfilepointerOut);
										close(filepointerOut);
										break;
									}
								}else if (rc == -1)
								{
									close (filepointerOut);
								}
							}
						}
						remove(workerOut);
					}
					fclose(hfilepointerOut);
					return 1;
				}else if (pid > 0)
				{
					handlerIDs[i]=pid;
					colors[i]=realloc(NULL,sizeof(char)*strlen(allshapesSep[i][5]) + 1);
					strcpy(colors[i],allshapesSep[i][5]);
					colors[i][strlen(allshapesSep[i][5])]='\0';
				}

			}else if (!strcmp(allshapesSep[i][0],"ring"))
			{
				pid = fork();
				if(pid <0){
					perror("fork");
					return 1;
				}else if(pid == 0){
					pid_t parentID = getpid();
	                int *workersIDs = realloc(NULL,sizeof(int)*numworkers);
	                char handlerOut[50];
	                sprintf(handlerOut,"%s/%d.out", argv[outputPos], parentID);
	                FILE* hfilepointerOut = fopen(handlerOut, "w");
	                int j;
					for (j = 0; j < numworkers; ++j)
					{
						struct pollfd fdarray [1];
						int rc, bytes_in;
						char *in;
						pid_t wid;
						int filepointerOut;
						in=malloc(1000);
						char workerOut[100];
						sprintf(workerOut,"%s/%d_w%d.fifo",argv[outputPos], getpid(),j);
						int retval=mkfifo(workerOut, 0777);
						if (retval == -1)
						{
							perror("mkfifo fail/n");
							exit(-1);
						}
						wid=fork();
						if (wid<0){
							perror("fork");
						}else if (wid == 0)
						{
							char s1[20],s2[20];
							sprintf(s1,"%d",offsets[j]);
							sprintf(s2,"%d",workers[j]);
							if (execlp("./ring","ring","-i",argv[inputPos],"-o",workerOut,"-a",
								allshapesSep[i][1],allshapesSep[i][2],allshapesSep[i][3],allshapesSep[i][4],"-f",s1,
								"-n",s2, (char*)NULL) == -1)
							{
									perror("execlp failed");
									exit(-1);
							}
						}else if (wid > 0)
						{
							filepointerOut=open(workerOut, O_RDWR,0);
							if (filepointerOut == -1)
							{
								perror("Read fifo fail");
								exit(-1);
							}
							for (;;){
								/*  initialize  poll  parameters  */
								fdarray [0].fd = filepointerOut;
								fdarray [0]. events = POLLIN;
								/*  wait  for  incomign  data  or  poll  timeout  */
								rc = poll(fdarray , 1, 300);
								if (rc == 0) {
									//printf("Poll timed -out.\n");
									close (filepointerOut);
									break;
								}
								else if ( (rc == 1) && (fdarray [0]. revents  ==  POLLIN)   ){
									if ( fdarray [0].fd == filepointerOut ){
										bytes_in = read(filepointerOut, in , 1000 );
										in[bytes_in ]='\0';
										fwrite(in,sizeof(char),strlen(in),hfilepointerOut);
										close(filepointerOut);
										break;
									}
								}else if (rc == -1)
								{
									close (filepointerOut);
								}
							}
						}
						remove(workerOut);
					}
					fclose(hfilepointerOut);
					return 1;
				}else if (pid > 0)
				{
					handlerIDs[i]=pid;
					colors[i]=realloc(NULL,sizeof(char)*strlen(allshapesSep[i][5]) + 1);
					strcpy(colors[i],allshapesSep[i][5]);
					colors[i][strlen(allshapesSep[i][5])]='\0';
				}
			}else if (!strcmp(allshapesSep[i][0],"square"))
			{
				pid = fork();
				if(pid <0){
					perror("fork");
					return 1;
				}else if(pid == 0){
					pid_t parentID = getpid();
	                int *workersIDs = realloc(NULL,sizeof(int)*numworkers);
	                char handlerOut[50];
	                sprintf(handlerOut,"%s/%d.out", argv[outputPos], parentID);
	                FILE* hfilepointerOut = fopen(handlerOut, "w");
	                int j;
					for (j = 0; j < numworkers; ++j)
					{
						struct pollfd fdarray [1];
						int rc, bytes_in;
						char *in;
						pid_t wid;
						int filepointerOut;
						in=malloc(1000);
						char workerOut[100];
						sprintf(workerOut,"%s/%d_w%d.fifo",argv[outputPos], getpid(),j);
						int retval=mkfifo(workerOut, 0777);
						if (retval == -1)
						{
							perror("mkfifo fail/n");
							exit(-1);
						}
						wid=fork();
						if (wid<0){
							perror("fork");
						}else if (wid == 0)
						{
							char s1[20],s2[20];
							sprintf(s1,"%d",offsets[j]);
							sprintf(s2,"%d",workers[j]);
							if (execlp("./square","square","-i",argv[inputPos],"-o",workerOut,"-a",
								allshapesSep[i][1],allshapesSep[i][2],allshapesSep[i][3],"-f",s1,
								"-n",s2, (char*)NULL) == -1)
							{
									perror("execlp failed");
									exit(-1);
							}
						}else if (wid > 0)
						{
							filepointerOut=open(workerOut, O_RDWR,0);
							if (filepointerOut == -1)
							{
								perror("Read fifo fail");
								exit(-1);
							}
							for (;;){
								/*  initialize  poll  parameters  */
								fdarray [0].fd = filepointerOut;
								fdarray [0]. events = POLLIN;
								/*  wait  for  incomign  data  or  poll  timeout  */
								rc = poll(fdarray , 1, 300);
								if (rc == 0) {
									//printf("Poll timed -out.\n");
									close (filepointerOut);
									break;
								}
								else if ( (rc == 1) && (fdarray [0]. revents  ==  POLLIN)   ){
									if ( fdarray [0].fd == filepointerOut ){
										bytes_in = read(filepointerOut, in , 1000 );
										in[bytes_in ]='\0';
										fwrite(in,sizeof(char),strlen(in),hfilepointerOut);
										close(filepointerOut);
										break;
									}
								}else if (rc == -1)
								{
									close (filepointerOut);
								}
							}
						}
						remove(workerOut);
					}
					fclose(hfilepointerOut);
					return 1;
				}else if (pid > 0)
				{
					handlerIDs[i]=pid;
					colors[i]=realloc(NULL,sizeof(char)*strlen(allshapesSep[i][4]) + 1);
					strcpy(colors[i],allshapesSep[i][4]);
					colors[i][strlen(allshapesSep[i][4])]='\0';
				}
			}else if (!strcmp(allshapesSep[i][0],"ellipse"))
			{
				pid = fork();
				if(pid <0){
					perror("fork");
					return 1;
				}else if(pid == 0){
					pid_t parentID = getpid();
	                //printf("Execution time! : circle\n");
	                int *workersIDs = realloc(NULL,sizeof(int)*numworkers);
	                char handlerOut[50];
	                sprintf(handlerOut,"%s/%d.out", argv[outputPos], parentID);
	                FILE* hfilepointerOut = fopen(handlerOut, "w");
	                int j;
					for (j = 0; j < numworkers; ++j)
					{
						struct pollfd fdarray [1];
						int rc, bytes_in;
						char *in;
						pid_t wid;
						int filepointerOut;
						in=malloc(1000);
						char workerOut[100];
						sprintf(workerOut,"%s/%d_w%d.fifo",argv[outputPos], getpid(),j);
						int retval=mkfifo(workerOut, 0777);
						if (retval == -1)
						{
							perror("mkfifo fail/n");
							exit(-1);
						}
						wid=fork();
						if (wid<0){
							perror("fork");
						}else if (wid == 0)
						{
							char s1[20],s2[20];
							sprintf(s1,"%d",offsets[j]);
							sprintf(s2,"%d",workers[j]);
							if (execlp("./ellipse","ellipse","-i",argv[inputPos],"-o",workerOut,"-a",
								allshapesSep[i][1],allshapesSep[i][2],allshapesSep[i][3],allshapesSep[i][4],"-f",s1,
								"-n",s2, (char*)NULL) == -1)
							{
									perror("execlp failed");
									exit(-1);
							}
						}else if (wid > 0)
						{
							filepointerOut=open(workerOut, O_RDWR,0);
							if (filepointerOut == -1)
							{
								perror("Read fifo fail");
								exit(-1);
							}
							for (;;){
								/*  initialize  poll  parameters  */
								fdarray [0].fd = filepointerOut;
								fdarray [0]. events = POLLIN;
								/*  wait  for  incomign  data  or  poll  timeout  */
								rc = poll(fdarray , 1, 300);
								if (rc == 0) {
									//printf("Poll timed -out.\n");
									close (filepointerOut);
									break;
								}
								else if ( (rc == 1) && (fdarray [0]. revents  ==  POLLIN)   ){
									if ( fdarray [0].fd == filepointerOut ){
										bytes_in = read(filepointerOut, in , 1000 );
										in[bytes_in ]='\0';
										fwrite(in,sizeof(char),strlen(in),hfilepointerOut);
										close(filepointerOut);
										break;
									}
								}else if (rc == -1)
								{
									close (filepointerOut);
								}
							}
						}
						remove(workerOut);
					}
					fclose(hfilepointerOut);
					return 1;
				}else if (pid > 0)
				{
					handlerIDs[i]=pid;
					colors[i]=realloc(NULL,sizeof(char)*strlen(allshapesSep[i][5]) + 1);
					strcpy(colors[i],allshapesSep[i][5]);
					colors[i][strlen(allshapesSep[i][5])]='\0';
				}
			}else if (!strcmp(allshapesSep[i][0],"exit")){
				endFlag=0;
			}
			else{
				printf("Not available shape.\n");
				return 1;
			}
		}
		for (i = 0; i < shCounter; ++i)
		{
			wait(NULL);		/*wait for all the handlers to finish*/
		}
		if (endFlag)		/*if user didnt gave yet end command*/
		{
			FILE *gnu;
			char scriptName[50];
			sprintf(scriptName,"%d_script.gnuplot",cliCounter);		/*create the scripts name*/
			gnu=fopen(scriptName,"w");		/*open the script to start writing there*/
			if (gnu == NULL)
			{
				printf("Error opening gnu file to write.\n");
				return 1;
			}
			fprintf(gnu, "set terminal png\nset size ratio -1\nset output \"./%d_image.png\"\nplot \\\n",cliCounter);
			for (i = 0; i < shCounter; ++i)
			{
				fprintf(gnu, "\"%s/%d.out\" notitle with points pointsize 0.5 linecolor rgb \"%s\",\\\n", argv[outputPos],handlerIDs[i],colors[i]);
			}
			pid_t gpid;
			gpid=fork();		/*create a child to exec the gnuplot as the parent continues to next cli line*/
			if (gpid == 0)
			{
				execlp("gnuplot","gnuplot",scriptName,NULL);
			}
			fclose(gnu);		/*close the script to start writing there*/
		}
	}

	fclose(filepointerIn);
	free(workers);
	free(offsets);
	char tmpPath[50];
	sprintf(tmpPath,"rm -r ./%s", argv[outputPos]);
	system(tmpPath);		/*remove the temp directory*/
	return 0;
}