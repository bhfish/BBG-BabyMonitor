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

void stopProg(void);
bool isStopping(void);

void setAlarmTrigger(bool trigger);
bool getAlarmTrigger(void);

void setAlarmArm(bool alarmArm);
bool getAlarmArm(void);

void setAlarmBuzzMode(int mode);
int getAlarmBuzzMode(void);

void setBbyRoomTemp(int temp);
int getBbyRoomTemp(void);

void setBbySoundLevel(int sound);
int getBbySoundLevel(void);

void setDispMode(dispMode_t mode);
dispMode_t getDispMode(void);

bool getSysInitStatus(void);
bool getAlarmSleepStatus(void);

#endif