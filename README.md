# CIS 450 Project 4: One-Lane Bridge

A C program using pthreads and semaphores to implement Greedy and FCFS
crossing policies. The bridge allows up to three vehicles traveling
in the same direction.

## Team Responsibilities

- Anthony: main(), VehicleAction(), CrossBridge(), headers, and Makefile
- Layth: AdmitWaitingVehicles()
- Jillian: ArriveBridge(), ExitBridge()

Use individual branches and pull requests for changes. Discuss changes
to shared variables or function interfaces before implementing them.

## Build

Run in Ubuntu:

```bash
make
```

## Current Status
Greedy and FCFS admission logic is implemented. Both policies were
tested with the provided 13-vehicle sequence, and all vehicles
completed crossing.

## Run

```bash
./onelanebridge greedy
./onelanebridge fcfs
```

