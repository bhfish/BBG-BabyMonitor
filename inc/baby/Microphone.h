//
// Created by vincent on 10/22/17.
//

#ifndef MICROPHONE_H
#define MICROPHONE_H

_Bool Microphone_startListening();
void Microphone_stopListening();
int Microphone_getCurrentDecibel();
_Bool Microphone_isDecibelNormal(int decibel);

#endif
