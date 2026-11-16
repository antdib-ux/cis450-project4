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

//Wait until this vehicle is allowed to enter the bridge
//Shared Variables:
//                  READ:  state_lock, vehicles (id and dir), queue_count
//                  WRITE: state_lock, waiting_queue, waiting_count, vehicle_gate
void ArriveBridge(vehicle_info *vehicle) {

    //blocking function that waits for semaphore to be made available before making changes to shared variables
    //when lock is available, hold it so no other thread has access to the shared variables
    Sem_wait(&state_lock);

    //send vehicle to the back of the queue
    //queue in FCFS order by default
    waiting_queue[queue_count++] = vehicle->id;

    //increment this direction's waiting count for GREEDY
    waiting_count[vehicle->dir]++;

    //admit next awaiting vehicle in FCFS or GREEDY
    //do this before releasing lock, so queued vehicles' spots are reserved first (prevent race condition)
    AdmitWaitingVehicles();

    //release semaphore; no longer needed by function
    Sem_post(&state_lock);

    //blocking function now checks for this vehicle's gate semaphore to be made available
    //when available, vehicle logic will continue in VehicleAction()
    Sem_wait(&vehicle_gate[vehicle->id]);
}

// Caller holds state_lock; reserve safe spots before signaling waiting vehicles.
// Scans waiting_queue (in arrival order) and admits every vehicle that is allowed to enter
// right now. Each admitted vehicle is reserved in its spot, removed from the queue, and open its gate.
void AdmitWaitingVehicles(void) {
    int i = 0;

    // Stop as soon as the bridge is full or when every waiting vehicle has been checked.
    while (i < queue_count && bridge_count < BRIDGE_LIMIT) {
        int id = waiting_queue[i];
        int dir = vehicles[id].dir;

        // Allow vehicles if the bridge is empty or already flowing its way.
        if (bridge_dir == EMPTY || bridge_dir == dir) {

            // Reserve the spot now so two callers can't both admit into the last slot.
            // This is why bridge_count includes vehicles that are signaled but haven't started crossing.
            bridge_dir = dir;
            bridge_count++;
            waiting_count[dir]--;

            // Remove this vehicle from the queue by shifting later entries left.
            for (int j = i; j < queue_count - 1; j++) {
                waiting_queue[j] = waiting_queue[j + 1];
            }
            queue_count--;

            // Open this vehicle's gate so its Sem_wait in ArriveBridge returns.
            Sem_post(&vehicle_gate[id]);

            // Don't advance i, the next vehicle just shifted into slot i.
        }
        else if (crossing_policy == FCFS) {
            // FCFS: the oldest waiting vehicle is blocked by direction,
            // so nobody behind it may pass, stop scanning.
            break;
        }
        else {
            // GREEDY: skip over the opposite direction vehicle and keep looking
            // for same direction vehicles further back.
            i++;
        }
    }
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
//Shared Variables:
//                  READ:  state_lock, bridge_count
//                  WRITE: state_lock, bridge_count, bridge_dir
void ExitBridge(vehicle_info *vehicle) {

    //"use" vehicle pointer by casting it to void
    //prevent defined but not used warning
    (void) vehicle;

    //blocking function that waits for semaphore to be made available before making changes to shared variables
    //when lock is available, hold it so no other thread has access to the shared variables
    Sem_wait(&state_lock);

    //one car exited; decrement amount on bridge
    bridge_count--;

    //if count reaches 0, set the direction to empty for FCS or GREEDY
    if (bridge_count <= 0) {
        bridge_dir = EMPTY;
    }

    //admit next awaiting vehicle in FCFS or GREEDY; same purpose as ArriveBridge()
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
