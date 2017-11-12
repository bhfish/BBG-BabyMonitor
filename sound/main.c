#include "Microphone.h"

int main() {
    Microphone_startListening();
    int volatile counter = 0;
    while (1) {
    	counter++;
    }

    Microphone_stopListening();

    return 0;
}