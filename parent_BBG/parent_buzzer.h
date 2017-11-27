#ifndef _PARENT_BUZZER_H_
#define _PARENT_BUZZER_H_

#include <stdio.h>
#include <stdlib.h>
#include "parent_gpio.h"
#include <pthread.h>


int pmwBuzzOn(void);
int pmwBuzzOff(void);
int pmwBuzzValueSet(int period, int cycle);
int pmwBuzzSound(int mode);
int pmwBuzzModeDefault(void);
int pmwBuzzInit(void);
void pmwBuzzCleanUp(void);
void pmwBuzzSelectPrv(void);
void pmwBuzzSelectNext(void);

#endif