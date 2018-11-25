#ifndef SCANNERCSVSORTERHEADER_H
#define SCANNERCSVSORTERHEADER_H
#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef enum{FALSE,TRUE} bool;
//extern char **fields;
extern char *allowedFields[];
extern int numFields;
extern int sortColumn;
extern int dbSize;
extern char *searchPath;
extern char *savePath;

typedef struct movie
{
	char ** data;
	char ** fields;
	struct movie *nextMovie;
	struct movie *previousMovie;
	int sortColumn;
} Movie;

//extern Movie *head;
void trim(char **strPtr);
int start();
void end();
int readLine(FILE *csv,char **buffer);
int getFields(FILE *csv, char **fields);
int setSortColumn(char *header);
int dbBuilder(FILE *csv,Movie **headPtr,char **fields);
int mergeSort(Movie **head);
void mergeSortStitcher(Movie **head);
int printCSV(Movie *head);
int scanDir();
int openCSV();
int saveCSV();
#endif
