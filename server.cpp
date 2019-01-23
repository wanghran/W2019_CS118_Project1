#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <sys/select.h>
#include <fstream>

const int buffsize = 20;

void SIGTERM_handler(int sig);

int main(int argc, char *argv[])
{
  if (argc != 3) 
  {
    fprintf(stderr, "either port number or file directory is not given.\n");
    return 7;
  }
  int portNum = atoi(argv[1]);
  std::string fileDir = argv[2];
  if (portNum <= 1024)
  {
    fprintf(stderr, "invalid port number.\n");
    return 7;
  }
  // create a socket using TCP IP

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    fprintf(stderr, "error when creating a socket with error number: %d, %s\n", errno, strerror(errno));
    return 1;
  }
  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    fprintf(stderr, "error when seeting socket operation with error number: %d, %s\n", errno, strerror(errno));
    return 1;
  }

  if (fcntl(sockfd, F_SETFD, O_NONBLOCK) == -1)
  {
    fprintf(stderr, "error when setting socket to non-blocking mode with error number: %d, %s\n", errno, strerror(errno));
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portNum); // short, network byte order
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    fprintf(stderr, "error when binding socket with error number: %d, %s\n", errno, strerror(errno));
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1)
  {
    fprintf(stderr, "error when listen to the socket with error number: %d, %s\n", errno, strerror(errno));
    return 3;
  }

  while(1)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    if (select(sockfd+1, &readfds, NULL, NULL, NULL) > 0) 
    {
      if (FD_ISSET(sockfd, &readfds))
      {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSockfd == -1)
        {
          perror("accept");
          return 4;
        }
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
        std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // read/write data from/into the connection
        bool isEnd = false;
        char buf[buffsize];
        memset(buf, '\0', buffsize);
        std::stringstream ss;

        std::ofstream logfile;
        logfile.open(fileDir.append("/1.txt"), std::ios::trunc | std::ios::out);
        if (!logfile.is_open()) {
          fprintf(stderr, "error open logfile\n");
          return -1;
        }

        while (!isEnd)
        {
          memset(buf, '\0', buffsize);

          if (recv(clientSockfd, buf, buffsize, 0) == -1)
          {
            perror("recv");
            return 5;
          }

          ss << buf << std::endl;
          // std::cout << buf << std::endl;

          logfile << buf << std::endl;

          if (send(clientSockfd, buf, buffsize, 0) == -1)
          {
            perror("send");
            return 6;
          }

          if (ss.str() == "close\n")
          {
            logfile.close();
            break;
          }

          ss.str("");
        }
        close(clientSockfd);
        
        return 0;
      }
    }
    else
    {
      fprintf(stderr, "error doing select with error number: %d, %s", errno, strerror(errno));
      return 8;
    }
  }
}

//   // accept a new connection
//   struct sockaddr_in clientAddr;
//   socklen_t clientAddrSize = sizeof(clientAddr);
//   int clientSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrSize);

//   if (clientSockfd == -1)
//   {
//     perror("accept");
//     return 4;
//   }

//   char ipstr[INET_ADDRSTRLEN] = {'\0'};
//   inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
//   std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

//   // read/write data from/into the connection
//   bool isEnd = false;
//   char buf[20] = {0};
//   std::stringstream ss;

//   while (!isEnd)
//   {
//     memset(buf, '\0', sizeof(buf));

//     if (recv(clientSockfd, buf, 20, 0) == -1)
//     {
//       perror("recv");
//       return 5;
//     }

//     ss << buf << std::endl;
//     std::cout << buf << std::endl;

//     if (send(clientSockfd, buf, 20, 0) == -1)
//     {
//       perror("send");
//       return 6;
//     }

//     if (ss.str() == "close\n")
//       break;

//     ss.str("");
//   }

//   close(clientSockfd);

//   return 0;
// }

void SIGTERM_handler(int sig)
{
  fprintf(stdout, "get sigterm, gracefully exit\n");
  exit(0);
}