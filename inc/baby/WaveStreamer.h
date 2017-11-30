 //
// Created by vincent on 11//11/2017.
//

#ifndef WAVE_STREAMER_H
#define WAVE_STREAMER_H

_Bool WaveStreamer_startStreaming(int segmentSize);
void WaveStreamer_stopStreaming();
void WaveStreamer_setSegment(short* buffer);

#endif