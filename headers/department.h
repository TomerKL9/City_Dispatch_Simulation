#ifndef DISPATCH_TASKS_H
#define DISPATCH_TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h> // For printf (assuming you have semi-hosting enabled)
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

/*!
 * @brief Structure to track mission progress for different departments.
 */
struct MissionProgress {
    int completed;			// Number of missions completed.
    int partiallyCompleted;	// Number of missions partially completed.
    int missed;				// Number of missions missed.
};
extern struct MissionProgress POLICE;
extern struct MissionProgress FIRE;
extern struct MissionProgress AMBULANCE;
extern struct MissionProgress CORONA;
extern uint32_t TIMEFORMISSION_MS;
extern int POLICE_DEPARTMENT;
extern int FIRE_DEPARTMENT;
extern int AMBULANCE_DEPARTMENT;
extern int CORONA_DEPARTMENT;
extern uint32_t Message_PRIORITY;
extern uint32_t Counting_PRIORITY;
extern uint32_t Event_PRIORITY;
extern uint32_t POLICE_PRIORITY;
extern uint32_t FIRE_PRIORITY;
extern uint32_t AMBULANCE_PRIORITY;
extern uint32_t CORONA_PRIORITY;
extern uint8_t maxVehiclesAvailableOnRoad;
extern uint8_t policeVehiclesAvailable;
extern uint8_t fireVehiclesAvailable;
extern uint8_t ambulanceVehiclesAvailable;
extern uint8_t coronaVehiclesAvailable;
extern volatile uint8_t ENDDEPFLAG;
extern QueueHandle_t policeQueue;
extern QueueHandle_t fireQueue;
extern QueueHandle_t ambulanceQueue;
extern QueueHandle_t coronaQueue;

extern SemaphoreHandle_t policeQueueMutex;
extern SemaphoreHandle_t fireQueueMutex;
extern SemaphoreHandle_t ambulanceQueueMutex;
extern SemaphoreHandle_t coronaQueueMutex;
extern SemaphoreHandle_t printMutex;
void createDepartments();
void policeTask(void *pvParameters);
void fireTask(void *pvParameters);
void ambulanceTask(void *pvParameters);
void coronaTask(void *pvParameters);
void countingTaskPolice();
void countingTaskAmbulance();
void countingTaskFire();
void countingTaskCorona();
#endif /* DISPATCH_TASKS_H */
