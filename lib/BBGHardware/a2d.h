/*
    A2D.h
    Module to provide A2D related functionalities
*/

#ifndef A2D_H
#define A2D_H

#define MAX_A2D_VALUE   4095

// enable A2D functionality for the specified analog input number
_Bool A2D_init(int AINNum);

// read the analog value from the specified analog input number
int A2D_getAnalogReading(int AINNum);

#endif