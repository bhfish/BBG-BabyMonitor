#ifndef _PARENT_GPIO_H_
#define _PARENT_GPIO_H_

#include <stdio.h>
#include <stdlib.h>
#include "parent_publicFunc.h"

#define GPIO_DRCTN_FILE_PATH(gpioNum) "/sys/class/gpio/gpio" STR_CONVERT(gpioNum) "/direction"
#define GPIO_VALUE_FILE_PATH(gpioNum) "/sys/class/gpio/gpio" STR_CONVERT(gpioNum) "/value"
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"

#define GPIO_FILE_PATH_MAX 100


/**
*Add gpio definition here.
**/
#define GPIO_PIN_JSUP  26  //Joystick Up
#define GPIO_PIN_JSPB  27  //Joystick Pushed
#define GPIO_PIN_JSRT  47  //Joystick Right
#define GPIO_PIN_JSDN  46  //Joystick Down
#define GPIO_PIN_JSLFT 65   //Joystick Left

#define GPIO_PIN_DISPLAY_CLK  2
#define GPIO_PIN_DISPLAY_DIO  3

/*
typedef enum
{
	gpioJSUP = 26,  //Joystick Up
	gpioJSPB = 27,  //Joystick Pushed
	gpioJSRT = 47,  //Joystick Right
	gpioJSDN = 46,  //Joystick Down
	gpioJSLFT= 65   //Joystick Left
}gpioNum_t;


int gpioValueFileRead(gpioNum_t num);


*/


//int gpioDirSetOut(const int gpio);
//int gpioDirSetIn(const int gpio);
//int gpioReadD(const int gpio);
//int gpioExport(const int gpio);
#endif