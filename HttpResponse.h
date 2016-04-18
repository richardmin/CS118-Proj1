#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <vector>

class HttpResponse
{
public:
	HttpResponse();
	bool setUrl(std::string url);
	bool setMethod(std::string method);
	std::vector<uint8_t> endcode();
private:
};

#endif