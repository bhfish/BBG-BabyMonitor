/*
    temperatureMonitor.h
    Module to spawn a thread to monitor and record the current room temperature from TMP36 for every 5 minutes
    send an alarm request to parent's BBG if such recorded room temperature is abnormal
*/

#ifndef TEMPERATURE_MONITOR_H
#define TEMPERATURE_MONITOR_H

// spawn and start a new thread for temperature recording
_Bool TemperatureMonitor_startMonitoring(void);

// get current baby's room temperature
int TemperatureMonitor_getCurrentTemperature(void);

// return whether specified temperature is out of range/abnormal
_Bool TemperatureMonitor_isTemperatureNormal(int temperature);

// stop the tread and exit gracefully
void TemperatureMonitor_stopMonitoring(void);

#endif