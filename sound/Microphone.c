//
// Created by vincent on 10/22/17.
//

#include<alsa/asoundlib.h>
#include<math.h>
#include<stdbool.h>
#include <alloca.h>

static _Bool initializeDeviceSettings();
static double getAverageAmplitude();
static double getAverageDecibels(double averageAmplitude);

#define PCM_DEVICE_NAME "plughw:U0x46d0x825,0"
#define SAMPLE_RATE_IN_HERTZ 48000
#define NUMBER_OF_CHANNELS 1

static const snd_pcm_uframes_t PERIOD_SIZE = 4800;

static snd_pcm_t *pcmDevice;
static snd_pcm_hw_params_t *deviceSettings;

_Bool Microphone_Initialize() {
    if (snd_pcm_open(&pcmDevice, PCM_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        printf("Error: unable to open PCM device");
        return false;
    }

    if (!initializeDeviceSettings()) {
        return false;
    }

    return true;
}

void Microphone_Stop() {
    snd_pcm_close(pcmDevice);
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

static double getAverageAmplitude(short *buffer, int bufferSize) {
    long long amplitudeSquareSum = 0LL;
    for (int i = 0 ; i < bufferSize ; i++) {
        amplitudeSquareSum += (buffer[i] * buffer[i]);
    }

    return sqrt((amplitudeSquareSum / (double) bufferSize));
}

static double getAverageDecibels(double averageAmplitude) {
    return 20 * log10(averageAmplitude);
}

void* sendAverageDecibels() {
    snd_pcm_uframes_t bufferSize = 2 * PERIOD_SIZE * 2;
    short buffer[bufferSize];
    while (1) {
        snd_pcm_sframes_t frames;
        frames = snd_pcm_readi(pcmDevice, &buffer, bufferSize);
        if (frames < 0) {
            printf("Error: reading frames (%s)\n", snd_strerror(frames));
            break;
        }

        double averageAmplitude = getAverageAmplitude(buffer, bufferSize);
        double averageDecibels = getAverageDecibels(averageAmplitude);
        printf("\r current amplitude: %f current decibel: %f", averageAmplitude, averageDecibels);
        fflush(stdout);
    }

    return NULL;
}