#ifndef _PARENT_GPIO_H_
#define _PARENT_GPIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

#define GPIO_DRCTN_FILE_PATH(gpioNum) "/sys/class/gpio/gpio" STR_CONVERT(gpioNum) "/direction"
#define GPIO_VALUE_FILE_PATH(gpioNum) "/sys/class/gpio/gpio" STR_CONVERT(gpioNum) "/value"
#define GPIO_EXPORT    "/sys/class/gpio/export"
#define GPIO_UNEXPORT  "/sys/class/gpio/unexport"

#define GPIO_FILE_PATH_MAX 100

#define GPIO_DIR_OUT true
#define GPIO_DIR_IN  false

/**
*Add gpio definition here.
**/
//Joystick
#define GPIO_PIN_JSUP  26   //Joystick Up
#define GPIO_PIN_JSPB  27   //Joystick Pushed
#define GPIO_PIN_JSRT  47   //Joystick Right
#define GPIO_PIN_JSDN  46   //Joystick Down
#define GPIO_PIN_JSLFT 65   //Joystick Left

//extended 4-digit display
#define GPIO_PIN_DISPLAY_CLK  7
#define GPIO_PIN_DISPLAY_DIO  20


//Zen cape display
#define GPIO_PIN_ZEN_DISP_RIGHT 44
#define GPIO_PIN_ZEN_DISP_LEFT  61


/**
*gpio functions.
**/
int gpioWriteD(int gpioPin, int value);
int gpioReadD(int gpioPin);
int gpioSetDirection(int gpioPin, _Bool isOut);
int gpioPinInit(int gpioPin, _Bool isOut);

#endif