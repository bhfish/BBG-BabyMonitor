#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include "A2D.h"
#include "temperatureMonitor.h"

#define TMP36_AIN_NUM                       3

// reference from https://www.lullabytrust.org.uk/safer-sleep-advice/baby-room-temperature/
#define MAX_TEMPERATURE_IN_CELSIUS_ALLOW    20
#define MIN_TEMPERATURE_IN_CELSIUS_ALLOW    16
#define REF_VOLTAGE                         1.8
#define MONITOR_TIME_INTERVAL_IN_S          300

static pthread_t temperatureThread;
static _Bool stopMonitoring = false;

static float covertAnalogToVoltage(int A2DReadingVal);
static int covertVoltageToTemperature(float voltage);
static void *startTemperatureThread(void *args);

_Bool TemperatureMonitor_startMonitoring
(
    void
)
{
    if (!A2D_init(TMP36_AIN_NUM) || pthread_create(&temperatureThread, NULL, &startTemperatureThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in temperatureMonitor module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

void TemperatureMonitor_stopMonitoring
(
    void
)
{
    void *temperatureThreadExitStatus;

    stopMonitoring = true;

    if (pthread_join(temperatureThread, &temperatureThreadExitStatus) != 0) {
        printf("[ERROR] failed to join with terminated temperature thread failed reason: %s\n", strerror(errno));
    }

    if (temperatureThreadExitStatus != PTHREAD_CANCELED)
    {
        // bad things happened...
        printf("[ERROR] abnormal termination state of temperature thread\n");
        printf("try to send a cancel request and wish temperature thread can normally be terminated...\n");

        if(pthread_cancel(temperatureThread) != 0)
        {
            printf("[ERROR] failed to send a cancel request to temperature thread failed reason: %s\n", strerror(errno));
        }
    }
}

// define the duty of temperature thread
static void *startTemperatureThread
(
    void *args  // [in] list of argument passed by pthread_create()
)
{
    int A2DReadingVal, currentTemperature;
    float convertedVoltageVal;
    struct timespec monitorTime;
    struct timespec remainTime;

    monitorTime.tv_sec = MONITOR_TIME_INTERVAL_IN_S;
    monitorTime.tv_nsec = 0;


    while (!stopMonitoring) {
        A2DReadingVal = A2D_getAnalogReading(TMP36_AIN_NUM);

        if (A2DReadingVal <= 0) {
            printf("[ERROR] unable to read current baby's room temperature\n");
            // TODO raise alarm
        }

        convertedVoltageVal = covertAnalogToVoltage(A2DReadingVal);
        currentTemperature = covertVoltageToTemperature(convertedVoltageVal);

        if (currentTemperature < MIN_TEMPERATURE_IN_CELSIUS_ALLOW || currentTemperature > MAX_TEMPERATURE_IN_CELSIUS_ALLOW)
        {
            // TODO send temperature to parent's BBG
            printf("[WARN] detect abnormal temperature!\n");
        }

        nanosleep(&monitorTime, &remainTime);
    }

    pthread_exit(PTHREAD_CANCELED);
}

// convert the specified analog input value into voltage value in v
static float covertAnalogToVoltage
(
    int A2DReadingVal  // [in] analog readings
)
{
    return (float) A2DReadingVal / (float) MAX_A2D_VALUE * (float) REF_VOLTAGE;
}

// convert the specified voltage into temperature in Celsius reference from https://learn.adafruit.com/tmp36-temperature-sensor/overview
static int covertVoltageToTemperature
(
    float voltage  // [in] voltage value
)
{
    return (1000.0f * voltage - 500.0f) / 10.0f + 0.5f;
}