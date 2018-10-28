all: mergesort.o scannerCSVSorter.c
	gcc mergesort.o scannerCSVSorter.c -o scannerCSVSorter -g

mergesort.o: mergesort.c
	gcc mergesort.c -c -g

clean:
	rm scannerCSVSorter mergesort.o
