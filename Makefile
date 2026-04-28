CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
BUILD_DIR = build

CLIENT = client
HOSPITAL = hospital_server
AUTH = authentication_server
APPT = appointment_server
PRES = prescription_server

SHA256 = sha256.cpp

$(CLIENT):
	$(CXX) $(CXXFLAGS) -I . client.cpp $(SHA256) -o $(CLIENT)

$(HOSPITAL):
	$(CXX) $(CXXFLAGS) -I . hospital_server.cpp $(SHA256) -o $(HOSPITAL)

$(AUTH):
	$(CXX) $(CXXFLAGS) -I . authentication_server.cpp $(SHA256) -o $(AUTH)

$(APPT):
	$(CXX) $(CXXFLAGS) -I . appointment_server.cpp $(SHA256) -o $(APPT)

$(PRES):
	$(CXX) $(CXXFLAGS) -I . prescription_server.cpp $(SHA256) -o $(PRES)

mkbuilddir:
	mkdir -p $(BUILD_DIR)

all: $(CLIENT) $(HOSPITAL) $(AUTH) $(APPT) $(PRES)

stop:
	pkill hospital_server || true
	pkill authentication_server || true
	pkill appointment_server || true
	pkill prescription_server || true

reset:
	@echo "Data files are in the project root; no seed directory to restore from."

restart: stop reset all

clean:
	rm -f $(CLIENT) $(HOSPITAL) $(AUTH) $(APPT) $(PRES)
	rm -rf $(BUILD_DIR)

# ---------------------------------------------------------------------------
# doctest.h make macros for testing during development
# ---------------------------------------------------------------------------
TEST_FLAGS = -std=c++17 -g -Wall -I .

test_sha256: mkbuilddir tests/test_sha256.cpp sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_sha256.cpp sha256.cpp -o $(BUILD_DIR)/test_sha256
	./$(BUILD_DIR)/test_sha256

test_auth: mkbuilddir tests/test_auth.cpp sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_auth.cpp sha256.cpp -o $(BUILD_DIR)/test_auth
	./$(BUILD_DIR)/test_auth

test_hospital: mkbuilddir tests/test_hospital.cpp sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_hospital.cpp sha256.cpp -o $(BUILD_DIR)/test_hospital
	./$(BUILD_DIR)/test_hospital

test_appointment: mkbuilddir tests/test_appointment.cpp sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_appointment.cpp sha256.cpp -o $(BUILD_DIR)/test_appointment
	./$(BUILD_DIR)/test_appointment

test_prescription: mkbuilddir tests/test_prescription.cpp sha256.cpp
	$(CXX) $(TEST_FLAGS) tests/test_prescription.cpp sha256.cpp -o $(BUILD_DIR)/test_prescription
	./$(BUILD_DIR)/test_prescription

test: test_sha256 test_auth test_hospital test_appointment test_prescription
