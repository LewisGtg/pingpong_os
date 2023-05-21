main: pingpong-wait

pingpong-wait: queue.o ppos_core.o pingpong-wait.c
	gcc -g -Wall pingpong-wait.c ppos_core.o queue.o -o pingpong-wait

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong-contab pingpong-contab-prio