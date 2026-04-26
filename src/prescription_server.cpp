#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../include/common.h"

class PrescriptionServer
{
public:
   void boot();
   void run();

   struct Prescription
   {
      std::string doctorName, patientHash, treatment, frequency;
      bool found = false;
   };

   void loadPrescriptions(const std::string &filepath = "data/prescriptions.txt");
   void savePrescriptions(const std::string &filepath = "data/prescriptions.txt");
   bool addPrescription(const std::string &doctor, const std::string &patientHash,
                        const std::string &treatment, const std::string &frequency);
   Prescription findPrescription(const std::string &patientHash);

   std::vector<Prescription> prescriptions;  // public for testing

private:
   void handlePrescribe(Message &msg, sockaddr_in &from);
   void handleViewPrescription(Message &msg, sockaddr_in &from);

   int sockfd = -1;
};

void PrescriptionServer::boot()
{
   loadPrescriptions();
   sockfd = makeUDPSocket(PORT_PRESC);
   std::cout << "The Prescription Server is up and running." << std::endl;
}

void PrescriptionServer::run()
{
   Message msg{};
   sockaddr_in from{};
   while (true)
   {
      udpRecv(sockfd, msg, from);
      // handleRequest dispatch will go here
   }
}

void PrescriptionServer::loadPrescriptions(const std::string &filepath)
{
}

void PrescriptionServer::savePrescriptions(const std::string &filepath)
{
}

bool PrescriptionServer::addPrescription(const std::string &doctor, const std::string &patientHash,
                                          const std::string &treatment, const std::string &frequency)
{
   return false;
}

PrescriptionServer::Prescription PrescriptionServer::findPrescription(const std::string &patientHash)
{
   return Prescription{};
}

void PrescriptionServer::handlePrescribe(Message &msg, sockaddr_in &from) {}
void PrescriptionServer::handleViewPrescription(Message &msg, sockaddr_in &from) {}

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main()
{
   PrescriptionServer s;
   s.boot();
   s.run();
}
#endif
