#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include "httpd.h"
#include "HTTPResponser.hpp"

void HTTPResponser::getHTTPHeader(HTTPInstruction* inst, HTTPHeader* resp, std::string doc_root)
{
	resp->httpVersion 	= "HTTP/1.1 ";
    resp->server 		= "httpd";
    resp->isBinary      = false;

    // Check malformed
    if (inst->isMalformed || inst->host.empty()) {
        resp->statusCode = "400 Client Error";
        return;
    }

    // Check path
    std::string path;
	if (inst->url == "/") {
    	path = doc_root + "/index.html";
    } else {
    	path = doc_root + inst->url;
    }
    char rootBuffer[PATH_MAX + 1]; 
    resp->rootPath = realpath(doc_root.c_str(), rootBuffer);
    char pathBuffer[PATH_MAX + 1]; 
    resp->fullPath = realpath(path.c_str(), pathBuffer);
    
    if (resp->fullPath) {
        if (!isValidPath(resp->fullPath, resp->rootPath)) {
            resp->statusCode = "404 Not Found";
            return;
        }
        if (errno == ENOENT || errno == ENOTDIR) {
            resp->statusCode = "404 Not Found";
            return;
        } 
        if (errno == EACCES) {
            printf("case1\n");
            resp->statusCode = "403 Forbidden";
            return;
        }
    }

    // Check accessibility
    struct stat sb;
    if (stat(resp->fullPath, &sb) != 0) {
        if (errno == EACCES) {
            printf("case2\n");
            resp->statusCode = "403 Forbidden";
            return;
        } 
        if (errno == ENOENT || errno == EFAULT || errno == ENOTDIR) {
            resp->statusCode = "404 Not Found";
            return;
        }
    } else {
        if (!S_ISREG(sb.st_mode)) {
            printf("case3\n");
            resp->statusCode = "403 Forbidden";
            return;
        } 
        if (!(sb.st_mode & S_IROTH)) {
            resp->statusCode = "403 Forbidden";
            return;
        }
    }

    // Valid file
    resp->statusCode    = "200 OK";
	resp->contentType	= getContentType(resp->fullPath);
	resp->contentLength	= getContentLength(resp->fullPath);
	resp->lastModified	= getLastModified(resp->fullPath);

    if (strncmp(resp->contentType.c_str(), "image", 5) == 0) {
        resp->isBinary = true;
    }
    
    
}

bool HTTPResponser::isValidPath(const char* fullPath, const char* rootPath)
{
    if (!fullPath || !rootPath) {
        return false;
    }

    // std::string fullPathStr(fullPath);
    // std::string rootPathStr(rootPath);

    // return fullPathStr.find(rootPathStr) == 0;

    return strncmp(fullPath, rootPath, strlen(rootPath)) == 0;
}

std::string HTTPResponser::getContentType(const char* fullPath)
{
    if (!fullPath) {
        return "";
    }

	std::string fullPathStr = std::string(fullPath);
    size_t idx = fullPathStr.find_last_of(".");
    std::string suffix = fullPathStr.substr(idx + 1);

    std::string type;
    if (suffix == "jpg" || suffix == "jpeg") {
    	type = "image/jpeg";
    } else if (suffix == "png") {
    	type = "image/png";
    } else if (suffix == "html" || suffix == "htm") {
    	type = "text/html";
    } else {
    	// Fill more file types supported
    }

    return type;
}

std::string HTTPResponser::getContentLength(const char* fullPath)
{
    if (!fullPath) {
        return "";
    }

    struct stat sb;
    if (stat(fullPath, &sb) == -1) {
       perror("stat");
       exit(EXIT_FAILURE);
    }

    char lengthBuf[10];
    sprintf(lengthBuf, "%ld", (unsigned long)sb.st_size);
    std::string length(lengthBuf);

    // std::string length = std::to_string(sb.st_size);

    // FILE *fp = fopen(fullPath, "rb");
    // fseek(fp, 0, SEEK_END);
    // int lengthOfFile = ftell(fp);
    // std::string length = std::to_string(lengthOfFile);

    return length;
}

std::string HTTPResponser::getLastModified(const char* fullPath)
{
    if (!fullPath) {
        return "";
    }

	struct stat sb;
	if (stat(fullPath, &sb) == -1) {
       perror("stat");
       exit(EXIT_FAILURE);
    }

    std::string tmp = asctime(gmtime(&sb.st_mtime));
    // tmp.pop_back();
    tmp.erase(tmp.size() - 1);
    std::vector<std::string> strs;
    size_t messageSize = HTTPParser::split(tmp, strs, " ");
    if (messageSize != 5) {
        return "";
    }
    std::string time = strs[0]+", "+strs[2]+" "+strs[1]+" "+strs[4]+" "+strs[3]+" "+"GMT";

    return time;
}

// Generate the response message
std::string HTTPResponser::getHeaderMsg(HTTPHeader* resp) {

    std::string initailLine;
    initailLine.append(resp->httpVersion);
    initailLine.append(resp->statusCode);
    initailLine.append("\r\n");

    std::string headerPairs;
    headerPairs.append("Server: ");
    headerPairs.append(resp->server);
    headerPairs.append("\r\n");

    if (resp->statusCode == "200 OK") {
        headerPairs.append("Last-Modified: ");
        headerPairs.append(resp->lastModified);
        headerPairs.append("\r\n");

        headerPairs.append("Content-Type: ");
        headerPairs.append(resp->contentType);
        headerPairs.append("\r\n");

        headerPairs.append("Content-Length: ");
        headerPairs.append(resp->contentLength);
        headerPairs.append("\r\n");
    }
    headerPairs.append("\r\n");
    
    std::string responseStr = initailLine + headerPairs;
    return responseStr;
}

void HTTPResponser::sendBodyMsg(HTTPHeader* resp, int clntSocket) {
    char bodyBuffer[BUFSIZE]; // Buffer for incoming request
    memset(bodyBuffer, 0, BUFSIZE);

    int contentSize = std::atoi(resp->contentLength.c_str());

    FILE * fp;
    if (resp->isBinary) {
        if ((fp = fopen(resp->fullPath, "rb")) == NULL) {
            perror ("Error opening file");
        }
    } else {
        if ((fp = fopen(resp->fullPath, "r")) == NULL) {
            perror ("Error opening file");
        }
    }

    if (resp->isBinary) {
        int cnt = 0;
        int sendLength = BUFSIZE;
        while(!feof(fp)) {
            cnt += BUFSIZE;
            if (cnt > contentSize) {
                sendLength = contentSize % BUFSIZE;
            }
            fread(bodyBuffer, sizeof(char), BUFSIZE, fp);

            ssize_t numBytesSent;
            numBytesSent = send(clntSocket, bodyBuffer, sendLength, 0);
            if (numBytesSent < 0) 
                DieWithError("send() failed");
            memset(bodyBuffer, 0, BUFSIZE);
        }
    } else {
        while (fgets(bodyBuffer, BUFSIZE, fp) != NULL) {
            // strcat(content, buffer);
            ssize_t numBytesSent;
            numBytesSent = send(clntSocket, bodyBuffer, strlen(bodyBuffer), 0);
            if (numBytesSent < 0) {
                DieWithError("send() failed");
            }
            memset(bodyBuffer, 0, BUFSIZE);
        }
    }
    fclose (fp);
}

void HTTPResponser::show(HTTPHeader* resp) {
    std::string tmp;

    tmp.append("statusCode: ");
    tmp.append(resp->statusCode);
    tmp.append("\r\n");

    tmp.append("lastModified: ");
    tmp.append(resp->lastModified);
    tmp.append("\r\n");

    tmp.append("contentType: ");
    tmp.append(resp->contentType);
    tmp.append("\r\n");

    tmp.append("contentLength: ");
    tmp.append(resp->contentLength);
    tmp.append("\r\n");

    tmp.append("isBinary: ");
    tmp.append(resp->isBinary ? "true" : "false");
    tmp.append("\r\n");

    printf("Show response header:\n%s\n", tmp.c_str());
}