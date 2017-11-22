#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
// TODO remove later
#include <unistd.h>     // sleep
#include <pthread.h>    // pthread_*
#include "dataRecorder.h"
#include "tcpSender.h"
#include "udpListener.h"
#include "temperatureMonitor.h"
#include "accelerometerMonitor.h"
#include "Microphone.h"
#include "videoStreaming.h"

static _Bool restartReqSent = false;
static _Bool isSystemRunning = false;

static void startBabyMonitor(void);
static void stopBabyMonitor(void);

int main(int argc, char const *argv[])
{
    /*
        if there is a failure during the initialization of any modules in BABY'S BBG, render appropriate error message to web interface so,
        user can choose restart (via web interface) the entire system (main thread/program should be running until BBG restart). However this
        logic is dependent to the initialization status of the UDP server
    */

    startBabyMonitor();

    while (true) {
        if (restartReqSent) {
            stopBabyMonitor();
            startBabyMonitor();
        }
    }

    return 0;
}

// *********NOTE********** ONLY the UDP server can make this request!
void BabayMonitor_requestSystemRestart(void)
{
    restartReqSent = true;
}

_Bool BabayMonitor_isSystemRunning(void)
{
    // TODO get the status of parent's BBG
    return isSystemRunning;
}

// initialize required resources for baby monitor system
static void startBabyMonitor(void)
{
    /*
        baby's BBG startup sequence should follow the order of
        1) video/sound
        2) sender
        3) UDP server (user web interface)
        4) other modules
    */

    if ( !Video_startStreaming() ) {
        printf("[ERROR] failed to init video module\n");

        return;
    }


    if ( !Microphone_startListening() ) {
        printf("[ERROR] failed to init microphone module\n");

        return;
    }

    if ( !TCPSender_init() ) {
        printf("[ERROR] failed to init sender module\n");

        return;
    }

    if ( !UDPListener_startListening() ) {
        printf("[ERROR] failed to init UDP listener module\n");

        return;
    }

    if ( !AccelerometerMonitor_startMonitoring() ) {
        printf("[ERROR] failed to init accelerometerMonitor module\n");

        return;
    }

    if ( !TemperatureMonitor_startMonitoring() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        return;
    }

    if ( !DataRecorder_startRecording() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        return;
    }

    isSystemRunning = true;
}

// stop/de-allocate initialized modules/resources
static void stopBabyMonitor(void)
{
    isSystemRunning = false;

    Video_stopStreaming();
    TCPSender_cleanUp();
    AccelerometerMonitor_stopMonitoring();
    TemperatureMonitor_stopMonitoring();
    DataRecorder_stopRecording();
    UDPListener_stopListening();
    Microphone_stopListening();
}
