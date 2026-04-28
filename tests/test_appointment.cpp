#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "../appointment_server.cpp"

// Known booked entries from data/appointments.txt (seed data)
static const std::string HOUSE_BOOKED_HASH =
    "5bae2874f10f5aa70d8c2a172e51c599d06767c6a1e7f95ce99919c911be44d0"; // 11:00 Dr.House
static const std::string STRANGE_BOOKED_HASH =
    "094a026efa74a08c4fa311e4ca7c582442013ece03c7539a2e932dd0e8e57001"; // 13:00 Dr.Strange
static const std::string DOLITTLE_BOOKED_HASH =
    "4c38c8225d53388d7632bc459b9a0447d22cb67b9e590d19ac6d4530f818a367"; // 15:00 Dr.Dolittle

TEST_SUITE("AppointmentServer::loadAppointments")
{
   TEST_CASE("loads all 3 doctors")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.appointmentData.size() == 3);
   }

   TEST_CASE("each doctor has exactly 8 time slots")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.appointmentData["Dr.House"].size() == 8);
      CHECK(s.appointmentData["Dr.Strange"].size() == 8);
      CHECK(s.appointmentData["Dr.Dolittle"].size() == 8);
   }
}

TEST_SUITE("AppointmentServer::isSlotAvailable")
{
   TEST_CASE("pre-booked slots are not available")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.isSlotAvailable("Dr.House", "11:00"));
      CHECK_FALSE(s.isSlotAvailable("Dr.Strange", "13:00"));
      CHECK_FALSE(s.isSlotAvailable("Dr.Dolittle", "15:00"));
   }

   TEST_CASE("empty slots are available")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.isSlotAvailable("Dr.House", "9:00"));
      CHECK(s.isSlotAvailable("Dr.Strange", "9:00"));
      CHECK(s.isSlotAvailable("Dr.Dolittle", "9:00"));
   }

   TEST_CASE("returns false for a doctor not in the system")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.isSlotAvailable("Dr.Nobody", "9:00"));
   }
}

TEST_SUITE("AppointmentServer::getAvailableSlots")
{
   TEST_CASE("returns 7 slots for Dr.House (one pre-booked)")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getAvailableSlots("Dr.House").size() == 7);
   }

   TEST_CASE("returns empty list for unknown doctor")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getAvailableSlots("Dr.Nobody").empty());
   }
}

TEST_SUITE("AppointmentServer::bookSlot")
{
   TEST_CASE("booking a free slot succeeds and marks it unavailable")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      bool ok = s.bookSlot("Dr.House", "9:00", "newhash64charxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "Flu");
      CHECK(ok == true);
      CHECK_FALSE(s.isSlotAvailable("Dr.House", "9:00"));
   }

   TEST_CASE("booking an already-taken slot fails")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.bookSlot("Dr.House", "11:00", "someotherhash", "Fever"));
   }

   TEST_CASE("booking a slot for an unknown doctor fails")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.bookSlot("Dr.Nobody", "9:00", "anyhash", "Flu"));
   }

   TEST_CASE("booking a slot outside 9:00-16:00 fails")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.bookSlot("Dr.House", "17:00", "anyhash", "Flu"));
      CHECK_FALSE(s.bookSlot("Dr.House", "8:00", "anyhash", "Flu"));
   }
}

TEST_SUITE("AppointmentServer::cancelSlot")
{
   TEST_CASE("cancelling a booked slot frees it")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.isSlotAvailable("Dr.House", "11:00"));
      bool ok = s.cancelSlot(HOUSE_BOOKED_HASH);
      CHECK(ok == true);
      CHECK(s.isSlotAvailable("Dr.House", "11:00"));
   }

   TEST_CASE("cancelling a hash not in the system fails")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK_FALSE(s.cancelSlot("0000000000000000000000000000000000000000000000000000000000000000"));
   }
}

TEST_SUITE("AppointmentServer::getPatientIllness")
{
   TEST_CASE("returns illness for a booked patient")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getPatientIllness(HOUSE_BOOKED_HASH) == "Flu");
      CHECK(s.getPatientIllness(STRANGE_BOOKED_HASH) == "Allergies");
      CHECK(s.getPatientIllness(DOLITTLE_BOOKED_HASH) == "Headache");
   }

   TEST_CASE("returns empty string for unknown hash")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getPatientIllness("0000000000000000000000000000000000000000000000000000000000000000") == "");
   }

   TEST_CASE("does not clear the slot after retrieval")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      s.getPatientIllness(HOUSE_BOOKED_HASH);
      CHECK_FALSE(s.isSlotAvailable("Dr.House", "11:00")); // slot still booked
   }
}

TEST_SUITE("AppointmentServer::getPatientDoctor + getPatientTime")
{
   TEST_CASE("returns correct doctor for booked patient")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getPatientDoctor(HOUSE_BOOKED_HASH) == "Dr.House");
      CHECK(s.getPatientDoctor(STRANGE_BOOKED_HASH) == "Dr.Strange");
      CHECK(s.getPatientDoctor(DOLITTLE_BOOKED_HASH) == "Dr.Dolittle");
   }

   TEST_CASE("returns correct time for booked patient")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getPatientTime(HOUSE_BOOKED_HASH) == "11:00");
      CHECK(s.getPatientTime(STRANGE_BOOKED_HASH) == "13:00");
      CHECK(s.getPatientTime(DOLITTLE_BOOKED_HASH) == "15:00");
   }

   TEST_CASE("returns empty string for unknown hash")
   {
      AppointmentServer s;
      s.loadAppointments("appointments.txt");
      CHECK(s.getPatientDoctor("0000000000000000000000000000000000000000000000000000000000000000") == "");
      CHECK(s.getPatientTime("0000000000000000000000000000000000000000000000000000000000000000") == "");
   }
}
