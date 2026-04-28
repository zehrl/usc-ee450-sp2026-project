#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../prescription_server.cpp"

TEST_SUITE("PrescriptionServer::loadPrescriptions")
{
   TEST_CASE("loads the 1 existing seed entry")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      CHECK(s.prescriptions.size() == 1);
   }

   TEST_CASE("seed entry has all four fields populated")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      auto &entry = s.prescriptions[0];
      CHECK_FALSE(entry.doctorName.empty());
      CHECK_FALSE(entry.patientHash.empty());
      CHECK_FALSE(entry.treatment.empty());
      CHECK_FALSE(entry.frequency.empty());
   }

   TEST_CASE("seed entry doctor is Dr.Strange")
   {
      // data/prescriptions.txt seed: Dr.Strange prescribed Ibuprofen Daily
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      CHECK(s.prescriptions[0].doctorName == "Dr.Strange");
      CHECK(s.prescriptions[0].treatment == "Ibuprofen");
      CHECK(s.prescriptions[0].frequency == "Daily");
   }
}

TEST_SUITE("PrescriptionServer::findPrescription")
{
   TEST_CASE("finds the existing seed entry by patientHash")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      std::string knownHash = s.prescriptions[0].patientHash;
      auto result = s.findPrescription(knownHash);
      CHECK(result.found == true);
      CHECK(result.doctorName == "Dr.Strange");
      CHECK(result.treatment == "Ibuprofen");
      CHECK(result.frequency == "Daily");
   }

   TEST_CASE("returns found=false for an unknown patientHash")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      auto result = s.findPrescription("0000000000000000000000000000000000000000000000000000000000000000");
      CHECK(result.found == false);
   }
}

TEST_SUITE("PrescriptionServer::addPrescription")
{
   TEST_CASE("newly added prescription is findable")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      s.addPrescription("Dr.House", "abcdef1234abcdef1234abcdef1234abcdef1234abcdef1234abcdef1234abcd",
                        "Antivirals", "Daily");
      auto result = s.findPrescription("abcdef1234abcdef1234abcdef1234abcdef1234abcdef1234abcdef1234abcd");
      CHECK(result.found == true);
      CHECK(result.doctorName == "Dr.House");
      CHECK(result.treatment == "Antivirals");
      CHECK(result.frequency == "Daily");
   }

   TEST_CASE("prescription count increases after add")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      size_t before = s.prescriptions.size();
      s.addPrescription("Dr.Dolittle", "deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef",
                        "Ibuprofen", "Weekly");
      CHECK(s.prescriptions.size() == before + 1);
   }

   TEST_CASE("all valid frequency values are accepted")
   {
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      // Spec defines exactly these four frequency strings
      for (const std::string freq : {"None", "Daily", "Bi-daily", "Weekly"})
      {
         bool ok = s.addPrescription("Dr.House", sha256hex(freq), "Antivirals", freq);
         CHECK(ok == true);
      }
   }
}

TEST_SUITE("PrescriptionServer round-trip")
{
   TEST_CASE("save then reload preserves all entries")
   {
      const std::string tmpFile = "/tmp/test_prescriptions.txt";
      PrescriptionServer s;
      s.loadPrescriptions("prescriptions.txt");
      s.addPrescription("Dr.House", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                        "Triptans", "Bi-daily");
      s.savePrescriptions(tmpFile);

      PrescriptionServer s2;
      s2.loadPrescriptions(tmpFile);
      CHECK(s2.prescriptions.size() == s.prescriptions.size());
      auto result = s2.findPrescription("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
      CHECK(result.found == true);
      CHECK(result.treatment == "Triptans");
      CHECK(result.frequency == "Bi-daily");
   }
}
