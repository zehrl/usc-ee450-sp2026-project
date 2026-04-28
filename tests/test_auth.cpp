#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../authentication_server.cpp"

TEST_SUITE("AuthServer::loadUsers")
{
   TEST_CASE("loads all entries from users.txt - may change from testing")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      CHECK(s.userCount() == 54); // May change from testing
   }

   TEST_CASE("each entry has two non empty full hashes")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      for (size_t i = 0; i < s.userCount(); i++)
      {
         CHECK(s.getUser(i).userHash.length() == 64);
         CHECK(s.getUser(i).passHash.length() == 64);
      }
   }

   TEST_CASE("doctors appear in the user list")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      std::string drHouseHash = "e240520749f49de984d68370037de6124994c97333570f6fcf588cf6df54ebe0";
      bool found = false;
      for (size_t i = 0; i < s.userCount(); i++)
         if (s.getUser(i).userHash == drHouseHash)
         {
            found = true;
            break;
         }
      CHECK(found);
   }
}

TEST_SUITE("AuthServer::authenticate")
{
   TEST_CASE("succeeds with a matching username+password hash pair")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      CHECK(s.authenticate(s.getUser(0).userHash, s.getUser(0).passHash) == true);
   }

   TEST_CASE("fails when the password hash is wrong")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      CHECK(s.authenticate(s.getUser(0).userHash, "0000000000000000000000000000000000000000000000000000000000000000") == false);
   }

   TEST_CASE("fails when the username hash is unknown")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      CHECK(s.authenticate("awfjeio;awefjio;awefjio;awefjio;awefjio;awefjio;awefjio;",
                           "awfjeio;awefjio;awefjio;awefjio;awefjio;awefjio;awefjio;") == false);
   }

   TEST_CASE("fails when credentials are swapped")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      CHECK(s.authenticate(s.getUser(0).passHash, s.getUser(0).userHash) == false);
   }

   TEST_CASE("authenticate works for all entries in users.txt")
   {
      AuthServer s;
      s.loadUsers("users.txt");
      for (size_t i = 0; i < s.userCount(); i++)
         CHECK(s.authenticate(s.getUser(i).userHash, s.getUser(i).passHash) == true);
   }
}
