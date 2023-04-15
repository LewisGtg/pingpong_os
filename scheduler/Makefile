main: scheduler

scheduler: queue.o ppos_core.o pingpong-scheduler.c
	gcc -g -Wall pingpong-scheduler.c ppos_core.o queue.o -o scheduler

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf scheduler