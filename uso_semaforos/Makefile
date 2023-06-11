main: pingpong-prodcons

pingpong-prodcons: queue.o ppos_core.o pingpong-prodcons.c
	gcc -g -Wall pingpong-prodcons.c ppos_core.o queue.o -o pingpong-prodcons

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

queue.o: queue.c queue.h
	gcc -c -Wall queue.c 

clean: 
	rm -rf *.o

purge: clean
	rm -rf pingpong-prodcons lgtg20.tar.gz

tar:
	tar -czf lgtg20.tar.gz ppos_core.c ppos_data.h ppos.h queue.c queue.h