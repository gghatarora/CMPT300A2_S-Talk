// REFERENCES: 
// Beej's Guide to Network Programming: https://beej.us/guide/bgnet/html/#datagram
// Dr. Brian Fraser's Workshop videos provided on coursys
// Read() and Write() system calls tutorial: https://youtu.be/-qbmY6gQYDg?si=qYYBsOnGv3STnpng

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "udp.h"
#include "IOThreads.h"
#include "list/list.h"

char* myPortNumber;
char* clientName;
char* clientPortNumber;
int listnerSocket;
int talkerSocket;

int main(int argc, char** argv) {

    // tells me somethings wrong if i have wrong number of arg
    if (argc != 4) {
        printf("Usage: %s <my port number> <client name> <client port number>\n", argv[0]);
        exit(-1);
    }

    // extract user input
    myPortNumber = argv[1];
    clientName = argv[2];
    clientPortNumber = argv[3];

    // create screen and keyboard lists
    List* kb_List = List_create();
    List* scrn_List = List_create();

    // initialize sockets
    IO_info kbIN = {kb_List};
    IO_info scrnIN = {scrn_List};
    UDP_socketIN tlkrIN = {kb_List, talkerSocket, myPortNumber, clientPortNumber, clientName};
    UDP_socketIN lstnrIN = {scrn_List, listnerSocket, myPortNumber, clientPortNumber, clientName};
    
    // create threads
    kb_init(kbIN);
    scrn_init(scrnIN);
    UDP_talker_init(tlkrIN);
    UDP_listener_init(lstnrIN);

    //join threads
    kb_waitForShutdown();
    scrn_waitForShutdown();
    UDP_listener_waitForShutdown();
    UDP_talker_waitForShutdown();

    return 0;
}