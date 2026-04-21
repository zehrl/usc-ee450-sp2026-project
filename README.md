# EE450 – Distributed Hospital Network System

## Author

Logan Zehr — EE450 Spring 2026, Section 2

## Overview

A distributed hospital management system built in C++ using UNIX sockets from scratch. Five processes communicate over TCP and UDP on localhost: a client, a central hospital server, and three backend servers (authentication, appointment, prescription).

## Architecture

```
Client ──TCP──► HospitalServer ──UDP──► AuthServer
                               ──UDP──► AppointmentServer
                               ──UDP──► PrescriptionServer
```

- **Client** connects to HospitalServer over a persistent TCP connection
- **HospitalServer** is the hub — handles all client requests, forwards to backends via UDP
- **Backend servers** (Auth, Appointment, Prescription) each listen on a UDP port and reply directly to HospitalServer

### Port Assignments

| Process               | Protocol | Port  |
|-----------------------|----------|-------|
| authentication_server | UDP      | 21570 |
| prescription_server   | UDP      | 22570 |
| appointment_server    | UDP      | 23570 |
| hospital_server       | UDP      | 25570 |
| hospital_server       | TCP      | 26570 |

### Data Files (`data/`)

| File               | Owner                | Contents                          |
|--------------------|----------------------|-----------------------------------|
| `users.txt`        | AuthServer           | userHash + passHash per line      |
| `hospital.txt`     | HospitalServer       | Doctor list + illness→treatment map |
| `appointments.txt` | AppointmentServer    | Doctor schedules, 8 slots each    |
| `prescriptions.txt`| PrescriptionServer   | Patient prescriptions             |

## Design Decisions

- **No `getaddrinfo`** — all sockets use `sockaddr_in` + `htons(port)` + `inet_addr("127.0.0.1")` directly
- **Constructor vs `boot()`** — constructors initialize data only (no sockets), enabling unit tests to instantiate classes without networking
- **`#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`** guard in each `.cpp` so tests can `#include` the source directly without duplicate `main`
- **`fork()` per client** in HospitalServer — parent immediately accepts next connection; child handles the session
- **SHA-256** via `third_party/sha256.h` (LekKit/sha256) — all credentials stored and transmitted as 64-char hex strings; on-screen display uses last 5 chars (`hash_suffix`)
- **Fixed-size `Message` struct** for all UDP communication — safe to pass directly to `sendto`/`recvfrom`

## Requirements

- g++ with C++17 support (`-std=c++17`)
- Make
- POSIX sockets (Linux/macOS)
- Graded on Ubuntu 20.04

## Build & Run

```bash
# Build all binaries into build/
make all

# Start all servers (background)
make run

# Run client
./build/client <username> <password>

# Stop all servers
make stop

# Clean build artifacts
make clean
```

Binaries are written to `build/`. Run servers before starting the client.

## Testing

Unit tests use [doctest](https://github.com/doctest/doctest) (header-only, in `third_party/`). Tests are **not included in the submission tarball**.

```bash
make test           # run all 5 test suites
make test_sha256    # SHA-256 helpers
make test_auth      # AuthServer logic
make test_hospital  # HospitalServer logic
make test_appointment   # AppointmentServer slot logic
make test_prescription  # PrescriptionServer logic
```

Each test suite compiles the corresponding `.cpp` directly and exercises data logic only — no sockets needed.

### Test status

| Suite             | Status  |
|-------------------|---------|
| test_sha256       | ✓ pass  |
| test_auth         | ✓ pass  |
| test_hospital     | ✓ pass  |
| test_appointment  | pending |
| test_prescription | pending |

## Development Environment

- Primary development: macOS (Apple Silicon)
- Remote testing: Ubuntu 22.04 via Multipass VM (`ee450-project`, static IP `192.168.65.2`)
- SSH config host alias: `ee450-project`

To start the remote VM:
```bash
multipass start ee450-project
```
