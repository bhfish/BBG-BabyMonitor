#ifndef _PARENT_BUZZER_H_
#define _PARENT_BUZZER_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int Buzzer_turnOn(void);
int Buzzer_turnOff(void);
int Buzzer_setValue(int period, int cycle);
int Buzzer_setSoundMode(int mode);
int Buzzer_setDefaultMode(void);
int Buzzer_init(void);
void Buzzer_cleanUp(void);
void Buzzer_slectPrevMode(void);
void Buzzer_slectNextMode(void);

#endif