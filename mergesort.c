#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multiThreadSorter.h"


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
	if(strcmp(left -> data[left -> sortColumn],right -> data[right -> sortColumn]) <= 0)
	{
		next = left;
		next -> nextMovie = mergeSortMerge(left -> nextMovie,right);
		Movie *temp = next -> nextMovie -> previousMovie;
		next -> nextMovie -> previousMovie = next;
		next -> previousMovie = temp;
	}
	else
	{
		next = right;
		next -> nextMovie = mergeSortMerge(left,right -> nextMovie);
		Movie *temp = next -> nextMovie -> previousMovie;
		next -> nextMovie -> previousMovie = next;
		next -> previousMovie = temp;
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
void mergeSortStitcher(Movie **hPtr)
{
	Movie *itter = *hPtr;
	while(itter -> nextMovie != NULL)
	{
		itter = itter -> nextMovie;
	}
	(*hPtr) -> previousMovie = itter;
}
