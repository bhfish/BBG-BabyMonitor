#ifndef _PARENT_PMW_H_
#define _PARENT_PMW_H_

#include <stdio.h>
#include <stdlib.h>

#define PMW_CHIP_NUM_BUZZ 2
#define PMW_CHIP_NUM_LEDR 4
#define PMW_CHIP_NUM_LEDG 6
#define PMW_CHIP_NUM_LEDB 4

#define PMW_NUM_BUZZ 0
#define PMW_NUM_LEDR 1
#define PMW_NUM_LEDG 0
#define PMW_NUM_LEDB 1

#define PMW_CONFIG_PATH(chipNum)           "/sys/class/pwm/pwmchip" STR_CONVERT(chipNum) "/export"
#define PMW_SYS_FS_PERIOD(chipNum, pwmNum) "/sys/class/pwm/pwmchip" STR_CONVERT(chipNum) "/pwm" STR_CONVERT(pwmNum) "/period"
#define PMW_SYS_FS_CYCLE(chipNum, pwmNum)  "/sys/class/pwm/pwmchip" STR_CONVERT(chipNum) "/pwm" STR_CONVERT(pwmNum) "/duty_cycle"
#define PMW_SYS_FS_ENABLE(chipNum, pwmNum) "/sys/class/pwm/pwmchip" STR_CONVERT(chipNum) "/pwm" STR_CONVERT(pwmNum) "/enable"


#endif