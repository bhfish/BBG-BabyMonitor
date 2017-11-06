#include <stdbool.h>
#include <time.h>
#include "parent_pmw.h"
#include "parent_publicFunc.h"


/*
* Steps to configure BBG
*
* 1. Load universal cape to slot manager
*    # echo cape-universaln > $SLOTS
*
* 2. Configure the CPU’s pin connected to the buzzer for use with PWM
*    # config-pin P9_22 pwm
*
* 3. Begin configuring the PWM functionality for the buzzer by exporting it:
#    echo 0 > /sys/class/pwm/pwmchip2/export
*/

/*
#define CAPE_UNIVERAL   "cape-universaln"


#define PMW_BUZZER_CONFIG_PATH "/sys/class/pwm/pwmchip2/export"
#define PMW_LED_R_CONFIG_PATH  "/sys/class/pwm/pwmchip4/export"
#define PMW_LED_G_CONFIG_PATH  "/sys/class/pwm/pwmchip6/export"
#define PMW_LED_B_CONFIG_PATH  "/sys/class/pwm/pwmchip4/export"

#define PMW_SYS_FS_BUZZER(FP) "/sys/class/pwm/pwmchip2/pwm0/" FP
#define PMW_SYS_FS_LED_R(FP)  "/sys/class/pwm/pwmchip4/pwm1/" FP
#define PMW_SYS_FS_LED_G(FP)  "/sys/class/pwm/pwmchip6/pwm0/" FP
#define PMW_SYS_FS_LED_B (FP) "/sys/class/pwm/pwmchip4/pwm0/" FP



typedef enum{
	pwmBuzzer = 0;
	pwmLedR,
	pwmLedG,
	pwmLedB
	pwmDeviceTotal
}pwmUse_t;

typedef struct
{
	char chipNum;
	char pwmNum;
}pwmDevice_t;


pmwDevice_t pwmList[pwmDeviceTotal];

*/

/*
static void load_uniCape(void)
{

}

static int pmwConfigExport(char* filePath)
{
	FILE *pfile = NULL;
	int res = 0;

	pfile = fopen(filePath, "w");

	if (pfile == NULL) 
	{
		printf("[PMW] ERROR: Unable to open config file for write.\n");
		return -1;
	}

	if (fprintf(pfile, "0") <= 0) 
	{ 
		printf("[PMW] ERROR: WRITING DATA\n");
		res = -1;
	}

	fclose(pfile);
	return res;
}
*/

/*
int pmwConfigExport(pmwUse_t device)
{
	int res = 0;
	int value = 0;

	res = fileWriteD(PMW_CONFIG_PATH(PMW_CHIP_NUM_BUZZ), &value);

	return res;
}

int pmwValuePeriod(pmwUse_t device, int value)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_PERIOD(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), &value);

	return res;
}

int pmwValueCycle(pmwUse_t device, int value)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_CYCLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), &value);

	return res;
}

int pmwValueEnable(pmwUse_t device, int value)
{
	int res = 0;

	res = fileWriteD(PMW_SYS_FS_ENABLE(PMW_CHIP_NUM_BUZZ, PMW_NUM_BUZZ), &value);

	return res;
}



int pmwLedRInit(void)
{

}

int pmwLedGInit(void)
{

}

int pmwLedBInit(void)
{

}


void pmwInit(void)
{

	pwmList[pwmBuzzer].chipNum = '2';
	pwmList[pwmBuzzer].pwmNum  = '0';

	pwmList[pwmLedR].chipNum = '4';
	pwmList[pwmLedR].pwmNum  = '1';

	pwmList[pwmLedG].chipNum = '6';
	pwmList[pwmLedG].pwmNum  = '0';

	pwmList[pwmLedB].chipNum = '4';
	pwmList[pwmLedB].pwmNum  = '0';

}
*/