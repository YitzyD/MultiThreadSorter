#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "simpleCSVSorter.h"

/*TODO: set numFields in getFields function by counting words read
	for any csv, would need to know is header or not*/

/*For Debugging: Prints database by column*/
int DEBUG = 0;
int printDB(int column, char *title)
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

int numFields;
int dbSize = 0;
int sortColumn = -1;
Movie *head = NULL;

int main(int argc, char *argv[])
{
	numFields = 28;
	if(argc < 2 || strcmp(argv[1],"-c") != 0)
	{
		dprintf(STDERR,"USE: ./SimpleCSVSorter -c header_name\n");
		return -1;
	}
	if(getFields() == -1)
	{
		return -1;
	}
	if(setSortColumn(argv[2]) == -1)
	{
		dprintf(STDERR,"%s is not a header name in file.\n",argv[2]);
		return -1;
	}
	dbBuilder();
	printDB(sortColumn,"\t\tCSV loaded:");
	mergeSort(&head);
	printDB(sortColumn,"\t\tCSV sorted:");
	printCSV();
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
int readLine(char **buffer)
{
	char in = '~';
	char *input = (char *)(malloc(sizeof(char) * 256));
	char *inputIndexZero = input;
	int counter = 0;
	int index = 0;
	int inString = FALSE; //set true if reading within a string, ie. quotation mark found, ignore delimiter characters until another is found
	while(in != '\n')
	{
		if(read(STDIN,&in,1) == 0)
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
int getFields()
{
	/*Reading line in as header fields*/
	fields = (char **) (malloc(sizeof(char *) * numFields));
	if(readLine(fields) == -1)
	{
		dprintf(STDERR,"CSV not formatted properly.");
		return -1;
	}
}
int dbBuilder()
{
	/*Reading subsequent lines and building movie DB linked list based on read data*/
	Movie *prevMovie = NULL;
	Movie *newMovie = NULL;
	int reading = 1;
	char **buffer;

	while(reading)
	{
		buffer = (char **)(malloc(sizeof(char *) * numFields));;
		if(readLine(buffer) == -1)
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
}
int printCSV()
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
