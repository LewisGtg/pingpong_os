main: teste1 teste2 teste3

teste1: ppos_core.o teste1.c
	gcc -Wall ppos_core.o teste1.c -o teste1 

teste2: ppos_core.o teste2.c
	gcc -Wall ppos_core.o teste2.c -o teste2 

teste3: ppos_core.o teste3.c
	gcc -Wall ppos_core.o teste3.c -o teste3 

ppos_core.o: ppos_core.c ppos_data.h
	gcc -c -Wall ppos_core.c

clean:
	rm -rf *.o

purge: clean
	rm -rf teste1 teste2 teste3

test:
	./test.sh
