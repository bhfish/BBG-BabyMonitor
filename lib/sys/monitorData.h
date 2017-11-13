/*
    monitorData.h
    header file to provide any related information on monitoring data(sound, temperature) of the baby monitoring system
*/

#ifndef MONITOR_DATA_H
#define MONITOR_DATA_H

#define TEMPERATURE_TYPE_DATA               "temperature"
#define SOUND_TYPE_DATA                     "sound"
#define ALARM_TYPE                          "alarm"

// such temporary data won't be stored in flash
#define DATA_FILE_FS                        "/tmp"

// csv format: <timestamp>,<data value>\n
#define DATA_FILE_EXTENSION                 ".csv"
#define TEMPERATURE_DATA_FILE_NAME          "temperature"
#define SOUND_DATA_FILE_NAME                "sound"

#define DATA_FILE_NAME_LEN                  50
#define DATA_FILE_LINE_LEN                  50

typedef enum
{
    TEMPERATURE,
    SOUND,
    UNKNOWN
}DATA_CATEGORY;

#endif