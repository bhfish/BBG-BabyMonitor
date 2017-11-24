#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include "tcpSender.h"
#include "A2D.h"
#include "dataRecorder.h"
#include "temperatureMonitor.h"
#include "watchDog.h"

#define TMP36_AIN_NUM                           3

// reference from https://www.lullabytrust.org.uk/safer-sleep-advice/baby-room-temperature/
#define MAX_TEMPERATURE_IN_CELSIUS_ALLOW        20

#ifdef DEMO_MODE
    #define MIN_TEMPERATURE_IN_CELSIUS_ALLOW    10
#else
    #define MIN_TEMPERATURE_IN_CELSIUS_ALLOW    16
#endif

#define REF_VOLTAGE                             1.8

#define MONITOR_TIME_INTERVAL_IN_S              1

static pthread_t temperatureThread;
static pthread_mutex_t currentTemperatureMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopMonitoring = false;
static int currentTemperature;

static float covertAnalogToVoltage(int A2DReadingVal);
static int covertVoltageToTemperature(float voltage);
static void *startTemperatureThread(void *args);

_Bool TemperatureMonitor_startMonitoring(void)
{
    if ( !A2D_init(TMP36_AIN_NUM) ) {
        printf("[ERROR] failed to initialize A2D functionalities\n");

        return false;
    }

    if (pthread_create(&temperatureThread, NULL, &startTemperatureThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in temperatureMonitor module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

int TemperatureMonitor_getCurrentTemperature(void)
{
    int temperature;

    pthread_mutex_lock(&currentTemperatureMutex);
    {
        temperature = currentTemperature;
    }
    pthread_mutex_unlock(&currentTemperatureMutex);

    return temperature;
}

_Bool TemperatureMonitor_isTemperatureNormal(int temperature)
{
    if (temperature < MIN_TEMPERATURE_IN_CELSIUS_ALLOW || temperature > MAX_TEMPERATURE_IN_CELSIUS_ALLOW) {
        return false;
    }

    return true;
}

void TemperatureMonitor_stopMonitoring(void)
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
static void *startTemperatureThread(void *args)
{
    int watchDogRefID, watchDogTimer, A2DReadingVal;
    int watchDogTimerCnt = 0;
    _Bool wasWatchDogRegisterationSuccess;
    float convertedVoltageVal;
    struct timespec monitorTime;
    struct timespec remainTime;

    monitorTime.tv_sec = MONITOR_TIME_INTERVAL_IN_S;
    monitorTime.tv_nsec = 0;

    wasWatchDogRegisterationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);
    watchDogTimer = WatchDog_getWatchDogTimeout();

    while (!stopMonitoring) {
        A2DReadingVal = A2D_getAnalogReading(TMP36_AIN_NUM);

        if (A2DReadingVal <= 0) {
            printf("[ERROR] unable to read current baby's room temperature\n");

            break;
        }

        convertedVoltageVal = covertAnalogToVoltage(A2DReadingVal);

        pthread_mutex_lock(&currentTemperatureMutex);
        {
            currentTemperature = covertVoltageToTemperature(convertedVoltageVal);
        }
        pthread_mutex_unlock(&currentTemperatureMutex);

        printf("current room temperature is: %d\n", currentTemperature);

        if ( !TemperatureMonitor_isTemperatureNormal(currentTemperature) ) {

            // CRITICAL as we don't tolerate any failures returned by the send function
            printf("[WARN] detect abnormal temperature!\n");

            // if this failed, there is nothing we can do much from baby's BBG side
            TCPSender_sendAlarmRequestToParentBBG();
        }

        TCPSender_sendDataToParentBBG(TEMPERATURE, currentTemperature);

        nanosleep(&monitorTime, &remainTime);
        watchDogTimerCnt++;

        if (watchDogTimerCnt == watchDogTimer && wasWatchDogRegisterationSuccess) {
            // it's time to kick the watch dog
            WatchDog_kickWatchDog(watchDogRefID);
            watchDogTimerCnt = 0;
        }
    }

    pthread_exit(PTHREAD_CANCELED);
}

// convert the specified analog input value into voltage value in v
static float covertAnalogToVoltage(int A2DReadingVal)
{
    return (float) A2DReadingVal / (float) MAX_A2D_VALUE * (float) REF_VOLTAGE;
}

// convert the specified voltage into temperature in Celsius reference from https://learn.adafruit.com/tmp36-temperature-sensor/overview
static int covertVoltageToTemperature(float voltage)
{
    return (1000.0f * voltage - 500.0f) / 10.0f + 0.5f;
}