main: pingpong-contab pingpong-contab-prio

pingpong-contab: queue.o ppos_core.o pingpong-contab.c
	gcc -g -Wall pingpong-contab.c ppos_core.o queue.o -o pingpong-contab

pingpong-contab-prio: queue.o ppos_core.o pingpong-contab-prio.c
	gcc -g -Wall pingpong-contab-prio.c ppos_core.o queue.o -o pingpong-contab-prio

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong-contab pingpong-contab-prio