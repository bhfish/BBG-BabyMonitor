/*
    watchDog.h
    spawn a background thread which will provide watch dog service to other modules in baby monitoring system
*/

#ifndef WATCH_DOG_H
#define WATCH_DOG_H

// create a thread which continuously serves the "kick" requests made by other modules
_Bool WatchDog_initWatchDog(void);

// register a client to the watchdog service by returning a specific client reference ID
_Bool WatchDog_registerToWatchDog(int *clientRefIDPtr);

// deregister the specified client from the watchdog service
void WatchDog_deregisterFromWatchDog(int clientRefID);

// kick the watch dog in order to prevent the watch dog timer from expired
void WatchDog_kickWatchDog(int clientRefID);

// get the current configured watch dog *HARD* timer in seconds
int WatchDog_getWatchDogTimer(void);

#endif