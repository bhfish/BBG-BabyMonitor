#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
#include <pthread.h>    // pthread_*
#include <string.h>     // strerror
#include <errno.h>      // errno
#include <sys/socket.h> // socket
#include <unistd.h>     // close
#include <netdb.h>      // sockaddr_in
#include <arpa/inet.h>  // inet_pton
#include <signal.h>     // signal
#include <fcntl.h>      // fcntl
#include "tcpSender.h"

#define MAX_SEND_MSG_LEN            500
#define MAX_RECV_MSG_LEN            500

// TODO: change it later
#define PARENT_BBG_IPV4_ADDR        "192.168.3.1"
#define PARENT_BBG_PORT_NUM         12345

static int clientSocketFD;
static _Bool wasConnectionSuccess = false;
static pthread_mutex_t connectionFlagMutex = PTHREAD_MUTEX_INITIALIZER;

static void getFormatedMsg(DATA_CATEGORY CATEGORY, int dataVal, char *message);
static void keepTCPClientAlive(int connectionErrNum);
static void reconnectToParentBBG(void);
static _Bool initTCPSocket(void);

_Bool TCPSender_init(void)
{
    if ( !initTCPSocket() ) {
        printf("[ERROR] failed to initialize a client socket to communicate to parent's BBG\n");

        pthread_mutex_lock(&connectionFlagMutex);
        {
            wasConnectionSuccess = false;
        }
        pthread_mutex_unlock(&connectionFlagMutex);

        return false;
    }

    pthread_mutex_lock(&connectionFlagMutex);
    {
        wasConnectionSuccess = true;
    }
    pthread_mutex_unlock(&connectionFlagMutex);

    return true;
}

_Bool TCPSender_sendDataToParentBBG(DATA_CATEGORY CATEGORY, int dataToSend)
{
    char msgToParentBBG[MAX_SEND_MSG_LEN] = {0};

    getFormatedMsg(CATEGORY, dataToSend, msgToParentBBG);

    if (!wasConnectionSuccess) {
        reconnectToParentBBG();
    }

    if (wasConnectionSuccess) {
        if (send(clientSocketFD, msgToParentBBG, strlen(msgToParentBBG), MSG_DONTWAIT| MSG_NOSIGNAL) == -1) {
            printf("[ERROR] failed to send %s to parent's BBG reason: %s\n", msgToParentBBG, strerror(errno));
            keepTCPClientAlive(errno);

            return false;
        }
    }

    return true;
}

_Bool TCPSender_sendAlarmRequestToParentBBG(void)
{
    char msgToParentBBG[MAX_SEND_MSG_LEN] = {0};

    sprintf(msgToParentBBG, "%s%c%c", "alarm", DATA_CATEGORY_VALUE_SEPERATOR, END_OF_DATA_VALUE_SEPERATOR);

    if (!wasConnectionSuccess) {
        reconnectToParentBBG();
    }

    if (wasConnectionSuccess) {
        if (send(clientSocketFD, msgToParentBBG, strlen(msgToParentBBG), MSG_DONTWAIT | MSG_NOSIGNAL) == -1) {
            printf("[ERROR] failed to send %s to parent's BBG reason: %s\n", msgToParentBBG, strerror(errno));
            keepTCPClientAlive(errno);

            return false;
        }
    }

    return true;
}

void TCPSender_cleanUp(void)
{
    close(clientSocketFD);
}

// create a TCP socket to talk to parent's BBG
static _Bool initTCPSocket(void)
{
    struct sockaddr_in parentBBGAddr;

    parentBBGAddr.sin_family = AF_INET;
    parentBBGAddr.sin_port = htons(PARENT_BBG_PORT_NUM);

    // covert IPV4 address from string to binary format
    if (inet_pton(AF_INET, PARENT_BBG_IPV4_ADDR, &parentBBGAddr.sin_addr) == 0) {
        printf("[ERROR] invalid IP address: %s\n", PARENT_BBG_IPV4_ADDR);

        return false;
    }

    clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocketFD == -1) {
        printf("[ERROR] failed to initialize client's TCP socket reason: %s\n", strerror(errno));

        return false;
    }

    if (connect(clientSocketFD, (struct sockaddr*)&parentBBGAddr, sizeof(parentBBGAddr)) == -1) {
        printf("[ERROR] failed to connect to parent's BBG reason %s\n", strerror(errno));
        close(clientSocketFD);

        return false;
    }

    return true;
}

// format the data into string format of "<data category>:<data value>;"
static void getFormatedMsg(DATA_CATEGORY CATEGORY, int dataVal, char *message)
{
    switch (CATEGORY) {
        case TEMPERATURE:
            sprintf(message, "%s%c%d%c", TEMPERATURE_TYPE_DATA, DATA_CATEGORY_VALUE_SEPERATOR,
                                                dataVal, END_OF_DATA_VALUE_SEPERATOR);
            break;
        case SOUND:
            sprintf(message, "%s%c%d%c", SOUND_TYPE_DATA, DATA_CATEGORY_VALUE_SEPERATOR,
                                                dataVal, END_OF_DATA_VALUE_SEPERATOR);
            break;
        case UNKNOWN:
            // parent's BBG doesn't needs any data from accelerometer
            sprintf(message, "%c", END_OF_DATA_VALUE_SEPERATOR);

            break;
    }
}

// by default, TCP client will be killed if it tried to send to a disconnected host. Handle this by ignoring the corresponding "kill" signal
static void keepTCPClientAlive(int connectionErrNum)
{
    /*
        SIGPIPE action: termination; Broken pipe: write to pipe with no readers
        EPIPE  The local end has been shut down on a connection oriented socket.
    */
    if (connectionErrNum == EPIPE) {
        /*
            the entire monitoring system shouldn't be killed as user may still need to
            be able to access to baby's video streaming
        */
        signal(SIGPIPE, SIG_IGN);

        pthread_mutex_lock(&connectionFlagMutex);
        {
            wasConnectionSuccess = false;
        }
        pthread_mutex_unlock(&connectionFlagMutex);
    }
}

// re-establish the connection to parent's BBG
static void reconnectToParentBBG()
{
    printf("Connection to parent's BBG was dropped or not established. Retry connection...\n");
    TCPSender_cleanUp();
    TCPSender_init();
}
