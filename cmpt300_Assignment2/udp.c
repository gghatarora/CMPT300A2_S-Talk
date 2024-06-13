// REFERENCES: 
// Beej's Guide to Network Programming: https://beej.us/guide/bgnet/html/#datagram
// Dr. Brian Fraser's Workshop videos provided on coursys
// Read() and Write() system calls tutorial: https://youtu.be/-qbmY6gQYDg?si=qYYBsOnGv3STnpng

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#include "udp.h"
#include "IOThreads.h"

// Struct Info:
// struct addrinfo: preps socket address structures, need to call when making connection
// struct sockaddr: holds socket address info for many types of sockets
// struct sockaddr_in: makes it eaasy to reference elements of socket address (IPv4)
// struct sockaddr_storage: for some calls, dont know in advance if it will fill out struct
//                          sockaddr with IPv4 or IPv6 addr. Therefore we call this struct
//                          and then cast it to the type you need

static pthread_mutex_t s_talkerMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_talkerCond = PTHREAD_COND_INITIALIZER;

int isTalkerDoneWaiting = 0;
pthread_t threadListener;
pthread_t threadTalker;

void* UDP_listener(void* args){

    //initializations
    UDP_socketIN* sockInfo = (UDP_socketIN*) args;
    List* outputList = sockInfo->info;

    int socketfd = sockInfo->sockfd; 
    char* myPort = sockInfo->myPort;
    char* cliPort = sockInfo->cliPort;
    char* cliName = sockInfo->cliName;

    int status; //in beej doc they name it rv idk why
    int numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage cli_addr;
    socklen_t cliAddr_length;
    char buf[MAXBUFLENGTH];

    memset(&hints, 0, sizeof hints); //ensure hints is an empty struct
    hints.ai_family = AF_INET; //use IPv4
    hints.ai_socktype = SOCK_DGRAM; //datagram socket type
    //hints.ai_flags = AI_PASSIVE; //fill in my IP
        
    //First we call getaddrinfo() to get a pointer to a linked list of results(the info of the host (my) server and sockets)
    if((status = getaddrinfo("asb9804u-a04.csil.sfu.ca", myPort, &hints, &servinfo)) != 0){    
        fprintf(stderr, "getaddrinfo listener error: %s\n", gai_strerror(status));
        printf("status is giving int %d\n", status);
        exit(1);
    }

    //servinfo now points to a linked list of struct addrinfo hints
    //iterate over each result in the linked list:
    for(p = servinfo; p != NULL; p = p->ai_next) {
        
        //retrieve the file descriptor of the current result in the linked list
        if((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }

        //bind the socket to the host IP
        if(bind(socketfd, p->ai_addr, p->ai_addrlen) == -1){
            close(socketfd);
            perror("listener: bind");
            continue;
        }

        break;

    }

    //bind fails
    if(p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
    }
    
    //frees the linked list pointed to by servinfo
    freeaddrinfo(servinfo); 

    printf("listener: waiting to recvfrom...\n");

    
    
    //loops as listener waits for some UDP datagram
    while(1){

        cliAddr_length = sizeof(cli_addr);
        while((numbytes = recvfrom(socketfd, buf, MAXBUFLENGTH-1, 0, (struct sockaddr *) &cli_addr, &cliAddr_length)) == -1);

        //termination if user enters "!"
        if(*buf  == '!'){
            //end program
            scrnOutputSignaller();
            pthread_cancel(threadTalker);
            pthread_cancel(threadListener);
            pthread_cancel(getSCRNThread());
            pthread_cancel(getKBThread());
            return NULL;
        }

        buf[numbytes] ='\0'; //set last element to be null

        //lock as we add the message to the output list
        pthread_mutex_lock(getOutListMutex());
        for(int i = 0; i < numbytes; i++){ 
            //need to store all chars in the message arrived in the buffer to msgChar
            char* msgChar = (char*) malloc(sizeof(char)); 
            *msgChar = buf[i];

            //added the latest message to the front of the list
            List_prepend(outputList, msgChar);
            free(msgChar);
        }
        pthread_mutex_unlock(getOutListMutex()); //release the mutex

        //tell our screen output thread that a message is ready to be printed.
        scrnOutputSignaller();
        
    }
    return NULL;
}


void* UDP_talker(void* args) {

    // instantiate struct
    UDP_socketIN* sockInfo = (UDP_socketIN*) args;
    List* inputList = sockInfo->info;

    int _socketfd = sockInfo->sockfd;
    char* _cliPort = sockInfo->cliPort;
    char* _cliName = sockInfo->cliName;


    char* msgBuffer[MAXBUFLENGTH];
    char* writeElement;

    // sockaddr_in in netinet header file
    // sockaddr_in makes it easier to reference socket address elements

    int status;
    int numbytes;
    int msgLen = 0;
    struct addrinfo hints, *srvInfo, *p;
    memset(&hints, 0, sizeof hints); // ensures struct is empty
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //datagram socket type

    //get a pointer to a linked list of results(the info of the client server and sockets)
    if ((status = getaddrinfo(_cliName, _cliPort, &hints, &srvInfo)) != 0) {
        fprintf(stderr, "getaddrinfo talker error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make socket
    for (p = srvInfo; p != NULL; p = p->ai_next) {
        if ((_socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(1);
    }
    
    // waiting for keyboard input
    while(1) {
        
        // talker waits for signal from keyboard
        while(isTalkerDoneWaiting != 1);
        printf("waiting for input..\n");
        talker_isWaiting();

        //lock while we remove from the input list and send to client
        pthread_mutex_lock(getInListMutex());

        // breaks out busy-wait when talker is done waiting
        // add char to message buffer
        writeElement = (char*) List_trim(inputList);
        msgBuffer[msgLen++] = writeElement;

        pthread_mutex_unlock(getInListMutex());

        if ((numbytes = sendto(_socketfd, msgBuffer, msgLen, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    }
    

    freeaddrinfo(srvInfo);

    printf("talker: sent %d bytes to %s\n", numbytes, _cliName);
    close(_socketfd);

    

    return 0;
}


//FUNCTIONS USED TO HELP COMMUNICATE USING CONDITION VARS
void talkerSignaller() {
    pthread_mutex_lock(&s_talkerMutex);
    pthread_cond_signal(&s_talkerCond);
    pthread_mutex_unlock(&s_talkerMutex);
}

void talker_isWaiting() {
    pthread_mutex_lock(&s_talkerMutex);
    isTalkerDoneWaiting = 1;
    pthread_cond_wait(&s_talkerCond, &s_talkerMutex);
    pthread_mutex_unlock(&s_talkerMutex);
}


//USED TO CREATE THREADS FOR LISTENER AND TALKER
void UDP_listener_init(UDP_socketIN socketInfo){
    pthread_create(&threadListener, NULL, UDP_listener, &socketInfo);
}

void UDP_talker_init(UDP_socketIN socketInfo){
    pthread_create(&threadTalker, NULL, UDP_talker, &socketInfo);
}

//USED TO TELL THE PROCESS TO WAIT FOR EACH THREAD TO FINISH RUNNING BEFORE ENDING
void UDP_listener_waitForShutdown(){
    pthread_join(threadListener, NULL);
}

void UDP_talker_waitForShutdown(){
    pthread_join(threadTalker, NULL);
}


//USED TO ACCESS THESE THREADS IN OTHER FILES
pthread_t getListenerThread(){
    return threadListener;
}

pthread_t getTalkerThread(){
    return threadTalker;
}