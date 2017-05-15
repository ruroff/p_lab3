# Makefile 

binary: main.o 
	gcc -o binary main.o -lrt -lpthread

main.o: main.c
	gcc -c main.c


all: binary
	



clean:
	rm -f *.o binary