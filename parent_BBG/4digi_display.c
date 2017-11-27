#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parent_publicFunc.h"
#include "parent_joystick.h"
#include "parent_gpio.h"
#include "parent_process.h"
#include "4digi_display.h"

#define HIGH 1
#define LOW  0

#define DISPLAY_ON 0x88
#define CMD_AUTO_ADDR 0x40
#define START_ADDR 0xc0
#define NUM_DIGITS 4
#define COLON_FLAG 0x80
#define ASCII_0 48
#define ASCII_9 57

const static char displayDigits[10]={0x3f,
									 0x06,
									 0x5b,
									 0x4f,
									 0x66,
									 0x6d,
									 0x7d,
									 0x07,
									 0x7f,
									 0x67,};

const static char displayCharState[4] = {0x77, 0x38, 0x79, 0x6d};

static void wait1(void);
static int setClk(int val);
static int setDio(int val);
static void digiDispStartInput(void);
static void digiDispStopInput(void);
static void digiDispWrite(char data);
static pthread_t digiDisplay_thread;
//static char convertChar(char ch,_Bool colon);


/**
* Delay 1us
**/
static void wait1(void)
{
    sleep_usec(1);
}

/**
* Write to 4digit-display clock line
**/
static int setClk(int val)
{
	int res = fileWriteD(GPIO_VALUE_FILE_PATH(GPIO_PIN_DISPLAY_CLK), val);
	return res;
}

/**
* Write to 4digit-display data line
**/
static int setDio(int val)
{
	int res = fileWriteD(GPIO_VALUE_FILE_PATH(GPIO_PIN_DISPLAY_DIO), val);
	return res;
}

/**
* Set direction of clk gpio to be output
**/
static int setGpioClkOut()
{
	int res = 0;

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_DISPLAY_CLK), "out");

	if(res != 0)
	{
		printf("...Error: Set GPIO Clk direction out.\n");
	}

	return res;
}

/**
* Set direction of data gpio to be input
**/
static int setGpioDioIn()
{
	int res = 0;

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_DISPLAY_DIO), "in");

	if(res != 0)
	{
		printf("...Error: Set GPIO Dio direction in.\n");
	}

	return res;
}

/**
* Set direction of data gpio to be output
**/
static int setGpioDioOut()
{
	int res = 0;

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_DISPLAY_DIO), "out");

	if(res != 0)
	{
		printf("...Error: Set GPIO Dio direction out.\n");
	}

	return res;
}

/**
* Start send data to 4digit-display
**/
static void digiDispStartInput(void)
{
	/*
	* When CLK is high, and DIO goes from high to low, input begins
	*/
	setClk(HIGH);
	setDio(HIGH);
	wait1();
	setDio(LOW);
	wait1();
}

/**
* Stop send data to 4digit-display
**/
static void digiDispStopInput(void)
{
	/*
	* When CLK is high, and DIO goes from low to high, input ends
	*/
	setClk(LOW);
	setDio(LOW);
	wait1();
	setClk(HIGH);
	wait1();
	setDio(HIGH);
	wait1();
}


/**
* Send data to 4digit-display
**/
static void digiDispWrite(char data)
{
	/*
	*Send each bit of data
	*/
	for(int i=0; i<8; i++)
	{
		//transfer data when clock is low, from low bit to high bit
		setClk(LOW);
		setDio(data&0x01);
		data>>=1;
		wait1();
		setClk(HIGH);
		wait1();
	}
	
	/*
	* End of 8th clock cycle is the start of ACK from TM1637
	*/
	setClk(LOW);
	setGpioDioIn();
	wait1();

	//Check that we are getting the ACK from the device
	assert(fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_DISPLAY_DIO))==0);
	setClk(HIGH);
	wait1();
	setClk(LOW);
	setGpioDioOut();
}


/**
* Display number on 4digit-display
**/
int digiDispNum(int num)
{
	if(num < 0 || num > 9999)
	{
		printf("Error: 4Digi display invalid number: %d", num);
		return -1;
	}

	//Start write to display
	digiDispStartInput();
	digiDispWrite(CMD_AUTO_ADDR);
	digiDispStopInput();

	//Write data
	digiDispStartInput();
	digiDispWrite(START_ADDR);

    //Write the first digit
    digiDispWrite(displayCharState[getDispMode()]);

    //Write the second digit
    digiDispWrite(displayDigits[(num/100)%10]);

    //Write the third digit
    digiDispWrite(displayDigits[(num/10)%10]);

    //Write the last digit
    digiDispWrite(displayDigits[num%10]);

	digiDispStopInput();

	//Stop writing
	digiDispStartInput();
	digiDispWrite(DISPLAY_ON | 0x07);
	digiDispStopInput();

	return 0;
}

/**
* 4digit-display main task
**/
static void digiDispTask(void)
{
	while(!isStopping()){
		dispMode_t dispMode = getDispMode();

		if(dispMode == dispModeAlarmArm){	
			if(getAlarmArm())
				//Display '1111' if armed 
				digiDispNum(1111);
			else
				//Display '0000' if not armed
				digiDispNum(0);

		}else if(dispMode == dispModeAlarmSound)
			//Display current buzzer mode number
			digiDispNum(getAlarmBuzzMode());

		else if(dispMode == dispModeTemp)
			digiDispNum(getBbyRoomTemp());

		else if(dispMode == dispModeSound)
			digiDispNum(getBbySoundLevel());

		sleep_msec(100);	
	}
}

/**
* Thread cleanup
**/
void digiDispCleanUp(void)
{
	//Start clear to display
	digiDispStartInput();
	digiDispWrite(CMD_AUTO_ADDR);
	digiDispStopInput();

	//Write data
	digiDispStartInput();
	digiDispWrite(START_ADDR);

    //clear four digits
    digiDispWrite(0);
    digiDispWrite(0);
    digiDispWrite(0);
    digiDispWrite(0);

	digiDispStopInput();

	//Stop writing
	digiDispStartInput();
	digiDispWrite(DISPLAY_ON | 0x07);
	digiDispStopInput();

	fileWriteD(GPIO_UNEXPORT, GPIO_PIN_DISPLAY_DIO);
	fileWriteD(GPIO_UNEXPORT, GPIO_PIN_DISPLAY_CLK);

	pthread_join( digiDisplay_thread, NULL);
}

/**
* Thread initialization
**/
int digiDispInit(void)
{
	int res= 0;

	//Iinit GPIO
	if(res == 0)
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_DISPLAY_DIO);

	if(res == 0)
		res = fileWriteD(GPIO_EXPORT, GPIO_PIN_DISPLAY_CLK);

	if(res == 0)
		res = setGpioDioOut();
	
	if(res == 0)
		res = setGpioClkOut();

	if(res == 0)
		digiDispNum(0);

	if(res == 0)
	{
		res = pthread_create(&digiDisplay_thread, NULL,  (void *)&digiDispTask, NULL);
	
    	if( res )
		{
			printf("Thread creation failed: %d\n", res);
		}
	}
	
	if(res !=0)
		printf("Error: 4 digit display init\n");

	return res;
}