#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include "multiThreadSorter.h"
#include <stdarg.h>
#include <pthread.h>


/*TODO: *set numFields in getFields function by counting words read
	for any csv, would need to know is header or not
	*check for failed mallocs
*/

/*For Debugging: Prints database by column*/
int DEBUG = 0;
/*int printDB(Movie *head,int column, char *title)
{
	if(!DEBUG){return -1;}
	printf("%s\n",title);
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		printf("%s: %s\n",fields[column],curMovie -> data[column]);
		curMovie = curMovie -> nextMovie;
	}
}*/
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
char *allowedFields[] = {"color","director_name","num_critics_for_review","duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name","actor_1_facebook_likes","gross","genres","actor_1_name","movie_title","num_voted_users","cast_total_facebook_likes","actor_3_name","facenumber_in_poster","plot_keywords","plot_keywords","movie_imdb_link","num_user_for_reviews","language","country","content_rating","budget","title_year","actor_2_facebook_likes","imdb_score","aspect_ratio","movie_facebook_likes"};
int numThreads = 1;
pthread_t initTID;
int numFields;
int dbSize = 0;
//int sortColumn = -1;
char *sortString = NULL;
char *searchDir = NULL;
char *saveDir = NULL;

pthread_mutex_t numThreadsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t allSortedMutex = PTHREAD_MUTEX_INITIALIZER;

//char **fields;
Movie *allSortedCSV = NULL;
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
void *fileHandlerRunner(void *arg)
{
	char *path = (char *)arg;
	fflush(0);
	printf("%lx, ",pthread_self());
	prog_log("Processing file: %s",path);
	if(path == NULL)
	{
		prog_log("Unknown error occured.");
		pthread_exit(NULL);
	}
	if(strcmp(&(path[strlen(path) - 4]),".csv") != 0)
	{
		prog_log("Skipping file: File not a .csv.");
		pthread_exit(NULL);
	}
	/*char *needle = (char *)malloc(256 * sizeof(char));
	needle[0] = '\0';
	strcpy(needle,"-sorted-");
	strcat(needle,sortString);
	if(strstr(path,needle) != NULL)
	{
		prog_log("Skipping sorted csv.");
		free(needle);
		pthread_exit(NULL);
	}*/
	FILE *csv = fopen(path,"r");
	if(csv == NULL)
	{
		prog_log("Error opening file: %s",path);
		pthread_exit(NULL);
	}
	Movie *head;
	char **fields = (char **) (malloc(sizeof(char *) * numFields));
	/*int i;
	for(i = 0;i<numFields;i++)
	{
		fields[i] = (char *)malloc(sizeof(char) * 256);
	}*/
	int buildsts = dbBuilder(csv,&head,fields);
	if(buildsts == -1 || head == NULL)
	{
		pthread_exit(NULL);
	}
	mergeSort(&head);
	mergeSortStitcher(&head);
	/*if(saveDir == NULL)
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
	}*/
	pthread_mutex_lock(&allSortedMutex);
	if(allSortedCSV == NULL)
	{
		allSortedCSV = head;
	}
	else
	{
		Movie *oldEnd = allSortedCSV -> previousMovie;
		oldEnd -> nextMovie = head;
		allSortedCSV -> previousMovie = head -> previousMovie;
		head -> previousMovie = oldEnd;
	}
	pthread_mutex_unlock(&allSortedMutex);
	if(pthread_self() != initTID)
	{
		pthread_exit(NULL);
	}
}
/*Spawns a thread to handle directories*/
void *dirHandlerRunner(void *arg)
{
	fflush(0);
	printf("%lx, ",pthread_self());
	prog_log("Processing dir: %s",(char *)arg);
	scanDir((char *)arg);
	if(pthread_self() != initTID)
	{
		pthread_exit(NULL);
	}
}
/*Scans the directory at path, running the appropriate handler for files/directories*/
int scanDir(char *path)
{
	DIR *dir = opendir(path);
	if(dir == NULL)
	{
		prog_log("Directory: %s does not exist",path);
		return -1;
	}
	struct dirent *content = readdir(dir);
	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * 15);
	int count = 0;
	char **generatedArgs = (char **)malloc(sizeof(char **) * 50);
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
		if(count == sizeof(threads)/sizeof(pthread_t))
		{
			threads = (pthread_t *)realloc(threads,(sizeof(threads) * 2));
		}
		if(count == sizeof(generatedArgs)/sizeof(char **))
		{
			generatedArgs = (char **)realloc(generatedArgs,sizeof(generatedArgs) * 2);
		}
		if(content -> d_type == DT_DIR)
		{
			generatedArgs[count] = newPath;
			pthread_create(&threads[count],NULL,dirHandlerRunner,generatedArgs[count]);
			count++;
		}
		else if(content -> d_type == DT_REG)
		{
			generatedArgs[count] = newPath;
			pthread_create(&threads[count],NULL,fileHandlerRunner,generatedArgs[count]);
			count++;
		}
		content = readdir(dir);
	}
	int i;
	for(i = 0;i < count;i++)
	{
		pthread_join(threads[i],NULL);
	}
	for(i = 0;i < count; i++)
	{
		//free(generatedArgs[i]);
	}
	pthread_mutex_lock(&numThreadsMutex);
	numThreads += count;
	pthread_mutex_unlock(&numThreadsMutex);
	free(content);
	//free(threads);
	return count;
}
/*Initials UI and and controls program order*/
int start()
{
	initTID = pthread_self();
	printf("Initial PID: %d\n",getpid());
	printf("TIDS of all spawned threads: ");
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
		scanDir(searchDir);
		printf("\nTotal number of threads: %d\n",numThreads);
		//do final mergesort
		mergeSort(&allSortedCSV);
		mergeSortStitcher(&allSortedCSV);
		//save final mergesort
		saveCSV(allSortedCSV);
		end();
		return 0;
	}
}
/*Trims leading/trailing whitespace*/
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
			//dbSize++;
			continue;
		}

	}
	return counter;
}
/*Matches the header in fields, and returns the index*/
int getSortColumn(char *header, char **fields)
{
	int i;
	for(i = 0;i<numFields;i++)
	{
		if(strcmp(header,fields[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}
/*Gets the first line of a CSV file into fields*/
int getFields(FILE *csv, char** fields)
{
	/*Reading line in as header fields*/
	if(readLine(csv,fields) == -1)
	{
		//dprintf(STDERR,"CSV not formatted properly.");
		return -1;
	}
	return 0;
}
/*Builds a Movie DB - linked list - into headPtr from a CSV*/
int dbBuilder(FILE *csv, Movie **headPtr, char **fields)
{
	Movie *head = NULL;
	if(getFields(csv, fields) == -1)
	{
		//free(fields);
		return -1;
	}
	int sortColumn = getSortColumn(sortString,fields);
	if(sortColumn == -1)
	{
		//free(fields);
		head = NULL;
		return -1;
	}
	/*Reading subsequent lines and building movie DB linked list based on read data*/
	Movie *prevMovie = NULL;
	Movie *newMovie = NULL;
	int reading = 1;
	char **buffer = (char **)(malloc(sizeof(char *) * numFields));;

	while(reading)
	{
		if(readLine(csv,buffer) == -1)
		{
			reading = 0;
			free(buffer);
			break;
		}
		newMovie = (Movie *)(malloc(sizeof(Movie)));
		newMovie -> sortColumn = sortColumn;
		newMovie -> data = buffer;
		newMovie -> nextMovie = NULL;
		newMovie -> previousMovie = NULL;
		newMovie -> fields = fields;
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
/*Saves the Movie DB in head with the fields line into a new file at SaveDir path*/
int saveCSV(Movie *head,char **fields)
{
	char *newFile = (char *)malloc(256 * sizeof(char));
	newFile[0] = '\0';
	if(saveDir == NULL)
	{
		saveDir = (char *)malloc(sizeof(char*) * 256);
		getcwd(saveDir,256);
	}
	memcpy(newFile,saveDir,strlen(saveDir));
	strcat(newFile,"/AllFiles-sorted-");
	strcat(newFile,sortString);
	strcat(newFile,".csv");
	FILE *out = fopen(newFile,"w");
	if(out == NULL)
	{
		prog_log("Unable to write new file.");
		return -1;
	}
	prog_log("Saving to file: %s",newFile);
	if(head == NULL)
	{
		fprintf(out," ");
		fclose(out);
		free(newFile);
		return 0;
	}
	int i;
	for(i = 0; i < numFields; i++)
	{
		fprintf(out,"%s",allowedFields[i]);
		if(i != numFields -1) {fprintf(out,",");}
	}
	printf("\n");
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		int i = 0;
		int j = 0;
		for(i = 0; i < numFields; i++)
		{
				fprintf(out,"%s",curMovie -> data[i]);/*
			if(strcmp(allowedFields[i],(curMovie -> fields)[i]) == 0)
			{
			}
			else
			{
				fprintf(out," ");
			}*/
			if(i != numFields -1) {fprintf(out,",");}
		}
		fprintf(out,"\n");
		curMovie = curMovie -> nextMovie;
	}
	fclose(out);
	free(newFile);
}
/*
int printCSV(Movie *head)
{
	int i;
	for(i = 0; i < numFields; i++)
	{
		printf("\"%s\"",allowedFields[i]);
		if(i != numFields -1) {printf(",");}
	}
	printf("\n");
	Movie *curMovie = head;
	while(curMovie != NULL)
	{
		int i = 0;
		for(i = 0; i < numFields; i++)
		{
			printf("\"%s\"",curMovie -> data[i]);
			if(i != numFields -1) {printf(",");}
		}
		printf("\n");
		curMovie = curMovie -> nextMovie;
	}
}*/
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
