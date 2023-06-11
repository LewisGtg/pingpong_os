main: pingpong-mqueue

pingpong-mqueue: queue.o ppos_core.o pingpong-mqueue.c
	gcc -g -Wall pingpong-mqueue.c ppos_core.o queue.o -o pingpong-mqueue -lm

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong-mqueue lgtg20.tar.gz

tar:
	tar -czf lgtg20.tar.gz ppos_core.c ppos_data.h ppos.h queue.c queue.h