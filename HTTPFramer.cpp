#include <assert.h>
#include <string>
#include "HTTPFramer.hpp"

const std::string HTTPFramer::delim = "\r\n\r\n";

void HTTPFramer::append(std::string chars)
{
	// PUT YOUR CODE HERE
	buffer.append(chars);
}

bool HTTPFramer::hasMessage() const
{
	// PUT YOUR CODE HERE
	return !buffer.empty();
}

std::string HTTPFramer::topMessage() const
{
	// PUT YOUR CODE HERE
	size_t idx = buffer.find(delim);

	if (idx == std::string::npos) {
        return "";
	}

	return buffer.substr(0, idx);
}

void HTTPFramer::popMessage()
{
	// PUT YOUR CODE HERE
	size_t idx = buffer.find(delim);

	if (idx == std::string::npos) {
        return;
	}

	buffer.erase(0, idx + delim.length());
}

void HTTPFramer::printToStream(std::ostream& stream) const
{
	// (OPTIONAL) PUT YOUR CODE HERE--useful for debugging
	stream << buffer << std::endl;
}


