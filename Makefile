CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRC_DIR = src
BUILD_DIR = build

CLIENT = client
HOSPITAL = hospital_server
AUTH = authentication_server
APPT = appointment_server
PRES = prescription_server

all: $(CLIENT) $(HOSPITAL) $(AUTH) $(APPT) $(PRES)

$(CLIENT):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/client.cpp -o $(CLIENT)

$(HOSPITAL):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/hospital_server.cpp -o $(HOSPITAL)

$(AUTH):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/authentication_server.cpp -o $(AUTH)

$(APPT):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/appointment_server.cpp -o $(APPT)

$(PRES):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/prescription_server.cpp -o $(PRES)

clean:
	rm -f $(CLIENT) $(HOSPITAL) $(AUTH) $(APPT) $(PRES)