#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "httpd.h"



void usage(char * argv0)
{
	std::cerr << "Usage: " << argv0 << " listen_port docroot_dir" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}

	long int port = strtol(argv[1], NULL, 10);

	if (errno == EINVAL || errno == ERANGE) {
		usage(argv[0]);
		return 2;
	}

	if (port <= 0 || port > USHRT_MAX) {
		std::cerr << "Invalid port: " << port << std::endl;
		return 3;
	}

	std::string doc_root = argv[2];

	start_httpd(port, doc_root);

	return 0;
}



