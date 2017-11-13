// student name: WEICHAO(Wilson) LIN
// student number: 301204628
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <string.h>     // strerror, strlen
#include <errno.h>      // errno
#include <ctype.h>      // tolower
#include <pthread.h>    // pthread_*
#include <sys/socket.h> // socket, accept, bind, recvfrom
#include <unistd.h>     // close
#include <arpa/inet.h>  // inet_ntoa
#include <netdb.h>      // sockaddr_in
#include <fcntl.h>      // fcntl
#include "temperatureMonitor.h"
#include "udpListener.h"
#include "babyMonitor.h"

#define LISTEN_PORT_NUM                    8809
#define MAX_RECV_MSG_LENGTH                500
#define MAX_RESPONSE_MSG_LENGTH            1000

static int serverSocketFD;
static pthread_t listenerThread;
static _Bool stopListening = false;

static _Bool InitUDPSocket(void);
static void RespondClientsRequest(char *clientRequest, int clientFd, struct sockaddr_in clientAddr);
static void SendResponseMsgToClient(void *response, size_t responseLen, int clientFd, struct sockaddr_in clientAddr);
static void *StartListenerThread(void *args);

_Bool UDPListener_startListening(void)
{
    /*
        1) spawn a new thread

        the new thread will do
        1.1) init a non-blocking UDP socket continually
        1.1) fetch client's message
        1.2) respond to client's message
    */
    if ( !InitUDPSocket() ) {
        return false;
    }

    if (pthread_create(&listenerThread, NULL, &StartListenerThread, NULL) != 0)
    {
        printf("[ERROR] failed to create a thread in listener module reason: %s\n", strerror(errno));

        return false;
    }

    return true;
}


void UDPListener_stopListening(void)
{
    /*
        1) change the global variable in order to tell the listener thread to exit
        2) join the listener thread in order to check its cancellation response
        3) check the cancellation status
    */

    void *listenerThreadExitStatus;
    stopListening = true;

    close(serverSocketFD);

    if (pthread_join(listenerThread, &listenerThreadExitStatus) != 0)
    {
        printf("[ERROR] failed to join with terminating listener thread failed reason: %s\n", strerror(errno));
    }

    if (listenerThreadExitStatus != PTHREAD_CANCELED)
    {
        // bad things happened...
        printf("[ERROR] abnormal termination state of listener thread\n");
        printf("try to send a cancel request and wish listener thread can normally be terminated...\n");

        if(pthread_cancel(listenerThread) != 0)
        {
            printf("[ERROR] failed to send a cancel request to listener thread failed reason: %s\n", strerror(errno));
        }
    }
}

// send the specific response to client
static void SendResponseMsgToClient(void *response, size_t responseLen, int clientFd, struct sockaddr_in clientAddr)
{
    if (sendto( clientFd, response, responseLen, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr) ) == -1) {
        printf("[ERROR] failed to send %s to client's address %s\n", (char *) response, inet_ntoa(clientAddr.sin_addr));
    }

    #ifdef DEBUG_MODE
        printf("[DEBUG] sent %s\n", (char*) response);
    #endif
}

// parse and respond to client's specified request
static void RespondClientsRequest(char *clientRequest, int clientFd, struct sockaddr_in clientAddr)
{
    char responseMsg[MAX_RESPONSE_MSG_LENGTH] = {0};

    if (strcmp(clientRequest, "getMonitorBBGStatus") == 0) {
        if ( BabayMonitor_isSystemRunning() ) {
            sprintf(responseMsg, "getMonitorBBGStatus:Active");
        }
        else {
            sprintf(responseMsg, "getMonitorBBGStatus:Inactive");
        }
    }
    else if (strcmp(clientRequest, "getTemperature") == 0) {
        sprintf(responseMsg, "getTemperature:%d", TemperatureMonitor_getCurrentTemperature());
    }

    SendResponseMsgToClient(responseMsg, strlen(responseMsg), clientFd, clientAddr);
}

// create an UDP socket for listening incoming UDP packets
static _Bool InitUDPSocket(void)
{
    /*
        socket reference:
        AF_INET: IPv4 Internet protocols
        PF_INET: the manifest constant used under 4.x BSD for protocol families
        SOCK_DGRAM: supports datagrams (connectionless, unreliable messages of a fixed maximum length)
        INADDR_ANY: INADDR_ANY is specified in the bind call, the socket will be bound to all local interfaces.
    */
    struct sockaddr_in serverAddr;

    serverSocketFD = socket(PF_INET, SOCK_DGRAM, 0);

    if (serverSocketFD == -1)
    {
        printf("[ERROR] failed to create a socket reason: %s\n", strerror(errno));

        return false;
    }

    if (fcntl(serverSocketFD, F_SETFL, O_NONBLOCK) == -1)
    {
        printf("[ERROR] failed to set the socket as O_NONBLOCK reason: %s\n", strerror(errno));
        close(serverSocketFD);

        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(LISTEN_PORT_NUM);

    if (bind(serverSocketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("[ERROR] failed to bind to the created socket reason: %s\n", strerror(errno));
        close(serverSocketFD);

        return false;
    }

    return true;
}

// listener thread begins its duties here
static void *StartListenerThread(void *args)
{
    int msgLenFromClient;
    char recvMsg[MAX_RECV_MSG_LENGTH] = {0};
    struct sockaddr_in clientAddr;
    unsigned int clientAddrLen = sizeof(clientAddr);

    while (!stopListening)
    {
        // it's ok to failed to receive a message from client because of UDP. Just wait for the next message
        msgLenFromClient = recvfrom(serverSocketFD, recvMsg, MAX_RECV_MSG_LENGTH, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

        // remove the trialling new line character since user will hit "Enter" key to supply the request
        recvMsg[strlen(recvMsg) - 1] = '\0';

        if (msgLenFromClient == -1)
        {
            // exit the program if catched any other errors other than unavailable client message
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                printf("[ERROR] failed to receive a message from client reason %s\n", strerror(errno));

                break;
            }
        }
        else if (msgLenFromClient > 0)
        {
            RespondClientsRequest(recvMsg, serverSocketFD, clientAddr);
        }

        memset(recvMsg, '\0', MAX_RECV_MSG_LENGTH);
    }

    pthread_exit(PTHREAD_CANCELED);
}