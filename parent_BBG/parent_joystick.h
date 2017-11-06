#ifndef _PARENT_JOYST_H_
#define _PARENT_JOYST_H_

#include <stdio.h>
#include <stdlib.h>
#include "parent_gpio.h"

typedef enum
{
	JOYST_UP = 0,
	JOYST_DOWN,
	JOYST_RIGHT,
	JOYST_LEFT,
	JOYST_PUSH,
	JOYST_NONE	
} joystkDrctn_t;

extern joystkDrctn_t joystkDirection;

void joystkInit(void);

#endif