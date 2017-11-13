 //
// Created by vincent on 11//11/2017.
//

#ifndef WAVE_STREAMER_H
#define WAVE_STREAMER_H

_Bool WaveStreamer_startStreaming();
void WaveStreamer_stopStreaming();
_Bool WaveStreamer_sendBuffer(void* buffer, int bufferSize);

#endif