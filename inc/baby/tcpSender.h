/*
    tcpSender.h
    module to provide functionalities for sending data/alarm request to parent's BBG via TCP

    REFERENCE:
    any data that needed to be sending to parent's BBG will be formated as the following string/message
    "<data type>:<data value>;"
    e.g. "temperature:23;" or "sound:77;"
    or alarm
    e.g. "alarm:temperature:23"

    NOTE:
    this module will not be responsible for checking the validity and integrity of such data
*/
#ifndef TCP_SENDER_H
#define TCP_SENDER_H

#include "monitorData.h"

#define DATA_CATEGORY_VALUE_SEPERATOR     ':'
#define END_OF_DATA_VALUE_SEPERATOR       ';'

// initialize necessary resources and establish TCP connection to parent's BBG
_Bool TCPSender_init(void);

// send specified data to parent's BBG via TCP socket
_Bool TCPSender_sendDataToParentBBG(int dataToSend, DATA_CATEGORY CATEGORY);

// send alarm request to parent's BBG via TCP socket
_Bool TCPSender_sendAlarmRequestToParentBBG(void);

// de-allocate resource gracefully
void TCPSender_cleanUp(void);

#endif