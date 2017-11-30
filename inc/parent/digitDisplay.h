#ifndef DIGIT_DISPLAY_H_
#define DIGIT_DISPLAY_H_

#include <pthread.h>

int DigitDisplay_displayNum(int num);
int DigitDisplay_init(void);
void DigitDisplay_cleanUp(void);

#endif