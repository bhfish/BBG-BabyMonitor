#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <time.h>       // time
#include "monitorData.h"
#include "dataRecorder.h"

#ifdef DEMO_MODE
    #define RECORD_DATA_TIME_INTERVAL_IN_S      10
#else
    #define RECORD_DATA_TIME_INTERVAL_IN_S      900
#endif

// if we record data for every second, a total of 3600 data will be recorded hourly
#define MAX_NUM_DATA_TO_RECORD                  4000

static pthread_t recorderThread;
static pthread_mutex_t recordTemperatureMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t recordSoundMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopRecording = false;
static int temperatureDataArr[MAX_NUM_DATA_TO_RECORD];
static int soundDataArr[MAX_NUM_DATA_TO_RECORD];
static int currNumTemperatureData = 0;
static int currNumSoundData = 0;

static int getDataArrAvgVal(int *arr, int arrSize);
static void saveAvgDataToFS(DATA_CATEGORY CATEGORY, int numData);
static void *startRecorderThread(void *args);

_Bool DataRecorder_startRecording(void)
{
    /*
        1) "zero out" both data array
        2) initialized the recorder thread
    */

    memset(temperatureDataArr, 0, MAX_NUM_DATA_TO_RECORD);
    memset(soundDataArr, 0, MAX_NUM_DATA_TO_RECORD);

    if (pthread_create(&recorderThread, NULL, &startRecorderThread, NULL) != 0) {
        printf("[ERROR] failed to create a thread in dataRecorder module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}

void DataRecorder_recordData(int dataToRecord, DATA_CATEGORY CATEGORY)
{
    /*
        1) if either data array is full, save it into fs
        2) append such data into data array
    */
    switch (CATEGORY) {
        case TEMPERATURE:

            pthread_mutex_lock(&recordTemperatureMutex);
            {
                if (currNumTemperatureData == MAX_NUM_DATA_TO_RECORD) {
                    saveAvgDataToFS(CATEGORY, MAX_NUM_DATA_TO_RECORD);
                    memset(temperatureDataArr, 0, MAX_NUM_DATA_TO_RECORD);
                    currNumTemperatureData = 0;
                }

                temperatureDataArr[currNumTemperatureData++] = dataToRecord;
            }
            pthread_mutex_unlock(&recordTemperatureMutex);

            break;
        case SOUND:

            pthread_mutex_lock(&recordSoundMutex);
            {
                if (currNumSoundData == MAX_NUM_DATA_TO_RECORD) {
                    saveAvgDataToFS(CATEGORY, MAX_NUM_DATA_TO_RECORD);
                    memset(soundDataArr, 0, MAX_NUM_DATA_TO_RECORD);
                    currNumSoundData = 0;
                }

                soundDataArr[currNumSoundData++] = dataToRecord;
            }
            pthread_mutex_unlock(&recordSoundMutex);

            break;
        case ACCELERATION:
            // we don't have such data to be recorded
            break;
    }
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

        pthread_mutex_lock(&recordTemperatureMutex);
        {
            if (currNumTemperatureData > 0) {
                saveAvgDataToFS(TEMPERATURE, currNumTemperatureData);
            }
        }
        pthread_mutex_unlock(&recordTemperatureMutex);

        pthread_mutex_lock(&recordSoundMutex);
        {
            if (currNumSoundData > 0) {
                saveAvgDataToFS(SOUND, currNumTemperatureData);
            }
        }
        pthread_mutex_unlock(&recordSoundMutex);
    }

    pthread_exit(PTHREAD_CANCELED);
}

// save the average data of the specified data category as csv format to the fs
static void saveAvgDataToFS(DATA_CATEGORY CATEGORY, int numData)
{
    int dataAvgVal;
    char recordFilePath[DATA_FILE_NAME_LEN] = {0};
    char fileContents[DATA_FILE_LINE_LEN] = {0};
    FILE *fPtr;

    switch (CATEGORY) {
        case TEMPERATURE:
            sprintf(recordFilePath, "%s/%s%s", DATA_FILE_FS, TEMPERATURE_DATA_FILE_NAME, DATA_FILE_EXTENSION);
            dataAvgVal = getDataArrAvgVal(temperatureDataArr, numData);

            break;
        case SOUND:
            sprintf(recordFilePath, "%s/%s%s", DATA_FILE_FS, SOUND_DATA_FILE_NAME, DATA_FILE_EXTENSION);
            dataAvgVal = getDataArrAvgVal(soundDataArr, numData);

            break;
        case ACCELERATION:
            // we don't have such data to be recorded
            break;
    }

    fPtr = fopen(recordFilePath, "a");

    if (fPtr == NULL) {
        printf("[ERROR] dataRecorder: failed to open %s reason: %s\n", recordFilePath, strerror(errno));

        return;
    }

    sprintf(fileContents, "%ld,%d\n", time(NULL), dataAvgVal);

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

// calculate the average data of the specified data array
static int getDataArrAvgVal(int *arr, int arrSize)
{
    int dataSum = 0;

    for (int i = 0; i < arrSize; ++i) {
        dataSum += arr[i];
    }

    return dataSum / arrSize;
}