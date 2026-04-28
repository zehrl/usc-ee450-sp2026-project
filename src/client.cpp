/**
 * @file client.cpp
 * @brief Client code that interacts with hospital server
 */

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/common.h"

/**
 * @brief Client code that interacts with hospital server
 */
class Client
{
public:
   /**
    * @brief Client data
    * @param username Client username
    * @param password Client password
    */
   Client(const std::string &username, const std::string &password);

   /**
    * @brief Loads data into memory and begins server socket(s)
    */
   void boot();

   /**
    * @brief Actual server loop that handles commands
    */
   void run();

private:
   /**
    * @brief Checks authentication of user (patient vs. doctor)
    */
   void authenticate();

   /**
    * @brief The primary loop for the patient flow
    */
   void patientCommandLoop();

   /**
    * @brief The primary loop for the doctor flow
    */
   void doctorCommandLoop();

   /**
    * @brief Server command call for "lookup"
    */
   void cmdLookup();

   /**
    * @brief Server command call for "lookup <doctor>"
    * @param doctor Server command call for doctor lookup
    */
   void cmdLookupDoctor(const std::string &doctor);

   /**
    * @brief Server command call for "schedule..."
    * @param doctor The doctor
    * @param time Timeslot (ex. 10:00)
    * @param illness Ilness (ex. "Flu")
    */
   void cmdSchedule(const std::string &doctor, const std::string &time, const std::string &illness);

   /**
    * @brief Server command call for "view_appointment"
    */
   void cmdViewAppointment();

   /**
    * @brief Server command call for "view_appointment"
    */
   void cmdCancel();

   /**
    * @brief Server command call for "view_prescription"
    */
   void cmdViewPrescription();

   /**
    * @brief Server command call for "view_appointment"
    */
   void cmdViewAppointments();

   /**
    * @brief Server command call for "prescribe..."
    * @param patient Patient hash
    * @param frequency The frequency -  “None”, “Daily”, “Bi-daily”, “Weekly”
    */
   void cmdPrescribe(const std::string &patient, const std::string &frequency);

   /**
    * @brief Server command call for "view_prescription"
    * @param patient The patient hash
    */
   void cmdViewPrescriptionDoctor(const std::string &patient);

   /**
    * @brief Server command call for "help"
    */
   void cmdHelp();

   /**
    * @brief Sends TCP message
    * @param msg Message (see include/common.h)
    */
   void tcpSend(const Message &msg);

   /**
    * @brief Receives TCP message
    * @param msg Message (see include/common.h)
    */
   Message tcpRecv();

   int sockfd = -1;
   int localPort = -1;
   std::string username, userHash, passHash;
   bool isDoctor = false;
};

Client::Client(const std::string &username, const std::string &password) : username(username)
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
   strncpy(msg.type, "AUTH", sizeof(msg.type));
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

      if (input == "lookup")
         cmdLookup();
      else if (input.rfind("lookup ", 0) == 0)
         cmdLookupDoctor(input.substr(7));
      else if (input == "view_appointment")
         cmdViewAppointment();
      else if (input == "view_prescription")
         cmdViewPrescription();
      else if (input == "cancel")
         cmdCancel();
      else if (input.rfind("schedule ", 0) == 0)
      {
         std::istringstream ss(input.substr(9));
         std::string doctor, time, illness;
         ss >> doctor >> time >> illness;
         cmdSchedule(doctor, time, illness);
      }
      else if (input == "help")
         cmdHelp();
      else if (input == "quit")
      {
         std::cout << "You have successfully been logged out." << std::endl;
         close(sockfd);
         exit(0);
      }
      else
         std::cout << "Unknown command." << std::endl;
   }
}

void Client::doctorCommandLoop()
{
   std::string input;
   while (true)
   {
      std::cout << "\nPlease enter a command: ";
      std::getline(std::cin, input);

      if (input == "view_appointments")
         cmdViewAppointments();
      else if (input.rfind("view_prescription ", 0) == 0)
         cmdViewPrescriptionDoctor(input.substr(18));
      else if (input.rfind("prescribe ", 0) == 0)
      {
         std::istringstream ss(input.substr(10));
         std::string suffix, frequency;
         ss >> suffix >> frequency;
         cmdPrescribe(suffix, frequency);
      }
      else if (input == "help")
         cmdHelp();
      else if (input == "quit")
      {
         std::cout << "You have successfully been logged out." << std::endl;
         close(sockfd);
         exit(0);
      }
      else
         std::cout << "Unknown command." << std::endl;
   }
}

void Client::cmdLookup()
{
   Message msg{};
   strncpy(msg.type, "LOOKUP", sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1) - 1);
   tcpSend(msg);

   std::cout << username << " sent a lookup request to the hospital server." << std::endl;

   Message resp = tcpRecv();

   std::cout << "The client received the response from the hospital server using TCP over port " << localPort << ".\nThe following doctors are available:" << std::endl;
   std::string packed(resp.field1);
   std::istringstream ss(packed);
   std::string name;
   while (std::getline(ss, name, '|'))
      std::cout << name << std::endl;
}

void Client::cmdLookupDoctor(const std::string &doctor)
{
   Message msg{};
   strncpy(msg.type, "LOOKUP_DOC", sizeof(msg.type));
   strncpy(msg.field1, doctor.c_str(), sizeof(msg.field1) - 1);
   strncpy(msg.field2, userHash.c_str(), sizeof(msg.field2) - 1);
   tcpSend(msg);

   std::cout << "Patient " << username << " sent a lookup request to the hospital server for " << doctor << "." << std::endl;

   Message resp = tcpRecv();

   if (resp.status == 1)
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << ".\n\n"
                << doctor << " has no time slots available." << std::endl;
      return;
   }

   std::string packed(resp.field1);
   std::istringstream ss(packed);
   std::string slot;

   if (resp.status == 2)
   {
      std::cout << "The client received the response from the hospital server using TCP over port " << localPort << ".\n\nAll time blocks are available for " << doctor << "." << std::endl;
   }
   else
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << ".\n\n"
                << doctor << " is available at times: " << std::endl;
      while (std::getline(ss, slot, '|'))
         std::cout << slot << std::endl;
   }
}

void Client::cmdSchedule(const std::string &doctor, const std::string &time, const std::string &illness)
{
   Message msg{};
   strncpy(msg.type, "SCHEDULE", sizeof(msg.type));
   strncpy(msg.field1, doctor.c_str(), sizeof(msg.field1) - 1);
   strncpy(msg.field2, time.c_str(), sizeof(msg.field2) - 1);
   strncpy(msg.field3, illness.c_str(), sizeof(msg.field3) - 1);
   strncpy(msg.field4, userHash.c_str(), sizeof(msg.field4) - 1);
   tcpSend(msg);

   std::cout << username << " sent an appointment schedule request to the hospital server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
   {
      std::string available(resp.field1);
      if (available.empty())
      {
         std::cout << "The client received the response from the hospital server using TCP over port " << localPort << "\n\nUnable to schedule an appointment with " << doctor << " at this time, as all time blocks have been taken up." << std::endl;
      }
      else
      {
         std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nUnable to schedule an appointment with " << doctor << " at " << time << ". Other available time blocks are" << std::endl;
         std::istringstream ss(available);
         std::string slot;
         while (std::getline(ss, slot, '|'))
            std::cout << slot << std::endl;
      }
   }
   else
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nAn appointment has been successfully scheduled for patient " << username << " with " << doctor << " at " << time << "." << std::endl;
   }
}

void Client::cmdViewAppointment()
{
   Message msg{};
   strncpy(msg.type, "VIEW_APPT", sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1) - 1);
   tcpSend(msg);

   std::cout << username << " sent a request to view their appointment to the Hospital Server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
   {
      std::cout << "The client received the response from the hospital server using TCP over client port " << localPort << "\n\nYou do not have an appointment today." << std::endl;
      return;
   }

   std::cout << "The client received the response from the hospital server using TCP over port " << localPort << "\n\nYou have an appointment scheduled with " << resp.field1 << " at " << resp.field2 << "." << std::endl;
}

void Client::cmdCancel()
{
   Message msg{};
   strncpy(msg.type, "CANCEL", sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1) - 1);
   tcpSend(msg);

   std::cout << username << " sent a cancellation request to the Hospital Server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou have no appointments available to cancel." << std::endl;
   else
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou have successfully cancelled your appointment with " << resp.field1 << " at " << resp.field2 << "." << std::endl;
}

void Client::cmdViewPrescription()
{
   Message msg{};
   strncpy(msg.type, "VIEW_PRESCRIPTION", sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1) - 1);
   tcpSend(msg);

   std::cout << username << " sent a request to view their prescription to the Hospital Server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou do not have a prescription to look up." << std::endl;
   }
   else if (strcmp(resp.field3, "None") == 0)
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou were not prescribed any treatment by \"" << resp.field1 << " following your diagnosis." << std::endl;
   }
   else
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou have been prescribed " << resp.field2 << ", to be taken " << resp.field3 << ", by " << resp.field1 << "." << std::endl;
   }
}

void Client::cmdViewAppointments()
{
   Message msg{};
   strncpy(msg.type, "VIEW_APPTS", sizeof(msg.type));
   strncpy(msg.field1, userHash.c_str(), sizeof(msg.field1) - 1);
   tcpSend(msg);

   std::cout << username << " sent a request to view their scheduled appointments to the Hospital Server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
   {
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou do not have any appointments scheduled." << std::endl;
      return;
   }

   std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\n"
             << username << " is scheduled at times: " << std::endl;
   std::string packed(resp.field1);
   std::istringstream ss(packed);
   std::string time, patientHash, illness;
   while (std::getline(ss, time, '|') &&
          std::getline(ss, patientHash, '|') &&
          std::getline(ss, illness, '|'))
   {
      std::cout << time << std::endl;
   }
}

void Client::cmdPrescribe(const std::string &patient, const std::string &frequency)
{
   Message msg{};
   strncpy(msg.type, "PRESCRIBE", sizeof(msg.type));
   strncpy(msg.field1, patient.c_str(), sizeof(msg.field1) - 1); // hash suffix
   strncpy(msg.field2, frequency.c_str(), sizeof(msg.field2) - 1);
   strncpy(msg.field3, userHash.c_str(), sizeof(msg.field3) - 1); // doctor's hash
   tcpSend(msg);

   std::cout << username << " sent a request to the Hospital Server to prescribe " << patient << " following their diagnosis." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nPrescription failed. No appointment found for that patient." << std::endl;
   else
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\nYou have successfully prescribed " << patient << " with " << resp.field3 << ", to be taken " << resp.field4 << "." << std::endl;
}

void Client::cmdViewPrescriptionDoctor(const std::string &patient)
{
   Message msg{};
   strncpy(msg.type, "VIEW_PRESCRIPTION", sizeof(msg.type));
   strncpy(msg.field1, patient.c_str(), sizeof(msg.field1) - 1);  // hash suffix
   strncpy(msg.field2, "doctor", sizeof(msg.field2) - 1);         // flag for resolution
   strncpy(msg.field3, userHash.c_str(), sizeof(msg.field3) - 1); // doctor's own hash
   tcpSend(msg);

   std::cout << username << " sent a request to view " << patient << " prescription to the Hospital Server." << std::endl;

   Message resp = tcpRecv();

   if (resp.status != 0)
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\n"
                << patient << " does not have a prescription." << std::endl;
   else
      std::cout << "The client received the response from the Hospital Server using TCP over port " << localPort << "\n\n"
                << patient << " has been prescribed " << resp.field2 << ", to be taken " << resp.field3 << ", by " << resp.field1 << "." << std::endl;
}

void Client::cmdHelp()
{
   if (isDoctor)
   {
      std::cout << "Please enter the command:\n"
                << "<view_appointments>,\n"
                << "<prescribe <patient> <frequency>>,\n"
                << "<view_prescription>,\n"
                << "<quit>" << std::endl;
   }
   else
   {
      std::cout << "Please enter the command: \n"
                << "<lookup>,\n"
                << "<lookup <doctor>>,\n"
                << "<schedule <doctor> <start_time> <illness>>,\n"
                << "<cancel>,\n"
                << "<view_appointment>,\n"
                << "<view_prescription>,\n"
                << "<quit>" << std::endl;
   }
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

/**
 * @brief Entry point that starts up client code
 * @param argc Username
 * @param argv Password (raw)
 * @return N/A
 */
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
