#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
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
   Message msg{};
   strncpy(msg.type,   "AUTH",           sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1));
   strncpy(msg.field2, passHash.c_str(), sizeof(msg.field2));

   tcpSend(msg);
   std::cout << username << " sent an authentication request to the hospital server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status == 1)
   {
      std::cout << "The credentials are incorrect. Please try again." << std::endl;
      close(sockfd);
      exit(0);
   }

   isDoctor = (strcmp(resp.field1, "doctor") == 0);

   if (isDoctor)
      std::cout << username << " received the authentication result. Authentication successful. You have been granted doctor access." << std::endl;
   else
      std::cout << username << " received the authentication result. Authentication successful. You have been granted patient access." << std::endl;
}

void Client::patientCommandLoop()
{
   std::string input;
   while (true)
   {
      std::cout << "\nPlease enter a command: ";
      std::getline(std::cin, input);

      if (input == "lookup")                        cmdLookup();
      else if (input.rfind("lookup ", 0) == 0)      cmdLookupDoctor(input.substr(7));
      else if (input.rfind("schedule ", 0) == 0)
      {
         std::istringstream ss(input.substr(9));
         std::string doctor, time, illness;
         ss >> doctor >> time >> illness;
         cmdSchedule(doctor, time, illness);
      }
      else if (input == "quit")                      { close(sockfd); exit(0); }
      else std::cout << "Unknown command." << std::endl;
   }
}

void Client::doctorCommandLoop()
{
   std::string input;
   while (true)
   {
      std::cout << "\nPlease enter a command: ";
      std::getline(std::cin, input);

      if (input == "quit") { close(sockfd); exit(0); }
      else std::cout << "Unknown command." << std::endl;
   }
}

void Client::cmdLookup()
{
   Message msg{};
   strncpy(msg.type, "LOOKUP", sizeof(msg.type));
   tcpSend(msg);

   Message resp = tcpRecv();

   // Split the '|'-delimited doctor list from field1
   std::string packed(resp.field1);
   std::istringstream ss(packed);
   std::string name;
   std::cout << "The list of doctors available are: " << std::endl;
   while (std::getline(ss, name, '|'))
      std::cout << name << std::endl;
}

void Client::cmdLookupDoctor(const std::string &doctor)
{
   Message msg{};
   strncpy(msg.type,   "LOOKUP_DOC",    sizeof(msg.type));
   strncpy(msg.field1, doctor.c_str(),  sizeof(msg.field1) - 1);
   tcpSend(msg);

   Message resp = tcpRecv();

   if (resp.status != 0)
   {
      std::cout << "Doctor " << doctor << " has no available slots." << std::endl;
      return;
   }

   std::string packed(resp.field1);
   std::istringstream ss(packed);
   std::string slot;
   std::cout << "The available slots for " << doctor << " are: " << std::endl;
   while (std::getline(ss, slot, '|'))
      std::cout << slot << std::endl;
}

void Client::cmdSchedule(const std::string &doctor, const std::string &time, const std::string &illness)
{
   Message msg{};
   strncpy(msg.type,   "SCHEDULE",       sizeof(msg.type));
   strncpy(msg.field1, doctor.c_str(),   sizeof(msg.field1) - 1);
   strncpy(msg.field2, time.c_str(),     sizeof(msg.field2) - 1);
   strncpy(msg.field3, illness.c_str(),  sizeof(msg.field3) - 1);
   strncpy(msg.field4, userHash.c_str(), sizeof(msg.field4) - 1);
   tcpSend(msg);

   Message resp = tcpRecv();

   if (resp.status != 0)
      std::cout << "The requested slot is not available." << std::endl;
   else
      std::cout << "Appointment scheduled with " << doctor << " at " << time << " for " << illness << "." << std::endl;
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
