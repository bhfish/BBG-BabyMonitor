/*
    babyMonitor.h
    the main program of the baby monitor system which provide system related functionalities
*/

#ifndef BABY_MONITOR_H
#define BABY_MONITOR_H

// get the running status of the baby monitor system
_Bool BabayMonitor_getSystemRunningStatus(void);

// set the running status of the baby monitor system
void BabayMonitor_setSystemRunningStatus(_Bool newRunningStatus);

#endif