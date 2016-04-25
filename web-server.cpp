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
#include <signal.h>
#include <vector>

char* stringToCString(std::string s);
void resolveIP(std::string& hostname); //note this only gets the first IP
void handle_one_connection(struct sockaddr_in clientAddr, int clientSockfd);

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

	  //Have each new connection get its own thread to resolve request
	  std::thread t(handle_one_connection, clientAddr, clientSockfd);
	  thread_vec.push_back(move(t));
  }

 // int n_threads = thread_vec.size();
 // for (int i = 0; i < n_threads; i++)
//	  thread_vec[i].join();
  return 0;
}

void handle_one_connection(struct sockaddr_in clientAddr, int clientSockfd) {
	//HANDLING A NEW CONNECTION
	//TODO: SPAWN THREADS FOR CODE FOLLOWING THIS LINE IN THE FUTURE, have main thread continue to accept connections
	char ipstr[INET_ADDRSTRLEN] = { '\0' };
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Accept a connection from: " << ipstr << ":" <<
		ntohs(clientAddr.sin_port) << std::endl;

	// read/write data from/into the connection
	bool isEnd = false;
	char buf[20] = { 0 };
	int total_size = 0;
	int size_recv;
	std::stringstream ss;

	while (!isEnd) {
		memset(buf, '\0', sizeof(buf));

		if (size_recv = recv(clientSockfd, buf, 20, 0) == -1) {
			// perror("recv");
			// exit(5);
      break; //just close the sockFD is we can't receive from the client: if the client goes away we assume the port is free
		}
		total_size += size_recv;
		ss << buf << std::endl;
		std::cout << buf << std::endl;

	}

	if (send(clientSockfd, ss.str(), total_size, 0) == -1)
		//		if (send(clientSockfd, buf, 20, 0) == -1) {
		// perror("send");
		// exit(6);
		break; //just close the sockFD is we can't receive from the client: if the client goes away we assume the port is free
	}

	if (ss.str() == "close\n")
		break;

//	ss.str("");

	close(clientSockfd);
}

char* stringToCString(std::string s)
{
	const char* s_cstr = s.data(); //get a const char* version
	char* s_cpy = (char *)malloc(sizeof(char) * (strlen(s_cstr) + 1));
	if (s_cpy == NULL)
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