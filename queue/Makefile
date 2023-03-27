main: queue.o testafila.c
	gcc -g -Wall testafila.c queue.o -o testafila

teste: queue.o teste.c
	gcc -g -Wall teste.c queue.o -o teste

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf testafila teste