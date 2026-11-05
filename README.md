# CIS 450 Project 4: team starter

This is a proposed common setup for Anthony, Layth, and Jillian. It compiles,
but it is NOT a finished bridge simulation. Arrival, admission, and exit are
marked placeholders. Real crossing runs are disabled until those are complete.

## Ownership

| Member | Sections |
| --- | --- |
| Anthony | bridge.h declarations, shared definitions and initialization, main(), VehicleAction(), CrossBridge(), Makefile |
| Layth | ArriveBridge() and AdmitWaitingVehicles(), including Greedy and FCFS selection |
| Jillian | ExitBridge(), calling the shared admission helper after a departure |

All three functions remain in onelanebridge.c to keep the project small.
Use separate branches and edit your assigned functions to reduce merge conflicts.
The helper is a proposed addition to the split: Layth and Jillian should agree
on its contract before implementing their functions.

## Build and check in Ubuntu

```bash
make
make check
```

The check only confirms that the starter runs through semaphore initialization
and cleanup. It does not validate synchronization or crossing output.

After the team implements and reviews the three placeholders, change
BRIDGE_LOGIC_READY to 1 in bridge.h, rebuild, and run:

```bash
make
./onelanebridge greedy
./onelanebridge fcfs
```

## Shared names and starting values

| Name | Initial value | Meaning |
| --- | --- | --- |
| bridge_count | 0 | Admitted vehicles, including any with a reserved spot awaiting CPU time |
| bridge_dir | EMPTY (-1) | Direction of admitted vehicles; NORTH=0 and SOUTH=1 |
| crossing_policy | GREEDY or FCFS from command line | Which admission rules to use |
| waiting_count[2] | {0, 0} | Waiting northbound and southbound vehicles |
| waiting_queue[13] | Empty; queue_count=0 | Vehicle IDs in arrival order; valid entries are indices 0 through queue_count-1 |
| state_lock | Semaphore value 1 | Protects all state and queue updates |
| vehicle_gate[13] | All semaphore values 0 | Each vehicle's individual permission to cross |

The vehicles[] array stays available for looking up a queued ID's direction.
Array position equals vehicle ID for this assignment's supplied data.
Queue entries can be removed by shifting later entries left; no linked list is
needed for 13 cars. waiting_count can be derived from the queue, but is included
to match our proposed shared names; update it whenever a car joins or leaves it.

## Contract between arrival and exit

1. ArriveBridge locks state_lock, appends the vehicle ID to the queue, updates
   its waiting count, and calls AdmitWaitingVehicles.
2. ArriveBridge releases state_lock, then waits on vehicle_gate[vehicle->id].
   Always wait, even if the helper has already posted this gate.
3. AdmitWaitingVehicles runs ONLY while state_lock is held. It reserves each
   allowed spot BEFORE posting a vehicle gate, and never waits on a gate itself.
   It removes admitted IDs from the queue and updates waiting_count, bridge_count,
   and bridge_dir consistently. Each gate is posted once for that car.
4. ExitBridge locks state_lock, decrements bridge_count, clears bridge_dir if
   the bridge becomes empty, calls AdmitWaitingVehicles, and releases state_lock.
5. CrossBridge prints and calls the original OSTEP Spin(5) without holding
   state_lock. Both directions must support up to three simultaneous cars.

For Greedy, the helper may skip opposite-direction waiting vehicles to fill
available spots in the current direction. When empty, this proposal uses the
oldest queued car's direction. For FCFS, consider the queue front and never skip
an earlier waiting car. Consecutive same-direction cars may enter together, up
to three. Stop when the front car cannot safely enter.

The assignment's FCFS prose says no opposite-direction vehicles may be waiting,
but its sample admits southbound cars 2, 3, and 4 together at time 6 while
northbound cars are waiting. This proposed contract follows arrival order and
the sample's consecutive same-direction groups; confirm with the instructor if
the team needs clarification of that wording.

## Timing and verification

The supplied absolute arrival times are 0 through 12 seconds. Each array entry
stores the delay BEFORE creating that vehicle: 0 for vehicle 0, then 1 for each
remaining vehicle. The document's field comment describes a delay before the
next vehicle, so this local convention is explicit to avoid an off-by-one error.
The resulting arrival pattern is the same. CPU scheduling and usleep overhead
can cause small timestamp differences, especially at simultaneous arrivals/exits.
Do not hardcode the expected crossing output.

After integration, compare both policies with the assignment's output, check
that all 13 cars finish, and verify capacity, direction safety, and FCFS admission
order under the state lock. Check actual admission order rather than assuming
printf order proves FCFS: admitted threads can be scheduled in a different order.
Comment each block you add or modify with what it does and why.

## OSTEP reference files

common.h and common_threads.h are unmodified copies from the official OSTEP
repository. common.h supplies GetTime() and Spin(); common_threads.h supplies
the Pthread_* and Sem_* wrappers. Do not compile with -DNDEBUG: these original
wrappers use assert() around their operations.

- Readers/writers starting reference: https://github.com/remzi-arpacidusseau/ostep-code/blob/master/threads-sema/rwlock.c
- common.h: https://github.com/remzi-arpacidusseau/ostep-code/blob/master/include/common.h
- common_threads.h: https://github.com/remzi-arpacidusseau/ostep-code/blob/master/include/common_threads.h

This starter uses that example's protected-counter pattern and thread wrappers.
The bridge-specific admission and release logic still needs to be implemented.
