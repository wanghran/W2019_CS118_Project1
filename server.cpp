#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <sys/select.h>
#include <stdio.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#define ERROR_CODE -1

const int buffsize = 1024;
int cid = 1;
std::string fileDir;
void signal_handler(int sig);

void receive(int clientSockfd);

int main(int argc, char *argv[])
{
  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);
  if (argc != 3)
  {
    fprintf(stderr, "ERROR: either port number or file directory is not given.\n");
    exit(ERROR_CODE);
  }
  int portNum = atoi(argv[1]);
  fileDir = argv[2];

  struct stat info;
  if (stat(fileDir.c_str(), &info))
  {
    mkdir(fileDir.c_str(), 0666);
  }

  if (portNum < 1024 || portNum > 65535)
  {
    fprintf(stderr, "ERROR: invalid port number.\n");
    exit(ERROR_CODE);
  }
  // create a socket using TCP IP

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    fprintf(stderr, "error when creating a socket with error number: %d, %s\n", errno, strerror(errno));
    exit(ERROR_CODE);
  }
  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    fprintf(stderr, "error when seeting socket operation with error number: %d, %s\n", errno, strerror(errno));
    exit(ERROR_CODE);
  }

  if (fcntl(sockfd, F_SETFD, O_NONBLOCK) == -1)
  {
    fprintf(stderr, "error when setting socket to non-blocking mode with error number: %d, %s\n", errno, strerror(errno));
    exit(ERROR_CODE);
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
    exit(ERROR_CODE);
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1)
  {
    fprintf(stderr, "error when listen to the socket with error number: %d, %s\n", errno, strerror(errno));
    exit(ERROR_CODE);
  }

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);

  while(1)
  {
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
          exit(ERROR_CODE);
        }
        char ipstr[INET_ADDRSTRLEN] = {'\0'};
        inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
        std::cout << "Accept a connection from: " << ipstr << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // read/write data from/into the connection

        receive(clientSockfd);

        // char buf[buffsize];
        // memset(buf, '\0', sizeof(buf));
        // int receive;
        // // std::stringstream ss;

        // std::ofstream logfile;
        // logfile.open(fileDir.append("/1.file"), std::ios::trunc | std::ios::out | std::ios::binary);
        // if (!logfile.is_open()) {
        //   fprintf(stderr, "error open logfile\n");
        //   exit(ERROR_CODE);
        // }

        // while(1)
        // {
        //   memset(buf, '\0', sizeof(buf));

        //   receive = recv(clientSockfd, buf, buffsize, 0);
        //   // std::cout << receive << std::endl;
        //   if (receive == -1)
        //   {
        //     perror("recv");
        //     exit(ERROR_CODE);
        //   }
        //   if (receive == 0)
        //   {
        //     // std::cout << "file done!" << std::endl;
        //     logfile.close();
        //     break;
        //   }

        //   logfile.write(buf, receive);

        // }
        close(clientSockfd);
        return 0;
      }
    }
    else
    {
      fprintf(stderr, "error doing select with error number: %d, %s", errno, strerror(errno));
      exit(ERROR_CODE);
    }
  }
}

void signal_handler(int sig)
{
  if (sig == SIGTERM)
  {
    fprintf(stdout, "get sigterm, gracefully exit\n");
    exit(0);
  }
  if (sig == SIGQUIT)
  {
    fprintf(stdout, "get sigquit, gracefully exit\n");
    exit(0);
  }
}

void receive(int clientSockfd)
{
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(clientSockfd, &readfds);
  struct timeval tv;
  tv.tv_sec = 15;
  tv.tv_usec = 0;
  char buf[buffsize];
  memset(buf, '\0', sizeof(buf));
  int receive = 0;
  std::ofstream logfile;
  std::string fileName("/");
  fileName.append(std::to_string(cid));
  fileName.append(".file");
  logfile.open(fileDir.append(fileName), std::ios::trunc | std::ios::out | std::ios::binary);
  if (!logfile.is_open())
  {
    fprintf(stderr, "error open logfile\n");
    exit(ERROR_CODE);
  }

  while(1)
  {
    int se = select(clientSockfd + 1, &readfds, NULL, NULL, &tv) > 0;
    if (se > 0)
    {
      if (FD_ISSET(clientSockfd, &readfds))
      {
        while (1)
        {
          memset(buf, '\0', sizeof(buf));

          receive = recv(clientSockfd, buf, buffsize, 0);

          if (receive == -1)
          {
            perror("recv");
            exit(ERROR_CODE);
          }
          if (receive == 0)
          {
            break;
          }

          logfile.write(buf, receive);
        }
      }
    }
    else if (se == 0)
    {
      std::string err = "ERROR";
      logfile.write(err.c_str(), err.length());
      return;
    }
    else
    {
      perror("select");
      exit(ERROR_CODE);
    }
  }
}