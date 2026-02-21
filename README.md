# EE450 – Distributed Hospital Network System

## Overview
This project implements a distributed hospital system in C++. The system consists of a client, hospital server, authentication server, appointment server, and prescription server communicating over TCP sockets. Built with Makefile for Ubuntu VM grading and managed with Git for version control.

## System Architecture
### Components
- Client
- Hospital Server
- Authentication Server
- Appointment Server
- Prescription Server

### Communication Model
- Transport protocol(s)
- Port usage
- Inter-server workflow

## Features
- User authentication (SHA-256)
- Appointment scheduling
- Appointment cancellation
- Prescription management
- Doctor/patient role handling


## Requirements
- Ubuntu (grading VM)
- g++ (C++17)
- Make

## Build Instructions
```
make
./hospital_server
./authentication_server
./appointment_server
./prescription_server
./client
```

## Data Files

users.txt
hospital.txt
appointments.txt
prescriptions.txt

- Design Decisions
- Architecture choices
- Data handling strategy
- Concurrency model

## Testing
- Manual test procedure
- Integration test flow
- Known Limitations
- Any constraints or edge cases

## Author

Logan Zehr
EE450 – Spring 2026, Section 2
