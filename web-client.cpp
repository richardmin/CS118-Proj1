#include "HttpRequest.h"
#include "HttpResponse.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>


#include <boost/network/uri.hpp>


char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP

int main(int argc, char* argv[])
{
  int portnum = 80;
  char* domain;

  //==================READ ARGUMENTS================
  if(argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " <URL>" << std::endl;
    exit(1);
  }

  std::string unparsedURL = std::string(argv[1]);
  std::cout << unparsedURL << std::endl;

  int schemaEnd = unparsedURL.find("://");
  if(schemaEnd == -1)
  {
    std::cout << "You must specify a schema" << std::endl;
    exit(1);
  }
  std::string schema = unparsedURL.substr(0, schemaEnd);
  if(schema != "http://")
  {
    std::cout << "You must use precisely \"http://\" schema." << std::endl;
    exit(1);
  }

  int 


  exit(1);



  // HttpRequest h = new HttpRequest(argv[1]);


  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htonl(portnum);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // send/receive data to/from connection
  bool isEnd = false;
  std::string input;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    std::cout << "send: ";
    std::cin >> input;
    if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ss << buf << std::endl;
    std::cout << "echo: ";
    std::cout << buf << std::endl;

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(sockfd);

  return 0;
}

char* stringToCString(std::string s)
{
  const char* s_cstr = s.data(); //get a const char* version
  char* s_cpy = (char *)malloc(sizeof(char) * (strlen(s_cstr)+1));
  if(s_cpy == NULL)
  {
    std::cout << "Malloc Failed" << std::endl;
    exit(1);
  }

  for(uint i = 0; i <= strlen(s_cstr); i++)
  {
    s_cpy[i] = s_cstr[i];
  }

  return s_cpy;

}
void resolveIP(std::string& hostname)
{
  char* hostname_cstr = stringToCString(hostname);


  struct addrinfo hints;
  struct addrinfo* res;

  // prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // get address
  int status = 0;
  if ((status = getaddrinfo(hostname_cstr, "80", &hints, &res)) != 0) {
    std::cerr << "couldn't resolve IP address: " << gai_strerror(status) << std::endl;
    exit(1);
  }

    // convert address to IPv4 address
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;

    // convert the IP to a string and print it:
  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(res->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
  hostname = std::string(ipstr);

  freeaddrinfo(res); // free the linked list
  free(hostname_cstr);
}