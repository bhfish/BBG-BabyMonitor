/*
    babyMonitor.h
    the main program of the baby monitor system which provide system related functionalities
*/

#ifndef BABY_MONITOR_H
#define BABY_MONITOR_H

// *********NOTE********** ONLY the UDP server can make this request! afterwards, the baby monitor system will be restarted
void requestSystemRestart(void);

#endif