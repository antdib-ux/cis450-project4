# CIS 450 Project 4: One-Lane Bridge

A C program using pthreads and semaphores to implement Greedy and FCFS
crossing policies. The bridge allows up to three vehicles traveling
in the same direction.

## Team Responsibilities

- Anthony: main(), VehicleAction(), CrossBridge(), headers, and Makefile
- Layth: ArriveBridge()
- Jillian: ExitBridge()

Use individual branches and pull requests for changes. Discuss changes
to shared variables or function interfaces before implementing them.

## Build

Run in Ubuntu:

```bash
make
```

## Current Status

The framework is set up. Arrival, admission, and exit logic are unfinished,
so crossing runs are currently disabled. AdmitWaitingVehicles() is a
proposed shared helper for the team to review.

## Run After Integration

```bash
./onelanebridge greedy
./onelanebridge fcfs
```
