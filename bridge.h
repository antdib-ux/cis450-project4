#ifndef BRIDGE_H
#define BRIDGE_H

#include <semaphore.h>

// Use named values so directions and policies are easy to read in the code.
#define VEHICLE_COUNT 13
#define BRIDGE_LIMIT 3
#define NORTH 0
#define SOUTH 1
#define EMPTY -1
#define GREEDY 0
#define FCFS 1

// Change this to 1 only after both bridge functions and the helper are complete.
// This keeps the unfinished starter from producing unsafe crossing output.
#define BRIDGE_LOGIC_READY 0

// These are the individual vehicle fields required by the project document.
typedef struct _vehicle_info {
    int id;
    int dir;
    double inter_arrival_t;
} vehicle_info;

// Define shared values once in onelanebridge.c; extern lets other code use them.
// bridge_count includes reserved spots for vehicles signaled but not yet running.
extern vehicle_info vehicles[VEHICLE_COUNT];
extern int crossing_policy;
extern int bridge_count;
extern int bridge_dir;
extern int waiting_count[2];
extern int waiting_queue[VEHICLE_COUNT];
extern int queue_count;

// state_lock protects all bridge and queue changes. Each vehicle waits on its
// own gate so FCFS never depends on which thread a shared semaphore wakes up.
extern sem_t state_lock;
extern sem_t vehicle_gate[VEHICLE_COUNT];

// Keep the three project functions and thread entry point consistent for everyone.
void *VehicleAction(void *arg);
void ArriveBridge(vehicle_info *vehicle);
void CrossBridge(vehicle_info *vehicle);
void ExitBridge(vehicle_info *vehicle);

// Layth owns this shared admission helper. Call it with state_lock held.
// It reserves safe spots and signals eligible vehicles without blocking.
void AdmitWaitingVehicles(void);

#endif
