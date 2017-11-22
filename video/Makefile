CFLAGS= -lv4l2
CC=gcc $(CFLAGS)

all: capture main

capture:
	$(CC) capture.c -o capture

main: 
	gcc main.c -o main -lpthread

clean:
	rm -f capture capture.o
	rm -f main main.o

