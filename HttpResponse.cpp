#include "HttpResponse.h"
#include <iostream>

HttpResponse::HttpResponse()
{
	
}

bool HttpResponse::setUrl(std::string url)
{
	return false;
}

bool HttpResponse::setMethod(std::string method)
{
	//disclaimer: you may wish to change the method parameters
	return false;
}

std::vector<uint8_t> HttpResponse::endcode()
{
	std::vector<uint8_t> temp;
	return temp;
}