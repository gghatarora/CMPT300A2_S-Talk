#ifndef IO_THREADS_H_
#define IO_THREADS_H_

#define MAXBUFLENGTH 500

#include <pthread.h>
#include "list/list.h"

typedef struct IO_info_s IO_info;
struct IO_info_s {
    List* info;
};

//Waits for keyboard input;
//Once received, should add input to a list of messages 
//that are to be sent to a remote process.
void* kbInput(void* args);

//Takes the message off the list once received;
//Prints out characters to screen
void* scrnOutput(void* args);

// Signals the writer to output contents to screen
void scrnOutputSignaller();

// screen output is waiting for signal
void scrn_isWaiting();

// initializes pthread for scrnOutput function
void scrn_init(IO_info ioInfo);

// initializes pthread for kbInput function
void kb_init(IO_info ioInfo);

// waits for the thread to finish before shutdown
void scrn_waitForShutdown();

// waits for the thread to finish before shutdown
void kb_waitForShutdown();

//used to access threads and mutexes in other files. 
pthread_mutex_t* getOutListMutex();
pthread_mutex_t* getInListMutex();
pthread_t getKBThread();
pthread_t getSCRNThread();

#endif