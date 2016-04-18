#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
#include <iostream>

void resolveIP(std::string& hostname); //note this only gets the first IP

int main(int argc, char *argv[])
{
  std::string hostname = "google.com";
  if(argc == 2)
  	hostname = std::string(argv[1]);
  
  std::cout << "before parsing: " << hostname  << std::endl;
  resolveIP(hostname);
  std::cout << "after parsing: " << hostname  << std::endl;
}



void resolveIP(std::string& hostname)
{
  const char* hostname_cstr = hostname.data(); //get a const char* version
  char* hostname_cpy = (char *)malloc(sizeof(char) * (strlen(hostname_cstr)+1));
  for(int i = 0; i <= strlen(hostname_cstr); i++)
  {
  	hostname_cpy[i] = hostname_cstr[i];
  }

  struct addrinfo hints;
  struct addrinfo* res;

  // prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // get address
  int status = 0;
  if ((status = getaddrinfo(hostname_cpy, "80", &hints, &res)) != 0) {
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
}