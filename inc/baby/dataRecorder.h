/*
    dataRecorder.h
    module to spawn a thread which continually records time series data such as temperature(Celsius) and sound(decibel) in babyBBG's tmpfs for
    every 15 minutes. Data will be saved as csv file format

    NOTE:
    this module will not be responsible for checking the validity and integrity of such data
*/

#ifndef DATA_RECORDER_H
#define DATA_RECORDER_H

#include "monitorData.h"

// initialize a background thread which continually records time series data for every 15 minutes
_Bool DataRecorder_startRecording(void);

// record the data in run time. data will be queued up for 15 minutes, and to be calculated to an average and saved into the fs afterwards
void DataRecorder_recordData(int dataToRecord, DATA_CATEGORY CATEGORY);

// stop the background thread and exit gracefully
void DataRecorder_stopRecording(void);

#endif