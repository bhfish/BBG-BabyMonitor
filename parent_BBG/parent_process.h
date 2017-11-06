#ifndef _PARENT_PROCESS_H_
#define _PARENT_PROCESS_H_

#include <stdbool.h>

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

#endif