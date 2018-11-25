all: mergesort.o multiThreadSorter.c
	gcc mergesort.o multiThreadSorter.c -o multiThreadSorter -g -pthread

mergesort.o: mergesort.c
	gcc mergesort.c -c -g

clean:
	rm multiThreadSorter mergesort.o
