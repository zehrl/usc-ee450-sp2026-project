#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../include/common.h"

class AuthServer
{
public:
   void boot();
   void run();

   // public for testing
   struct User
   {
      std::string userHash;
      std::string passHash;
   };

   void loadUsers(const std::string &filepath = "data/users.txt");
   bool authenticate(const std::string &userHash, const std::string &passHash);
   size_t userCount() const { return users.size(); }
   User getUser(size_t i) const { return users[i]; }

private:
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
   if (strcmp(msg.type, "AUTH") != 0) return;

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

int main()
{
   AuthServer s;
   s.boot();
   s.run();
}
#endif