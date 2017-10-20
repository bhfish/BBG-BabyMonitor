CFLAGS= -lv4l2
CC=gcc $(CFLAGS)

all: capture

capture:
	$(CC) capture.c -o capture

clean:
	rm -f capture capture.o

