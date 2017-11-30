#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <time.h>       // nanosleep
#include <dirent.h>     // opendir
#include "a2d.h"

#define A2D_SYS_FILE_SIZE                   50
#define A2D_VIRTUAL_CAPE                    "BB-ADC"
#define A2D_VIRTUAL_CAPE_CONFIG_FILE_PATH   "/sys/devices/platform/bone_capemgr/slots"
#define A2D_FILE_SYSTEM                     "/sys/bus/iio/devices"

// wait for specified analog input number to be active
static _Bool waitForDeviceReady(void);

int A2D_getAnalogReading
(
    int AINNum          // [in] analog input number
)
{
    int A2DReadingVal;
    char voltageFile[A2D_SYS_FILE_SIZE] = {0};
    sprintf(voltageFile, "%s/iio:device0/in_voltage%d_raw", A2D_FILE_SYSTEM, AINNum);

    FILE *fPtr = fopen(voltageFile, "r");

    if (fPtr == NULL) {
        printf("[ERROR] A2D: fopen %s failed reason %s\n", voltageFile, strerror(errno));

        return -1;
    }

    if (fscanf(fPtr, "%d", &A2DReadingVal) <= 0) {
        printf("[ERROR] A2D: unable to read value from %s reason %s\n", voltageFile, strerror(errno));
        fclose(fPtr);

        return -1;
    }

    fclose(fPtr);

    return A2DReadingVal;
}

_Bool A2D_init
(
    int AINNum  // [in] analog input number
)
{
    // load A2D virtual cape
    FILE *fPtr = fopen(A2D_VIRTUAL_CAPE_CONFIG_FILE_PATH, "w");

    if (fPtr == NULL) {
        printf("[ERROR] A2D: fopen %s failed reason %s\n", A2D_VIRTUAL_CAPE_CONFIG_FILE_PATH, strerror(errno));

        return false;
    }

    if (fprintf(fPtr, "%s", A2D_VIRTUAL_CAPE) <= 0) {
        printf("[ERROR] A2D: unable to load the A2D virtual cape\n");

        fclose(fPtr);
        return false;
    }

    fclose(fPtr);

    return waitForDeviceReady();
}

static _Bool waitForDeviceReady
(
    void
)
{
    struct timespec waitTime;
    const int MAX_WAIT_TIME = 5;
    int timeElapsed = 0;
    char deviceDir[A2D_SYS_FILE_SIZE] = {0};
    sprintf(deviceDir, "%s/iio:device0", A2D_FILE_SYSTEM);

    waitTime.tv_sec = 1;
    waitTime.tv_nsec = 0;

    while (opendir(deviceDir) == NULL) {
        if (timeElapsed == MAX_WAIT_TIME) {
            return false;
        }

        timeElapsed++;
        nanosleep(&waitTime, NULL);
    }

    return true;
}