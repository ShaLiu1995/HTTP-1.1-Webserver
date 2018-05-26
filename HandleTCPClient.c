#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <assert.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "httpd.h"
#include "HTTPFramer.hpp"
#include "HTTPParser.hpp"
#include "HTTPResponser.hpp"



void HandleTCPClient(int clntSocket, std::string doc_root)
{
    char buffer[BUFSIZE]; // Buffer for incoming request
    memset(buffer, 0, BUFSIZE);

    // Receive part of the request from the client
    ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    if (numBytesRcvd < 0)
        DieWithError("recv() failed");

    HTTPFramer framer;

    // Send received string and receive again until end of stream
    while (numBytesRcvd > 0) { // 0 indicates end of stream
        // printf("Buffer is:\n%s", buffer);
        framer.append(buffer);

    	HTTPInstruction inst;
        HTTPHeader resp;
        inst.isMalformed = false;
        inst.hasInitialLine = false;

        while (framer.hasMessage()) {
        	std::string message = framer.topMessage();
        	// printf("Message is:%s[CRLF]\n", message.c_str());
        	framer.popMessage();
        	HTTPParser::parse(message, &inst);
        }
        
            
        // Generate const char* response to send
        // char response[BUFSIZE];
        HTTPResponser::getHTTPHeader(&inst, &resp, doc_root);
        std::string header = HTTPResponser::getHeaderMsg(&resp);

        // HTTPParser::show(&inst);
        // HTTPResponser::show(&resp);

        ssize_t numBytesSent;
        numBytesSent = send(clntSocket, header.c_str(), header.length(), 0);
        if (numBytesSent < 0) 
            DieWithError("send() failed");


        if (resp.statusCode == "200 OK") {
            HTTPResponser::sendBodyMsg(&resp, clntSocket);
        }
        
        // See if there is more data to receive
        memset(buffer, 0, BUFSIZE);
        numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        if (numBytesRcvd < 0) {
            DieWithError("recv() failed");
        }
    }

    close(clntSocket); // Close client socket
}


