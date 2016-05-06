#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
#include <vector>
#include <fstream>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP
std::vector<std::string> split_by_carriage_return(std::string input);

int main(int argc, char* argv[])
{
  int portnum = 80;
  std::string protocol, domain, port, path, query, fragment, requestString, fileName;

  //==================READ ARGUMENTS================
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <URL> [url] [url] ... " << std::endl;
    exit(1);
  }

  for(int z = 1; z < argc; z++)
  {
    
    std::string url = std::string(argv[z]);
    // Regex from http://stackoverflow.com/a/27372789 and http://tools.ietf.org/html/rfc3986#appendix-B
    boost::regex ex("([^:/?#]+)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch url_matches;

    if(regex_match(url.c_str(), url_matches, ex)) 
    {
        protocol = std::string(url_matches[1].first, url_matches[1].second);
        domain = std::string(url_matches[2].first, url_matches[2].second);
        port = std::string(url_matches[3].first, url_matches[3].second);
        path = std::string(url_matches[4].first, url_matches[4].second);
        fileName = path.substr(path.find_last_of("/")+1);
        if(fileName == "")
        {
          fileName = path.substr(path.substr(0, path.find_last_of("/")).find_last_of("/")+1);
          fileName = fileName.substr(0, fileName.size()-1);
        }
        if(fileName == "")
        {
          fileName = "index.html";
        }
        query = std::string(url_matches[5].first, url_matches[5].second);
        fragment = std::string(url_matches[6].first, url_matches[6].second);
    }
    else
    {
      std::cerr << "Invalid URL! Please carefully check your spelling. Note that a schema must be provided." << std::endl;
      exit(1);
    }


    if(protocol.compare("http"))
    {
      std::cerr << "Sorry, non-http isn't currently supported. You specified: " << protocol << std::endl;
      exit(1);
    }

    if(port.length() != 0) //port specified
    {
      std::stringstream convert(port);
      if(!(convert >> portnum))
      {
        std::cerr << "<port> must be a integer!" << std::endl;
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
    requestString.append(" HTTP/1.1\r\n\r\n");  
  }
    // std::cerr << requestString << std::endl;
    // exit(5);
    //------CONNECT TO THE SERVER --------------//
    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portnum);     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr(domain_cstr);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    // std::cerr << "domain: " << domain_cstr << ":" << portnum << std::endl;
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
    // std::cerr << "Set up a connection from: " << ipstr << ":" <<
      // ntohs(clientAddr.sin_port) << std::endl;






    // ---------- SEND DATA TO THE SERVER --------- //
    // send/receive data to/from connection

      // std::cerr << requestString << std::endl;
      if (send(sockfd, requestString.c_str(), requestString.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    //------------- RECEIVE SERVER RESPONSE ------------- //
    std::string persistentData;
    for(int i = 1; i < argc; i++)
    {
      int rn_found = 0;
      bool r_found = false;
      char buf[20] = {0};
      std::string unparsedHeaders;
      char messageBody[20] = {0};
      int messageBodyLength = 0;
      std::stringstream ss;
      while (1) {
        memset(buf, '\0', sizeof(buf));
        memset(messageBody, '\0', sizeof(messageBody));

        ssize_t x;
        if ((x = recv(sockfd, buf, 20, 0)) == -1) {
          perror("recv");
          return 5;
        }
        ss << buf;

        uint i;
        for(i = 0; i < x; i++)
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
          unparsedHeaders += buf[i];
        }
        i++;
        for(; i < x; i++)
        {
          messageBody[messageBodyLength] = buf[i];
          messageBodyLength++;
        }
        if(rn_found >= 2)
          break;
        ss.str("");
      }

      std::cerr << unparsedHeaders << std::endl;


      //Trying to do some parsing.. :/
      // std::cerr << RequestString << std::endl;
      std::vector<std::string> RequestVector = split_by_carriage_return(unparsedHeaders);
      boost::char_separator<char> sep(" ");

      {// lazy solution to make the variables non permanent
        std::string headerLine = RequestVector[0];
        
        boost::tokenizer<boost::char_separator<char>> tokens(headerLine, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator it = tokens.begin();
        if (it == tokens.end() || (*it != "HTTP/1.0" && *it != "HTTP/1.1")) {
          std::cerr << "Server responded unexpectedly! (Not a HTTP 1.1 or 1.0 response)" << std::endl;
          exit(1);
        }
        ++it;

        if (it == tokens.end() || *it != "200") {
          std::cerr << "Server error code: " << *it << std::endl;
          exit(1);
        }

        ++it;

        if (it == tokens.end() || *it != "OK") {
          std::cerr << "Status not OK!" << std::endl;
          exit(1);
        }
      }


      int content_length = -1;
      for(uint i = 1; i < RequestVector.size(); i++)
      {
        std::string headerLine = RequestVector[i];
        boost::tokenizer<boost::char_separator<char>> tokens(headerLine, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator it = tokens.begin();

        if(it != tokens.end() && boost::iequals(*it, "content-length:"))
        {
          ++it;
          if(it != tokens.end())
          {
            content_length = std::stoi(*it);
          }
        }
      }

      // -------- PREPARE THE FILE STREAM TO OUTPUT TO ------------ //
      std::string parsedfileName = fileName;

      struct stat st;
      int st_result;

      int j = 1;
      while((st_result = stat(parsedfileName.c_str(), &st)) == 0)
      {
        parsedfileName = fileName;
        parsedfileName += " (";
        parsedfileName += std::to_string(j);
        parsedfileName += ")";
        j++;
      }

      std::ofstream of(parsedfileName);
        of.write(messageBody, messageBodyLength);  


      ss.str("");
      
      int i = 0;
      ssize_t x;
      char buf2[1] = {0};
      for(i = 0; i < content_length; i+=x) //if content-length is defined, this will only get characters to the content-length. Otherwise, it functions as a while loop until no more bytes can be read.
      {
        memset(buf2, '\0', sizeof(buf2));
        
        if ((x = recv(sockfd, buf2, 1, 0)) == -1) {
          perror("recv");
          return 5;
        }
        if(x == 0) //no more bytes to read. Persistent connections are NOT supported.
          break;

        of.write(buf2, x);
      }

      of.close();
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
    std::cerr << "Malloc Failed" << std::endl;
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



std::vector<std::string> split_by_carriage_return(std::string input) {
  std::vector<std::string> str_vector;
  std::size_t index, n_lines = 0;
  std::string line;
  while (1) {
    index = input.find("\r\n");
    //There were NO \r\n's at all
    if (n_lines == 0 && index == input.size()) {
      break;
    }
    //There are no more \r\n's
    if (index == input.size())
      break;

    line = input.substr(0, index);
    //If we have consecutive carriage returns, we reached the end of one header section
    if (line.size() <= 0) {
      break;
    }
    str_vector.push_back(line);
    n_lines++;
    input = input.substr(index + 2);
  }
  return str_vector;
}