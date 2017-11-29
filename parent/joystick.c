#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "util.h"
#include "joystick.h"
#include "gpio.h"
#include "alarmMonitor.h"

static joystkDrctn_t joystkDirection= JOYST_NONE;
static pthread_t jsThreadId;

/**
* Function to get joystick dirction
**/
joystkDrctn_t Joystick_getPressedDirection(void){
    return joystkDirection;
}

/**
* Function to determine joystick dirction
**/
static joystkDrctn_t getJsDrctn(void)
{
	if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSUP))){
		return JOYST_UP;
	}else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSDN))){
		return JOYST_DOWN;
	}else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSRT))){
		return JOYST_RIGHT;
	}else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSLFT))){
		return JOYST_LEFT;
	}else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSPB))){
		return JOYST_PUSH;
	}else{
		return JOYST_NONE;
	}
}

/**
* The task to pull joystick dirction every 100ms
**/
static void joystkTask(void)
{
	while(!AlarmMonitor_isStopping()){
		joystkDirection = getJsDrctn();
        sleep_msec(100);
	}
}

/**
* Thread cleanup
**/
void Joystick_cleanUp(void)
{
	printf("...Stopping joystick thread.\n");
	pthread_join(jsThreadId, NULL);
}

/**
* Export GPIO
**/
static int joystkGpioExport(void)
{
	int res = 0;

	res = fileWriteD(GPIO_EXPORT, GPIO_PIN_JSUP);

	if(res == 0){
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_JSPB);
	}

	if(res == 0){
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_JSRT);
	}

	if(res == 0){
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_JSDN);
	}

	if(res == 0){
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_JSLFT);
	}

	return res;
}

/**
* Set GPIO direction
**/
static int joystkGpioSetDrctnIn(void)
{
	int res = 0;

	char* value = "in";

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSUP), value);

	if(res == 0){
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSPB), value);
	}

	if(res == 0){
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSRT), value);
	}

	if(res == 0){
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSDN), value);
	}

	if(res == 0){
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSLFT), value);
	}

	return res;
}

/**
* Thread initialization
**/
int Joystick_init(void)
{
	int res = 0;
	joystkDirection = JOYST_NONE;

	res = joystkGpioExport();

	if(res == 0){
		res = joystkGpioSetDrctnIn();
	}

	if(res == 0){
		printf("...Creating joystick thread.\n");
		res = pthread_create(&jsThreadId, NULL,  (void *)&joystkTask, NULL);
	}

    return res;
}