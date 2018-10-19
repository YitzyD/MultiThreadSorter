#ifndef SIMPLECSVSORTERHEADER_H
#define SIMPLECSVSORTERHEADER_H
#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef enum{FALSE,TRUE} bool;
char **fields;
extern int numFields;
extern int sortColumn;
extern int dbSize;

typedef struct movie
{
	char ** data;
	struct movie *nextMovie;
	struct movie *previousMovie;
} Movie;

extern Movie *head;
void trim(char **strPtr);
int readLine(char **buffer);
int getFields();
int setSortColumn(char *header);
int dbBuilder();
int mergeSort();
int printCSV();
#endif
