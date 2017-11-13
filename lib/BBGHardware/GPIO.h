/*
    GPIO.h
    Module to provide GPIO related functionalities
*/

#ifndef GPIO_H_INCLUDE_GUARD
#define GPIO_H_INCLUDE_GUARD

// tell Linux to handle the pin as GPIO by writing specified Linux GPIO number to the export file
_Bool GPIO_initPin(int GPIONum);

// set the specified GPIO number as input; on/off
_Bool GPIO_setPinAsInput(int GPIONum);

// get the status on/off from a specified input GPIO number
_Bool GPIO_getInputPinStatus(int GPIONum, int *pinStatus);

#endif