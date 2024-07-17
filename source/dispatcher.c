#include "dispatcher.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"


/*!
 * @brief Generates random events and dispatches them to corresponding department queues.
 *
 * This function runs as a FreeRTOS task and generates random events at irregular intervals
 * between 1 to RANDOMDELAYDISPATCHER milliseconds. It then inserts the generated event into
 * the appropriate department queue (police, fire, ambulance, corona) after acquiring the
 * respective queue's mutex. It also prints the event details using a print mutex to ensure
 * thread-safe printing.
 *
 * @param pvParameters Pointer to task parameters.
 */
void RandomEvent(void *pvParameters) {
	while (!ENDDISFLAG) {
	    uint32_t delay = rand() % RANDOMDELAYDISPATCHER + 1; // Random number between 1 to 1000
	    int eventType = rand() % 4;//Random event type: 0 to 3 (representing police, fire, ambulance, corona)
	    uint8_t numCars = rand() % MAXCARPERMISSION + 1; // Random number of cars: 1 to 3
	    vTaskDelay(pdMS_TO_TICKS(delay));

	    switch (eventType) {
	               case 0:
	                   if (xSemaphoreTake(policeQueueMutex, portMAX_DELAY) == pdTRUE) {
	                       if (uxQueueSpacesAvailable(policeQueue) > 0) {
	                           xQueueSend(policeQueue, &numCars, portMAX_DELAY);
	                           if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
	                               PRINTF("Event generated: Police dispatched with %u vehicles.\n\r", numCars);
	                               xSemaphoreGive(printMutex);
	                           }
	                       } else {
	                           PRINTF("Error: Police queue is full.\n\r");
	                           for(;;){
	                           }
	                       }
	                       xSemaphoreGive(policeQueueMutex);
	                   }
	                   break;
	               case 1:
	                   if (xSemaphoreTake(fireQueueMutex, portMAX_DELAY) == pdTRUE) {
	                       if (uxQueueSpacesAvailable(fireQueue) > 0) {
	                           xQueueSend(fireQueue, &numCars, portMAX_DELAY);
	                           if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
	                               PRINTF("Event generated: Fire dispatched with %u vehicles.\n\r", numCars);
	                               xSemaphoreGive(printMutex);
	                           }
	                       } else {
	                           PRINTF("Error: Fire queue is full.\n\r");
	                           for(;;){
	                           }
	                       }
	                       xSemaphoreGive(fireQueueMutex);
	                   }
	                   break;
	               case 2:
	                   if (xSemaphoreTake(ambulanceQueueMutex, portMAX_DELAY) == pdTRUE) {
	                       if (uxQueueSpacesAvailable(ambulanceQueue) > 0) {
	                           xQueueSend(ambulanceQueue, &numCars, portMAX_DELAY);
	                           if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
	                               PRINTF("Event generated: Ambulance dispatched with %u vehicles.\n\r", numCars);
	                               xSemaphoreGive(printMutex);
	                           }
	                       } else {
	                           PRINTF("Error: Ambulance queue is full.\n\r", numCars);
	                           for(;;){
	                           }
	                       }
	                       xSemaphoreGive(ambulanceQueueMutex);
	                   }
	                   break;
	               case 3:
	                   if (xSemaphoreTake(coronaQueueMutex, portMAX_DELAY) == pdTRUE) {
	                       if (uxQueueSpacesAvailable(coronaQueue) > 0) {
	                           xQueueSend(coronaQueue, &numCars, portMAX_DELAY);
	                           if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
	                               PRINTF("Event generated: Corona dispatched with %u vehicles.\n\r", numCars);
	                               xSemaphoreGive(printMutex);
	                           }
	                       } else {
	                           PRINTF("Error: Corona queue is full.\n\r", numCars);
	                           for(;;){
	                           }
	                       }
	                       xSemaphoreGive(coronaQueueMutex);
	                   }
	                   break;
	               default:
	                   break;
	           }
	       }
	       vTaskSuspend(NULL);
	   }

/*!
 * @brief Creates and starts a task for generating random events.
 *
 * This function creates a FreeRTOS task named "RandomEvent" using `xTaskCreate`.
 * The task is responsible for generating random events within the application.
 */
void createDispatcherTasks(void) {
    xTaskCreate(RandomEvent, "RandomEvent", configMINIMAL_STACK_SIZE + 100, NULL, Event_PRIORITY, NULL);
}
