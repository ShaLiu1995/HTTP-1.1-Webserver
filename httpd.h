#ifndef HTTPD_H
#define HTTPD_H

#include <string>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

// Start httpd
void start_httpd(unsigned short port, std::string doc_root);
// Handle new TCP client
void HandleTCPClient(int clntSocket, std::string doc_root);
// Error handling function
void DieWithError(const char *errorMessage); 
// Create TCP server socket
int CreateTCPServerSocket(unsigned short port);
// Accept TCP connection request
int AcceptTCPConnection(int servSock); 

// Size Constants
enum sizeConstants {
  MAXPATHLENGTH = 4096,
  BUFSIZE = 512,
  MAXPENDING = 5
};

#endif // HTTPD_H
