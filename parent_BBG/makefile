CFLAGS= -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror
CC=arm-linux-gnueabihf-gcc $(CFLAGS)
OBJECTS=parent_process.o parent_joystick.o parent_buzzer.o parent_publicFunc.o parent_gpio.o 4digi_display.o tcp_server.o i2c.o seg_disp.o


sorter: $(OBJECTS) 
	$(CC) $(OBJECTS) -lpthread -o proj

clean:
	rm -f proj $(OBJECTS) 

run: proj
	./proj