CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRC_DIR = src
BUILD_DIR = build

CLIENT = client
HOSPITAL = hospital_server
AUTH = authentication_server
APPT = appointment_server
PRES = prescription_server

SHA256 = third_party/sha256.cpp

$(CLIENT):
	$(CXX) $(CXXFLAGS) -I include -I third_party $(SRC_DIR)/client.cpp $(SHA256) -o $(BUILD_DIR)/$(CLIENT)

$(HOSPITAL):
	$(CXX) $(CXXFLAGS) -I include -I third_party $(SRC_DIR)/hospital_server.cpp $(SHA256) -o $(BUILD_DIR)/$(HOSPITAL)

$(AUTH):
	$(CXX) $(CXXFLAGS) -I include -I third_party $(SRC_DIR)/authentication_server.cpp $(SHA256) -o $(BUILD_DIR)/$(AUTH)

$(APPT):
	$(CXX) $(CXXFLAGS) -I include -I third_party $(SRC_DIR)/appointment_server.cpp $(SHA256) -o $(BUILD_DIR)/$(APPT)

$(PRES):
	$(CXX) $(CXXFLAGS) -I include -I third_party $(SRC_DIR)/prescription_server.cpp $(SHA256) -o $(BUILD_DIR)/$(PRES)

mkbuilddir:
	mkdir -p $(BUILD_DIR)

all: mkbuilddir $(CLIENT) $(HOSPITAL) $(AUTH) $(APPT) $(PRES)

run: all
	$(BUILD_DIR)/hospital_server &
	$(BUILD_DIR)/authentication_server &
	$(BUILD_DIR)/appointment_server &
	$(BUILD_DIR)/prescription_server &

stop:
	pkill hospital_server || true
	pkill authentication_server || true
	pkill appointment_server || true
	pkill prescription_server || true

reset:
	cp data/seed/appointments.txt data/appointments.txt
	cp data/seed/prescriptions.txt data/prescriptions.txt
	cp data/seed/users.txt data/users.txt
	cp data/seed/hospital.txt data/hospital.txt

clean:
	rm -rf $(BUILD_DIR)/*

# ---------------------------------------------------------------------------
# Test targets (uses doctest.h — NOT included in submission)
# Each test binary compiles the corresponding .cpp source directly so the
# tests can access all public members without a separate link step.
# Run all tests: make test
# ---------------------------------------------------------------------------
TEST_FLAGS = -std=c++17 -g -Wall -I include -I third_party

test_sha256: mkbuilddir tests/test_sha256.cpp third_party/sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_sha256.cpp third_party/sha256.cpp -o $(BUILD_DIR)/test_sha256
	./$(BUILD_DIR)/test_sha256

test_auth: mkbuilddir tests/test_auth.cpp third_party/sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_auth.cpp third_party/sha256.cpp -o $(BUILD_DIR)/test_auth
	./$(BUILD_DIR)/test_auth

test_hospital: mkbuilddir tests/test_hospital.cpp third_party/sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_hospital.cpp third_party/sha256.cpp -o $(BUILD_DIR)/test_hospital
	./$(BUILD_DIR)/test_hospital

test_appointment: mkbuilddir tests/test_appointment.cpp third_party/sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_appointment.cpp third_party/sha256.cpp -o $(BUILD_DIR)/test_appointment
	./$(BUILD_DIR)/test_appointment

test_prescription: mkbuilddir tests/test_prescription.cpp third_party/sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_prescription.cpp third_party/sha256.cpp -o $(BUILD_DIR)/test_prescription
	./$(BUILD_DIR)/test_prescription

test: reset test_sha256 test_auth test_hospital test_appointment test_prescription