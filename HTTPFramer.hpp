#ifndef HTTPFRAMER_HPP
#define HTTPFRAMER_HPP

#include <iostream>

class HTTPFramer {
public:
	void append(std::string chars);

	// Does this buffer contain at least one complete message?
	bool hasMessage() const;

	// Returns the first instruction
	std::string topMessage() const;

	// Removes the first instruction
	void popMessage();

	// prints the string to an ostream (useful for debugging)
	void printToStream(std::ostream& stream) const;

protected:
	// PUT ANY FIELDS YOU NEED HERE
	std::string buffer;
	// Static Constant Delimiter
	static const std::string delim;
};

#endif // HTTPFRAMER_HPP
