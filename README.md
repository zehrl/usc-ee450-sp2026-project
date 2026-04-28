# USC EE450: Distributed Hospital Network System

---

## Student/Author

Name: Logan Zehr
USC ID: 5225993570

## Summary of Work

In this project, I built a rudimentary, command line hospital management system used by patients and doctors. This project requires multiple terminals to run the given server files (on the same machine). It allows the client to view appointments, doctors, book appointments, and prescribe/view prescriptions. It follows hashing/HIPA requirements. This project was built using socket programming and C++.

---

## Code Files

`client.cpp` -  The client process (patients and doctors). This entry into the program accepts a username and password as CLI arguments. It connects to HospitalServer over TCP, authenticates, then presents a role specific flow based on the authenitcation (patient or doctor).

`hospital_server.cpp` - This is the server that interfaces with the client. It first listens for client TCP connections. After receiving a request, it forwards it to the appropriate backend server using UDP. Any response is sent back to the client. This server utilizes the hospital database .txt file.

`authentication_server.cpp` - Backend server responsible for verifying credentials. Utilizes users.txt and receives "AUTH" messages over UDP from HospitalServer. It then returns a pass/fail back to the hospital server.

`appointment_server.cpp` - Backend server that manages appointment time slots. Utilizes appointments.txt. This server handles `lookup`, `schedule`, `view appointment(s)`, and `cancel` client commands over UDP.

`prescription_server.cpp` - Backend server that manages prescriptions. Utilizes prescriptions.txt. This server handles `prescribe` and `view prescription` client commands over UDP.

`common.h`  - Shared header used by source code. This defines the `Message` data structure used for communication. All helper TCP/UDP methods are included as well (`makeTCPServerSocket`, `makeTCPClientSocket`, `makeUDPSocket`, `udpSend`, `udpRecv`). A small SH256 hashing helper method is in here as well.

---

## Message Format

All communication uses a shared `Message` structure. This is the messiest/hardest part of the code to understand:

```
struct Message {
    char type[32];
    char field1[256];
    char field2[256];
    char field3[256];
    char field4[256];
    int  status;
};
```

The `type` field selects the command; the numbered fields carry parameters whose meaning depends on the command type. Here is a formatted table to differentiate the meaning of Message based on the type field:

| type | field1 | field2 | field3 | field4 | status meaning |
|---|---|---|---|---|---|
| `AUTH` | userHash | passHash | — | — | 0 = ok, 1 = fail |
| `LOOKUP` | userHash | — | — | — | 0 = patient, 1 = doctor, 2 = not found |
| `LOOKUP_DOC` | doctorName | userHash | — | — | 0 = has free slots, 1 = no free slots, 2 = all free |
| `SCHEDULE` | doctorName | timeSlot | illness | patientHash | 0 = booked, 1 = conflict |
| `VIEW_APPT` | patientHash | — | illness | — | 0 = found, 1 = none |
| `CANCEL` | patientHash | timeSlot | — | — | 0 = cancelled, 1 = not found |
| `VIEW_APPTS` | patientHash | — | — | — | 0 = found, 1 = none |
| `PRESCRIBE` | patientSuffix | frequency | doctorHash | patientHash | 0 = ok, 1 = fail |
| `VIEW_PRESCRIPTION` | patientSuffix | "doctor" (if doctor) | doctorHash (if doctor) | — | 0 = found, 1 = none |


---

## Idiosyncrasies

None known. This project has been tested and everything appears to work as expected.

---

## Reused Code

The SHA256 code (`sha256.h` and `sha256.cpp`) was written by **LekKit**. This was provided in the project spec. Source: https://github.com/LekKit/sha256

---

## Ubuntu Version

Ubuntu 20.04 LTS for testing/development

---

## Build & Run

`sudo apt install build-essential` on Ubuntu server will download required make/g++ packages for project usage

```bash
# Build all binaries
make all

# Build individually
make client
make hospital_server
make authentication_server
make appointment_server
make prescription_server

# Run servers first (in separate terminals)
./authentication_server
./appointment_server
./prescription_server
./hospital_server

# Run client
./client <username> <password>

# Stop all servers
make stop
```

---

## Port Assignments

Ports use the last 3 digits of USC ID (570):

authentication_server - UDP 21570
prescription_server - UDP 22570
appointment_server - UDP 23570
hospital_server - UDP 25570, TCP 26570