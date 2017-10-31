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
#include "sender.h"

#define MAX_SEND_MSG_LEN            500

// TODO: change it later
#define PARENT_BBG_IPV4_ADDR        "192.168.2.1"
#define PARENT_BBG_PORT_NUM         8808

static int clientSocketFD;
static struct sockaddr_in parentBBGAddr;

static void getFormatedMsg(int dataVal, DATA_CATEGORY CATEGORY, char *message);
static _Bool initTCPSocket(void);

_Bool Sender_init(void)
{
    if ( !initTCPSocket() ) {
        printf("[ERROR] failed to initialize a client socket to communicate to parent's BBG\n");

        return false;
    }

    return true;
}

_Bool Sender_sendDataToParentBBG(int dataToSend, DATA_CATEGORY CATEGORY)
{
    char msgToParentBBG[MAX_SEND_MSG_LEN] = {0};

    getFormatedMsg(dataToSend, CATEGORY, msgToParentBBG);

    if (send(clientSocketFD, msgToParentBBG, strlen(msgToParentBBG), MSG_DONTWAIT) == -1) {
        printf("[ERROR] failed to send %s to parent's BBG reason: %s\n", msgToParentBBG, strerror(errno));

        return false;
    }

    return true;
}

void Sender_cleanUp(void)
{
    close(clientSocketFD);
}

// create a TCP socket to talk to parent's BBG
static _Bool initTCPSocket(void)
{
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
static void getFormatedMsg(int dataVal, DATA_CATEGORY CATEGORY, char *message)
{
    switch (CATEGORY) {
        case TEMPERATURE:
            sprintf(message, "%s%c%d%c", TEMPERATURE_TYPE_DATA, DATA_TYPE_VALUE_SPLITOR,
                                                dataVal, END_OF_DATA_VALUE_SPLITOR);
            break;
        case SOUND:
            sprintf(message, "%s%c%d%c", SOUND_TYPE_DATA, DATA_TYPE_VALUE_SPLITOR,
                                                dataVal, END_OF_DATA_VALUE_SPLITOR);
            break;
    }
}