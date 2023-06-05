main: pingpong

pingpong: queue.o ppos_core.o pingpong.c
	gcc -g -Wall pingpong.c ppos_core.o queue.o -o pingpong

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong lgtg20.tar.gz

tar:
	tar -czf lgtg20.tar.gz ppos_core.c ppos_data.h ppos.h queue.c queue.h