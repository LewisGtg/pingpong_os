main: pingpong-semaphore pingpong-racecond

pingpong-semaphore: queue.o ppos_core.o pingpong-semaphore.c
	gcc -g -Wall pingpong-semaphore.c ppos_core.o queue.o -o pingpong-semaphore

pingpong-racecond: queue.o ppos_core.o pingpong-racecond.c
	gcc -g -Wall pingpong-racecond.c ppos_core.o queue.o -o pingpong-racecond

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong-semaphore pingpong-racecond lgtg20.tar.gz

tar:
	tar -czf lgtg20.tar.gz ppos_core.c ppos_data.h ppos.h queue.c queue.h