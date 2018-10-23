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
extern char *searchPath;
extern char *savePath;

typedef struct movie
{
	char ** data;
	struct movie *nextMovie;
	struct movie *previousMovie;
} Movie;

extern Movie *head;
void trim(char **strPtr);
int start();
void end();
int readLine(FILE *csv,char **buffer);
int getFields(FILE *csv);
int setSortColumn(char *header);
int dbBuilder(FILE *csv,Movie **headPtr);
int mergeSort();
int printCSV(Movie *head);
int scanDir();
int openCSV();
int saveCSV();
#endif
