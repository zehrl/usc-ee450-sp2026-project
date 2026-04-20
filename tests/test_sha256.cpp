// Depends on: include/common.h, third_party/sha256.cpp
// Compile via: make test_sha256

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../third_party/doctest.h"
#include "../include/common.h"

TEST_SUITE("sha256hex") {
    // These three inputs and their expected hashes come directly from the
    // project spec (page 5), so they serve as ground-truth correctness checks.
    TEST_CASE("matches spec example #1") {
        CHECK(sha256hex("Welcome to EE450!") ==
              "2579794c502c6c6c9f48da90e606188cee7609e1889a2af8caa2eee00bb1fd9d");
    }
    TEST_CASE("matches spec example #2") {
        CHECK(sha256hex("199xyz@$") ==
              "7f7a9dc72b1ddaa311477624f64cbfc1b53caff50dcf16469d7e36fe155a4228");
    }
    TEST_CASE("matches spec example #3") {
        CHECK(sha256hex("0.27#&") ==
              "2f8aa685fdfd3d32a7f7efc7681cc4a816e39381334f05cc3b132dd5ca83c00e");
    }
    TEST_CASE("output is always 64 characters") {
        CHECK(sha256hex("Welcome to EE450!").length() == 64);
        CHECK(sha256hex("").length() == 64);
        CHECK(sha256hex("x").length() == 64);
    }
    TEST_CASE("hashing is case-sensitive") {
        CHECK(sha256hex("hello") != sha256hex("Hello"));
        CHECK(sha256hex("hello") != sha256hex("HELLO"));
    }
    TEST_CASE("same input always produces same output") {
        CHECK(sha256hex("EE450") == sha256hex("EE450"));
    }
}

TEST_SUITE("hashSuffix") {
    // The spec defines hash_suffix as the last 5 hex characters of a SHA-256
    // hash, used in all server on-screen messages instead of the full hash.
    TEST_CASE("returns last 5 chars of a full hash") {
        std::string hash = "2579794c502c6c6c9f48da90e606188cee7609e1889a2af8caa2eee00bb1fd9d";
        CHECK(hashSuffix(hash) == "1fd9d");
    }
    TEST_CASE("works on any 64-char hash") {
        CHECK(hashSuffix(sha256hex("Welcome to EE450!")) == "1fd9d");
        CHECK(hashSuffix(sha256hex("199xyz@$"))          == "a4228");
        CHECK(hashSuffix(sha256hex("0.27#&"))            == "3c00e");
    }
    TEST_CASE("composed: sha256hex then hashSuffix is consistent") {
        std::string input = "TestUser";
        std::string full  = sha256hex(input);
        CHECK(hashSuffix(full) == full.substr(59));
    }
}
