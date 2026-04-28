/**
 * @file appointment_server.cpp
 * @brief Appointment server that interacts with appointment database
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "../include/common.h"

/**
 * @brief Appointment server handles retrieving and updating appointments.
 */
class AppointmentServer
{
public:
   /**
    * @brief Loads data into memory and begins server socket(s)
    */
   void boot();

   /**
    * @brief Actual server loop that handles commands
    */
   void run();

   /**
    * @brief Loads the appointment data into memory
    * @param filepath Filepath of data .txt file
    */
   void loadAppointments(const std::string &filepath = "data/appointments.txt");

   /**
    * @brief Saves the appointment into the .txt database
    * @param filepath Path of .txt database
    */
   void saveAppointments(const std::string &filepath = "data/appointments.txt");

   /**
    * @brief Provides a list of available time slots for a specific doctor
    * @param doctorName The doctor name
    * @return Available time slots as a vector of strings
    */
   std::vector<std::string> getAvailableSlots(const std::string &doctorName);

   /**
    * @brief Determines if a given timeslot for a doctor is available
    * @param doctorName The name of the doctor
    * @param time The timeslot (ex. "9:00" format)
    * @return True/False if timeslot is available
    */
   bool isSlotAvailable(const std::string &doctorName, const std::string &time);

   /**
    * @brief Books the timeslot in memory for a given doctor, patient, timeslot and reason/illness
    * @param doctorName The doctor name
    * @param time The time slot (ex. "9:00")
    * @param patientHash The full hash of the patient
    * @param illness The illness/reason for visit
    * @return True - Success, False - Failed
    */
   bool bookSlot(const std::string &doctorName, const std::string &time,
                 const std::string &patientHash, const std::string &illness);

   /**
    * @brief Removes the timeslot in memory for a given patient
    * @param patientHash The full hash of the patient
    * @return True - Success, False - Failed
    */
   bool cancelSlot(const std::string &patientHash);

   /**
    * @brief Returns the illness of a patient
    * @param patientHash The full hash of the patient
    * @return Illness as a string
    */
   std::string getPatientIllness(const std::string &patientHash);

   /**
    * @brief Returns the doctor of a patient
    * @param patientHash The full hash of the patient
    * @return The doctor
    */
   std::string getPatientDoctor(const std::string &patientHash);

   /**
    * @brief Returns the patients scheduled time slot
    * @param patientHash The full hash of the patient
    * @return The patient time slot, or "" if it does not exist
    */
   std::string getPatientTime(const std::string &patientHash);

   /**
    * @brief Time slot data structure
    */
   struct Slot
   {
      std::string time; // Ex. 9:00
      std::string patientHash; // Full SH256 hash
      std::string illness; // ex. "Flu"
   };

   std::map<std::string, std::vector<Slot>> appointmentData; // Holds doctor and time slot data

private:
   /**
    * @brief Handles "LOOKUP_DOC" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleLookupDoctor(Message &msg, sockaddr_in &from);

   /**
    * @brief Handles "SCHEDULE" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleSchedule(Message &msg, sockaddr_in &from);

   /**
    * @brief Handles "VIEW_APPT" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleViewAppointment(Message &msg, sockaddr_in &from);

   /**
    * @brief Handles "CANCEL" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleCancel(Message &msg, sockaddr_in &from);

   /**
    * @brief Handles "VIEW_APPTS" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleViewAppointments(Message &msg, sockaddr_in &from);

   /**
    * @brief Handles "GET_ILLNESS" command
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
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
      if (strcmp(msg.type, "LOOKUP_DOC") == 0)
         handleLookupDoctor(msg, from);
      else if (strcmp(msg.type, "SCHEDULE") == 0)
         handleSchedule(msg, from);
      else if (strcmp(msg.type, "VIEW_APPT") == 0)
         handleViewAppointment(msg, from);
      else if (strcmp(msg.type, "CANCEL") == 0)
         handleCancel(msg, from);
      else if (strcmp(msg.type, "VIEW_APPTS") == 0)
         handleViewAppointments(msg, from);
      else if (strcmp(msg.type, "GET_ILLNESS") == 0)
         handleGetIllness(msg, from);
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
   if (!isSlotAvailable(doctorName, time))
      return false;

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

   std::cout << "Appointment Server has received a doctor availability request." << std::endl;

   auto it = appointmentData.find(doctorName);
   if (it == appointmentData.end())
   {
      std::cout << doctorName << " was not found in the system." << std::endl;
      msg.status = 1;
      msg.field1[0] = '\0';
      udpSend(sockfd, msg, ntohs(from.sin_port));
      return;
   }

   std::cout << doctorName << " was found in the system." << std::endl;

   std::vector<std::string> slots = getAvailableSlots(doctorName);

   std::string packed;
   for (size_t i = 0; i < slots.size(); i++)
   {
      if (i > 0)
         packed += "|";
      packed += slots[i];
   }

   bool allFree = (slots.size() == it->second.size());
   msg.status = slots.empty() ? 1 : (allFree ? 2 : 0);
   strncpy(msg.field1, packed.c_str(), sizeof(msg.field1) - 1);
   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleSchedule(Message &msg, sockaddr_in &from)
{
   std::string doctor(msg.field1);
   std::string time(msg.field2);
   std::string illness(msg.field3);
   std::string patientHash(msg.field4);
   std::string suffix = hashSuffix(patientHash);

   std::cout << "Appointment scheduling request received (time: " << time << ", doctor: " << doctor << ", patient hash suffix: " << suffix << ", illness: " << illness << ")." << std::endl;

   bool ok = bookSlot(doctor, time, patientHash, illness);
   msg.status = ok ? 0 : 1;

   if (ok)
   {
      std::cout << "Appointment has been scheduled successfully for user " << suffix << " with " << doctor << "." << std::endl;
      saveAppointments();
   }
   else
   {
      std::cout << "The requested appointment time is not available." << std::endl;
      std::vector<std::string> available = getAvailableSlots(doctor);
      std::string packed;
      for (size_t i = 0; i < available.size(); i++)
      {
         if (i > 0)
            packed += "|";
         packed += available[i];
      }
      strncpy(msg.field1, packed.c_str(), sizeof(msg.field1) - 1);
   }

   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleViewAppointment(Message &msg, sockaddr_in &from)
{
   std::string patientHash(msg.field1);
   std::string suffix = hashSuffix(patientHash);

   std::cout << "Appointment Server has received a view appointment command for the user with hash suffix " << suffix << "." << std::endl;

   std::string doctor = getPatientDoctor(patientHash);
   std::string time = getPatientTime(patientHash);
   std::string illness = getPatientIllness(patientHash);

   if (doctor.empty())
   {
      std::cout << "The user with hash suffix " << suffix << " has no appointment in the system." << std::endl;
      msg.status = 1;
   }
   else
   {
      std::cout << "Returning details regarding the appointment for the user with hash suffix " << suffix << "." << std::endl;
      msg.status = 0;
      strncpy(msg.field1, doctor.c_str(), sizeof(msg.field1) - 1);
      strncpy(msg.field2, time.c_str(), sizeof(msg.field2) - 1);
      strncpy(msg.field3, illness.c_str(), sizeof(msg.field3) - 1);
   }

   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleCancel(Message &msg, sockaddr_in &from)
{
   std::string patientHash(msg.field1);
   std::string suffix = hashSuffix(patientHash);

   std::cout << "Appointment Server has received a cancel appointment command for the user with hash suffix: " << suffix << "." << std::endl;

   std::string doctor = getPatientDoctor(patientHash);
   std::string time = getPatientTime(patientHash);

   bool ok = cancelSlot(patientHash);
   msg.status = ok ? 0 : 1;

   if (ok)
   {
      std::cout << "Successfully canceled appointment." << std::endl;
      saveAppointments();
      strncpy(msg.field1, doctor.c_str(), sizeof(msg.field1) - 1);
      strncpy(msg.field2, time.c_str(), sizeof(msg.field2) - 1);
   }
   else
   {
      std::cout << "Error: Failed to find appointment." << std::endl;
   }

   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleViewAppointments(Message &msg, sockaddr_in &from)
{
   std::string doctorName(msg.field1);

   std::cout << "Appointment Server has received a request to view appointments scheduled for " << doctorName << "." << std::endl;

   auto it = appointmentData.find(doctorName);

   std::string packed;
   if (it != appointmentData.end())
   {
      for (const auto &slot : it->second)
      {
         if (!slot.patientHash.empty())
         {
            // Use | here as delimiter
            if (!packed.empty())
               packed += "|";
            packed += slot.time + "|" + slot.patientHash + "|" + slot.illness;
         }
      }
   }

   if (packed.empty())
   {
      std::cout << "No appointments have been made for " << doctorName << "." << std::endl;
      msg.status = 1;
   }
   else
   {
      std::cout << "Returning the scheduled appointments for " << doctorName << "." << std::endl;
      msg.status = 0;
      strncpy(msg.field1, packed.c_str(), sizeof(msg.field1) - 1);
   }

   udpSend(sockfd, msg, ntohs(from.sin_port));
}
void AppointmentServer::handleGetIllness(Message &msg, sockaddr_in &from)
{
   std::string suffix(msg.field1);
   std::string doctorName(msg.field2);

   std::cout << "Appointment Server has received a request from Hospital Server regarding information about a user with hash suffix " << suffix << " from " << doctorName << "." << std::endl;

   for (auto &[doctor, slots] : appointmentData)
   {
      for (auto &slot : slots)
      {
         if (!slot.patientHash.empty() && hashSuffix(slot.patientHash) == suffix)
         {
            msg.status = 0;
            std::string illness = slot.illness;
            std::string patientHash = slot.patientHash;
            std::string slotTime = slot.time;

            strncpy(msg.field1, illness.c_str(), sizeof(msg.field1) - 1);
            strncpy(msg.field2, patientHash.c_str(), sizeof(msg.field2) - 1);

            std::cout << "Sending back the requested information to the Hospital server." << std::endl;

            slot.patientHash = "";
            slot.illness = "";
            saveAppointments();

            std::cout << "Successfully removed " << suffix << " appointment slot, " << slotTime << " is now free to be scheduled for tomorrow." << std::endl;

            udpSend(sockfd, msg, ntohs(from.sin_port));
            return;
         }
      }
   }

   msg.status = 1;
   udpSend(sockfd, msg, ntohs(from.sin_port));
}

#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
/**
 * @brief Entry point that starts up server
 * @return N/A
 */
int main()
{
   AppointmentServer s;
   s.boot();
   s.run();
}
#endif
