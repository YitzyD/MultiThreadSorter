#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include "simpleCSVSorter.h"
#include <stdarg.h>

/*TODO: set numFields in getFields function by counting words read
	for any csv, would need to know is header or not*/

/*For Debugging: Prints database by column*/
int DEBUG = 1;
int printDB(Movie *head,int column, char *title)
{
	if(!DEBUG){return -1;}
	printf("%s\n",title);
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		printf("%s: %s\n",fields[column],curMovie -> data[column]);
		curMovie = curMovie -> nextMovie;
	}
}
void prog_log(char *msg,...)
{
	if(DEBUG)
	{
		va_list args;
		va_start(args,msg);
		printf("\t[LOG]: \033[32m");
		vprintf(msg,args);
		printf("\033[0m\n");
	}
}

int numFields;
int dbSize = 0;
int sortColumn = -1;
char *sortString;
char *searchDir = NULL;
char *saveDir = NULL;
//Movie *head = NULL;

int main(int argc, char *argv[])
{
	numFields = 28;
	if(argc < 2 || strcmp(argv[1],"-c") != 0)
	{
		dprintf(STDERR,"USE: ./SimpleCSVSorter -c header_name [-c startingDirectory] [-o outputDirectory]\n");
		return -1;
	}
	else
	{
		sortString = (char *)malloc(128 * sizeof(char));
		strcpy(sortString,argv[2]);
	}
	if(argc > 3 && strcmp(argv[3],"-d") == 0 && strcmp(argv[4],"") != 0)
	{
		searchDir = (char *) malloc(256 * sizeof(char));
		strcpy(searchDir,argv[4]);
	}
	else
	{
		searchDir = NULL;
	}
	if(argc > 5 && strcmp(argv[5],"-o") == 0 && strcmp(argv[6],"") != 0)
	{
		saveDir = (char *) malloc(256 * sizeof(char));
		strcpy(saveDir,argv[6]);
	}
	else
	{
		saveDir = NULL;
	}
	/*if(dbBuilder() == 0)
	{
		printDB(head,sortColumn,"\t\tCSV loaded:");
		mergeSort(&head);
		printDB(head,sortColumn,"\t\tCSV sorted:");
		//printCSV(head);
		saveCSV();
	}*/
	start();
}
int scanDir()
{
	if(searchDir == NULL)
	{
		searchDir = (char *) malloc(256 * sizeof(char));
		getcwd(searchDir,256);
	}
	DIR *dir = opendir(searchDir);
	struct dirent *contents = readdir(dir);
	if(dir == NULL)
	{
		return -1;
	}
	while(contents != NULL)
	{
		if(contents -> d_type == DT_DIR)
		{
			prog_log("Found directory: %s",contents -> d_name);
		}
		else if(contents -> d_type == DT_REG && strcmp(&(contents -> d_name[strlen(contents -> d_name) - 4]),".csv") == 0)
		{
			prog_log("Found CSV file: %s",contents -> d_name);
			char *filePath = (char *)malloc(512 * sizeof(char));
			strcpy(filePath,searchDir);
			strcat(filePath,"/");
			strcat(filePath,contents -> d_name);
			FILE *csv = fopen(filePath,"r");
			if(csv == NULL)
			{
				prog_log("Error opening file: %s",filePath);
				break;
			}
			Movie *head;
			int buildsts = dbBuilder(csv,&head);
			if(buildsts == -1 || head == NULL) {break;}
			mergeSort(&head);
			printDB(head,sortColumn,"\t\tCSV sorted:");
			//printCSV(head);

			fclose(csv);
			free(filePath);
	}
		contents = readdir(dir);
	}
}
int start()
{
	scanDir();
}
void trim(char **strPtr)
{
	char *str = *strPtr;
	/*remove leading spaces*/
	int index = 0;
	char curChar = str[index];
	while(curChar == ' ')
	{
		index++;
		curChar = str[index];
		*strPtr = &(str[index]);
	}
	/*remove trailing spaces*/
	index = strlen(str) - 1;
	curChar = str[index];
	while(curChar == ' ')
	{
		str[index] = '\0';
		index--;
		curChar = str[index];
	}
}
/*Read a line (until \n or EOF) from STDIN using comma as delimiter, placing each input into the buffer string array. Returns number of strings read, or -1 if at EOF*/
int readLine(FILE *csv, char **buffer)
{
	char in = '~'; //yay squigly
	char *input = (char *)(malloc(sizeof(char) * 256));
	char *inputIndexZero = input;
	int counter = 0;
	int index = 0;
	int inString = FALSE; //set true if reading within a string, ie. quotation mark found, ignore delimiter characters until another is found
	while(in != '\n')
	{
		in = fgetc(csv);
		if(in == EOF)
		{
			//reached EOF
			return -1;
		}
		if(in == '"')
		{
			inString = !inString;
			continue;
		}
		input[counter] = in;
		counter++;
		if((in == ',' || in == '\n') && inString == FALSE)
		{
			input[counter - 1] = '\0';
			buffer[index] = (char *) malloc(sizeof(char) * counter);
			trim(&input);
			strcpy(buffer[index],input);
			index++;
			counter = 0;
			dbSize++;
			continue;
		}

	}
	free(inputIndexZero);
	return counter;
}
int setSortColumn(char *header)
{
	int i;
	for(i = 0;i<numFields;i++)
	{
		if(strcmp(header,fields[i]) == 0)
		{
			sortColumn = i;
			return 0;
		}
	}
	return -1;
}
int getFields(FILE *csv)
{
	/*Reading line in as header fields*/
	fields = (char **) (malloc(sizeof(char *) * numFields));
	if(readLine(csv,fields) == -1)
	{
		//dprintf(STDERR,"CSV not formatted properly.");
		return -1;
	}
}
int dbBuilder(FILE *csv, Movie **headPtr)
{
	Movie *head = NULL;
	if(getFields(csv) == -1)
	{
		return -1;
	}
	if(setSortColumn(sortString) == -1)
	{
		head = NULL;
		return -1;
	}
	/*Reading subsequent lines and building movie DB linked list based on read data*/
	Movie *prevMovie = NULL;
	Movie *newMovie = NULL;
	int reading = 1;
	char **buffer;

	while(reading)
	{
		buffer = (char **)(malloc(sizeof(char *) * numFields));;
		if(readLine(csv,buffer) == -1)
		{
			reading = 0;
			free(buffer);
			break;
		}
		newMovie = (Movie *)(malloc(sizeof(Movie)));
		newMovie -> data = buffer;
		newMovie -> nextMovie = NULL;
		if(head == NULL)
		{
			head = newMovie;
		}
		else
		{
			prevMovie -> nextMovie = newMovie;
		}
		newMovie -> previousMovie = prevMovie;
		prevMovie = newMovie;
		buffer = (char **)(malloc(sizeof(char *) * numFields));
	}
	head -> previousMovie = prevMovie;
	*headPtr = head;
}
int printCSV(Movie *head)
{
	int i;
	for(i = 0; i < numFields; i++)
	{
		printf("\"%s\"",fields[i]);
		if(i != numFields -1) {printf(",");}
	}
	printf("\n");
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		for(int i = 0; i < numFields; i++)
		{
			printf("\"%s\"",curMovie -> data[i]);
			if(i != numFields -1) {printf(",");}
		}
		printf("\n");
		curMovie = curMovie -> nextMovie;
	}
}
void end()
{
	if(searchDir != NULL)
	{
		free(searchDir);
	}
	if(saveDir != NULL)
	{
		free(saveDir);
	}
}
