#include <netinet/in.h>
#include <string>
#include "../third_party/sha256.h"

#include <arpa/inet.h> // inet_addr()
#include <unistd.h>    // close()
#include <cstdlib>     // exit()
#include <cstdio>      // perror()

#define PORT_AUTH 21570
#define PORT_PRESC 22570
#define PORT_APPT 23570
#define PORT_HOSP_UDP 25570
#define PORT_HOSP_TCP 26570
#define HOST "127.0.0.1"

#define MAX_BACKLOG 10 // Maximum connections server can handle

// SHA-256 helpers
inline std::string sha256hex(const std::string &input)
{
   return SHA256::hashString(input); // SHA256 class is in sha256.h
}
inline std::string hashSuffix(const std::string &hash)
{
   if (hash.length() < 5)
      return hash;
   return hash.substr(hash.length() - 5);
}

// TCP
int makeTCPServerSocket(int port)
{
   // Create the socket/file descriptor
   int fd = socket(
       AF_INET,     // Address family: IPv4
       SOCK_STREAM, // Socket type: Datagram
       0            // Protocol: default
   );
   if (fd < 0)
   {
      perror("socket");
      exit(1);
   }

   int opt = 1;
   setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   sockaddr_in addr{};
   addr.sin_family = AF_INET;              // Internetwork
   addr.sin_addr.s_addr = inet_addr(HOST); // Convert IP address to binary and set
   addr.sin_port = htons(port);            // Correct byte order and set port
   if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
   {
      perror("bind");
      exit(1);
   }

   if (listen(fd, MAX_BACKLOG))
   {
      perror("listen");
      exit(1);
   }

   return fd;
} // socket → setsockopt → bind → listen

int makeTCPClientSocket(int port)
{
   int fd = socket(AF_INET, SOCK_STREAM, 0);
   if (fd < 0)
   {
      perror("socket");
      exit(1);
   }
   
   sockaddr_in addr{};
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr(HOST);
   addr.sin_port = htons(port);

   if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
   {
      perror("connect");
      exit(1);
   }
   return fd;
}; // socket → connect to HOST:port

// UDP Message
struct Message
{
   char type[32];    // "AUTH", "SCHEDULE", "LOOKUP_DOC", "CANCEL", etc.
   char field1[256]; // userHash / doctorName / patientHash
   char field2[256]; // passHash / timeSlot
   char field3[256]; // illness / treatment / response data
   char field4[256]; // frequency
   int status;       // 0=success, 1=failure
};

// UDP
inline int makeUDPSocket(int port)
{
   // Create the socket/file descriptor
   int fd = socket(
       AF_INET,    // Address family: IPv4
       SOCK_DGRAM, // Socket type: Datagram
       0           // Protocol: default - OS picks UDP
   );
   if (fd < 0)
   {
      perror("socket");
      exit(1);
   }

   sockaddr_in addr{};
   addr.sin_family = AF_INET;              // Internetwork
   addr.sin_addr.s_addr = inet_addr(HOST); // Convert IP address to binary and set
   addr.sin_port = htons(port);            // Correct byte order and set port
   if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
   {
      perror("bind");
      exit(1);
   }
   return fd;

} // socket → bind to HOST:port

inline void udpSend(int sockfd, const Message &msg, int targetPort)
{
   sockaddr_in dest{};
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = inet_addr(HOST);
   dest.sin_port = htons(targetPort);
   sendto(sockfd, &msg, sizeof(msg), 0, (sockaddr *)&dest, sizeof(dest));
}

inline void udpRecv(int sockfd, Message &msg, sockaddr_in &senderAddr)
{
   socklen_t len = sizeof(senderAddr);
   recvfrom(sockfd, &msg, sizeof(msg), 0, (sockaddr *)&senderAddr, &len);
}