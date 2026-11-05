// Starter framework: adapt the OSTEP readers/writers pattern to bridge state.
// Anthony owns setup and crossing; Layth owns arrival; Jillian owns exit.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "bridge.h"
#include "common.h"
#include "common_threads.h"

// Store the supplied directions in ID order. The first car arrives immediately;
// each later car arrives one second after the previous car is created.
vehicle_info vehicles[VEHICLE_COUNT] = {
    {0, NORTH, 0.0},
    {1, NORTH, 1.0},
    {2, SOUTH, 1.0},
    {3, SOUTH, 1.0},
    {4, SOUTH, 1.0},
    {5, NORTH, 1.0},
    {6, NORTH, 1.0},
    {7, SOUTH, 1.0},
    {8, SOUTH, 1.0},
    {9, NORTH, 1.0},
    {10, NORTH, 1.0},
    {11, NORTH, 1.0},
    {12, NORTH, 1.0}
};

// Start with an empty bridge and queue. Both policies use this same state,
// but choose waiting vehicles differently in AdmitWaitingVehicles().
int crossing_policy = GREEDY;
int bridge_count = 0;
int bridge_dir = EMPTY;
int waiting_count[2] = {0, 0};
int waiting_queue[VEHICLE_COUNT];
int queue_count = 0;
sem_t state_lock;
sem_t vehicle_gate[VEHICLE_COUNT];

// Record one starting time so every crossing line uses the same elapsed clock.
static double start_time;

// Anthony: each thread follows the exact arrival/crossing/exit sequence required.
void *VehicleAction(void *arg) {
    vehicle_info *vehicle = (vehicle_info *) arg;
    ArriveBridge(vehicle);
    CrossBridge(vehicle);
    ExitBridge(vehicle);
    return NULL;
}

// Layth: enqueue the ID and update waiting_count under state_lock, then call
// AdmitWaitingVehicles(). Release state_lock BEFORE waiting on this car's gate.
// Always wait on the gate, including when the helper admits this car immediately.
void ArriveBridge(vehicle_info *vehicle) {
    (void) vehicle;
    fprintf(stderr, "ArriveBridge() is not implemented yet.\n");
    exit(EXIT_FAILURE);
}

// Layth: caller holds state_lock. For each admitted car, remove its queue entry,
// decrement its waiting count, set bridge_dir, reserve a spot with bridge_count++,
// and post that car's gate. Never reserve more than BRIDGE_LIMIT spots.
// Greedy may skip opposite-direction cars to fill the current direction.
// FCFS must use the queue front; stop if that car cannot safely enter.
// When the bridge is empty, this proposal selects the oldest queued car's direction.
void AdmitWaitingVehicles(void) {
    fprintf(stderr, "AdmitWaitingVehicles() is not implemented yet.\n");
    exit(EXIT_FAILURE);
}

// Anthony: print when crossing starts and use the required OSTEP five-second
// Spin(). Do not hold state_lock during Spin(), so other cars can enter or exit.
void CrossBridge(vehicle_info *vehicle) {
    double elapsed = GetTime() - start_time;
    char direction = vehicle->dir == NORTH ? 'N' : 'S';
    printf("Time %4.1f: Vehicle %2d (%c) crossing\n",
           elapsed, vehicle->id, direction);
    fflush(stdout);
    Spin(5);
}

// Jillian: take state_lock, decrement bridge_count, and set bridge_dir to EMPTY
// if the count reaches zero. Call AdmitWaitingVehicles() before releasing the lock
// so waiting cars can use newly available spots under either crossing policy.
void ExitBridge(vehicle_info *vehicle) {
    (void) vehicle;
    fprintf(stderr, "ExitBridge() is not implemented yet.\n");
    exit(EXIT_FAILURE);
}

// Anthony: choose the policy, initialize semaphores, create all car threads at
// the supplied intervals, then join them before destroying the semaphores.
int main(int argc, char *argv[]) {
    pthread_t threads[VEHICLE_COUNT];
    int check_setup = argc == 2 && strcmp(argv[1], "--check") == 0;

    // Reject unknown options so a typo cannot silently select another policy.
    if (argc != 2 || (!check_setup && strcmp(argv[1], "greedy") != 0
                     && strcmp(argv[1], "fcfs") != 0)) {
        fprintf(stderr, "Usage: %s greedy|fcfs|--check\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "fcfs") == 0) {
        crossing_policy = FCFS;
    }

    // One thread may change bridge state at a time. Vehicle gates start closed
    // and open only when the admission helper has reserved a safe crossing spot.
    Sem_init(&state_lock, 1);
    for (int i = 0; i < VEHICLE_COUNT; i++) {
        Sem_init(&vehicle_gate[i], 0);
    }

    // Allow everyone to verify the starter builds and initializes before the
    // bridge logic exists. Real runs stay disabled until the team finishes it.
    int result = EXIT_SUCCESS;
    if (check_setup) {
        printf("Setup OK: %d vehicles, bridge limit %d, semaphores initialized.\n",
               VEHICLE_COUNT, BRIDGE_LIMIT);
        printf("Crossing logic still needs implementation and testing.\n");
    } else if (!BRIDGE_LOGIC_READY) {
        fprintf(stderr, "Starter only: finish ArriveBridge(), "
                "AdmitWaitingVehicles(), and ExitBridge(), then set "
                "BRIDGE_LOGIC_READY to 1 in bridge.h.\n");
        result = EXIT_FAILURE;
    } else {
        // Sleep before each creation: the array stores the delay BEFORE this car,
        // giving absolute arrivals 0, 1, 2, ... 12 seconds for the supplied pattern.
        start_time = GetTime();
        for (int i = 0; i < VEHICLE_COUNT; i++) {
            usleep((useconds_t) (vehicles[i].inter_arrival_t * 1000000.0));
            Pthread_create(&threads[i], NULL, VehicleAction, &vehicles[i]);
        }
        for (int i = 0; i < VEHICLE_COUNT; i++) {
            Pthread_join(threads[i], NULL);
        }
    }

    // Destroy shared semaphores only after all workers have finished using them.
    for (int i = 0; i < VEHICLE_COUNT; i++) {
        if (sem_destroy(&vehicle_gate[i]) != 0) {
            perror("sem_destroy");
            return EXIT_FAILURE;
        }
    }
    if (sem_destroy(&state_lock) != 0) {
        perror("sem_destroy");
        return EXIT_FAILURE;
    }
    return result;
}
