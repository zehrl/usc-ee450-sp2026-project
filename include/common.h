/**
 * @file common.h
 * @brief Common methods and variables shared in the source cpp files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include "../third_party/sha256.h"

// Last 3 digits of USC ID: 570 used for each port number
#define PORT_AUTH 21570
#define PORT_PRESC 22570
#define PORT_APPT 23570
#define PORT_HOSP_UDP 25570
#define PORT_HOSP_TCP 26570
#define HOST "127.0.0.1"

#define MAX_BACKLOG 10 // Maximum connections server can handle

/**
 * @brief A helper method that utilizes the SHA256 library.
 * @param input Input string to hash
 * @return Output hashed string
 */
inline std::string sha256hex(const std::string &input)
{
   return SHA256::hashString(input);
}

/**
 * @brief Provides a hash suffix - the last 5 characters of the hash
 * @param hash The SHA256 hash string
 * @return The 5 character hash suffix
 */
inline std::string hashSuffix(const std::string &hash)
{
   return hash.substr(hash.length() - 5);
}

/**
 * @brief Creates a server TCP socket
 * @param port The given port number to use alongside the socket
 * @return The fd value as an integer (used in socket programming)
 */
int makeTCPServerSocket(int port)
{
   // Create the socket/file descriptor
   int fd = socket(
       AF_INET, // Address family: IPv4
       SOCK_STREAM, // Socket type: Datagram
       0 // Protocol: default
   );
   if (fd < 0)
   {
      perror("socket");
      exit(1);
   }

   int opt = 1;
   setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   sockaddr_in addr{};
   addr.sin_family = AF_INET; // Internetwork
   addr.sin_addr.s_addr = inet_addr(HOST); // Convert IP address to binary and set
   addr.sin_port = htons(port); // Correct byte order and set port
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
}

/**
 * @brief Creates a client TCP socket
 * @param port The given port number to use alongside the socket
 * @return The fd value as an integer (used in socket programming)
 */
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
};

/**
 * @brief Message is a general purpose data structure used for this project. The "type" indicates the command, and each field holds information based on the type.
 * 
 * Types: "AUTH", "LOOKUP", "LOOKUP_DOC", "SCHEDULE", "VIEW_APPT", "CANCEL", "VIEW_APPTS", "PRESCRIBE", "VIEW_PRESECRIPTION"
 */
struct Message
{
   char type[32];    // Type of command ("AUTH", "LOOKUP", "LOOKUP_DOC", "SCHEDULE", "VIEW_APPT", "CANCEL", "VIEW_APPTS", "PRESCRIBE", "VIEW_PRESCRIPTION")
   char field1[256]; // Primary identifier: userHash (AUTH/LOOKUP), doctorName (LOOKUP_DOC/SCHEDULE), patientHash (VIEW_APPT/CANCEL), patientSuffix (PRESCRIBE), or packed response data
   char field2[256]; // Secondary parameter: passHash (AUTH), timeSlot (SCHEDULE), userHash (LOOKUP_DOC), "doctor" role flag (VIEW_PRESCRIPTION from doctor), or frequency (PRESCRIBE)
   char field3[256]; // Tertiary parameter: illness (SCHEDULE/VIEW_APPT response), treatment (prescription response), or doctorHash (PRESCRIBE/VIEW_PRESCRIPTION from doctor)
   char field4[256]; // Quaternary parameter: patientHash (SCHEDULE request), frequency (PRESCRIBE via PrescriptionServer)
   int status;       // Result code: 0 = success, 1 = failure/not found, 2 = all slots free (LOOKUP_DOC only)
};

/**
 * @brief Creates a UDP socket
 * @param port The given port number to use alongside the socket
 * @return The fd value as an integer (used in socket programming)
 */
inline int makeUDPSocket(int port)
{
   // Create the socket/file descriptor
   int fd = socket(
       AF_INET, // Address family: IPv4
       SOCK_DGRAM, // Socket type: Datagram
       0 // Protocol: default - OS picks UDP
   );
   if (fd < 0)
   {
      perror("socket");
      exit(1);
   }

   sockaddr_in addr{};
   addr.sin_family = AF_INET; // Internetwork
   addr.sin_addr.s_addr = inet_addr(HOST); // Convert IP address to binary and set
   addr.sin_port = htons(port); // Correct byte order and set port
   if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0)
   {
      perror("bind");
      exit(1);
   }
   return fd;

}

/**
 * @brief Sends a UDP Message over a given socket and target port
 * @param sockfd Socket to send to using socket.h
 * @param msg the Message to send
 * @param targetPort The target port to send to
 */
inline void udpSend(int sockfd, const Message &msg, int targetPort)
{
   sockaddr_in dest{};
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = inet_addr(HOST);
   dest.sin_port = htons(targetPort);
   sendto(sockfd, &msg, sizeof(msg), 0, (sockaddr *)&dest, sizeof(dest));
}

/**
 * @brief Receives a UDP Message over a given socket and sender address
 * @param sockfd Socket to receive to using socket.h
 * @param msg the Message to receive
 * @param senderAddr The sender's IP addresss
 */
inline void udpRecv(int sockfd, Message &msg, sockaddr_in &senderAddr)
{
   socklen_t len = sizeof(senderAddr);
   recvfrom(sockfd, &msg, sizeof(msg), 0, (sockaddr *)&senderAddr, &len);
}
