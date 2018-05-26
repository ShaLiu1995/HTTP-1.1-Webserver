#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "HTTPParser.hpp"

void HTTPParser::parse(std::string message, HTTPInstruction* inst) 
{
	if (message.length() == 0) {
		return;
	}

	std::vector<std::string> lines;
	split(message, lines, "\r\n");

	for (std::string line : lines) {
		// Handle the initial line
		if (!inst->hasInitialLine) {
			// Split the line
			std::vector<std::string> strs;
			size_t messageSize = split(line, strs, " ");

			// Parse the line
			if (messageSize == 3) {
				inst->method = strs[0];
				inst->url = strs[1];
				inst->httpVersion = strs[2];
			} else {
				inst->isMalformed = true;
			}

			// Mark there is an initial line
			inst->hasInitialLine = true;
		} 
		// Handle the key-value pairs in header
		else {
			// Split by the first colon
			size_t idx = line.find_first_of(":");
			if (idx == std::string::npos) {
				inst->isMalformed = true;
				return;
			}

			std::string fieldName = line.substr(0, idx);
			std::string fieldValue = line.substr(idx + 1);

			fieldName = trim(fieldName, " ");
			fieldValue = trim(fieldValue,  " ");

		
			if (fieldName == "Host") {
				inst->host = fieldValue;
			} else if (fieldName == "User-Agent") {
				inst->userAgent = fieldValue;
			} else if (fieldName == "Connection") { 
				inst->connection = fieldValue;
			} else {
				// fill another key-value paris if necessary
			}
		}
	}

	if (inst->connection != "close") {
		resetInst(inst);
	}	
}

std::string& HTTPParser::trimRight(std::string& txt, const std::string& delimiters)
{
	return txt.erase(txt.find_last_not_of(delimiters) + 1);
}

std::string& HTTPParser::trimLeft(std::string& txt, const std::string& delimiters)
{
	return txt.erase(0, txt.find_first_not_of(delimiters));
}

std::string& HTTPParser::trim(std::string& txt, const std::string& delimiters)
{
	return trimLeft(trimRight(txt, delimiters), delimiters);
}

size_t HTTPParser::split(std::string& txt, std::vector<std::string>& strs, const std::string& ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while(pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

void HTTPParser::resetInst(HTTPInstruction* inst) {
	inst->method 			= "";
	inst->url 				= "";
	inst->httpVersion 		= "";
	inst->host 				= "";
	inst->userAgent 		= "";
	inst->connection 		= "";
	inst->isMalformed 		= false;
	inst->hasInitialLine	= false;
}

void HTTPParser::show(HTTPInstruction* inst) {
    std::string tmp;
    
    tmp.append("Method: ");
    tmp.append(inst->method);
    tmp.append("\r\n");

    tmp.append("URL: ");
    tmp.append(inst->url);
    tmp.append("\r\n");

    tmp.append("HTTP Version: ");
    tmp.append(inst->httpVersion);
    tmp.append("\r\n");

    tmp.append("Host: ");
    tmp.append(inst->host);
    tmp.append("\r\n");

    tmp.append("User-Agent: ");
    tmp.append(inst->userAgent);
    tmp.append("\r\n");

    tmp.append("Connection: ");
    tmp.append(inst->connection);
    tmp.append("\r\n");

    tmp.append("isMalformed: ");
    tmp.append(inst->isMalformed ? "true" : "false");
    tmp.append("\r\n");

    tmp.append("hasInitialLine: ");
    tmp.append(inst->hasInitialLine ? "true" : "false");
    tmp.append("\r\n");

    printf("Show request:\n%s\n", tmp.c_str());
}
