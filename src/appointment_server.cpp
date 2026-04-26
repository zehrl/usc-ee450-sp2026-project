#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "../include/common.h"

class AppointmentServer
{
public:
   void boot();
   void run();

   // public for testing
   void loadAppointments(const std::string &filepath = "data/appointments.txt");
   void saveAppointments(const std::string &filepath = "data/appointments.txt");
   std::vector<std::string> getAvailableSlots(const std::string &doctorName);
   bool isSlotAvailable(const std::string &doctorName, const std::string &time);
   bool bookSlot(const std::string &doctorName, const std::string &time,
                 const std::string &patientHash, const std::string &illness);
   bool cancelSlot(const std::string &patientHash);
   std::string getPatientIllness(const std::string &patientHash);
   std::string getPatientDoctor(const std::string &patientHash);
   std::string getPatientTime(const std::string &patientHash);

   struct Slot
   {
      std::string time, patientHash, illness;
   };

   std::map<std::string, std::vector<Slot>> appointmentData; // Holds doctor and time slot data

private:
   void handleLookupDoctor(Message &msg, sockaddr_in &from);
   void handleSchedule(Message &msg, sockaddr_in &from);
   void handleViewAppointment(Message &msg, sockaddr_in &from);
   void handleCancel(Message &msg, sockaddr_in &from);
   void handleViewAppointments(Message &msg, sockaddr_in &from);
   void handleGetIllness(Message &msg, sockaddr_in &from);

   int sockfd = -1;
};

void AppointmentServer::boot()
{
   loadAppointments();
   sockfd = makeUDPSocket(PORT_APPT);
   std::cout << "Appointment Server is up and running using UDP on port " << PORT_APPT << "." << std::endl;
}

void AppointmentServer::run()
{
   Message msg{};
   sockaddr_in from{};
   while (true)
   {
      udpRecv(sockfd, msg, from);
      if      (strcmp(msg.type, "LOOKUP_DOC") == 0) handleLookupDoctor(msg, from);
      else if (strcmp(msg.type, "SCHEDULE")   == 0) handleSchedule(msg, from);
   }
}

void AppointmentServer::loadAppointments(const std::string &filepath)
{
   std::ifstream file(filepath);
   std::string line, section;

   std::string doctor = "";
   std::vector<Slot> timeslots;
   while (std::getline(file, line))
   {
      // Capture all data for doctor
      if (line.empty())
      {
         if (!doctor.empty())
         {
            // Insert doctor and timeslots into appointments
            appointmentData[doctor] = timeslots;
         }

         // End of doctor's timeslots, reset doctor name for next doctor
         doctor = "";
         timeslots.clear();
         continue;
      }

      // Capture doctors name
      if (doctor.empty())
      {
         doctor = line;
         continue;
      }

      // Capture appointment timeslot data
      std::istringstream ss(line);
      AppointmentServer::Slot slot;

      ss >> slot.time >> slot.patientHash >> slot.illness;
      timeslots.push_back(slot);
   }

   if (!doctor.empty())
   {
      appointmentData[doctor] = timeslots;
   }
}

void AppointmentServer::saveAppointments(const std::string &filepath)
{
   std::ofstream file(filepath);
   for (const auto &[doctor, slots] : appointmentData)
   {
      file << doctor << "\n";
      for (const auto &slot : slots)
         file << slot.time << " " << slot.patientHash << " " << slot.illness << "\n";
      file << "\n";
   }
}

bool AppointmentServer::isSlotAvailable(const std::string &doctorName, const std::string &time)
{
   auto it = appointmentData.find(doctorName);
   if (it == appointmentData.end())
      return false;

   for (const auto &slot : it->second)
   {
      if (slot.time == time)
         return slot.patientHash.empty();
   }
   return false;
}

std::vector<std::string> AppointmentServer::getAvailableSlots(const std::string &doctorName)
{
   std::vector<std::string> available;
   auto it = appointmentData.find(doctorName);
   if (it == appointmentData.end())
      return available;

   for (const auto &slot : it->second)
   {
      if (isSlotAvailable(doctorName, slot.time))
         available.push_back(slot.time);
   }
   return available;
}

bool AppointmentServer::bookSlot(const std::string &doctorName, const std::string &time,
                                 const std::string &patientHash, const std::string &illness)
{
   if (!isSlotAvailable(doctorName, time)) return false;

   for (auto &slot : appointmentData[doctorName])
   {
      if (slot.time == time)
      {
         slot.patientHash = patientHash;
         slot.illness = illness;
         return true;
      }
   }
   return false;
}

bool AppointmentServer::cancelSlot(const std::string &patientHash)
{
   for (auto &[doctor, slots] : appointmentData)
   {
      for (auto &slot : slots)
      {
         if (slot.patientHash == patientHash)
         {
            slot.patientHash = "";
            slot.illness = "";
            return true;
         }
      }
   }
   return false;
}

std::string AppointmentServer::getPatientIllness(const std::string &patientHash)
{
   for (const auto &[doctor, slots] : appointmentData)
   {
      for (const auto &slot : slots)
      {
         if (slot.patientHash == patientHash)
            return slot.illness;
      }
   }
   return "";
}

std::string AppointmentServer::getPatientDoctor(const std::string &patientHash)
{
   for (const auto &[doctor, slots] : appointmentData)
   {
      for (const auto &slot : slots)
      {
         if (slot.patientHash == patientHash)
            return doctor;
      }
   }
   return "";
}

std::string AppointmentServer::getPatientTime(const std::string &patientHash)
{
   for (const auto &[doctor, slots] : appointmentData)
   {
      for (const auto &slot : slots)
      {
         if (slot.patientHash == patientHash)
            return slot.time;
      }
   }
   return "";
}

void AppointmentServer::handleLookupDoctor(Message &msg, sockaddr_in &from)
{
   std::string doctorName(msg.field1);
   std::vector<std::string> slots = getAvailableSlots(doctorName);

   std::string packed;
   for (size_t i = 0; i < slots.size(); i++)
   {
      if (i > 0) packed += "|";
      packed += slots[i];
   }

   msg.status = slots.empty() ? 1 : 0;
   strncpy(msg.field1, packed.c_str(), sizeof(msg.field1) - 1);
   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleSchedule(Message &msg, sockaddr_in &from)
{
   std::string doctor(msg.field1);
   std::string time(msg.field2);
   std::string illness(msg.field3);
   std::string patientHash(msg.field4);

   bool ok = bookSlot(doctor, time, patientHash, illness);
   msg.status = ok ? 0 : 1;

   if (ok) saveAppointments();

   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleViewAppointment(Message &msg, sockaddr_in &from) {}
void AppointmentServer::handleCancel(Message &msg, sockaddr_in &from) {}
void AppointmentServer::handleViewAppointments(Message &msg, sockaddr_in &from) {}
void AppointmentServer::handleGetIllness(Message &msg, sockaddr_in &from) {}

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main()
{
   AppointmentServer s;
   s.boot();
   s.run();
}
#endif