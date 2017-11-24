#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include "dataRecorder.h"
#include "tcpSender.h"
#include "udpListener.h"
#include "temperatureMonitor.h"
#include "Microphone.h"
#include "videoStreaming.h"
#include "watchDog.h"

static _Bool isSystemRunning = false;

static void startBabyMonitor(void);

int main(int argc, char const *argv[])
{
    int watchDogRefID, watchDogTimer;
    _Bool wasWatchDogRegisterationSuccess;

    struct timespec kickTime;
    struct timespec remainTime;

    startBabyMonitor();

    if (isSystemRunning) {
        wasWatchDogRegisterationSuccess = WatchDog_registerToWatchDog(&watchDogRefID);
        watchDogTimer = WatchDog_getWatchDogTimeout();

        // would likely kick the watch dog a bit earlier as this timeout is a HARD timeout
        kickTime.tv_sec = watchDogTimer - 5;
        remainTime.tv_nsec = 0;

        if ( !wasWatchDogRegisterationSuccess ) {
            printf("[ERROR] main thread unable to register to watch dog\n");
        }
    }

    while (true) {
        if (wasWatchDogRegisterationSuccess) {
            // it's time to kick the dog
            WatchDog_kickWatchDog(watchDogRefID);
        }

        nanosleep(&kickTime, &remainTime);
    }

    return 0;
}

_Bool BabayMonitor_isSystemRunning(void)
{
    return isSystemRunning;
}

// initialize required resources for baby monitor system
static void startBabyMonitor(void)
{
    /*
        baby's BBG startup sequence should follow the order of
        1) video/sound
        2) sender (communication to parent's BBG)
        3) UDP server (user web interface)
        4) other modules
    */

    // if ( !Video_startStreaming() ) {
    //     printf("[ERROR] failed to init video module\n");

    //     return;
    // }

    // if ( !Microphone_startListening() ) {
    //     printf("[ERROR] failed to init microphone module\n");

    //     return;
    // }

    // if ( !TCPSender_init() ) {
    //     printf("[ERROR] failed to init sender module\n");

    //     return;
    // }

    // if ( !UDPListener_startListening() ) {
    //     printf("[ERROR] failed to init UDP listener module\n");

    //     return;
    // }

    // if ( !TemperatureMonitor_startMonitoring() ) {
    //     printf("[ERROR] failed to init temperatureMonitor module\n");

    //     return;
    // }

    // if ( !DataRecorder_startRecording() ) {
    //     printf("[ERROR] failed to init temperatureMonitor module\n");

    //     return;
    // }

    if ( !WatchDog_initWatchDog() ) {
        printf("[ERROR] failed to init watch dog module\n");

        return;
    }

    isSystemRunning = true;
}
