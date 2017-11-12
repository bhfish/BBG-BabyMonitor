#ifndef _PARENT_BUZZER_H_
#define _PARENT_BUZZER_H_

#include <stdio.h>
#include <stdlib.h>
#include "parent_gpio.h"
#include <pthread.h>

extern pthread_t buzzer_thread;

int pmwBuzzOn(void);
int pmwBuzzOff(void);
int pmwBuzzValueSet(int period, int cycle);
int pmwBuzzSound(int mode);
int pmwBuzzModeDefault(void);
int pmwBuzzLoop(void);
int pmwBuzzInit(void);
void pmwBuzzSelectPrv(void);
void pmwBuzzSelectNext(void);

#endif