/*
    babyMonitor.h
    the main program of the baby monitor system which provide system related functionalities
*/

#ifndef BABY_MONITOR_H
#define BABY_MONITOR_H

// get the running status of the baby monitor system
_Bool BabayMonitor_isSystemRunning(void);

// *********NOTE********** ONLY the UDP server can make this request! afterwards, the baby monitor system will be restarted
void BabayMonitor_requestSystemRestart(void);

#endif