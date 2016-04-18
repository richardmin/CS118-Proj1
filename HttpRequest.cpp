#include "HttpRequest.h"
#include <iostream>


HttpRequest::HttpRequest()
{
	Scheme = "HTTP";
	HTTPVersion = "1.0";
}

HttpRequest::HttpRequest(char* URI)
{

}

bool HttpRequest::setUrl(std::string url)
{
	return false;
}

bool HttpRequest::setMethod(std::string method)
{
	//disclaimer: you may wish to change the method parameters
	return false;
}

std::vector<uint8_t> HttpRequest::endcode()
{
	std::vector<uint8_t> temp;
	return temp;
}