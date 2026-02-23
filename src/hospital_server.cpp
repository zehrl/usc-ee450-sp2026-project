#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define PORT "5000"    // Port of application
#define MAX_BACKLOG 10 // Maximum connections server can handle
#define SERVER_NAME "hospital_server"
#define BUFFER_SIZE 1024 // Buffer allocated for receiving messages

int main()
{
   int status;
   struct addrinfo hints, *res;
   int sockfd, clientFd;
   struct sockaddr_storage their_addr;
   socklen_t addrSize;
   char recvBuffer[BUFFER_SIZE];

   // Starting up message
   std::cout << "Starting up " << SERVER_NAME << "..." << std::endl;

   // Load up address structs with getaddrinfo()

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;     // use IPv4 or IPv6, whichever
   hints.ai_socktype = SOCK_STREAM; // use TCP
   hints.ai_flags = AI_PASSIVE;     // use server's IP address

   status = getaddrinfo(NULL, PORT, &hints, &res);
   if (status != 0)
   {
      std::cout << "❌ Failed to getaddrinfo(). Error: " << status << std::endl;
      return 1;
   }
   else
   {
      std::cout << "✅ getaddrinfo() successful" << std::endl;
   };

   // Create Socket on port for incoming request
   sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   status = bind(sockfd, res->ai_addr, res->ai_addrlen);
   if (status != 0)
   {
      std::cout << "❌ Failed to bind socket. Error: " << status << std::endl;
      return 1;
   }
   else
   {
      std::cout << "✅ Binding successful" << std::endl;
   };

   // Listen on port
   status = listen(sockfd, MAX_BACKLOG);
   if (status != 0)
   {
      std::cout << "❌ Failed to listen on port. Error: " << status << std::endl;
      return 1;
   }
   else
   {
      std::cout << "✅ Hospital server is up and running." << std::endl;
   };

   // Listen for requests
   while (true)
   {
      // Wait for incoming connection request and accept
      addrSize = sizeof their_addr;
      clientFd = accept(sockfd, (struct sockaddr *)&their_addr, &addrSize);

      // Receive request
      int bytesRecv = recv(clientFd, recvBuffer, sizeof(recvBuffer), 0);
      if (bytesRecv > 0)
      {
         recvBuffer[bytesRecv] = '\0'; // append null-terminate for printing at end of character array
         std::cout << "Received: " << recvBuffer << std::endl;
      }
      else if (bytesRecv == 0)
      {
         std::cout << "Client disconnected" << std::endl;
         close(clientFd);
         continue;
      }
      else
      {
         perror("recv");
         close(clientFd);
         continue;
      }

      // Send reply
      const char *msg = "Received message!\n";
      int len = strlen(msg);
      int bytesSent = send(clientFd, msg, len, 0);
      if (bytesSent > 0)
      {
         // std::cout << "Sent: " << msg << std::endl;
      }
      else if (bytesSent == 0)
      {
         std::cout << "Client disconnected" << std::endl;
         close(clientFd);
         continue;
      }
      else
      {
         perror("send");
         close(clientFd);
         continue;
      }

      // Close
      close(clientFd);
   }

   return 0;
}