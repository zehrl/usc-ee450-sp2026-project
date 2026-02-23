CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRC_DIR = src
BUILD_DIR = build

CLIENT = client
HOSPITAL = hospital_server
AUTH = authentication_server
APPT = appointment_server
PRES = prescription_server

$(CLIENT):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/client.cpp -o $(BUILD_DIR)/$(CLIENT)

$(HOSPITAL):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/hospital_server.cpp -o $(BUILD_DIR)/$(HOSPITAL)

$(AUTH):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/authentication_server.cpp -o $(BUILD_DIR)/$(AUTH)

$(APPT):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/appointment_server.cpp -o $(BUILD_DIR)/$(APPT)

$(PRES):
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/prescription_server.cpp -o $(BUILD_DIR)/$(PRES)

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

clean:
	rm -rf $(BUILD_DIR)/*