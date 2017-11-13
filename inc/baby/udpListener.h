/*
    udpListener.h
    Module to spawn a separate thread to listen to port 8809 for incoming UDP packets on a background thread.
    It responds to baby monitor system web interface


    nodejs server and C server program which runs in monitor BBG should agree the following message protocols

    GET request from nodejs server to C server
    e.g. getTemperature\n

    SET request from nodejs server to C server
    e.g. setTemperature:<value>\n

    C server responds to GET request from nodejs server
    e.g. getTemperature:<value>\n

    C server responds to SET request from nodejs server
    e.g. setTemperature:ok\n
*/

#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

// Begin/end the background thread which listens to port 8809
_Bool UDPListener_startListening(void);
void UDPListener_stopListening(void);

#endif