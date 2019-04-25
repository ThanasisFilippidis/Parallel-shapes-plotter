#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coordinates.h"


int isInRing(float x, float y, float a, float b, float r1, float r2){
	float diffx,diffy;
	diffx=x - a;
	diffx=diffx*diffx;
	diffy=y - b;
	diffy=diffy*diffy;
	r1=r1*r1;
	r2=r2*r2;

	if ((diffx + diffy <= r2) && (diffx + diffy >= r1))
	{
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	if (argc != 10 && argc != 12 && argc != 14)
	{
		printf("Wrong number of arguments.Use is:\n");
		printf("./ring -i InputBinaryFile -o OutputFile -a UtilityArgs [-f O set] [-n PointsToReadCount]\n");

		return 1;
	}

	FILE *filepointerIn, *filepointerOut;


	coordinates record;
	int recordCounter, offset, pointsToRead;
	float a,b,r1,r2;
	long fileSize;
	int flagf, flagn;
	flagf=0;
	flagn=0;

	int i;
	for (i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i],"-i"))
		{
			filepointerIn = fopen (argv[i + 1],"rb");
			if (filepointerIn == NULL)
			{
				printf("Unable to open input file.\n");
				return 1;
			}
			fseek(filepointerIn, 0 , SEEK_END);
			fileSize = ftell(filepointerIn);
			rewind(filepointerIn);
			recordCounter = (int) fileSize/sizeof(record);
			pointsToRead=recordCounter;
		}else if (!strcmp(argv[i],"-o"))
		{
			filepointerOut = fopen (argv[i + 1], "w");
			if (filepointerOut == NULL)
			{
				perror("Unable to open output file.");
			}
		}else if (!strcmp(argv[i],"-a"))
		{
			a=atof(argv[i + 1]);
			b=atof(argv[i + 2]);
			r1=atof(argv[i + 3]);
			r2=atof(argv[i + 4]);
			if (r1 >= r2)
			{
				printf("r2 has to be greater than r1.\n");
				return 1;
			}
		}else if (!strcmp(argv[i],"-f"))
		{
			offset=atoi(argv[i + 1]);
			flagf=1;
		}else if (!strcmp(argv[i],"-n"))
		{
			pointsToRead=atoi(argv[i + 1]);
			flagn=1;
		}
	}

	if (flagf)
	{
		if ((offset%2) != 0)
		{
			printf("Offset has to be even number.\n");
			return 1;
		}
		fseek(filepointerIn,offset,SEEK_SET);
	}

	if (flagn)
	{
		if (pointsToRead > recordCounter)
		{
			printf("Asked for more points than available. All the available are going to be used.\n");
			pointsToRead=recordCounter;
		}
	}


	for (i = 0; i < (pointsToRead); ++i)
	{
		fread(&record, sizeof(record), 1, filepointerIn);
		if (isInRing(record.x,record.y,a,b,r1,r2))
		{
			fprintf(filepointerOut, "%.2f\t%.2f\n", record.x, record.y);
		}
	}
	fclose(filepointerIn);
	fclose(filepointerOut);
	
	return 0;
}