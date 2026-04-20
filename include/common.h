#include <netinet/in.h>
#include <string>
#include "../third_party/sha256.h"

#define PORT_AUTH 21570
#define PORT_PRESC 22570
#define PORT_APPT 23570
#define PORT_HOSP_UDP 25570
#define PORT_HOSP_TCP 26570
#define HOST "127.0.0.1"


// SHA-256 helpers
inline std::string sha256hex(const std::string& input) {
    return SHA256::hashString(input);  // SHA256 class is in sha256.h
}
inline std::string hashSuffix(const std::string& hash) {
    if (hash.length() < 5) return hash;
    return hash.substr(hash.length() - 5);
}

// TCP
int makeTCPServerSocket(int port);   // socket → setsockopt → bind → listen
int makeTCPClientSocket(int port);   // socket → connect to HOST:port

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
int makeUDPSocket(int port);         // socket → bind to HOST:port
void udpSend(int sockfd, const Message& msg, int targetPort);
void udpRecv(int sockfd, Message& msg, sockaddr_in& senderAddr);

