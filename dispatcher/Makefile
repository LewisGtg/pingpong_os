main: teste

teste: ppos_core.o teste.c
	gcc -g -Wall teste.c ppos_core.o queue.o -o teste

ppos_core.o: queue.o ppos_core.c ppos_data.h
	gcc -c -Wall queue.o ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf teste