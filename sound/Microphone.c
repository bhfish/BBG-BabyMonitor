//
// Created by vincent on 10/22/17.
//

#include "WaveStreamer.h"
#include <alsa/asoundlib.h>
#include <math.h>
#include <stdbool.h>
#include <alloca.h>
#include <pthread.h>
#include <string.h>
#include "monitorData.h"
#include "tcpSender.h"
#include "watchDog.h"
#include <time.h>

static _Bool connectToDevice();
static _Bool initializeDeviceSettings();
static double getAverageAmplitude();
static void* listenOverMicrophone(void *args);
static void stopListening();
static _Bool shouldStopListening();
static void setCurrentDecibels(short *buffer, int bufferSize);
static void alertIfDecibelOutsideThreshHold();

#define PCM_DEVICE_NAME "plughw:U0x46d0x825,0"
#define SAMPLE_RATE_IN_HERTZ 48000
#define NUMBER_OF_CHANNELS 1
#define DB_OFFSET 72
#define MAX_AMPLITUDE 32767
#define MAX_DECIBEL_THRESH_HOLD 60
#define MIN_DECIBEL_THRESH_HOLD 0
#define WATCH_DOG_KICK_ITERATION 10

static const snd_pcm_uframes_t PERIOD_SIZE = 2400;
static snd_pcm_uframes_t bufferSize;

static snd_pcm_t *pcmDevice;
static snd_pcm_hw_params_t *deviceSettings;

static pthread_t listenerThread;
static pthread_mutex_t stopListeningMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopListeningFlag = false;

static pthread_mutex_t currentDecibelMutex = PTHREAD_MUTEX_INITIALIZER;
static double currentDecibel;

_Bool Microphone_startListening() {
    bufferSize = 2 * PERIOD_SIZE * 2;

    if (!connectToDevice() || !initializeDeviceSettings()) {
        printf("Error: unable to initialize device");
        return false;
    }

    if (pthread_create(&listenerThread, NULL, &listenOverMicrophone, NULL) != 0) {
        printf("Error: failed to create new listener thread\n");
        return false;
    }

    if (!WaveStreamer_startStreaming(bufferSize)) {
        return false;
    }

    return true;
}

void Microphone_stopListening() {
    stopListening();

    void* listenerThreadExitStatus;
    if (pthread_join(listenerThread, &listenerThreadExitStatus) != 0) {
        printf("Error: failed to join listener thread\n");
    }

    if (listenerThreadExitStatus != PTHREAD_CANCELED) {
        printf("Error: thread has not been canceled, attempting to terminate thread\n");
        if (pthread_cancel(listenerThread) != 0) {
            printf("Error: thread failed to cancel\n");
        }
    }

    WaveStreamer_stopStreaming();
    snd_pcm_close(pcmDevice);
}

int Microphone_getCurrentDecibel() {
    double decibel = 0.0;
    pthread_mutex_lock(&currentDecibelMutex);
    {
        decibel = currentDecibel;
    }
    pthread_mutex_unlock(&currentDecibelMutex);

    return (int) ceil(decibel);
}

_Bool Microphone_isDecibelNormal(int decibel) {
    if (decibel >= MAX_DECIBEL_THRESH_HOLD || decibel <= MIN_DECIBEL_THRESH_HOLD) {
        return false;
    }

    return true;
}

static _Bool connectToDevice() {
    if (snd_pcm_open(&pcmDevice, PCM_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        printf("Error: unable to open PCM device\n");
        return false;
    }

    return true;
}

static _Bool initializeDeviceSettings() {
    snd_pcm_hw_params_alloca(&deviceSettings);
    snd_pcm_hw_params_any(pcmDevice, deviceSettings);
    unsigned int sampleRate = SAMPLE_RATE_IN_HERTZ;
    if (
         snd_pcm_hw_params_any(pcmDevice, deviceSettings) < 0 ||
         snd_pcm_hw_params_set_access(pcmDevice, deviceSettings, SND_PCM_ACCESS_RW_INTERLEAVED) < 0 ||
         snd_pcm_hw_params_set_format(pcmDevice, deviceSettings, SND_PCM_FORMAT_S16_LE) < 0 ||
         snd_pcm_hw_params_set_rate_near(pcmDevice, deviceSettings, &sampleRate, 0) < 0 ||
         snd_pcm_hw_params_set_channels(pcmDevice, deviceSettings, NUMBER_OF_CHANNELS) < 0 ||
         snd_pcm_hw_params_set_period_size(pcmDevice, deviceSettings, PERIOD_SIZE, 0) < 0 ||
         snd_pcm_hw_params(pcmDevice, deviceSettings)
        ) {

        printf("Error: unable to set device settings\n");
        return false;
    }

    return true;
}

static void stopListening() {
    pthread_mutex_lock(&stopListeningMutex);
    {
        stopListeningFlag = true;
    }
    pthread_mutex_unlock(&stopListeningMutex);
}

static _Bool shouldStopListening() {
    _Bool stopListening;

    pthread_mutex_lock(&stopListeningMutex);
    {
        stopListening = stopListeningFlag;
    }
    pthread_mutex_unlock(&stopListeningMutex);

    return stopListening;
}

static void* listenOverMicrophone(void *args) {
    short buffer[bufferSize];
    int watchDogId = 0;
    _Bool watchDogRegistrationSuccess = WatchDog_registerToWatchDog(&watchDogId);
    int watchDogIterationCounter = 0;

    while (!shouldStopListening()) {
        snd_pcm_sframes_t frames;
        frames = snd_pcm_readi(pcmDevice, &buffer, bufferSize);
        if (frames < 0) {
            printf("Error: reading frames (%s)\n", snd_strerror(frames));

            pthread_exit(PTHREAD_CANCELED);
        }

        WaveStreamer_setSegment(buffer);
        setCurrentDecibels(buffer, bufferSize);
        alertIfDecibelOutsideThreshHold();

        if (watchDogRegistrationSuccess) {
            if (watchDogIterationCounter == WATCH_DOG_KICK_ITERATION) {
                WatchDog_kickWatchDog(watchDogId);
                watchDogIterationCounter = 0;
            } else {
                watchDogIterationCounter++;
            }
        }
    }

    pthread_exit(PTHREAD_CANCELED);
}

static double getAverageAmplitude(short *buffer, int bufferSize) {
    long long amplitudeSquareSum = 0LL;
    for (int i = 0 ; i < bufferSize ; i++) {
        amplitudeSquareSum += (buffer[i] * buffer[i]);
    }

    return sqrt((amplitudeSquareSum / (double) bufferSize));
}

static void setCurrentDecibels(short *buffer, int bufferSize) {
    double averageAmplitude = getAverageAmplitude(buffer, bufferSize);
    double averageDecibels = (20 * log10(averageAmplitude / MAX_AMPLITUDE)) + DB_OFFSET;

    pthread_mutex_lock(&currentDecibelMutex);
    {
        currentDecibel = averageDecibels;
    }
    pthread_mutex_unlock(&currentDecibelMutex);
}

static void alertIfDecibelOutsideThreshHold() {
    int decibel = Microphone_getCurrentDecibel();
    if ( !Microphone_isDecibelNormal(decibel)) {
        printf("Decibel is out side of the threshold with value of: %d\n", decibel);
        TCPSender_sendAlarmRequestToParentBBG();
    }

    TCPSender_sendDataToParentBBG(SOUND, decibel);
}
