#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <vector>

class HttpRequest
{
public:
	HttpRequest();
	bool setUrl(std::string url);
	bool setMethod(std::string method);
	std::vector<uint8_t> endcode();
private:
};

#endif