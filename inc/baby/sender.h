/*
    sender.h
    module to provide functionalities for sending data to parent's BBG via TCP

    REFERENCE:
    any data that needed to be sending to parent's BBG will be formated as the following string/message
    "<data type>:<data value>;"
    e.g. "temperature:23;" or "sound:77;"

    NOTE:
    this module will not be responsible for checking the validity and integrity of such data
*/
#ifndef SENDER_H
#define SENDER_H

#define TEMPERATURE_TYPE_DATA       "temperature"
#define SOUND_TYPE_DATA             "sound"
#define DATA_TYPE_VALUE_SPLITOR     ':'
#define END_OF_DATA_VALUE_SPLITOR   ';'

typedef enum
{
    TEMPERATURE,
    SOUND
}DATA_CATEGORY;

// initialize necessary resources and establish TCP connection to parent's BBG
_Bool Sender_init(void);

// send specified data to parent's BBG via TCP socket
_Bool Sender_sendDataToParentBBG(int dataToSend, DATA_CATEGORY CATEGORY);

// de-allocate resource gracefully
void Sender_cleanUp(void);

#endif