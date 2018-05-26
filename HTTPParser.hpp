#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <stdint.h>
#include <string>
#include <vector>

// using namespace std;

typedef struct HTTPInstruction_t {
	// DEFINE YOUR DATA STRUCTURE HERE
	std::string method;
	std::string url;
	std::string httpVersion;
	std::string host;
	std::string userAgent;
	std::string connection;
	bool isMalformed;
	bool hasInitialLine;

} HTTPInstruction;

class HTTPParser {
public:
	// Parse the message
	static void parse(std::string message, HTTPInstruction* inst);

	// Trim trailing whitespace characters
	static std::string& trimRight(std::string& txt, const std::string& delimiters);

	// Trim heading whitespace characters
	static std::string& trimLeft(std::string& txt, const std::string& delimiters);

	// Trim on the both sides
	static std::string& trim(std::string& txt, const std::string& delimiters);

	// Split message by the delimiter ch
	static size_t split(std::string& txt, std::vector<std::string>& strs, const std::string& ch);

	static void resetInst(HTTPInstruction* inst);

	static void show(HTTPInstruction* inst);
};

#endif // HTTPPARSER_HPP
