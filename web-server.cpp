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

char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP

int main(int argc, char* argv[])
{
  std::string hostname = "localhost";
  uint portnum = 4000;
  std::string file_dir = ".";


  //==============PARSE COMMAND LINE ARGUMENTS================
	//wrong number of arguments input
	if(argc > 4)
	{
		std::cout << "Usage: " << argv[0] << " [hostname] [port] [file-dir]" << std::endl;
		exit(1);
	}
  if(argc > 3) //all optional arguments present
  {
    file_dir = argv[3];
  }
  if(argc > 2)
  {
    std::stringstream convert(argv[2]);
    if(!(convert >> portnum)) //note that we don't check for overflow
    {
      std::cout << "<port> must be a integer!" << std::endl;
      exit(1);
    }
  }
  if(argc > 1)
  {
    hostname = std::string(argv[1]);
  }

  resolveIP(hostname); //resolves the IP passed in into the first IP address possibly calculated

  // verify that the hostnames are checked properly
  // std::cout << "hostname: " << hostname << " port num: " << portnum << " file_dir: " << file_dir << std::endl;

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portnum); 
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  exit(1);

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;

  // read/write data from/into the connection
  bool isEnd = false;
  char buf[20] = {0};
  std::stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (recv(clientSockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }

    ss << buf << std::endl;
    std::cout << buf << std::endl;


    if (send(clientSockfd, buf, 20, 0) == -1) {
      perror("send");
      return 6;
    }

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(clientSockfd);

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