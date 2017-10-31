#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // _Bool
// TODO remove later
#include <unistd.h>     // sleep
#include "sender.h"
#include "temperatureMonitor.h"
#include "accelerometerMonitor.h"

int main(int argc, char const *argv[])
{
    // Testing codes
    if ( !Sender_init() ) {
        printf("[ERROR] failed to init sender\n");

        exit(EXIT_FAILURE);
    }

    // send single data to parent
    Sender_sendDataToParentBBG(33, TEMPERATURE);

    // send multiple data to parent
    int sleepCnt = 0;

    while (sleepCnt < 100) {
        Sender_sendDataToParentBBG(sleepCnt + 100 / 20, TEMPERATURE);
        sleep(1);
        sleepCnt++;
    }

    return 0;
}