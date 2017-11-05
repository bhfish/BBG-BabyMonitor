#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
// TODO remove later
#include <unistd.h>     // sleep
#include "dataRecorder.h"
#include "sender.h"
#include "temperatureMonitor.h"
#include "accelerometerMonitor.h"

static _Bool restartReqSent = false;

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
void requestSystemRestart(void)
{
    restartReqSent = true;
}

// initialize required resources for baby monitor system
static void startBabyMonitor(void)
{
    /*
        baby's BBG startup sequence should follow the order of
        1) video/sound TODO
        2) sender
        3) UDP server (user web interface) TODO
        4) other modules
    */

    if ( !Sender_init() ) {
        printf("[ERROR] failed to init sender module\n");

        // TODO: render error to user interface
    }

    if ( !AccelerometerMonitor_startMonitoring() ) {
        printf("[ERROR] failed to init AccelerometerMonitor module\n");

        // TODO: render error to user interface
    }

    if ( !TemperatureMonitor_startMonitoring() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        // TODO: render error to user interface
    }

    if ( !DataRecorder_startRecording() ) {
        printf("[ERROR] failed to init temperatureMonitor module\n");

        // TODO: render error to user interface
    }
}

// stop/de-allocate initialized modules/resources
static void stopBabyMonitor(void)
{
    // TODO: video/sound and UDP server
    Sender_cleanUp();
    AccelerometerMonitor_stopMonitoring();
    TemperatureMonitor_stopMonitoring();
    DataRecorder_stopRecording();
}