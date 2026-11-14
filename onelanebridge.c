#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "bridge.h"
#include "common.h"
#include "common_threads.h"

// Vehicle directions and delays before creating each thread.
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

// Shared bridge state. Count includes spots reserved for signaled vehicles.
int crossing_policy = GREEDY;
int bridge_count = 0;
int bridge_dir = EMPTY;
int waiting_count[2] = {0, 0};
int waiting_queue[VEHICLE_COUNT];
int queue_count = 0;
sem_t state_lock;
sem_t vehicle_gate[VEHICLE_COUNT];

// Use the same starting time for every crossing message.
static double start_time;

// Each vehicle waits for entry, crosses, then releases its spot.
void *VehicleAction(void *arg) {
    vehicle_info *vehicle = (vehicle_info *) arg;
    ArriveBridge(vehicle);
    CrossBridge(vehicle);
    ExitBridge(vehicle);
    return NULL;
}

// Wait until this vehicle is allowed to enter the bridge.
void ArriveBridge(vehicle_info *vehicle) {
    (void) vehicle;
    fprintf(stderr, "ArriveBridge() is not implemented yet.\n");
    exit(EXIT_FAILURE);
}

// Caller holds state_lock; reserve safe spots before signaling waiting vehicles.
void AdmitWaitingVehicles(void) {
    fprintf(stderr, "AdmitWaitingVehicles() is not implemented yet.\n");
    exit(EXIT_FAILURE);
}

// Print the crossing time and simulate five seconds without holding state_lock.
void CrossBridge(vehicle_info *vehicle) {
    double elapsed = GetTime() - start_time;
    char direction = vehicle->dir == NORTH ? 'N' : 'S';
    printf("Time %4.1f: Vehicle %2d (%c) crossing\n",
           elapsed, vehicle->id, direction);
    fflush(stdout);
    Spin(5);
}

// Release this vehicle's spot and admit eligible waiting vehicles.
void ExitBridge(vehicle_info *vehicle) {

    //"use" vehicle pointer by casting it to void
    //prevent defined but not used warning
    (void) vehicle;

    //blocking function that waits for semaphore to be made available before making changes to shared variables
    //when lock is available, hold it so no other thread has access to the shared variables
    Sem_wait(&state_lock);

    //one car exited; decrement amount on bridge
    bridge_count--;

    //if count reaches 0, set the direction to empty; flag raised for other vehicles
    if (bridge_count <= 0) {
        bridge_dir = EMPTY;
    }

    //signal to waiting vehicles that they can enter
    //do this before releasing lock, so queued vehicles' spots are reserved first (prevent race condition)
    AdmitWaitingVehicles();

    //release semaphore; no longer needed by function
    Sem_post(&state_lock);
}


// Set up bridge, run vehicle threads, clean up at the end.
int main(int argc, char *argv[]) {
   pthread_t threads[VEHICLE_COUNT];

   if (argc !=2 ) {
        fprintf(stderr, "Usage: %s greedy|fcfs\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "greedy") == 0) {
        crossing_policy = GREEDY;
    } else if (strcmp(argv[1], "fcfs") == 0) {
        crossing_policy = FCFS;
    } else {
        fprintf(stderr, "Usage: %s greedy|fcfs\n", argv[0]);
        return EXIT_FAILURE;
    }


    // Allow one thread to update bridge state
    //Vehicle gates start closed
    Sem_init(&state_lock, 1);
    for (int i = 0; i < VEHICLE_COUNT; i++){
        Sem_init(&vehicle_gate[i], 0);
    }

    // After arrival delay, create each vehicle
    start_time = GetTime();
    for (int i = 0; i < VEHICLE_COUNT; i++) {
        usleep((useconds_t) (vehicles[i].inter_arrival_t * 1000000.0));
        Pthread_create(&threads[i], NULL, VehicleAction, &vehicles[i]);
    }
    for (int i = 0; i < VEHICLE_COUNT; i++) {
    // Wait for this vehicle before cleaning up shared resources.
        Pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < VEHICLE_COUNT; i++) {
        // Vehicle gates are no longer needed after all threads finish.
        if (sem_destroy(&vehicle_gate[i]) != 0) {
            perror("sem_destroy");
            return EXIT_FAILURE;
        }
    }
    // Release the shared state semaphore after all threads finish.
    if (sem_destroy(&state_lock) != 0) {
        perror("sem_destroy");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
