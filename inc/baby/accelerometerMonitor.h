/*
    accelerometerMonitor.h
    Module to spawn a thread to monitor whether the installed camera has been moved by external force using MMA8452Q for every 500ms and
    send an alarm request to parent's BBG if such detection occurs
*/

#ifndef ACCELEROMETER_MONITOR_H
#define ACCELEROMETER_MONITOR_H

// spawn and start a new thread for temperature recording
_Bool AccelerometerMonitor_startMonitoring(void);

// stop the thread and exit gracefully
void AccelerometerMonitor_stopMonitoring(void);

#endif