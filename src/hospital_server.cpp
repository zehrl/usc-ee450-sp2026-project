#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/common.h"

#define MAX_BACKLOG 10 // Maximum connections server can handle
#define SERVER_NAME "hospital_server"
#define BUFFER_SIZE 1024 // Buffer allocated for receiving messages

class HospitalServer
{
public:
   void loadHospital(const std::string &filepath = "data/hospital.txt");
   bool isDoctor(const std::string &userHash);
   std::string getDoctorName(const std::string &userHash);
   std::string getTreatment(const std::string &illness);
   std::vector<std::string> getDoctorList();
   void boot();
   void run();
   void handleClient(int clientFd);

private:
   int tcpSock = -1, udpSock = -1;
   // ... data members
};

bool HospitalServer::isDoctor(const std::string &userHash)
{
   std::cout << "isDoctor() called" << std::endl;
   return false;
};

std::string HospitalServer::getDoctorName(const std::string &userHash)
{
   std::cout << "getDoctorName() called" << std::endl;
   return "";
};

std::string HospitalServer::getTreatment(const std::string &illness)
{
   std::cout << "getTreatment() called" << std::endl;
   return "";
};

std::vector<std::string> HospitalServer::getDoctorList()
{
   std::cout << "getDoctorList() called" << std::endl;

   std::vector<std::string> list;
   return list;
};

void HospitalServer::boot()
{
   loadHospital();
   this->tcpSock = makeTCPServerSocket(PORT_HOSP_TCP);
   this->udpSock = makeUDPSocket(PORT_HOSP_UDP);

   std::cout << "The Hospital Server is up and running." << std::endl;
}

void HospitalServer::run()
{
   while (true)
   {
      // Blocks until a client connects
      sockaddr_in clientAddr{};
      socklen_t clientLen = sizeof(clientAddr);
      
      int clientFd = accept(tcpSock, (sockaddr *)&clientAddr, &clientLen);
      if (clientFd < 0)
      {
         perror("accept");
         continue;
      }

      // Fork so the parent can immediately accept the next client
      if (fork() == 0)
      {
         // --- CHILD PROCESS ---
         close(tcpSock); // child doesn't need the listening socket
         handleClient(clientFd);
         close(clientFd);
         exit(0);
      }
      // --- PARENT PROCESS ---
      close(clientFd); // parent doesn't need this client's socket
   }
}

void HospitalServer::handleClient(int clientFd)
{
   Message msg{};
   while (recv(clientFd, &msg, sizeof(msg), MSG_WAITALL) > 0)
   {
      //   if      (strcmp(msg.type, "AUTH")     == 0) doAuthenticate(clientFd, msg);
      //   else if (strcmp(msg.type, "LOOKUP")   == 0) doLookup(clientFd, msg);
      // ... other commands
      memset(&msg, 0, sizeof(msg)); // clear for next message
   }
}

void HospitalServer::loadHospital(const std::string &filepath)
{
   std::cout << "loadHospital() called" << std::endl;
}

// Bandaid fix - normally we would separate our header definitions and our main method
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN // Do not use main if we're using doctest

int main()
{
   HospitalServer s;
   s.boot();
   s.run();
}
#endif