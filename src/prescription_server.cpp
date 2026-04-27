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
      if      (strcmp(msg.type, "PRESCRIBE")        == 0) handlePrescribe(msg, from);
      else if (strcmp(msg.type, "VIEW_PRESCRIPTION") == 0) handleViewPrescription(msg, from);
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

void PrescriptionServer::handlePrescribe(Message &msg, sockaddr_in &from)
{
   std::string doctorName(msg.field1);
   std::string suffix = hashSuffix(std::string(msg.field2));

   std::cout << "Prescription Server has received a request from " << doctorName
             << " to prescribe the user with hash suffix " << suffix << "." << std::endl;

   addPrescription(msg.field1, msg.field2, msg.field3, msg.field4);
   savePrescriptions();

   std::cout << "Successfully saved the prescription details for user with hash suffix: " << suffix << "." << std::endl;

   msg.status = 0;
   udpSend(sockfd, msg, ntohs(from.sin_port));
}

void PrescriptionServer::handleViewPrescription(Message &msg, sockaddr_in &from)
{
   std::string key(msg.field1);
   Prescription p;
   std::string suffix;

   if (key.length() == 5)
   {
      suffix = key;
      for (const auto &entry : prescriptions)
      {
         if (hashSuffix(entry.patientHash) == key)
         {
            p = entry;
            break;
         }
      }
   }
   else
   {
      suffix = hashSuffix(key);
      p = findPrescription(key);
   }

   std::cout << "The prescription server has received a request to view the prescription for the user with hash suffix: " << suffix << "." << std::endl;

   msg.status = p.found ? 0 : 1;
   if (p.found)
   {
      std::cout << "A prescription exists for this user." << std::endl;
      strncpy(msg.field1, p.doctorName.c_str(), sizeof(msg.field1) - 1);
      strncpy(msg.field2, p.treatment.c_str(),  sizeof(msg.field2) - 1);
      strncpy(msg.field3, p.frequency.c_str(),  sizeof(msg.field3) - 1);
   }
   else
   {
      std::cout << "There are no current prescriptions for this user." << std::endl;
   }
   udpSend(sockfd, msg, ntohs(from.sin_port));
}

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main()
{
   PrescriptionServer s;
   s.boot();
   s.run();
}
#endif
