#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpleCSVSorter.h"


Movie *getMiddle(Movie *h)
{
	Movie *f = h;
	Movie *b = h;
	if(h == NULL || h -> nextMovie == NULL)
	{
		return h;
	}
	while(f != NULL)
	{
		f = f -> nextMovie;
		if(f != NULL)
		{
			b = b -> nextMovie;
			f = f -> nextMovie;
		}
	}
	return b;
}


Movie *mergeSortMerge(Movie *left,Movie *right)
{
	if(left == NULL)
	{
		return right;
	}
	else if(right == NULL)
	{
		return left;
	}
	Movie *next;
	if(strcmp(left -> data[sortColumn],right -> data[sortColumn]) <=0)
	{
		next = left;
		next -> nextMovie = mergeSortMerge(left -> nextMovie,right);
	}
	else
	{
		next = right;
		next -> nextMovie = mergeSortMerge(left,right -> nextMovie);
	}
	return next;
}
int mergeSort(Movie **hPtr)
{
	Movie *h = *hPtr;
	if(h == NULL || h -> nextMovie == NULL)
	{
		return 0;
	}
	Movie *left = h;
	Movie *right = getMiddle(h);
	right -> previousMovie -> nextMovie = NULL;
	Movie *temp = h -> previousMovie;
	h -> previousMovie = right -> previousMovie;
	right -> previousMovie = temp;
	mergeSort(&left);
	mergeSort(&right);
	*hPtr = mergeSortMerge(left,right);
}
