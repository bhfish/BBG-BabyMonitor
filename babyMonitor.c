#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include "babyMonitor.h"
#include "dataRecorder.h"
#include "tcpSender.h"
#include "udpListener.h"
#include "temperatureMonitor.h"
#include "Microphone.h"
#include "videoStreaming.h"
#include "watchDog.h"

static _Bool isSystemRunning = false;
static pthread_mutex_t systemStatusMutex = PTHREAD_MUTEX_INITIALIZER;

static _Bool startBabyMonitor(void);


int main(int argc, char const *argv[])
{
    int watchDogRefID, watchDogTimer;
    _Bool wasRegistrationSuccess;

    struct timespec kickTime;
    struct timespec remainTime;

    BabayMonitor_setSystemRunningStatus(startBabyMonitor());

    wasRegistrationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);

    if (!wasRegistrationSuccess) {
        printf("[ERROR] main thread unable to register to watch dog\n");
    } else {
        watchDogTimer = WatchDog_getWatchDogTimer();

        // would likely kick the watch dog a bit earlier as this timeout is a HARD timeout
        kickTime.tv_sec = watchDogTimer - 5;
        remainTime.tv_nsec = 0;
    }

    while (true) {
        if (wasRegistrationSuccess) {
            // it's time to kick the dog
            WatchDog_kickWatchDog(watchDogRefID);
        }

        nanosleep(&kickTime, &remainTime);
    }

    return 0;
}

_Bool BabayMonitor_getSystemRunningStatus(void)
{
    _Bool runningStatus;

    pthread_mutex_lock(&systemStatusMutex);
    {
        runningStatus = isSystemRunning;
    }
    pthread_mutex_unlock(&systemStatusMutex);

    return runningStatus;
}

void BabayMonitor_setSystemRunningStatus(_Bool newRunningStatus)
{
    pthread_mutex_lock(&systemStatusMutex);
    {
        isSystemRunning = newRunningStatus;
    }
    pthread_mutex_unlock(&systemStatusMutex);
}

// initialize required resources for baby monitor system
static _Bool startBabyMonitor(void)
{
    /*
        baby monitoring system startup sequence should follow the order of
        1) watchdog
        2) video/sound
        3) sender (communication to parent's BBG)
        4) UDP server (user web interface)
        5) other modules
    */

    _Bool wasStartupSuccess = true;

    if ( !WatchDog_initWatchDog() ) {
        printf("[ERROR] failed to init watch dog module\n");

        wasStartupSuccess = false;
    }

    if ( !Video_startStreaming() ) {
        printf("[ERROR] failed to init video module\n");

        wasStartupSuccess = false;
    }

    if ( !Microphone_startListening() ) {
        printf("[ERROR] failed to init microphone module\n");

        wasStartupSuccess = false;
    }

    if ( !TCPSender_init() ) {
        printf("[ERROR] failed to init sender module\n");

        wasStartupSuccess = false;
    }

    if ( !UDPListener_startListening() ) {
        printf("[ERROR] failed to init UDP listener module\n");

        wasStartupSuccess = false;
    }

    if ( !TemperatureMonitor_startMonitoring() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        wasStartupSuccess = false;
    }

    if ( !DataRecorder_startRecording() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        wasStartupSuccess = false;
    }

    return wasStartupSuccess;
}
