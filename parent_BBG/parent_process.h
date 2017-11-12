#ifndef _PARENT_PROCESS_H_
#define _PARENT_PROCESS_H_

#include <stdbool.h>

#define DEVICEs_SLOTS "/sys/devices/platform/bone_capemgr/slots"

typedef enum
{
	dispModeAlarmArm =0,
	dispModeAlarmSound,
	dispModeTemp,
	dispModeSound
}dispMode_t;

//Program state settings
extern bool stopping;

extern bool alarmTriggered;
extern bool alarmStateArm;
extern int alarmBuzzMode;
extern int babySoundLevel;
extern int babyRoomTemp;
extern dispMode_t currentDispMode;

void setBbySoundLevel(int sound);
int getBbySoundLevel(void);
dispMode_t getDispMode(void);
bool getSysInitStatus(void);

#endif