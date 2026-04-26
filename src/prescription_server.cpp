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
   std::cout << "Prescription Server is up and running using UDP on port " << PORT_PRESC << "." << std::endl;
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
   std::ifstream file(filepath);
   std::string line;
   while (std::getline(file, line))
   {
      if (line.empty()) continue;
      std::istringstream ss(line);
      Prescription p;
      ss >> p.doctorName >> p.patientHash >> p.treatment >> p.frequency;
      p.found = true;
      prescriptions.push_back(p);
   }
}

void PrescriptionServer::savePrescriptions(const std::string &filepath)
{
   std::ofstream file(filepath);
   for (const auto &p : prescriptions)
      file << p.doctorName << " " << p.patientHash << " " << p.treatment << " " << p.frequency << "\n";
}

bool PrescriptionServer::addPrescription(const std::string &doctor, const std::string &patientHash,
                                          const std::string &treatment, const std::string &frequency)
{
   Prescription p;
   p.doctorName  = doctor;
   p.patientHash = patientHash;
   p.treatment   = treatment;
   p.frequency   = frequency;
   p.found       = true;
   prescriptions.push_back(p);
   return true;
}

PrescriptionServer::Prescription PrescriptionServer::findPrescription(const std::string &patientHash)
{
   for (const auto &p : prescriptions)
   {
      if (p.patientHash == patientHash)
         return p;
   }
   return Prescription{};  // found = false by default
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
