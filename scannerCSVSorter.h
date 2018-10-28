#ifndef SCANNERCSVSORTERHEADER_H
#define SCANNERCSVSORTERHEADER_H
#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef enum{FALSE,TRUE} bool;
//const char *allowedFields[] = {"color","director_name","num_critics_for_review","duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name","actor_1_facebook_likes","gross","genres","actor_1_name","movie_title","num_voted_users","cast_total_facebook_likes","actor_3_name","facenumber_in_poster","plot_keywords","plot_keywords","movie_imdb_link","num_user_for_reviews","language","country","content_rating","budget","title_year","actor_2_facebook_likes","imdb_score","aspect_ratio","movie_facebook_likes"};
extern char **fields;
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
