#include <stdio.h>
#include <stdlib.h>
#include "gpio.h"


#define BUFF_SIZE 1024

int gpioWriteD(int gpioPin, int value)
{
    char filePath[BUFF_SIZE];
	snprintf(filePath, BUFF_SIZE, "/sys/class/gpio/gpio%d/value", gpioPin);

	return fileWriteD(filePath, value);
}


/**
*Return the number in GPIO value file as int
*/
int gpioReadD(int gpioPin)
{
    char filePath[BUFF_SIZE];
	snprintf(filePath, BUFF_SIZE, "/sys/class/gpio/gpio%d/value", gpioPin);

	return fileReadD(filePath);
}

/**
*return '0' if successful
*/
int gpioSetDirection(int gpioPin, _Bool isOut)
{
    int res = 0;

	// Open direction file
	char filePath[BUFF_SIZE];
	snprintf(filePath, BUFF_SIZE, "/sys/class/gpio/gpio%d/direction", gpioPin);

    if (isOut)
        res = fileWriteS(filePath, "out");
    else
        res = fileWriteS(filePath, "in");

    return res;
}

/**
*return '0' if successful
*/
int gpioPinInit(int gpioPin, _Bool isOut)
{
    int res = 0;

    //Export GPIO pinê
    res = fileWriteD(GPIO_EXPORT, gpioPin);

    if (res == 0){
        //Set GPIO pin direction
        res = gpioSetDirection(gpioPin, isOut);
    }

    return res;
}
