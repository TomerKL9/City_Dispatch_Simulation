#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"


extern uint32_t Message_PRIORITY;
extern uint32_t Event_PRIORITY;

extern uint32_t RANDOMDELAYDISPATCHER;
extern uint8_t MAXCARPERMISSION;
extern int POLICE_DEPARTMENT;
extern int FIRE_DEPARTMENT;
extern int AMBULANCE_DEPARTMENT;
extern int CORONA_DEPARTMENT;

extern volatile uint8_t ENDDISFLAG;

extern QueueHandle_t policeQueue;
extern QueueHandle_t fireQueue;
extern QueueHandle_t ambulanceQueue;
extern QueueHandle_t coronaQueue;

extern SemaphoreHandle_t policeQueueMutex;
extern SemaphoreHandle_t fireQueueMutex;
extern SemaphoreHandle_t ambulanceQueueMutex;
extern SemaphoreHandle_t coronaQueueMutex;
extern SemaphoreHandle_t printMutex;

void RandomEvent(void *pvParameters);
void createDispatcherTasks(void);


#endif /* DISPATCHER_H */
