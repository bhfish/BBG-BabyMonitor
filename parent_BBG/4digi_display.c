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

const static displayCharState[4] = {0x77, 0x38, 0x79, 0x6d};

static void wait1(void);
static int setClk(int val);
static int setDio(int val);
static void digiDispStartInput(void);
static void digiDispStopInput(void);
static void digiDispWrite(char data);

pthread_t digiDisplay_thread;
//static char convertChar(char ch,_Bool colon);



//400ns delay
static void wait1(void)
{
	nanosleep(&delay1us, NULL);
}

static int setClk(int val)
{
	int res = fileWriteD(GPIO_VALUE_FILE_PATH(GPIO_PIN_DISPLAY_CLK), val);
	return res;
}

static int setDio(int val)
{
	int res = fileWriteD(GPIO_VALUE_FILE_PATH(GPIO_PIN_DISPLAY_DIO), val);
	return res;
}

/*
static int setGpioClkIn()
{
	int res = 0;
	char* value = "in";

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_DISPLAY_CLK), &value);

	if(res != 0)
	{
		printf("...Error: Set GPIO Clk direction in.\n");
	}

	return res;
}
*/

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

/*
static char convertChar(char ch,_Bool colon)
{
	char val=0;
	if((ASCII_0<=ch)&&(ch<=ASCII_9))
	{
		val=displayDigits[ch-ASCII_0];
	}
	if(colon)
	{
		return val|COLON_FLAG;
	}
	return val;
}
*/

int digiDispNum(int num)
{
	int i;
	int temp[NUM_DIGITS];

	if(num < 0 || num > 9999)
	{
		printf("Error: 4Digi display invalid number: %d", num);
		return -1;
	}

	//Conver number for each decimal position
	temp[0] = (num/1000)%10;
	temp[1] = (num/100)%10;	
	temp[2] = (num/10)%10;
	temp[3] = num%10;


	//Start write to display
	digiDispStartInput();
	digiDispWrite(CMD_AUTO_ADDR);
	digiDispStopInput();

	//Write data
	digiDispStartInput();
	digiDispWrite(START_ADDR);

	for(i = 0; i< NUM_DIGITS; i++)
	{
		digiDispWrite(displayDigits[temp[i]]);
	}
	digiDispStopInput();


	//Stop writing
	digiDispStartInput();
	digiDispWrite(DISPLAY_ON | 0x07);
	digiDispStopInput();

	return 0;
}

static void digiDispTask(void)
{
	while(!stopping)
	{
		if(currentDispMode == dispModeAlarmArm)
		{	
			if(alarmStateArm)
				//Display '1111' if armed 
				digiDispNum(1111);
			else
				//Display '0000' if not armed
				digiDispNum(0);
		}
		else if(currentDispMode == dispModeAlarmSound)

			//Display current buzzer mode number
			digiDispNum(alarmBuzzMode);

		else if(currentDispMode == dispModeTemp)
			digiDispNum(babyRoomTemp);

		else if(currentDispMode == dispModeSound)
			digiDispNum(babySoundLevel);

		nanosleep(&delay100ms, NULL);	
	}
}

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