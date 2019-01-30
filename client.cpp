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
#include <fstream>

#define ERROR_CODE -1

const int buffsize = 1024;

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    fprintf(stderr, "ERROR: either server address or port number or file is not given.\n");
    exit(ERROR_CODE);
  }
  char *hostname = NULL;
  char *hostIP = NULL;
  if (strcmp(argv[1], "localhost") == 0)
  {
    hostname = "localhost";
  }
  else
  {
    hostIP = argv[1];
  }

  int portNum = atoi(argv[2]);

  // FILE *input_file = fopen(argv[3], "r+");

  if (portNum < 1024 || portNum > 65535)
  {
    fprintf(stderr, "ERROR: invalid port number.\n");
    exit(ERROR_CODE);
  }
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portNum); // short, network byte order
  if (hostname){
    struct hostent *server;
    server = gethostbyname(hostname);
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
  }
  else
  {
    if (inet_addr(hostIP) == INADDR_NONE)
    {
      fprintf(stderr, "ERROR: invalid ip address\n");
      exit(ERROR_CODE);
    }
    else
    {
      serverAddr.sin_addr.s_addr = inet_addr(hostIP);
    }
    
  }
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("connect");
    exit(ERROR_CODE);
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1)
  {
    perror("getsockname");
    exit(ERROR_CODE);
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

  // send/receive data to/from connection
  // bool isEnd = false;
  // std::string input;
  char buf[buffsize] = {0};
  // std::stringstream ss;

  std::string dir("./");
  dir.append(argv[3]);
  std::ifstream ifs;

  try
  {
    ifs.open(dir, std::ios::in);
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    exit(ERROR_CODE);
  }

  while(ifs.good())
  {
    int byte_transfer = ifs.read(buf, sizeof(buf)).gcount();
    if (send(sockfd, buf, byte_transfer, 0) == -1)
    {
      perror("send");
      exit(ERROR_CODE);
    }
    memset(buf, '\0', sizeof(buf));
  }
  if (ifs.eof())
  {
    std::cout << "done!" << std::endl;
    ifs.close();
    close(sockfd);
    return 0;
  }
  else
  {
    std::cerr << "error when reading local files" << std::endl;
    exit(ERROR_CODE);
  }
  
  // while (!isEnd)
  // {
  //   memset(buf, '\0', sizeof(buf));

  //   std::cout << "send: ";
  //   std::cin >> input;
  //   if (send(sockfd, input.c_str(), input.size(), 0) == -1)
  //   {
  //     perror("send");
  //     return 4;
  //   }

  //   if (recv(sockfd, buf, buffsize, 0) == -1)
  //   {
  //     perror("recv");
  //     return 5;
  //   }
  //   ss << buf << std::endl;
  //   std::cout << "echo: ";
  //   std::cout << buf << std::endl;

  //   if (ss.str() == "close\n")
  //     break;

  //   ss.str("");
  // }

  // close(sockfd);
  // return 0;
}
