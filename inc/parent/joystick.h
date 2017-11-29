#ifndef _PARENT_JOYST_H_
#define _PARENT_JOYST_H_

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	JOYST_UP = 0,
	JOYST_DOWN,
	JOYST_RIGHT,
	JOYST_LEFT,
	JOYST_PUSH,
	JOYST_NONE
} joystkDrctn_t;

int Joystick_init(void);
void Joystick_cleanUp(void);
joystkDrctn_t Joystick_getPressedDirection(void);

#endif