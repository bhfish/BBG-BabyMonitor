#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <pthread.h>

int tcpServerInit(void);
void tcpServerCleanup(void);
#endif