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

void AlarmMonitor_stopProg(void);
_Bool AlarmMonitor_isStopping(void);

void AlarmMonitor_setAlarmTrigger(_Bool trigger);
_Bool AlarmMonitor_getAlarmTrigger(void);

void AlarmMonitor_setAlarmArm(_Bool alarmArm);
_Bool AlarmMonitor_getAlarmArm(void);

void AlarmMonitor_setAlarmBuzzMode(int mode);
int AlarmMonitor_getAlarmBuzzMode(void);

void AlarmMonitor_setBbyRoomTemp(int temp);
int AlarmMonitor_getBbyRoomTemp(void);

void AlarmMonitor_setBbySoundLevel(int sound);
int AlarmMonitor_getBbySoundLevel(void);

void AlarmMonitor_setDispMode(dispMode_t mode);
dispMode_t AlarmMonitor_getDispMode(void);

_Bool AlarmMonitor_getSysInitStatus(void);
_Bool AlarmMonitor_getAlarmSleepStatus(void);

#endif