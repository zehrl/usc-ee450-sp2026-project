/**
 * @file authentication_server.cpp
 * @brief Authentication server that interacts with the users database
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "common.h"

/**
 * @brief Authentication server that interacts with the users database
 */
class AuthServer
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
    * @brief Structure for user data
    */
   struct User
   {
      std::string userHash; // Full SH256 hash of username
      std::string passHash; // Full SH256 hash of password
   };

   /**
    * @brief Loads user data from database into memory
    * @param filepath File path of database .txt file
    */
   void loadUsers(const std::string &filepath = "users.txt");

   /**
    * @brief Checks if provided credentials matches any in the database
    * @param userHash // Full SH256 hash of username
    * @param passHash // Full SH256 hash of password
    * @return True - Success, False - Failed
    */
   bool authenticate(const std::string &userHash, const std::string &passHash);

   /**
    * @brief Returns how many users are in the database (used for testing)
    * @return Count of users
    */
   size_t userCount() const { return users.size(); }

   /**
    * @brief Index based getter for in memory users (used in testing)
    * @param i Index inside user vector
    * @return The User fo that index
    */
   User getUser(size_t i) const
   {
      return users[i];
   }

private:
   /**
    * @brief Handles auth requests
    * @param msg Message data contents (see include/common.h)
    * @param from Socket address in
    */
   void handleRequest(Message &msg, sockaddr_in &from);

   int sockFd = -1;
   std::vector<User> users;
};

bool AuthServer::authenticate(const std::string &userHash, const std::string &passHash)
{
   for (const auto &u : users)
   {
      if (u.userHash == userHash)
      {
         return u.passHash == passHash;
      }
   }

   return false;
}

void AuthServer::loadUsers(const std::string &filepath)
{
   std::ifstream file(filepath);
   std::string line, section;

   while (std::getline(file, line))
   {
      if (line.empty())
      {
         continue;
      }

      std::istringstream ss(line);
      AuthServer::User u;
      ss >> u.userHash >> u.passHash;
      users.push_back(u);
   }
}

void AuthServer::handleRequest(Message &msg, sockaddr_in &from)
{
   if (strcmp(msg.type, "AUTH") != 0)
      return;

   std::string suffix = hashSuffix(std::string(msg.field1));
   std::cout << "Authentication Server has received an authentication request for a user with hash suffix: " << suffix << "." << std::endl;

   bool ok = authenticate(msg.field1, msg.field2);
   msg.status = ok ? 0 : 1;

   if (ok)
      std::cout << "Authentication succeeded for a user with hash suffix: " << suffix << "." << std::endl;
   else
      std::cout << "Authentication failed for a user with hash suffix: " << suffix << "." << std::endl;

   std::cout << "The Authentication Server has sent the authentication result to the Hospital Server." << std::endl;

   udpSend(sockFd, msg, ntohs(from.sin_port));
}

void AuthServer::boot()
{
   loadUsers();
   sockFd = makeUDPSocket(PORT_AUTH);
   std::cout << "Authentication Server is up and running using UDP on port " << PORT_AUTH << "." << std::endl;
}

void AuthServer::run()
{
   Message msg{};
   sockaddr_in from{};
   while (true)
   {
      udpRecv(sockFd, msg, from);
      handleRequest(msg, from);
   }
}

// Bandaid fix - normally we would separate our header definitions and our main method
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN // Do not use main if we're using doctest

/**
 * @brief Entry point that starts up server
 * @return N/A
 */
int main()
{
   AuthServer s;
   s.boot();
   s.run();
}
#endif
