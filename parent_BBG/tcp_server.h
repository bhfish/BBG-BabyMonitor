#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <pthread.h>

extern pthread_t tcpServerThreadId;

int tcpServerInit(void);

#endif