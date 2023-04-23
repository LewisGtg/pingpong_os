main: preempcao

preempcao: queue.o ppos_core.o pingpong-preempcao.c
	gcc -g -Wall pingpong-preempcao.c ppos_core.o queue.o -o preempcao

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf preempcao