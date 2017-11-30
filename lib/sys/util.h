#ifndef _PARENT_PUBLIC_FUNC_H_
#define _PARENT_PUBLIC_FUNC_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STR_CONVERT(str) #str

extern const struct timespec delay1s;
extern const struct timespec delay400ns;
extern const struct timespec delay1us;
extern const struct timespec delay1ms;
extern const struct timespec delay100ms;
extern const struct timespec delay500ms;

void sleep_usec(long usec);
void sleep_msec(long msec);
int fileWriteD(char* filePath, int value);
int fileWriteS(char* filePath, char* value);
int fileReadD(char* filePath);


#endif