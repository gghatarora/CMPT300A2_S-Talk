// REFERENCES: 
// Beej's Guide to Network Programming: https://beej.us/guide/bgnet/html/#datagram
// Dr. Brian Fraser's Workshop videos provided on coursys
// Read() and Write() system calls tutorial: https://youtu.be/-qbmY6gQYDg?si=qYYBsOnGv3STnpng

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "IOThreads.h"
#include "udp.h"

//creating our mutexs
static pthread_mutex_t s_scrnMutex = PTHREAD_MUTEX_INITIALIZER; //for writing to the screen
static pthread_cond_t s_scrnCond = PTHREAD_COND_INITIALIZER; //condition variable for screen output
static pthread_mutex_t s_outputListMutex = PTHREAD_MUTEX_INITIALIZER; //for output list (shared by listener & scrnOutput)
static pthread_mutex_t s_inputListMutex = PTHREAD_MUTEX_INITIALIZER; //for input list (shared by talker & kbInput)

int isScrnDoneWaiting = 0;

//creating our threads
pthread_t threadScrnOutput;
pthread_t threadkbInput;


void* kbInput(void* args){
    IO_info* msgData = (IO_info*) args;
    List* inputsList = msgData->info;
    char messageBuf[MAXBUFLENGTH]; //contains a buffer of chars to be added into the message
    char* msgChar; // points to the char that was input into the message from the buffer.
    //while nothing is being read from the keyboard input, wait for input.
    while(1){
        if(read(0, messageBuf, MAXBUFLENGTH) == -1){
            perror("ERROR: Failed to read user input");
            exit(-1);
        }

        //allocate space for message
        msgChar = (char*) malloc(sizeof(char));
                        
        //if user types ! end s-talk session
        if(*messageBuf == '!'){
            //end program
            talkerSignaller();
            pthread_cancel(getListenerThread());
            pthread_cancel(getTalkerThread());
            pthread_cancel(threadScrnOutput);
            pthread_cancel(threadkbInput);
            return NULL;
        }

        if(List_count(inputsList) < LIST_MAX_NUM_NODES){
            //add the new input that was read into our list.

            pthread_mutex_lock(&s_inputListMutex);
            for(int i = 0; i < read(0, messageBuf, MAXBUFLENGTH); i++)
            *msgChar = messageBuf[i];
            List_prepend(inputsList, msgChar);
            free(msgChar);
            pthread_mutex_unlock(&s_outputListMutex);

            //tell the talker thread that a new message is ready to be sent.
            talkerSignaller();
        }

    }
    return NULL;
}

void* scrnOutput(void* args){
    IO_info* msgData = (IO_info*) args;
    List* outputList = msgData->info;
    char* writeElement;

    while(1) {

        while(isScrnDoneWaiting != 1);
        scrn_isWaiting(); // waiting for signal from listener

        // begin writing to screen
        pthread_mutex_lock(&s_outputListMutex);

        while (List_count(outputList) != 0) {
            writeElement = (char*) List_trim(outputList);
            write(STDOUT_FILENO, &writeElement, MAXBUFLENGTH);
            // free here //
        }

        pthread_mutex_unlock(&s_outputListMutex);

    }
    return NULL;
}

// Signals the writer to output contents to screen
void scrnOutputSignaller() {

    pthread_mutex_lock(&s_scrnMutex);
    pthread_cond_signal(&s_scrnCond);
    pthread_mutex_unlock(&s_scrnMutex);

}

// screen output is waiting for signal
void scrn_isWaiting() {

    pthread_mutex_lock(&s_scrnMutex);
    isScrnDoneWaiting = 1;
    pthread_cond_wait(&s_scrnCond, &s_scrnMutex);
    pthread_mutex_unlock(&s_scrnMutex);

}

// initializes pthread for scrnOutput function
void scrn_init(IO_info ioInfo) {
    pthread_create(&threadScrnOutput, NULL, scrnOutput, &ioInfo);
}

// initializes pthread for kbInput function
void kb_init(IO_info ioInfo) {
    pthread_create(&threadkbInput, NULL, kbInput, &ioInfo);
}

// waits for the thread to finish before shutdown
void scrn_waitForShutdown() {
    pthread_join(threadScrnOutput, NULL);
}

// waits for the thread to finish before shutdown
void kb_waitForShutdown() {
    pthread_join(threadkbInput, NULL);
}


//THESE FUNCTIONS ARE USED SO THAT WE CAN ACCESS STATIC MUTEXES AND THREADS IN OTHER FILES
pthread_mutex_t* getOutListMutex(){
    return &s_outputListMutex;
}

pthread_mutex_t* getInListMutex(){
    return &s_inputListMutex;
}

pthread_t getKBThread(){
    return threadkbInput;
}

pthread_t getSCRNThread(){
    return threadScrnOutput;
}