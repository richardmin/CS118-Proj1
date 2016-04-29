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
#include <regex>

#include <boost/regex.hpp>

char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP


int main(int argc, char* argv[])
{
  int portnum = 80;
  std::string protocol, domain, port, path, query, fragment, requestString;

  //==================READ ARGUMENTS================
  if(argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " <URL>" << std::endl;
    exit(1);
  }
  
  std::string url = std::string(argv[1]);
  // Regex from http://stackoverflow.com/a/27372789 and http://tools.ietf.org/html/rfc3986#appendix-B
  boost::regex ex("([^:/?#]+)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
  boost::cmatch url_matches;

  if(regex_match(url.c_str(), url_matches, ex)) 
  {
      protocol = std::string(url_matches[1].first, url_matches[1].second);
      domain = std::string(url_matches[2].first, url_matches[2].second);
      port = std::string(url_matches[3].first, url_matches[3].second);
      path = std::string(url_matches[4].first, url_matches[4].second);
      query = std::string(url_matches[5].first, url_matches[5].second);
      fragment = std::string(url_matches[6].first, url_matches[6].second);
  }
  else
  {
    std::cout << "Invalid URL! Please carefully check your spelling. Note that a schema must be provided." << std::endl;
    exit(1);
  }


  if(protocol.compare("http"))
  {
    std::cout << "Sorry, non-http isn't currently supported. You specified: " << protocol << std::endl;
    exit(1);
  }

  if(port.length() != 0) //port specified
  {
    std::stringstream convert(port);
    if(!(convert >> portnum))
    {
      std::cout << "<port> must be a integer!" << std::endl;
      exit(1);
    }
  }

  resolveIP(domain);
  char* domain_cstr = stringToCString(domain);

  //----------FORMAT REQUEST STRING ------------------//
  requestString = std::string("GET ");
  if(path.length() != 0)
    requestString.append(path);
  else
    requestString.append("/");

  if(query.length() != 0)
  {
    requestString.append("?q=");
    requestString.append(query);
  }
  if(fragment.length() != 0)
  {
    requestString.append("#");
    requestString.append(fragment);
  }

  requestString.append(" HTTP/1.0\r\n\r\n");  
  // std::cout << requestString << std::endl;
  // exit(5);
  //------CONNECT TO THE SERVER --------------//
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portnum);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr(domain_cstr);
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
  std::cout << "domain: " << domain_cstr << ":" << portnum << std::endl;
  free(domain_cstr);


  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  //get client info; mostly just need the port number that was assigned by kernel
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






  // ---------- SEND DATA TO THE SERVER --------- //
  // send/receive data to/from connection

    if (send(sockfd, requestString.c_str(), requestString.size(), 0) == -1) {
    perror("send");
    return 4;
  }


  //------------- RECEIVE SERVER RESPONSE ------------- //
  char buf[20] = {0};
  std::stringstream ss;
  std::string receivedData;

  int rn_found = 0;
  bool r_found = false;
  bool header_split_found = false;
  while (1) {
    memset(buf, '\0', sizeof(buf));

    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ss << buf;

    for(uint i = 0; i < strlen(buf); i++)
    {
      if(buf[i] == '\r')
      {
        if(r_found)
          rn_found = 0;
        
        r_found = true;

      }
      else if(buf[i] == '\n')
      {

        if(r_found)
          rn_found++;
        else
          rn_found = 0;

        r_found = false;
      }
      else
      {
        r_found = false;
        rn_found = 0;
      }

      if(rn_found >= 2)
        break;

    }
    receivedData.append(ss.str());
    if(rn_found >= 2 && header_split_found)
      break;
    else if(rn_found >= 2)
    {
      header_split_found = true;
      rn_found = 0;
    }
    

    
    ss.str("");
  }

  std::cout << receivedData << std::endl;
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

