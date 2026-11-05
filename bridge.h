#ifndef BRIDGE_H
#define BRIDGE_H

#include <semaphore.h>

#define VEHICLE_COUNT 13
#define BRIDGE_LIMIT 3
#define NORTH 0
#define SOUTH 1
#define EMPTY -1
#define GREEDY 0
#define FCFS 1



// Information passed to each vehicle thread.
typedef struct _vehicle_info {
    int id;
    int dir;
    double inter_arrival_t;
} vehicle_info;

// Shared state; bridge_count includes reserved crossing spots.
extern vehicle_info vehicles[VEHICLE_COUNT];
extern int crossing_policy;
extern int bridge_count;
extern int bridge_dir;
extern int waiting_count[2];
extern int waiting_queue[VEHICLE_COUNT];
extern int queue_count;

// Protect bridge state and give each vehicle its own entry signal.
extern sem_t state_lock;
extern sem_t vehicle_gate[VEHICLE_COUNT];

void *VehicleAction(void *arg);
void ArriveBridge(vehicle_info *vehicle);
void CrossBridge(vehicle_info *vehicle);
void ExitBridge(vehicle_info *vehicle);

// Call with state_lock held; this helper must not block.
void AdmitWaitingVehicles(void);

#endif
