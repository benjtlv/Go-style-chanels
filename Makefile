CC = gcc
GTK = `pkg-config --libs --cflags gtk+-3.0`
CFLAGS = -c -std=gnu99 -Wall 
LDLIBS = -lrt 
SOURCES = channel.c static_fifo.c 
OBJECTS = $(SOURCES:.c=.o)
PACKAGE =  chanchanchan-gonzalez-elkrieff-canonici
.PHONY: all 

all : mandelbrot era eratosthene life ring

mandelbrot : $(OBJECTS) mandelbrot.o
	$(CC) $(LDLIBS) -lm $(GTK) $(OBJECTS) mandelbrot.o -o $@

mandelbrot.o : $(OBEJCTS) mandelbrot.c
	$(CC) $(CFLAGS) -O3 -ffast-math -g -pthread $(GTK) mandelbrot.c -o $@

era : $(OBJECTS) era.o
	$(CC) $(LDLIBS) -lm $(OBJECTS) -pthread era.o -o $@

era.o : $(OBJECTS) era.c
	$(CC) $(CFLAGS) era.c -o $@

.c.o : 
	$(CC) $(CFLAGS) $< -o $@

eratosthene: $(OBJECTS) eratosthene.o
	$(CC) $(LDLIBS) -pthread eratosthene.o -o $@

eratosthene.o : eratosthene.c
	$(CC) $(CFLAGS) eratosthene.c -o $@

life : $(OBJECTS) life.o life_game.o
	$(CC) $(LDLIBS) $(GTK) $(OBJECTS) life.o life_game.o -o $@ 

life.o : life.c
	$(CC) $(CFLAGS) $(GTK) life.c -o $@

life_game.o : life_game.c
	$(CC) $(CFLAGS) $(GTK) life_game.c -o $@

ring : $(OBJECTS) ring.o
	$(CC) $(LDLIBS) -pthread $(OBJECTS) ring.o -o $@

ring.o : ring.c
	$(CC) $(CFLAGS) ring.c -o $@

dist:
	rm -fr $(PACKAGE)
	mkdir $(PACKAGE)
	for i in `cat distributed_files`;  do	\
	  if test -f $$i; then					\
	    rsync -R $$i $(PACKAGE);				\
	  else							\
	    mkdir -p $$i;					\
	  fi;							\
	done
	tar cvfz $(PACKAGE).tar.gz $(PACKAGE)

clean :
	rm *.o
	rm mandelbrot
	rm era
	rm eratosthene
	rm life
	rm ring
	rm fichier
