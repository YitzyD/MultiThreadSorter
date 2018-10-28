#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include "scannerCSVSorter.h"
#include <stdarg.h>

/*TODO: *set numFields in getFields function by counting words read
	for any csv, would need to know is header or not
	*check for failed mallocs
*/

/*For Debugging: Prints database by column*/
int DEBUG = 0;
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
		fflush(0);
		printf("\t[LOG]: \033[32m");
		vprintf(msg,args);
		printf("\033[0m\n");
	}
}
int processCount = 1;
pid_t initPID;
int numFields;
int dbSize = 0;
int sortColumn = -1;
char *sortString = NULL;
char *searchDir = NULL;
char *saveDir = NULL;
//Movie *head = NULL;
/*TODO: allow sub/super sets of allowed cols. check for extra cols to headers/vice versa*/
int main(int argc, char *argv[])
{
	numFields = 28;
	sortString = NULL;
	searchDir = NULL;
	saveDir = NULL;
	if(argc < 3)
	{
		dprintf(STDERR,"USE: ./SimpleCSVSorter -c header_name [-c startingDirectory] [-o outputDirectory]\n");
		return -1;
	}
	int i = 0;
	for(i = 0; i < argc; i++)
	{
		if(strcmp(argv[i],"-c") == 0)
		{
			if(argc == i + 1 || argv[i + 1][0] == '-')
			{
				dprintf(STDERR,"-c requires header_name\n");
				return -1;
			}
			sortString = (char *)malloc(128 * sizeof(char));
			strcpy(sortString,argv[i + 1]);
			i++;
			continue;
		}
		if(strcmp(argv[i],"-d") == 0)
		{
			if(argc == i + 1 || argv[i + 1][0] == '-')
			{
				dprintf(STDERR,"-d requires startingDirectory\n");
				return -1;
			}
			searchDir = (char *) malloc(256 * sizeof(char));
			strcpy(searchDir,argv[i + 1]);
			i++;
			continue;
		}
		if(strcmp(argv[i],"-o") == 0)
		{
			if(argc == i + 1 || argv[i + 1][0] == '-')
			{
				dprintf(STDERR,"-o requires outputDirectory\n");
				return -1;
			}
			saveDir = (char *) malloc(256 * sizeof(char));
			strcpy(saveDir,argv[i + 1]);
			i++;
			continue;
		}
		if(strcmp(argv[i],"--debug") == 0)
		{
			DEBUG = 1;
			continue;
		}
	}
	if(sortString == NULL)
	{
		dprintf(STDERR,"USE: ./SimpleCSVSorter -c header_name [-c startingDirectory] [-o outputDirectory]\n");
		return -1;
	}
	return start();
}
/*
* Forks process to open, process, and sort file. Returns PID of child process,  or -1 if failed
*/
int fileHandler(char *path,char *name)
{
	fflush(0);
	int pid = fork();
	if(pid == 0)
	{
		printf("%d, ",getpid());
		prog_log("Processing file: %s",path);
		if(path == NULL)
		{
			prog_log("Unknown error occured.");
			return -1;
		}
		if(strcmp(&(path[strlen(path) - 4]),".csv") != 0)
		{
			prog_log("Skipping file: File not a .csv.");
			return -1;
		}
		char *needle = (char *)malloc(256 * sizeof(char));
		needle[0] = '\0';
		strcpy(needle,"-sorted-");
		strcat(needle,sortString);
		if(strstr(path,needle) != NULL)
		{
			prog_log("Skipping sorted csv.");
			return -1;
		}
		FILE *csv = fopen(path,"r");
		if(csv == NULL)
		{
			prog_log("Error opening file: %s",path);
			return -1;
		}
		Movie *head;
		int buildsts = dbBuilder(csv,&head);
		if(buildsts == -1 || head == NULL)
		{
			return -1;
		}
		mergeSort(&head);
		if(saveDir == NULL)
		{
			saveCSV(head,path);
		}
		else
		{
			char *newPath = (char *)malloc(256 * sizeof(char));
			strcpy(newPath,saveDir);
			strcat(newPath,name);
			saveCSV(head,newPath);
			free(newPath);
		}
		return 0;
	}
	else
	{
		return pid;
	}
}
/*Forks process to scan directory of path for more files. Returns PID of child process, -1 if failed, or (-1 - the number of children) created if a child*/
int dirHandler(char *path)
{
	fflush(0);
	int pid = fork();
	if(pid == 0)
	{
		printf("%d, ",getpid());
		prog_log("Processing dir: %s",path);
		return -1 - scanDir(path);
	}
	else
	{
		return pid;
	}
}
int scanDir(char *path)
{
	DIR *dir = opendir(path);
	if(dir == NULL)
	{
		prog_log("Directory: %s does not exist",path);
		return -1;
	}
	struct dirent *content = readdir(dir);
	pid_t *children = (pid_t *)malloc(256 * sizeof(pid_t));
	int count = 1;
	while(content != NULL)
	{
		if(strcmp(content -> d_name,".") == 0 || strcmp(content -> d_name,"..") == 0)
		{
			content = readdir(dir);
			continue;
		}
		char *newPath = (char *)malloc(256 * sizeof(char));
		newPath[0] = '\0';
		strcpy(newPath,path);
		if(strcmp(&(path[strlen(path) - 1]),"/") != 0)
		{
			strcat(newPath,"/");
		}
		strcat(newPath,content -> d_name);
		if(content -> d_type == DT_DIR)
		{
			int ret = dirHandler(newPath);
			if(ret <= 0)
			{
				if(ret != -1)
				{
					return (ret + 1) * -1;
				}
				return -1;
			}
			else
			{
				children[count -1] = ret;
				processCount++;
				count++;
			}
		}
		else if(content -> d_type == DT_REG)
		{
			int ret = fileHandler(newPath,content -> d_name);
			if(ret == 0 || ret == -1)
			{
				return 0;
			}
			else
			{
				children[count - 1] = ret;
				count++;
				processCount++;
			}
		}
		free(newPath);
		content = readdir(dir);
	}
	int i;
	for(i = 0;i < count - 1;i++)
	{
		int status = 0;
		waitpid(children[i],&status,0);
		if(WEXITSTATUS(status) > 0)
		{
			processCount += (WEXITSTATUS(status));
		}
	}
	free(content);
	free(children);
	return count;
}
int start()
{
	initPID = getpid();
	printf("Initial PID: %d\n",initPID);
	printf("PIDS of all child processes: ");
	fflush(0);
	if(searchDir == NULL)
	{
		char *path = (char *)malloc(256 * sizeof(char));
		getcwd(path,256);
		scanDir(path);
		free(path);
	}
	else
	{
		int ret = scanDir(searchDir);
		if(getpid() == initPID)
		{
			printf("\nTotal number of processes: %d\n",processCount);
			end();
			return 0;
		}
		return ret;
	}
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
		buffer = (char **)(malloc(sizeof(char *) * numFields));
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
int saveCSV(Movie *head,char *path)
{
	char *newFile = (char *)malloc(256 * sizeof(char));
	newFile[0] = '\0';
	memcpy(newFile,path,strlen(path) - 4);
	newFile[strlen(path) - 3] = '\0';
	strcat(newFile,"-sorted-");
	strcat(newFile,sortString);
	strcat(newFile,".csv");
	FILE *out = fopen(newFile,"w");
	if(out == NULL)
	{
		prog_log("Unable to write new file.");
		return -1;
	}
	prog_log("Saving to file: %s",newFile);
	int i;
	for(i = 0; i < numFields; i++)
	{
		fprintf(out,"\"%s\"",fields[i]);
		if(i != numFields -1) {fprintf(out,",");}
	}
	printf("\n");
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		for(int i = 0; i < numFields; i++)
		{
			fprintf(out,"\"%s\"",curMovie -> data[i]);
			if(i != numFields -1) {fprintf(out,",");}
		}
		fprintf(out,"\n");
		curMovie = curMovie -> nextMovie;
	}
	fclose(out);
	free(newFile);
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
