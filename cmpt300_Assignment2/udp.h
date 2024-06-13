#ifndef UDP_H_
#define UDP_H_

#define MAXBUFLENGTH 500

#include <pthread.h>
#include "list/list.h"

typedef struct UDP_socketIN_s UDP_socketIN;
struct UDP_socketIN_s {
    List* info;
    int sockfd; // file descriptor
    char* myPort; // host port #
    char* cliPort; // client port #
    char* cliName; // host name to connect to or IP
};

//Waits for a UDP datagram
void* UDP_listener(void* args);

//Sends UDP datagram to remote process
void* UDP_talker(void* args);

//initializes our socket thread struct and creates pthread
void UDP_listener_init(UDP_socketIN socketLstnrInfo);
void UDP_talker_init(UDP_socketIN socketTlkerInfo);

//Calls pthread_join and waits for each thread to finish running before ending the program
void UDP_talker_waitForShutdown();
void UDP_listener_waitForShutdown();

//used to communicate between the talker and keyboard input threads
void talkerSignaller(); //used by keyboard to signal the condition to talker when a message is in the input list
void talker_isWaiting(); //tells the talker to wait using pthread_cond_wait. waits for list to populate before sending

//used to access threads in other files.
pthread_t getListenerThread();
pthread_t getTalkerThread();


#endif