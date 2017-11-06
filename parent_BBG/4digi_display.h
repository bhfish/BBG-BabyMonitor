#ifndef _4DIGI_DISPLAY_H_
#define _4DIGI_DISPLAY_H_

#include <pthread.h>

extern pthread_t digiDisplay_thread;

int digiDispNum(int num);
int digiDispInit(void);

void digiTest(void);
void digiTest1(void);

#endif