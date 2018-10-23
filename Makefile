all: mergesort.o simpleCSVSorter.c
	gcc mergesort.o simpleCSVSorter.c -o simpleCSVSorter -g

mergesort.o: mergesort.c
	gcc mergesort.c -c -g

clean:
	rm simpleCSVSorter mergesort.o
