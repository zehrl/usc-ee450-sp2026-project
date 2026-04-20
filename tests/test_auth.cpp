// Depends on: src/authentication_server.cpp, include/common.h, third_party/sha256.cpp
// Compile via: make test_auth
//
// NOTE: AuthServer::users must be declared public (or add a getUserCount()
// method) for the size check below to compile.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../third_party/doctest.h"
#include "../src/authentication_server.cpp"

TEST_SUITE("AuthServer::loadUsers") {
    TEST_CASE("loads all 53 entries from users.txt") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        CHECK(s.users.size() == 53);
    }

    TEST_CASE("each entry has two non-empty 64-char hashes") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        for (const auto& [uHash, pHash] : s.users) {
            CHECK(uHash.length() == 64);
            CHECK(pHash.length() == 64);
        }
    }

    TEST_CASE("doctors appear in the user list") {
        // Doctors are also users — their username hashes come from hospital.txt
        AuthServer s;
        s.loadUsers("data/users.txt");

        // Dr.House username hash (from hospital.txt line 2)
        std::string drHouseHash = "e240520749f49de984d68370037de6124994c97333570f6fcf588cf6df54ebe0";
        bool found = false;
        for (const auto& [uHash, pHash] : s.users)
            if (uHash == drHouseHash) { found = true; break; }
        CHECK(found);
    }
}

TEST_SUITE("AuthServer::authenticate") {
    TEST_CASE("succeeds with a matching username+password hash pair") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        // Use the first entry in the file — whatever it is, it must authenticate
        auto [uHash, pHash] = s.users[0];
        CHECK(s.authenticate(uHash, pHash) == true);
    }

    TEST_CASE("fails when the password hash is wrong") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        auto [uHash, _] = s.users[0];
        CHECK(s.authenticate(uHash, "0000000000000000000000000000000000000000000000000000000000000000") == false);
    }

    TEST_CASE("fails when the username hash is unknown") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        CHECK(s.authenticate("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
                              "deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef") == false);
    }

    TEST_CASE("fails when credentials are swapped") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        auto [uHash, pHash] = s.users[0];
        // passing password as username and vice versa should fail (unless they
        // happen to match, which is astronomically unlikely with SHA-256)
        CHECK(s.authenticate(pHash, uHash) == false);
    }

    TEST_CASE("authenticate works for all entries in users.txt") {
        AuthServer s;
        s.loadUsers("data/users.txt");
        for (const auto& [uHash, pHash] : s.users)
            CHECK(s.authenticate(uHash, pHash) == true);
    }
}
