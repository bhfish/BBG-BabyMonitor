#include <stdio.h>
#include <stdlib.h>
#include "parent_gpio.h"


#define BUFF_SIZE 1024

/**
* Read the value from selected gpio value file, and return the value.
**/
/*
int gpioValueFileRead(gpioNum_t num)
{
	int jValue;
	FILE *pfile;
	char gpioNum[2];


	sprintf(gpioNum, "%d", num);

	pfile = fopen( GPIO_VALUE_FILE_PATH(gpioNum), "r");
	
	if (pfile == NULL) 
	{
		printf("ERROR: Unable to open value file of GPIO %d.\n", num);
		return -1;
	}

	//Get value and conver char to int
	jValue = fgetc(pfile) - '0';

	fclose(pfile);
	
	return jValue;
}
*/
/*
int gpioExport(const int gpio)
{
	int res = 0;
	int value = gpio;

	res = fileWriteD(GPIO_EXPORT, &value);

	if(res != 0)
	{
		printf("...Error: Export GPIO %d.\n", gpio);
	}

	return res;
}
*/
/*
int gpioDirSetOut(const int gpio)
{
	int res = 0;
	char* value = "out";

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(gpio), &value);

	if(res != 0)
	{
		printf("...Error: Set GPIO %d direction out.\n", gpio);
	}

	return res;
}

int gpioDirSetIn(const int gpio)
{
	int res = 0;
	char* value = "in";

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(gpio), &value);

	if(res != 0)
	{
		printf("...Error: Set GPIO %d direction in.\n", gpio);
	}

	return res;
}

int gpioReadD(const int gpio)
{
	return fileReadD(GPIO_VALUE_FILE_PATH(gpio));
}
*/



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

    if (res == 0)
    {
        //Set GPIO pin direction
        res = gpioSetDirection(gpioPin, isOut);
    }

    return res;
}
