# flags per la compilazione pedantic
CFLAGS = -std=c89 -pedantic 


clean:
	rm -f *.o #pulizia dei file .o

taxi: taxi.c lib_simulazione.h  #compilazione del taxi.c
	gcc -o taxi taxi.c $(CFLAGS)

lib: lib_simulazione.c  #compilazione della libreria
	gcc -c lib_simulazione.c $(CFLAGS)

prog: progetto.c lib_simulazione.h #compilazione del file progetto.c
	gcc progetto.c lib_simulazione.o -o progetto $(CFLAGS)
run:
	./progetto #run dell'eseguibile 
