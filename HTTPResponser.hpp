#ifndef HTTPRESPONSER_HPP
#define HTTPRESPONSER_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include "HTTPParser.hpp"

// using namespace std;

typedef struct HTTPHeader_t {
    char* rootPath;
    char* fullPath;
    std::string httpVersion;
    std::string statusCode;
    std::string server;
    std::string lastModified;
    std::string contentType;
    std::string contentLength;
    bool isBinary;
    //std::string bodyText;

} HTTPHeader;

class HTTPResponser {
public:
	
	static void getHTTPHeader(HTTPInstruction* inst, HTTPHeader* resp, std::string doc_root);

    static bool isValidPath(const char* fullPath, const char* rootPath);

	static std::string getContentType(const char* fullPath);

	static std::string getContentLength(const char* fullPath);

	static std::string getLastModified(const char* fullPath);

    static std::string getHeaderMsg(HTTPHeader* resp);

    static void sendBodyMsg(HTTPHeader* resp, int clntSocket);

    static void show(HTTPHeader* resp);
    

};

#endif // HTTPRESPONSER_HPP
