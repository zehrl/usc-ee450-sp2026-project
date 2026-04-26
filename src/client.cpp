#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/common.h"

class Client
{
public:
   Client(const std::string &username, const std::string &password);
   void boot();
   void run();

private:
   void authenticate();
   void patientCommandLoop();
   void doctorCommandLoop();
   void cmdLookup();
   void cmdLookupDoctor(const std::string &doctor);
   void cmdSchedule(const std::string &doctor, const std::string &time, const std::string &illness);
   void cmdViewAppointment();
   void cmdCancel();
   void cmdViewPrescription();
   void cmdViewAppointments();
   void cmdPrescribe(const std::string &patient, const std::string &frequency);
   void cmdViewPrescriptionDoctor(const std::string &patient);
   void tcpSend(const Message &msg);
   Message tcpRecv();

   int sockfd = -1;
   int localPort = -1;
   std::string username, userHash, passHash;
   bool isDoctor = false;
};

Client::Client(const std::string &username, const std::string &password)
   : username(username)
{
   userHash = sha256hex(username);
   passHash = sha256hex(password);
}

void Client::boot()
{
   sockfd = makeTCPClientSocket(PORT_HOSP_TCP);

   sockaddr_in localAddr{};
   socklen_t len = sizeof(localAddr);
   getsockname(sockfd, (sockaddr *)&localAddr, &len);
   localPort = ntohs(localAddr.sin_port);

   std::cout << "The client is up and running." << std::endl;
}

void Client::run()
{
   authenticate();
   if (isDoctor)
      doctorCommandLoop();
   else
      patientCommandLoop();
}

void Client::authenticate()
{
}

void Client::patientCommandLoop()
{
}

void Client::doctorCommandLoop()
{
}

void Client::cmdLookup()
{
}

void Client::cmdLookupDoctor(const std::string &doctor)
{
}

void Client::cmdSchedule(const std::string &doctor, const std::string &time, const std::string &illness)
{
}

void Client::cmdViewAppointment()
{
}

void Client::cmdCancel()
{
}

void Client::cmdViewPrescription()
{
}

void Client::cmdViewAppointments()
{
}

void Client::cmdPrescribe(const std::string &patient, const std::string &frequency)
{
}

void Client::cmdViewPrescriptionDoctor(const std::string &patient)
{
}

void Client::tcpSend(const Message &msg)
{
   send(sockfd, &msg, sizeof(msg), 0);
}

Message Client::tcpRecv()
{
   Message msg{};
   recv(sockfd, &msg, sizeof(msg), MSG_WAITALL);
   return msg;
}

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
      std::cerr << "Usage: " << argv[0] << " <username> <password>" << std::endl;
      return 1;
   }

   Client c(argv[1], argv[2]);
   c.boot();
   c.run();
   return 0;
}
