#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <vector>
#include <stdint.h>
class HttpRequest
{
public:
	HttpRequest();
	HttpRequest(char* URI);
	bool setUrl(std::string url);
	bool setMethod(std::string method);
	std::vector<uint8_t> endcode();
private:
	std::string Scheme;
	std::string HTTPVersion;
	int port;
	
};

#endif