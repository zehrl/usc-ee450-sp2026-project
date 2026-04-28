# USC EE450 – Distributed Hospital Network System

## Author

Logan Zehr - EE450 Spring 2026, Section 2
USC ID: 5225-9935-70

## Overview

A distributed hospital management system built in C++ using UNIX sockets from scratch. Five processes communicate over TCP and UDP on localhost: a client, a central hospital server, and three backend servers (authentication, appointment, prescription).

## Requirements

- g++ with C++17 support (`-std=c++17`)
- Make
- Ubuntu 20.04
- multipass (optional for development using VM)

## Build & Run

```bash
# Build all binaries
make all

OR for individual builds

make {client | hospital_server | authentication_server | appointment_server | prescription_server}

# Run servers (do this first)
./{authentication_server | appointment_server | hospital_server | prescription_server}

# Run client
./client <username> <password>

# Stop all servers
make stop

# Macro: Stop servers, reset seed data, and compile all code
make restart
```

## Architecture

- **Client** connects to HospitalServer over a persistent TCP connection
- **HospitalServer** is the central server that handles all client requests, forwards to backends via UDP
- **Backend servers** Authentication, Appointment, and Prescription servers each listen on a UDP port and reply directly to HospitalServer

### Port Assignments

Ports follow the spec as following (using last 3 digits of USC ID - 570 -> port+XXX): 

| Server/Process        | Protocol | Port  |
|-----------------------|----------|-------|
| authentication_server | UDP      | 21570 |
| prescription_server   | UDP      | 22570 |
| appointment_server    | UDP      | 23570 |
| hospital_server (1 of 2)       | UDP      | 25570 |
| hospital_server (2 of 2)       | TCP      | 26570 |
| Client                | 2 TCPs   | -     |

## Design Decisions

### common.h

**TCP/UDP Message structure**

Inside of `include/common.h` is the Message data structure. This may be the messiest and hardest portion to understand. Below is the usage in all of the servers and their respective methods. By using a central message structure, it was easy to develop. Abstraction of this message for future iterations would certainly make the code easier to interprate.

```
struct Message
{
   char type[32];
   char field1[256];
   char field2[256];
   char field3[256];
   char field4[256];
   int status;
};
```

`type` = Type of command ("AUTH", "LOOKUP", "LOOKUP_DOC", "SCHEDULE", "VIEW_APPT", "CANCEL", "VIEW_APPTS", "PRESCRIBE", "VIEW_PRESCRIPTION")
`field1` = 1st identifier: userHash (AUTH/LOOKUP), doctorName (LOOKUP_DOC/SCHEDULE), patientHash (VIEW_APPT/CANCEL), patientSuffix
`field2` = 2nd parameter: passHash (AUTH), timeSlot (SCHEDULE), userHash (LOOKUP_DOC), "doctor" role flag (VIEW_PRESCRIPTION from doctor), or frequency (PRESCRIBE)
`field3` = 3rd parameter: illness (SCHEDULE/VIEW_APPT response), treatment (prescription response), or doctorHash (PRESCRIBE/VIEW_PRESCRIPTION from doctor)
`field4` = 4th parameter: patientHash (SCHEDULE request), frequency (PRESCRIBE via PrescriptionServer)
`status` = Result code: 0 = success, 1 = failure/not found, 2 = all slots free (for `LOOKUP_DOC` only)

**TCP/UDP Socket Programming helper methods**

Common.h holds the shared functionality for each source file/server. By combining UDP/TCP socket programming (ex. `makeTCPServerSocket`, `makeTCPClientSocket`, `makeUDPSocket`, `udpSend`, `udpRecv` etc.), it was much easier to develop the servers after the initial design of the first server.

### Booting and Running of Servers

Inside each server, you will notice the abstraction/separation of "booting" (designating ports/sockets) and "running" (handling the incoming/outgoing messages and then running the appropriate commands). This separation was realized necessary early because of how the testing suite works. By first testing internal getting/setting of data before adding socket programming, I was able to get the main logic completed rather quickly.

## Documentation

Each file uses [Doxygen](https://www.doxygen.nl/manual/docblocks.html) documentation for ease of use/readability.

## Development

### Testing

Unit tests use [doctest](https://github.com/doctest/doctest) (header-only, in `third_party/`). I chose this lightweight testing library because it resides on a single header file. C++ can be notoriously hard to develop and test on. This small set back of writing tests before and during development proved to be invaluable. For future reference, here is how to run the tests:

```bash
make test # run all 5 test suites
make test_auth # AuthServer logic
make test_hospital # HospitalServer logic
make test_appointment # AppointmentServer slot logic
make test_prescription # PrescriptionServer logic
make test_sha256 # SHA-256 helpers/sanity check
```

### Environment

- Remote testing: Ubuntu 20.04 via Multipass VM (`ee450-project`)
- SSH config host alias: `ee450-project`

To start the remote VM:
```bash
multipass start ee450-project
```
