#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <time.h>       // time
#include "temperatureMonitor.h"
#include "monitorData.h"
#include "dataRecorder.h"

#ifdef DEMO_MODE
    #define RECORD_DATA_TIME_INTERVAL_IN_S      10
#else
    #define RECORD_DATA_TIME_INTERVAL_IN_S      900
#endif

static pthread_t recorderThread;
static pthread_mutex_t recordTemperatureMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t recordSoundMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopRecording = false;

static void saveAvgDataToFS(DATA_CATEGORY CATEGORY, int numData);
static void *startRecorderThread(void *args);

_Bool DataRecorder_startRecording(void)
{
    if (pthread_create(&recorderThread, NULL, &startRecorderThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in dataRecorder module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

void DataRecorder_stopRecording(void)
{
    void *recorderThreadExitStatus;

    stopRecording = true;

    pthread_mutex_destroy(&recordTemperatureMutex);
    pthread_mutex_destroy(&recordSoundMutex);

    if (pthread_join(recorderThread, &recorderThreadExitStatus) != 0) {
        printf("[ERROR] failed to join with terminated data recorder thread failed reason: %s\n", strerror(errno));
    }

    if (recorderThreadExitStatus != PTHREAD_CANCELED)
    {
        // bad things happened...
        printf("[ERROR] abnormal termination state of data recorder thread\n");
        printf("try to send a cancel request and wish data recorder thread can normally be terminated...\n");

        if(pthread_cancel(recorderThread) != 0)
        {
            printf("[ERROR] failed to send a cancel request to data recorder thread failed reason: %s\n", strerror(errno));
        }
    }
}

// define the duty of recorder thread
static void *startRecorderThread(void *args)
{

    struct timespec recordTime;
    struct timespec remainTime;

    recordTime.tv_sec = RECORD_DATA_TIME_INTERVAL_IN_S;
    recordTime.tv_nsec = 0;


    while (!stopRecording) {
        nanosleep(&recordTime, &remainTime);

        // TODO: get sound decibel data
        saveAvgDataToFS(TEMPERATURE, TemperatureMonitor_getCurrentTemperature());
    }

    pthread_exit(PTHREAD_CANCELED);
}

// save the specified data as csv format to the data file
static void saveAvgDataToFS(DATA_CATEGORY CATEGORY, int dataVal)
{
    char recordFilePath[DATA_FILE_NAME_LEN] = {0};
    char fileContents[DATA_FILE_LINE_LEN] = {0};
    FILE *fPtr;

    switch (CATEGORY) {
        case TEMPERATURE:
            sprintf(recordFilePath, "%s/%s%s", DATA_FILE_FS, TEMPERATURE_DATA_FILE_NAME, DATA_FILE_EXTENSION);

            break;
        case SOUND:
            sprintf(recordFilePath, "%s/%s%s", DATA_FILE_FS, SOUND_DATA_FILE_NAME, DATA_FILE_EXTENSION);

            break;
        case UNKNOWN:
            // we don't have such data to be recorded
            break;
    }

    fPtr = fopen(recordFilePath, "a");

    if (fPtr == NULL) {
        printf("[ERROR] dataRecorder: failed to open %s reason: %s\n", recordFilePath, strerror(errno));

        return;
    }

    sprintf(fileContents, "%ld,%d\n", time(NULL), dataVal);

    if (fprintf(fPtr, "%s", fileContents) <= 0) {

        // ENOMEM Insufficient storage space is available.
        if (errno != ENOMEM) {
            printf("[ERROR] dataToRecord: failed to write data into %s reason: %s\n", recordFilePath, strerror(errno));
        }
        else {
            // overwrite the data file in order to free up some space; re-initialize the file pointer with different mode
            fclose(fPtr);

            fPtr = fopen(recordFilePath, "w");

            if (fPtr == NULL) {
                printf("[ERROR] dataRecorder: failed to open %s reason: %s\n", recordFilePath, strerror(errno));

                return;
            }

            fprintf(fPtr, "%s", fileContents);
        }
    }

    #ifdef DEBUG_MODE
        printf("[DEBUG] written record %s to %s\n", fileContents, recordFilePath);
    #endif

    fclose(fPtr);
}