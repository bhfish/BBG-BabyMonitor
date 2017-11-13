 //
// Created by vincent on 11//11/2017.
//

#ifndef WAVE_STREAMER_H
#define WAVE_STREAMER_H

_Bool WaveStreamer_startStreaming();
void WaveStreamer_stopStreaming();
void WaveStreamer_setBuffer(void* buffer, int bufferSize);

#endif