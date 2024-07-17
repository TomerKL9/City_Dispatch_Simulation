#include "department.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>


int SEMAPHOREDELAY = 100; // the delay for getting the semaphore.
int CHECKQUEUEDELAY = 200; // the delay for checking queue.

/*!
@Creates the main department task for the corresponding department.
*/
void createDepartments() {
	xTaskCreate(policeTask, "PoliceTask", configMINIMAL_STACK_SIZE*4, NULL, POLICE_PRIORITY, NULL);
	xTaskCreate(fireTask, "FireTask", configMINIMAL_STACK_SIZE*4, NULL, FIRE_PRIORITY, NULL);
	xTaskCreate(ambulanceTask, "AmbulanceTask", configMINIMAL_STACK_SIZE*4, NULL, AMBULANCE_PRIORITY, NULL);
	xTaskCreate(coronaTask, "coronaTask", configMINIMAL_STACK_SIZE*4, NULL, CORONA_PRIORITY, NULL);
}

/*!
 * @brief Main task for the police department.
 *
 * This function represents the main task for the police department in a simulation system.
 * It continuously checks for new tasks in the police queue, attempts to dispatch available
 * police vehicles to handle the tasks, and creates counting tasks accordingly. It also
 * prints status messages about the dispatching process and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void policeTask(void *pvParameters) {
    int receivedMissions = 0;
    char taskName[20];
    char message[200];
    uint8_t numCars;

    while (!ENDDEPFLAG) {
        // Attempt to take the mutex to protect critical section for accessing queue and policeVehiclesAvailable
        if (xSemaphoreTake(policeQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
            // Wait to receive number of cars from the queue
            if (xQueueReceive(policeQueue, &numCars, pdMS_TO_TICKS(SEMAPHOREDELAY))) {
                // Determine number of cars to send
                uint8_t carsToSend = (policeVehiclesAvailable >= numCars) ? numCars : policeVehiclesAvailable;


                // Print appropriate message based on availability
                if (policeVehiclesAvailable >= numCars) {
                    snprintf(message, sizeof(message), "%d cars dispatched, %d cars were available in POLICE_DEPARTMENT\r\n",
                             carsToSend, policeVehiclesAvailable);
                    POLICE.completed++;
                } else if (policeVehiclesAvailable != 0) {
                    snprintf(message, sizeof(message), "not enough resources, sending %d cars from %d POLICE_DEPARTMENT\r\n",
                             carsToSend, numCars);
                    POLICE.partiallyCompleted++;
                } else {
                    snprintf(message, sizeof(message), "no resources, can't send %d cars POLICE_DEPARTMENT\r\n",
                             numCars);
                    POLICE.missed++;
                }

                // Dispatch vehicles (simulate by decrementing available count)
			    policeVehiclesAvailable -= carsToSend;

                // Release the mutex protecting the vehicle count and queue
                xSemaphoreGive(policeQueueMutex);

                // Attempt to take the print mutex to protect printing
                if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
                    // Print the message directly (assuming PRINTF is a function that prints the message)
                    PRINTF(message);

                    // Release the print mutex
                    xSemaphoreGive(printMutex);
                } else {
                    // Print mutex take failed
                    snprintf(message, sizeof(message), "Error: Failed to take print mutex in policeTask\r\n");
                    PRINTF(message);
                }

                // Create countingTasks based on carsToSend
                for (int taskIndex = 0; taskIndex < carsToSend; ++taskIndex) {
                    // Increment receivedMissions only when creating a task
                    snprintf(taskName, sizeof(taskName), "countingTaskPOLICE%d", ++receivedMissions);
                    if (xTaskCreate(countingTaskPolice, taskName, configMINIMAL_STACK_SIZE * 2, NULL, POLICE_PRIORITY, NULL) != pdPASS) {
                        snprintf(message, sizeof(message), "Error: Failed to create task %s\r\n", taskName);
                        PRINTF(message);
                    }
                }
            } else {
                // Failed to receive from the queue, release the mutex
                xSemaphoreGive(policeQueueMutex);
//                snprintf(message, sizeof(message), "Nothing to receive from policeQueue in policeTask\r\n");
//                PRINTF(message);
            }
        } else {
            // Failed to take queue mutex, handle failure appropriately
            snprintf(message, sizeof(message), "Error: Failed to take policeQueueMutex in policeTask\r\n");
            PRINTF(message);
        }

        vTaskDelay(pdMS_TO_TICKS(CHECKQUEUEDELAY));
    }

    // Suspend the task if simulation ends
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS*configMAX_PRIORITIES));
    UBaseType_t policeQueueLength = uxQueueMessagesWaiting(policeQueue);
    snprintf(message, sizeof(message), "POLICE DEPARTMENT: %d missions completed, %d missions partially completed, %d missions missed, %d is waiting in queue\r\n",
             POLICE.completed, POLICE.partiallyCompleted, POLICE.missed, policeQueueLength);

    if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
        // Print the message directly (assuming PRINTF is a function that prints the message)
        PRINTF(message);

        // Release the print mutex
        xSemaphoreGive(printMutex);
    } else {
        // Handle print mutex take failure appropriately
        snprintf(message, sizeof(message), "Error: Failed to take print mutex for final message in policeTask\r\n");
        PRINTF(message);
    }

    vTaskSuspend(NULL);
}


/*!
 * @brief Main task for the fire department.
 *
 * This function represents the main task for the fire department in a simulation system.
 * It continuously checks for new tasks in the fire queue, attempts to dispatch available
 * fire vehicles to handle the tasks, and creates counting tasks accordingly. It also
 * prints status messages about the dispatching process and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void fireTask(void *pvParameters) {
    int receivedMissions = 0;
    char taskName[20];
    char message[200];
    uint8_t numTrucks;

    while (!ENDDEPFLAG) {
        // Attempt to take the mutex to protect critical section for accessing queue and fireVehiclesAvailable
        if (xSemaphoreTake(fireQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
            // Wait to receive number of trucks from the queue
            if (xQueueReceive(fireQueue, &numTrucks, pdMS_TO_TICKS(SEMAPHOREDELAY))) {
                // Determine number of trucks to send
                uint8_t trucksToSend = (fireVehiclesAvailable >= numTrucks) ? numTrucks : fireVehiclesAvailable;


                // Print appropriate message based on availability
                if (fireVehiclesAvailable >= numTrucks) {
                    snprintf(message, sizeof(message), "%d trucks dispatched, %d trucks were available in FIRE_DEPARTMENT\r\n",
                             trucksToSend, fireVehiclesAvailable);
                    FIRE.completed++;
                } else if (fireVehiclesAvailable != 0) {
                    snprintf(message, sizeof(message), "not enough resources, sending %d trucks from %d FIRE_DEPARTMENT\r\n",
                             trucksToSend, numTrucks);
                    FIRE.partiallyCompleted++;
                } else {
                    snprintf(message, sizeof(message), "insufficient resources, can't send %d trucks FIRE_DEPARTMENT\r\n",
                             numTrucks);
                    FIRE.missed++;
                }

                // Dispatch vehicles (simulate by decrementing available count)
                fireVehiclesAvailable -= trucksToSend;

                // Release the mutex protecting the vehicle count and queue
                xSemaphoreGive(fireQueueMutex);

                // Attempt to take the print mutex to protect printing
                if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
                    // Print the message directly (assuming PRINTF is a function that prints the message)
                    PRINTF(message);

                    // Release the print mutex
                    xSemaphoreGive(printMutex);
                } else {
                    // Print mutex take failed
                    snprintf(message, sizeof(message), "Error: Failed to take print mutex in fireTask\r\n");
                    PRINTF(message);
                }

                // Create countingTasks based on trucksToSend
                for (int taskIndex = 0; taskIndex < trucksToSend; ++taskIndex) {
                    // Increment receivedMissions only when creating a task
                    snprintf(taskName, sizeof(taskName), "countingTaskFIRE%d", ++receivedMissions);
                    if (xTaskCreate(countingTaskFire, taskName, configMINIMAL_STACK_SIZE * 2, NULL, FIRE_PRIORITY, NULL) != pdPASS) {
                        snprintf(message, sizeof(message), "Error: Failed to create task %s\r\n", taskName);
                        PRINTF(message);
                    }
                }
            } else {
                // Failed to receive from the queue, release the mutex
                xSemaphoreGive(fireQueueMutex);
//                snprintf(message, sizeof(message), "Nothing to receive from fireQueue in fireTask\r\n");
//                PRINTF(message);
            }
        } else {
            // Failed to take queue mutex, handle failure appropriately
            snprintf(message, sizeof(message), "Error: Failed to take fireQueueMutex in fireTask\r\n");
            PRINTF(message);
        }

        vTaskDelay(pdMS_TO_TICKS(CHECKQUEUEDELAY));
    }

    // Suspend the task if simulation ends
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS*configMAX_PRIORITIES));
    UBaseType_t fireQueueLength = uxQueueMessagesWaiting(fireQueue);
    snprintf(message, sizeof(message), "FIRE DEPARTMENT: %d missions completed, %d missions partially completed, %d missions missed, %d is waiting in queue\r\n",
             FIRE.completed, FIRE.partiallyCompleted, FIRE.missed, fireQueueLength);

    if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
        // Print the message directly (assuming PRINTF is a function that prints the message)
        PRINTF(message);

        // Release the print mutex
        xSemaphoreGive(printMutex);
    } else {
        // Handle print mutex take failure appropriately
        snprintf(message, sizeof(message), "Error: Failed to take print mutex for final message in fireTask\r\n");
        PRINTF(message);
    }

    vTaskSuspend(NULL);
}

/*!
 * @brief Main task for the ambulance department.
 *
 * This function represents the main task for the ambulance department in a simulation system.
 * It continuously checks for new tasks in the ambulance queue, attempts to dispatch available
 * ambulances to handle the tasks, and creates counting tasks accordingly. It also
 * prints status messages about the dispatching process and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void ambulanceTask(void *pvParameters) {
    int receivedMissions = 0;
    char taskName[20];
    char message[200];
    uint8_t numAmbulances;

    while (!ENDDEPFLAG) {
        // Attempt to take the mutex to protect critical section for accessing queue and ambulanceVehiclesAvailable
        if (xSemaphoreTake(ambulanceQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
            // Wait to receive number of ambulances from the queue
            if (xQueueReceive(ambulanceQueue, &numAmbulances, pdMS_TO_TICKS(SEMAPHOREDELAY))) {
                // Determine number of ambulances to send
                uint8_t ambulancesToSend = (ambulanceVehiclesAvailable >= numAmbulances) ? numAmbulances : ambulanceVehiclesAvailable;


                // Print appropriate message based on availability
                if (ambulanceVehiclesAvailable >= numAmbulances) {
                    snprintf(message, sizeof(message), "%d ambulances dispatched, %d ambulances were available in AMBULANCE_DEPARTMENT\r\n",
                             ambulancesToSend, ambulanceVehiclesAvailable);
                    AMBULANCE.completed++;
                } else if (ambulanceVehiclesAvailable != 0) {
                    snprintf(message, sizeof(message), "not enough resources, sending %d ambulances from %d AMBULANCE_DEPARTMENT\r\n",
                             ambulancesToSend, numAmbulances);
                    AMBULANCE.partiallyCompleted++;
                } else {
                    snprintf(message, sizeof(message), "insufficient resources, can't send %d ambulances AMBULANCE_DEPARTMENT\r\n",
                             numAmbulances);
                    AMBULANCE.missed++;
                }

                // Dispatch vehicles (simulate by decrementing available count)
                ambulanceVehiclesAvailable -= ambulancesToSend;

                // Release the mutex protecting the vehicle count and queue
                xSemaphoreGive(ambulanceQueueMutex);

                // Attempt to take the print mutex to protect printing
                if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
                    // Print the message directly (assuming PRINTF is a function that prints the message)
                    PRINTF(message);

                    // Release the print mutex
                    xSemaphoreGive(printMutex);
                } else {
                    // Print mutex take failed
                    snprintf(message, sizeof(message), "Error: Failed to take print mutex in ambulanceTask\r\n");
                    PRINTF(message);
                }

                // Create countingTasks based on ambulancesToSend
                for (int taskIndex = 0; taskIndex < ambulancesToSend; ++taskIndex) {
                    // Increment receivedMissions only when creating a task
                    snprintf(taskName, sizeof(taskName), "countingTaskAMBULANCE%d", ++receivedMissions);
                    if (xTaskCreate(countingTaskAmbulance, taskName, configMINIMAL_STACK_SIZE * 2, NULL, AMBULANCE_PRIORITY, NULL) != pdPASS) {
                        snprintf(message, sizeof(message), "Error: Failed to create task %s\r\n", taskName);
                        PRINTF(message);
                    }
                }
            } else {
                // Failed to receive from the queue, release the mutex
                xSemaphoreGive(ambulanceQueueMutex);
//                snprintf(message, sizeof(message), "Nothing to receive from ambulanceQueue in ambulanceTask\r\n");
//                PRINTF(message);
            }
        } else {
            // Failed to take queue mutex, handle failure appropriately
            snprintf(message, sizeof(message), "Error: Failed to take ambulanceQueueMutex in ambulanceTask\r\n");
            PRINTF(message);
        }

        vTaskDelay(pdMS_TO_TICKS(CHECKQUEUEDELAY));
    }

    // Suspend the task if simulation ends
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS*configMAX_PRIORITIES));
    UBaseType_t ambulanceQueueLength = uxQueueMessagesWaiting(ambulanceQueue);
    snprintf(message, sizeof(message), "AMBULANCE DEPARTMENT: %d missions completed, %d missions partially completed, %d missions missed, %d is waiting in queue\r\n",
             AMBULANCE.completed, AMBULANCE.partiallyCompleted, AMBULANCE.missed, ambulanceQueueLength);

    if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
        // Print the message directly (assuming PRINTF is a function that prints the message)
        PRINTF(message);

        // Release the print mutex
        xSemaphoreGive(printMutex);
    } else {
        // Handle print mutex take failure appropriately
        snprintf(message, sizeof(message), "Error: Failed to take print mutex for final message in ambulanceTask\r\n");
        PRINTF(message);
    }

    vTaskSuspend(NULL);
}

/*!
 * @brief Main task for the corona department.
 *
 * This function represents the main task for the corona department in a simulation system.
 * It continuously checks for new tasks in the corona queue, attempts to dispatch available
 * resources to handle the tasks, and creates counting tasks accordingly. It also
 * prints status messages about the dispatching process and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void coronaTask(void *pvParameters) {
    int receivedMissions = 0;
    char taskName[20];
    char message[200];
    uint8_t numCars;

    while (!ENDDEPFLAG) {
        // Attempt to take the mutex to protect critical section for accessing queue and coronaVehiclesAvailable
        if (xSemaphoreTake(coronaQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
            // Wait to receive number of cars from the queue
            if (xQueueReceive(coronaQueue, &numCars, pdMS_TO_TICKS(SEMAPHOREDELAY))) {
                // Determine number of cars to send
                uint8_t carsToSend = (coronaVehiclesAvailable >= numCars) ? numCars : coronaVehiclesAvailable;


                // Print appropriate message based on availability
                if (coronaVehiclesAvailable >= numCars) {
                    snprintf(message, sizeof(message), "%d cars dispatched, %d cars were available in CORONA_DEPARTMENT\r\n",
                             carsToSend, coronaVehiclesAvailable);
                    CORONA.completed++;
                } else if (coronaVehiclesAvailable != 0) {
                    snprintf(message, sizeof(message), "not enough troops, sending %d cars from %d CORONA_DEPARTMENT\r\n",
                             carsToSend, numCars);
                    CORONA.partiallyCompleted++;
                } else {
                    snprintf(message, sizeof(message), "no troops, can't send %d cars CORONA_DEPARTMENT\r\n",
                             numCars);
                    CORONA.missed++;
                }

                // Dispatch vehicles (simulate by decrementing available count)
                coronaVehiclesAvailable -= carsToSend;

                // Release the mutex protecting the vehicle count and queue
                xSemaphoreGive(coronaQueueMutex);

                // Attempt to take the print mutex to protect printing
                if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
                    // Print the message directly (assuming PRINTF is a function that prints the message)
                    PRINTF(message);

                    // Release the print mutex
                    xSemaphoreGive(printMutex);
                } else {
                    // Print mutex take failed
                    snprintf(message, sizeof(message), "Error: Failed to take print mutex in coronaTask\r\n");
                    PRINTF(message);
                }

                // Create countingTasks based on carsToSend
                for (int taskIndex = 0; taskIndex < carsToSend; ++taskIndex) {
                    // Increment receivedMissions only when creating a task
                    snprintf(taskName, sizeof(taskName), "countingTaskCORONA%d", ++receivedMissions);
                    if (xTaskCreate(countingTaskCorona, taskName, configMINIMAL_STACK_SIZE * 2, NULL, CORONA_PRIORITY, NULL) != pdPASS) {
                        snprintf(message, sizeof(message), "Error: Failed to create task %s\r\n", taskName);
                        PRINTF(message);
                    }
                }
            } else {
                // Failed to receive from the queue, release the mutex
                xSemaphoreGive(coronaQueueMutex);
//                snprintf(message, sizeof(message), "Nothing to receive from coronaQueue in coronaTask\r\n");
//                PRINTF(message);
            }
        } else {
            // Failed to take queue mutex, handle failure appropriately
            snprintf(message, sizeof(message), "Error: Failed to take coronaQueueMutex in coronaTask\r\n");
            PRINTF(message);
        }

        vTaskDelay(pdMS_TO_TICKS(CHECKQUEUEDELAY));
    }

    // Suspend the task if simulation ends
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS*configMAX_PRIORITIES));
    UBaseType_t coronaQueueLength = uxQueueMessagesWaiting(coronaQueue);
    snprintf(message, sizeof(message), "CORONA DEPARTMENT: %d missions completed, %d missions partially completed, %d missions missed, %d is waiting in queue\r\n",
             CORONA.completed, CORONA.partiallyCompleted, CORONA.missed, coronaQueueLength);


    if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
        // Print the message directly (assuming PRINTF is a function that prints the message)
        PRINTF(message);

        // Release the print mutex
        xSemaphoreGive(printMutex);
    } else {
        // Handle print mutex take failure appropriately
        snprintf(message, sizeof(message), "Error: Failed to take print mutex for final message in coronaTask\r\n");
        PRINTF(message);
    }

    vTaskSuspend(NULL);
}



/*!
 * @brief Task to count down and release a police vehicle after completing a mission.
 *
 * This function represents a task that is created whenever a police vehicle is dispatched
 * to handle a mission. It delays execution for a duration calculated based on the mission's
 * priority, then increments the count of available police vehicles once the mission is completed.
 * It also prints status messages about the completion of missions and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void countingTaskPolice(void *pvParameters) {
    char message[70];  // Sufficient size for the formatted message

    // Wait for TIMEFORMISSION_MS times priority
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS * (configMAX_PRIORITIES-POLICE_PRIORITY)));

    // Attempt to take the mutex to protect critical section
    if (xSemaphoreTake(policeQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
        // Increment the count of available police cars
    	policeVehiclesAvailable += 1;

        // Release the mutex protecting the vehicle count
        xSemaphoreGive(policeQueueMutex);

        // Format the message
        snprintf(message, sizeof(message), "Car is back, %d cars available in POLICE_DEPARTMENT\r\n",
        		policeVehiclesAvailable);

        // Attempt to take the mutex for printing
        if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
            // Print the message directly (assuming PRINTF is a function that prints the message)
            PRINTF(message);

            // Release the print mutex
            xSemaphoreGive(printMutex);
        } else {
            // Print mutex take failed
            snprintf(message, sizeof(message), "Error: Failed to take print mutex in countingTaskPolice\r\n");
            PRINTF(message);
        }
    } else {
        // Police queue mutex take failed
        snprintf(message, sizeof(message), "Error: Failed to take police queue mutex in countingTaskPolice\r\n");
        PRINTF(message);
    }

    // Delete the task once done (if it's a one-time task)
    vTaskDelete(NULL);
}

/*!
 * @brief Task to count down and release a fire vehicle after completing a mission.
 *
 * This function represents a task that is created whenever a fire vehicle is dispatched
 * to handle a mission. It delays execution for a duration calculated based on the mission's
 * priority, then increments the count of available fire vehicles once the mission is completed.
 * It also prints status messages about the completion of missions and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void countingTaskFire(void *pvParameters) {
    char message[70];  // Sufficient size for the formatted message

    // Wait for TIMEFORMISSION_MS times priority
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS * (configMAX_PRIORITIES-FIRE_PRIORITY)));

    // Attempt to take the mutex to protect critical section
    if (xSemaphoreTake(fireQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
        // Increment the count of available fire trucks
    	fireVehiclesAvailable += 1;

        // Release the mutex protecting the vehicle count
        xSemaphoreGive(fireQueueMutex);

        // Format the message
        snprintf(message, sizeof(message), "Car is back, %d cars available in FIRE_DEPARTMENT\r\n",
        		fireVehiclesAvailable);

        // Attempt to take the mutex for printing
        if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
            // Print the message directly (assuming PRINTF is a function that prints the message)
            PRINTF(message);

            // Release the print mutex
            xSemaphoreGive(printMutex);
        } else {
            // Print mutex take failed
            snprintf(message, sizeof(message), "Error: Failed to take print mutex in countingTaskFire\r\n");
            PRINTF(message);
        }
    } else {
        // Fire queue mutex take failed
        snprintf(message, sizeof(message), "Error: Failed to take fire queue mutex in countingTaskFire\r\n");
        PRINTF(message);
    }

    // Delete the task once done (if it's a one-time task)
    vTaskDelete(NULL);
}

/*!
 * @brief Task to count down and release an ambulance after completing a mission.
 *
 * This function represents a task that is created whenever an ambulance is dispatched
 * to handle a mission. It delays execution for a duration calculated based on the mission's
 * priority, then increments the count of available ambulances once the mission is completed.
 * It also prints status messages about the completion of missions and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void countingTaskAmbulance(void *pvParameters) {
    char message[70];  // Sufficient size for the formatted message

    // Wait for TIMEFORMISSION_MS times priority
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS * (configMAX_PRIORITIES-AMBULANCE_PRIORITY)));

    // Attempt to take the mutex to protect critical section
    if (xSemaphoreTake(ambulanceQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
        // Increment the count of available ambulances
    	ambulanceVehiclesAvailable += 1;

        // Release the mutex protecting the vehicle count
        xSemaphoreGive(ambulanceQueueMutex);

        // Format the message
        snprintf(message, sizeof(message), "Car is back, %d cars available in AMBULACE_DEPARTMENT\r\n",
        		ambulanceVehiclesAvailable);

        // Attempt to take the mutex for printing
        if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
            // Print the message directly (assuming PRINTF is a function that prints the message)
            PRINTF(message);

            // Release the print mutex
            xSemaphoreGive(printMutex);
        } else {
            // Print mutex take failed
            snprintf(message, sizeof(message), "Error: Failed to take print mutex in countingTaskAmbulance\r\n");
            PRINTF(message);
        }
    } else {
        // Ambulance queue mutex take failed
        snprintf(message, sizeof(message), "Error: Failed to take ambulance queue mutex in countingTaskAmbulance\r\n");
        PRINTF(message);
    }

    // Delete the task once done (if it's a one-time task)
    vTaskDelete(NULL);
}

/*!
 * @brief Task to count down and release resources after completing a mission.
 *
 * This function represents a task that is created whenever resources are dispatched
 * to handle a corona-related mission. It delays execution for a duration calculated based on the mission's
 * priority, then performs necessary cleanup once the mission is completed.
 * It also prints status messages about the completion of missions and handles mutexes to ensure
 * thread-safe access to shared resources.
 *
 * @param pvParameters Pointer to task parameters.
 */
void countingTaskCorona(void *pvParameters) {
    char message[70];  // Sufficient size for the formatted message

    // Wait for TIMEFORMISSION_MS times priority
    vTaskDelay(pdMS_TO_TICKS(TIMEFORMISSION_MS * (configMAX_PRIORITIES-CORONA_PRIORITY)));

    // Attempt to take the mutex to protect critical section
    if (xSemaphoreTake(coronaQueueMutex, pdMS_TO_TICKS(SEMAPHOREDELAY)) == pdTRUE) {
        // Increment the count of available cars
        coronaVehiclesAvailable += 1;

        // Release the mutex protecting the vehicle count
        xSemaphoreGive(coronaQueueMutex);

        // Format the message
        snprintf(message, sizeof(message), "Car is back, %d cars available in CORONA_DEPARTMENT\r\n",
                 coronaVehiclesAvailable);

        // Attempt to take the mutex for printing
        if (xSemaphoreTake(printMutex, portMAX_DELAY) == pdTRUE) {
            // Print the message directly (assuming PRINTF is a function that prints the message)
            PRINTF(message);

            // Release the print mutex
            xSemaphoreGive(printMutex);
        } else {
            // Print mutex take failed
            snprintf(message, sizeof(message), "Error: Failed to take print mutex in countingTaskCorona\r\n");
            PRINTF(message);
        }
    } else {
        // Corona queue mutex take failed
        snprintf(message, sizeof(message), "Error: Failed to take corona queue mutex in countingTaskCorona\r\n");
        PRINTF(message);
    }

    // Delete the task once done (if it's a one-time task)
    vTaskDelete(NULL);
}

