
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h> //fstat for file size

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <signal.h>
#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
std::string file_dir = ".";

char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP
void handle_one_connection(struct sockaddr_in clientAddr, int clientSockfd);
std::vector<std::string> split_by_carriage_return(std::string input, std::string& statusCode);
//std::vector<std::string> split_by_double_carriage_return(std::string input);


int main(int argc, char* argv[])
{
  std::string hostname = "localhost";
  uint portnum = 4000;


  //==============PARSE COMMAND LINE ARGUMENTS================
  //wrong number of arguments input
  if(argc > 4)
  {
    std::cerr << "Usage: " << argv[0] << " [hostname] [port] [file-dir]" << std::endl;
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
      std::cerr << "<port> must be a integer!" << std::endl;
      exit(1);
    }
  }
  if(argc > 1)
  {
    hostname = std::string(argv[1]);
  }

  resolveIP(hostname); //resolves the IP passed in into the first IP address possibly calculated

  // verify that the hostnames are checked properly
  // std::cerr << "hostname: " << hostname << " port num: " << portnum << " file_dir: " << file_dir << std::endl;

  char* hostname_cstr = stringToCString(hostname);
  
  signal(SIGPIPE, SIG_IGN); //we don't want SIGPIPEs, because we'll handle that manually ourselves in the threads. If the pipe closes, we simply close the socket.
  //==============SOCKET CREATION FOR CONNECTIONS===================
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  //set timeout
  
    struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;  


  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portnum); 
  addr.sin_addr.s_addr = inet_addr(hostname_cstr);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  free(hostname_cstr);

  //bind the socket
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }


  //Start and continue accepting connections
  std::vector<std::thread> thread_vec;
  while (1) {
    // set socket to listen status
    if (listen(sockfd, 1) == -1) {
      perror("listen");
      return 3;
    }

    // accept a new connection, each new one gets its own socket
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSockfd == -1) {
      perror("accept");
      return 4;
    }



    if (setsockopt(clientSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
      perror("setsockopt");
      return 1;
    }

    if (setsockopt(clientSockfd, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
      perror("setsockopt");
      return 1;
      }


    //Have each new connection get its own thread to resolve request
    std::thread t(handle_one_connection, clientAddr, clientSockfd);
    thread_vec.push_back(move(t));
  }

 // int n_threads = thread_vec.size();
 // for (int i = 0; i < n_threads; i++)
//    thread_vec[i].join();
  return 0;
}

void handle_one_connection(struct sockaddr_in clientAddr, int clientSockfd) {


    std::string statusCode = "200"; //Default success status code
    //HANDLING A NEW CONNECTION
    //TODO: SPAWN THREADS FOR CODE FOLLOWING THIS LINE IN THE FUTURE, have main thread continue to accept connections
    char ipstr[INET_ADDRSTRLEN] = { '\0' };
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cerr << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;

    // read/write data from/into the connection
    
    char buf[1] = { 0 };
    std::stringstream ss;

    while(1)
    {
    //Keep collecting data until we reach \r\n\r\n
      std::string RequestString;
      std::string ReplyString;
      std::string receivedData;
      std::string messageBody;
    
      
      int rn_found = 0;
      bool r_found = false;
      ss << messageBody;
      messageBody = "";      
      int messageBodyLength = 0;

      while (1) {
        memset(buf, '\0', sizeof(buf));


        ssize_t x;
        if ((x = recv(clientSockfd, buf, 1, 0)) == -1) {
          perror("recv");
          return;
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
          receivedData += buf[i];
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

      RequestString = receivedData;


    //Parse the first line of the HTTP Request Message
    //Should be of format GET /path HTTP/1.1
      std::vector<std::string> RequestVector = split_by_carriage_return(RequestString, statusCode);
      if (RequestVector.size() == 0)
      {
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1) 
          perror("send");
        return;
      }
      std::string headerLine = RequestVector[0];
      if (headerLine == "") {
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1) 
          perror("send");
        return;

      }
      std::string method, path, protocol;
      if (true) {
        boost::char_separator<char> sep(" ");
        boost::tokenizer<boost::char_separator<char>> tokens(headerLine, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator it = tokens.begin();

      if (std::distance(tokens.begin(), tokens.end()) == 0) { //The tokenized headerLine MUST have a length (ie. " " headerline invalid)
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1)
          perror("send");
        return;
      }

      //TODO: Give an error code instead of error message!!!!!!!???!?!?!???????
      if (it != tokens.end() && *it != "GET") {
        std::cerr << "Sorry, non-GET methods are not supported. You requested: " << *it << std::endl;
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1)
          perror("send");
        return;
      }
      method = *it;
      ++it;
      if (it != tokens.end() && (*it).substr(0, 1) != "/") {
        std::cerr << "Invalid path name given: " << *it << std::endl;
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1)
          perror("send");
        return;
      }
      path = *it;
      ++it;

      if (it != tokens.end() && *it != "HTTP/1.1" && *it != "HTTP/1.0") {
        std::cerr << "Sorry, non-HTTP/1.1 isn't currently supported. You specified: " << *it << std::endl;
        if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1)
          perror("send");
        return;
      }
      protocol = *it;
    }

    bool HOST_FOUND = false;
    std::string host = "";
    boost::char_separator<char> sep(" ");
    for(uint i = 1; i < RequestVector.size(); i++)
    {
      std::string headerLine = RequestVector[i];
      boost::tokenizer<boost::char_separator<char>> tokens(headerLine, sep);
      boost::tokenizer<boost::char_separator<char>>::iterator it = tokens.begin();

      if(it != tokens.end() && boost::iequals(*it, "Host:"))
      {
        ++it;
        HOST_FOUND = true;
        if(it != tokens.end())
        {
          host = *it;
        }
      }
    }

    if(!HOST_FOUND)
    {
      std::cerr << "Sorry, HOST header is mandatory." << std::endl;
      if (send(clientSockfd, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"), 0) == -1)
      {
        perror("send");
      }
      return;
    }


    //Get the file requested by the request path
    FILE *fp;
    std::string myfile;
    std::string relative_path = file_dir + path;
    int contentLength = 0;
    const char* file_path_cstr = relative_path.data();
    fp = fopen(file_path_cstr, "r");
    if (fp == NULL) {
      perror("open");
      if (statusCode == "200")  //Always return the FIRST error code even if client's request has multiple errors
        statusCode = "404";
      std::cerr << "Sorry, Couldnt find file with path: " << file_path_cstr << std::endl;
    }
    else {
      int fd = fileno(fp);
      struct stat fileStats;
      fstat(fd, &fileStats);
      if (S_ISREG(fileStats.st_mode))   //Check that the file is a REGular file ie. '/' will be considered invalid and return 404
        contentLength = fileStats.st_size;
      else {
        if (statusCode == "200")
          statusCode = "404";
        fclose(fp);
        fp = NULL;
      }
    }



    //TODO: Implement these variables!!!!!????????????!?!?!
    ReplyString.append(protocol);
    ReplyString.append(" ");
    ReplyString.append(statusCode);
    if (statusCode == "200")
      ReplyString.append(" OK ");
    else if (statusCode == "400")
      ReplyString.append(" Bad Request ");
    else if (statusCode == "404")
      ReplyString.append(" Not Found ");
    else
      std::cerr << "Bad status code.. how did you get here.." << std::endl;
    ReplyString.append("\r\n");
    ReplyString.append("Content-Length: ");
    std::string contentLength_str = std::to_string(contentLength);
    ReplyString.append(contentLength_str);
    ReplyString.append("\r\n\r\n");
    
    if (send(clientSockfd, ReplyString.c_str(), ReplyString.size(), 0) == -1) {
      perror("send");
      exit(-1);
    }
    else
    {
      std::cout << ReplyString << std::endl;
    }

    if (fp != NULL) {
      int ch;
      while ((ch = fgetc(fp)) != EOF) {
        // std::cout << "why hello there" << std::endl;
        if (send(clientSockfd, &ch, 1, 0) == -1) {
          perror("send");
          return;
        }
      }
    }


    if (fp != NULL)
      fclose(fp);
  }
  close(clientSockfd);
  
}


char* stringToCString(std::string s)
{
  const char* s_cstr = s.data(); //get a const char* version
  char* s_cpy = (char *)malloc(sizeof(char) * (strlen(s_cstr) + 1));
  if (s_cpy == NULL)
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


std::vector<std::string> split_by_carriage_return(std::string input, std::string& statusCode) {
  std::vector<std::string> str_vector;
  std::size_t index, n_lines = 0;
  std::string line;
  while (1) {
    index = input.find("\r\n");
    //There were NO \r\n's at all
    if (n_lines == 0 && index == input.size()) {
      statusCode = "400";
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