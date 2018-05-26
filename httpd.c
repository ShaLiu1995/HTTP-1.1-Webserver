#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>    /* for thread control */
#include "httpd.h"
#include "HTTPFramer.hpp"
#include "HTTPParser.hpp"


// Main program of a thread
void *ThreadMain(void *arg);

// Struct of arguments to pass to client thread
struct ThreadArgs {
    int clntSock;
    std::string doc_root;
};

void start_httpd(unsigned short port, std::string doc_root)
{
	// cerr << "Starting server (port: " << port <<
	// 	", doc_root: " << doc_root << ")" << endl;
    printf("Starting server (port: %hu , doc_root: %s)\n", port, doc_root.c_str());

    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    socklen_t  clntLen;              /* Length of client address data structure */

    echoServPort = port;  			 /* First arg:  local port */

	/* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port. htons() host to network short */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
            DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    // printf("echoServPort %u\n", echoServPort);
    // printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));


    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;


        if (setsockopt(clntSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            DieWithError("timeout");
        }

        /* Create separate memory for client argument */
        struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
        if (threadArgs == NULL)
            DieWithError("malloc() failed");
        threadArgs->clntSock = clntSock;
        threadArgs->doc_root = doc_root;

        /* Create thread */
        pthread_t threadID;
        int returnValue = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
        if (returnValue != 0)
            DieWithError("pthread_create() failed");
        printf("with thread %lu\n", (unsigned long int) threadID);

    }
    /* NOT REACHED */
}

void *ThreadMain(void *threadArgs) {
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket file descriptor from argument
    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    std::string doc_root = ((struct ThreadArgs *) threadArgs)->doc_root;
    free(threadArgs); // Deallocate memory for argument

    HandleTCPClient(clntSock, doc_root);

    return (NULL);
}