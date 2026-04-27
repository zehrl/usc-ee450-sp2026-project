#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/common.h"

#include <fstream>
#include <sstream>
#include <map>

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

private:
   void handleClient(int clientFd);
   void doAuthenticate(int clientFd, const Message &req);
   void doLookup(int clientFd, const Message &req);
   void doLookupDoctor(int clientFd, const Message &req);
   void doSchedule(int clientFd, const Message &req);
   void doViewAppointment(int clientFd, const Message &req);
   void doCancel(int clientFd, const Message &req);
   void doViewAppointments(int clientFd, const Message &req);
   void doPrescribe(int clientFd, const Message &req);
   void doViewPrescription(int clientFd, const Message &req);
   Message callAuth(const Message &req);
   Message callAppointment(const Message &req);
   Message callPrescription(const Message &req);

   int tcpSock = -1, udpSock = -1;

   struct Doctor
   {
      std::string name;
      std::string hash;
   };

   std::vector<Doctor> doctors;

   std::map<std::string, std::string> treatments; // illness → treatment

   // ... data members
};

bool HospitalServer::isDoctor(const std::string &userHash)
{
   for (const auto &d : doctors)
   {
      if (d.hash == userHash)
      {
         return true;
      }
   }

   return false;
};

std::string HospitalServer::getDoctorName(const std::string &userHash)
{
   for (const auto &d : doctors)
   {
      if (d.hash == userHash)
      {
         return d.name;
      }
   }

   return "";
};

std::string HospitalServer::getTreatment(const std::string &illness)
{
   auto t = treatments.find(illness);
   if (t != treatments.end())
   {
      return t->second;
   }
   return "";
};

std::vector<std::string> HospitalServer::getDoctorList()
{
   std::vector<std::string> names;

   for (const auto &d : doctors)
   {
      names.push_back(d.name);
   }

   return names;
};

void HospitalServer::boot()
{
   loadHospital();
   this->tcpSock = makeTCPServerSocket(PORT_HOSP_TCP);
   this->udpSock = makeUDPSocket(PORT_HOSP_UDP);

   std::cout << "Hospital Server is up and running using UDP on port " << PORT_HOSP_UDP << "." << std::endl;
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
      if      (strcmp(msg.type, "AUTH")       == 0) doAuthenticate(clientFd, msg);
      else if (strcmp(msg.type, "LOOKUP")     == 0) doLookup(clientFd, msg);
      else if (strcmp(msg.type, "LOOKUP_DOC") == 0) doLookupDoctor(clientFd, msg);
      else if (strcmp(msg.type, "SCHEDULE")    == 0) doSchedule(clientFd, msg);
      else if (strcmp(msg.type, "VIEW_APPT")  == 0) doViewAppointment(clientFd, msg);
      else if (strcmp(msg.type, "CANCEL")       == 0) doCancel(clientFd, msg);
      else if (strcmp(msg.type, "VIEW_APPTS")  == 0) doViewAppointments(clientFd, msg);
      else if (strcmp(msg.type, "PRESCRIBE")        == 0) doPrescribe(clientFd, msg);
      else if (strcmp(msg.type, "VIEW_PRESCRIPTION") == 0) doViewPrescription(clientFd, msg);
      memset(&msg, 0, sizeof(msg));
   }
}

Message HospitalServer::callAuth(const Message &req)
{
   Message msg = req;
   udpSend(udpSock, msg, PORT_AUTH);
   sockaddr_in from{};
   udpRecv(udpSock, msg, from);
   return msg;
}

Message HospitalServer::callAppointment(const Message &req)
{
   Message msg = req;
   udpSend(udpSock, msg, PORT_APPT);
   sockaddr_in from{};
   udpRecv(udpSock, msg, from);
   return msg;
}

Message HospitalServer::callPrescription(const Message &req)
{
   Message msg = req;
   udpSend(udpSock, msg, PORT_PRESC);
   sockaddr_in from{};
   udpRecv(udpSock, msg, from);
   return msg;
}

void HospitalServer::doAuthenticate(int clientFd, const Message &req)
{
   std::string suffix = hashSuffix(req.field1);

   std::cout << "Hospital Server received an authentication request from a user with hash suffix " << suffix << "." << std::endl;

   Message resp = callAuth(req);

   std::cout << "Hospital Server has sent an authentication request to the Authentication Server." << std::endl;
   std::cout << "Hospital server has received the response from the authentication server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;

   if (resp.status != 0)
   {
      // Authentication failed — send failure response back to client
      send(clientFd, &resp, sizeof(resp), 0);
      return;
   }

   std::cout << "User with a hash suffix " << suffix << " has been granted access to the system. Determining the access of the user." << std::endl;

   bool doctor = isDoctor(req.field1);
   if (doctor)
   {
      strncpy(resp.field1, "doctor", sizeof(resp.field1));
      std::cout << "User with hash suffix " << suffix << " will be granted doctor access." << std::endl;
   }
   else
   {
      strncpy(resp.field1, "patient", sizeof(resp.field1));
      std::cout << "User with hash " << suffix << " will be granted patient access." << std::endl;
   }

   std::cout << "Hospital Server has sent the response from Authentication Server to the client using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doLookup(int clientFd, const Message &req)
{
   std::string suffix = hashSuffix(std::string(req.field1));
   std::cout << "Hospital Server received a lookup request from a user with a hash suffix " << suffix << " over port " << PORT_HOSP_TCP << "." << std::endl;

   std::vector<std::string> names = getDoctorList();

   std::string packed;
   for (size_t i = 0; i < names.size(); i++)
   {
      if (i > 0) packed += "|";
      packed += names[i];
   }

   Message resp{};
   resp.status = 0;
   strncpy(resp.field1, packed.c_str(), sizeof(resp.field1) - 1);

   std::cout << "Hospital Server has sent the doctor lookup to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::loadHospital(const std::string &filepath)
{
   std::ifstream file(filepath);
   std::string line, section;

   while (std::getline(file, line))
   {
      // Determine section of .txt data file
      if (line == "[Doctors]")
      {
         section = "[Doctors]";
         continue;
      }
      if (line == "[Treatments]")
      {
         section = "[Treatments]";
         continue;
      }
      if (line.empty())
      {
         continue;
      }

      std::istringstream ss(line);

      if (section == "[Doctors]")
      {
         Doctor d;
         ss >> d.name >> d.hash;
         doctors.push_back(d);
      }
      else if (section == "[Treatments]")
      {
         std::string illness, treatment;
         ss >> illness >> treatment;
         treatments[illness] = treatment;
      }
   }
}

void HospitalServer::doLookupDoctor(int clientFd, const Message &req)
{
   std::string doctor(req.field1);
   std::string suffix = hashSuffix(std::string(req.field2));

   std::cout << "Hospital Server has received a lookup request from a user with hash suffix " << suffix << " to lookup " << doctor << " availability using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   Message resp = callAppointment(req);

   std::cout << "Hospital Server sent the doctor lookup request to the Appointment server." << std::endl;
   std::cout << "Hospital Server has received the response from Appointment Server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The Hospital Server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doSchedule(int clientFd, const Message &req)
{
   std::string suffix = hashSuffix(std::string(req.field4));

   std::cout << "Hospital Server has received a schedule request from a user with hash suffix: " << suffix << " to book an appointment using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   Message resp = callAppointment(req);

   std::cout << "Hospital Server has sent the schedule request to the appointment server." << std::endl;
   std::cout << "Hospital Server has received the response from Appointment Server using UDP over " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The hospital server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doViewAppointment(int clientFd, const Message &req)
{
   std::string suffix = hashSuffix(std::string(req.field1));

   std::cout << "Hospital server has received a view appointment request from a user with hash suffix " << suffix << " to view their appointment details using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   Message resp = callAppointment(req);

   std::cout << "Hospital Server has sent the view appointments request to the Appointment Server." << std::endl;
   std::cout << "Hospital Server has received the response from the appointment server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The hospital server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doCancel(int clientFd, const Message &req)
{
   std::string suffix = hashSuffix(std::string(req.field1));

   std::cout << "Hospital Server has received a cancel request from user with hash suffix: " << suffix << " to cancel their appointment using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   Message resp = callAppointment(req);

   std::cout << "The hospital server has sent the cancel request to the appointment server." << std::endl;
   std::cout << "Hospital Server has received the response from Appointment Server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The hospital server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doViewAppointments(int clientFd, const Message &req)
{
   std::string doctorName = getDoctorName(req.field1);

   std::cout << "Hospital Server has received a view appointments request from " << doctorName << " to view their schedule details using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   Message fwd = req;
   strncpy(fwd.field1, doctorName.c_str(), sizeof(fwd.field1) - 1);
   Message resp = callAppointment(fwd);

   std::cout << "The hospital server has sent the view appointments request to the Appointment Server." << std::endl;
   std::cout << "Hospital server has received the response from the Appointment server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The hospital server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
}

void HospitalServer::doPrescribe(int clientFd, const Message &req)
{
   std::string suffix(req.field1);
   std::string frequency(req.field2);
   std::string doctorName = getDoctorName(req.field3);

   std::cout << "Hospital Server has received a prescription request from " << doctorName << " for a user with hash suffix " << suffix << " using TCP over port " << PORT_HOSP_TCP << "." << std::endl;

   // Step 1: get illness + full patientHash from AppointmentServer (also cancels slot)
   Message getMsg{};
   strncpy(getMsg.type,   "GET_ILLNESS",      sizeof(getMsg.type));
   strncpy(getMsg.field1, suffix.c_str(),     sizeof(getMsg.field1) - 1);
   strncpy(getMsg.field2, doctorName.c_str(), sizeof(getMsg.field2) - 1);
   Message illnessResp = callAppointment(getMsg);

   std::cout << "Hospital Server has sent a request to fetch patients with hash suffix " << suffix << " illness information to the Appointment Server." << std::endl;

   if (illnessResp.status != 0)
   {
      Message resp{};
      resp.status = 1;
      send(clientFd, &resp, sizeof(resp), 0);
      return;
   }

   std::string illness(illnessResp.field1);
   std::string patientHash(illnessResp.field2);
   std::string treatment = getTreatment(illness);

   std::cout << "Hospital Server has received the illness response from the Appointment server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "Acquiring treatment for " << illness << " from the database." << std::endl;

   // Step 2: send prescription to PrescriptionServer
   Message prescMsg{};
   strncpy(prescMsg.type,   "PRESCRIBE",          sizeof(prescMsg.type));
   strncpy(prescMsg.field1, doctorName.c_str(),   sizeof(prescMsg.field1) - 1);
   strncpy(prescMsg.field2, patientHash.c_str(),  sizeof(prescMsg.field2) - 1);
   strncpy(prescMsg.field3, treatment.c_str(),    sizeof(prescMsg.field3) - 1);
   strncpy(prescMsg.field4, frequency.c_str(),    sizeof(prescMsg.field4) - 1);
   Message prescResp = callPrescription(prescMsg);

   std::cout << "Hospital server has sent the prescription request to the prescription server to prescribe " << treatment << "." << std::endl;
   std::cout << "Hospital server has received the response from the prescription server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "The hospital server has sent the response to the client." << std::endl;

   send(clientFd, &prescResp, sizeof(prescResp), 0);
}

void HospitalServer::doViewPrescription(int clientFd, const Message &req)
{
   bool isDocRequest = (strcmp(req.field2, "doctor") == 0);

   if (isDocRequest)
   {
      std::string patientSuffix(req.field1);
      std::string doctorName = getDoctorName(std::string(req.field3));
      std::cout << "Hospital Server has received a prescription request from " << doctorName
                << " to view a patient with hash suffix " << patientSuffix
                << " prescription details using TCP over port " << PORT_HOSP_TCP << "." << std::endl;
   }
   else
   {
      std::string suffix = hashSuffix(std::string(req.field1));
      std::cout << "Hospital Server has received a prescription request from a patient with hash suffix " << suffix
                << " to view their prescription details using TCP over port " << PORT_HOSP_TCP << "." << std::endl;
   }

   std::cout << "Hospital Server has sent the prescription request to the Prescription Server." << std::endl;

   Message resp = callPrescription(req);

   std::cout << "Hospital server has received the response from the prescription server using UDP over port " << PORT_HOSP_UDP << "." << std::endl;
   std::cout << "Hospital server has sent the response to the client." << std::endl;

   send(clientFd, &resp, sizeof(resp), 0);
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