#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../hospital_server.cpp"

// Known doctor hashes from data/hospital.txt
static const std::string DR_HOUSE_HASH = "e240520749f49de984d68370037de6124994c97333570f6fcf588cf6df54ebe0";
static const std::string DR_STRANGE_HASH = "e1f194b3a63f56e8405971c57a575dd214c57e66ca95aeb1a1bc697762125cff";
static const std::string DR_DOLITTLE_HASH = "ca0f6270f6e50c47e88ebb76187f258ece219cc50bf0c0146aca3f3e20b7eacf";

TEST_SUITE("HospitalServer::loadHospital")
{
   TEST_CASE("loads exactly 3 doctors")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.getDoctorList().size() == 3);
   }

   TEST_CASE("getDoctorList returns the correct names")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      auto list = s.getDoctorList();
      CHECK(std::find(list.begin(), list.end(), "Dr.House") != list.end());
      CHECK(std::find(list.begin(), list.end(), "Dr.Strange") != list.end());
      CHECK(std::find(list.begin(), list.end(), "Dr.Dolittle") != list.end());
   }

   TEST_CASE("loads all 6 treatment mappings")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      // Verify every illness from the spec has a mapping
      CHECK(s.getTreatment("Flu") == "Antivirals");
      CHECK(s.getTreatment("StrepThroat") == "Antibiotics");
      CHECK(s.getTreatment("Headache") == "Ibuprofen");
      CHECK(s.getTreatment("Fever") == "Acetaminophen");
      CHECK(s.getTreatment("Allergies") == "Antihistamines");
      CHECK(s.getTreatment("Migraine") == "Triptans");
   }
}

TEST_SUITE("HospitalServer::isDoctor")
{
   TEST_CASE("returns true for all three doctor hashes")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.isDoctor(DR_HOUSE_HASH) == true);
      CHECK(s.isDoctor(DR_STRANGE_HASH) == true);
      CHECK(s.isDoctor(DR_DOLITTLE_HASH) == true);
   }

   TEST_CASE("returns false for a non-doctor hash")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      // Use a known patient hash (first entry in users.txt that is not a doctor)
      CHECK(s.isDoctor("c255e45d89ec8bc70272aaa951956a4ce697a6e1a6edccdbfb12491dae84ae50") == false);
   }

   TEST_CASE("returns false for an all-zeros hash")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.isDoctor("0000000000000000000000000000000000000000000000000000000000000000") == false);
   }
}

TEST_SUITE("HospitalServer::getDoctorName")
{
   TEST_CASE("returns correct name for each doctor hash")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.getDoctorName(DR_HOUSE_HASH) == "Dr.House");
      CHECK(s.getDoctorName(DR_STRANGE_HASH) == "Dr.Strange");
      CHECK(s.getDoctorName(DR_DOLITTLE_HASH) == "Dr.Dolittle");
   }

   TEST_CASE("returns empty string for unknown hash")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.getDoctorName("0000000000000000000000000000000000000000000000000000000000000000") == "");
   }
}

TEST_SUITE("HospitalServer::getTreatment")
{
   TEST_CASE("returns empty string for an unknown illness")
   {
      HospitalServer s;
      s.loadHospital("hospital.txt");
      CHECK(s.getTreatment("UknownIllness") == "");
      CHECK(s.getTreatment("") == "");
      CHECK(s.getTreatment("flu") == ""); // case-sensitive
   }
}
