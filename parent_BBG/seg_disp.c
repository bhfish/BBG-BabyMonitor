#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "seg_disp.h"
#include "parent_gpio.h"
#include "parent_publicFunc.h"
#include "i2c.h"
#include "parent_process.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_61_DIRECTION "/sys/class/gpio/gpio61/direction"
#define GPIO_44_DIRECTION "/sys/class/gpio/gpio44/direction"
#define GPIO_61_VALUE "/sys/class/gpio/gpio61/value"
#define GPIO_44_VALUE "/sys/class/gpio/gpio44/value"
#define DEVICEs_SLOTS "/sys/devices/platform/bone_capemgr/slots"

#define I2C_DEVICE_ADDRESS 0x20

#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define SEG_DISP_STATE_TOTAL_NUM 4

#define SEG_DISP_CHAR_LEFT_DOWN  0
#define SEG_DISP_CHAR_LEFT_UP    1
#define SEG_DISP_CHAR_RIGHT_DOWN 2
#define SEG_DISP_CHAR_RIGHT_UP   3
#define SEG_DISP_CHAR_TOTAL      4

//display 'SE', 'AL', 'TE', 'SD'
static char segDispState[SEG_DISP_STATE_TOTAL_NUM][SEG_DISP_CHAR_TOTAL] = { {0xb0, 0x44, 0x31, 0x8c}, 
                                                                            {0x91, 0x8e, 0x21, 0x80}, 
                                                                            {0x04, 0x24, 0x31, 0x8c}, 
                                                                            {0xb0, 0x44, 0xa4, 0x26}};

static int segDispFd;
static pthread_t zenDisp_thread;
static _Bool stopped = false;

static void segDisplayLeft(void)
{
    int dispMode = (int)getDispMode();

    gpioWriteD(GPIO_PIN_ZEN_DISP_LEFT, 0);
    gpioWriteD(GPIO_PIN_ZEN_DISP_RIGHT, 0);

    writeI2cReg(segDispFd, REG_OUTA, segDispState[dispMode][SEG_DISP_CHAR_LEFT_DOWN]);
    writeI2cReg(segDispFd, REG_OUTB, segDispState[dispMode][SEG_DISP_CHAR_LEFT_UP]);
    
    gpioWriteD(GPIO_PIN_ZEN_DISP_LEFT, 1);

    sleep_msec(5);
}

static void segDisplayRight(void)
{
    int dispMode = (int)getDispMode();

    gpioWriteD(GPIO_PIN_ZEN_DISP_LEFT, 0);
    gpioWriteD(GPIO_PIN_ZEN_DISP_RIGHT, 0);

    writeI2cReg(segDispFd, REG_OUTA, segDispState[dispMode][SEG_DISP_CHAR_RIGHT_DOWN]);
    writeI2cReg(segDispFd, REG_OUTB, segDispState[dispMode][SEG_DISP_CHAR_RIGHT_UP]);
    
    gpioWriteD(GPIO_PIN_ZEN_DISP_RIGHT, 1);

    sleep_msec(5);
}

void* segDispTask(void* args)
{
	while(!stopped)
	{
        segDisplayLeft();
        segDisplayRight();
        sleep_msec(10);
	}

    return NULL;
}

int segDispInit(void)
{
    gpioPinInit(GPIO_PIN_ZEN_DISP_RIGHT, GPIO_DIR_OUT);
    gpioPinInit(GPIO_PIN_ZEN_DISP_LEFT, GPIO_DIR_OUT);

    //Configure I2C to enable accelerormeter
	segDispFd = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS_14SEG_DISPLAY);
	
    writeI2cReg(segDispFd, REG_DIRA, 0x00);
	writeI2cReg(segDispFd, REG_DIRB, 0x00);

    // Spawn thread
	pthread_create(&zenDisp_thread, NULL, segDispTask, NULL);

    return 0;
}

void segDispCleanup(void)
{
	stopped = true;
	pthread_join(zenDisp_thread, NULL);
}
