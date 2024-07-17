
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <stdlib.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "dispatcher.h"
#include "department.h"

/*!
 * @brief Mission progress structure for the each department.
 */
struct MissionProgress POLICE = {0};
struct MissionProgress FIRE = {0};
struct MissionProgress AMBULANCE = {0};
struct MissionProgress CORONA = {0};

uint32_t Message_PRIORITY =(configMAX_PRIORITIES - 1); 	//Priority for message printing task at the beginning of the simulation.
uint32_t Counting_PRIORITY =(configMAX_PRIORITIES - 1); //Priority for counting tasks at the beginning of the simulation.
uint32_t Event_PRIORITY =(configMAX_PRIORITIES - 1); //Priority for event dispatcher task.
uint32_t POLICE_PRIORITY =(configMAX_PRIORITIES - 1);//Priority for the POLICE department tasks.
uint32_t FIRE_PRIORITY =(configMAX_PRIORITIES - 2);//Priority for the FIRE department tasks.
uint32_t AMBULANCE_PRIORITY =(configMAX_PRIORITIES - 3);//Priority for the AMBULANCE department tasks.
uint32_t CORONA_PRIORITY =(configMAX_PRIORITIES - 4);//Priority for the CORONA department tasks.
int POLICE_DEPARTMENT = 0; // Identifier for the POLICE department.
int FIRE_DEPARTMENT = 1; // Identifier for the FIRE department.
int AMBULANCE_DEPARTMENT = 2; // Identifier for the AMBULANCE department.
int CORONA_DEPARTMENT = 3; // Identifier for the CORONA department.
uint32_t RANDOMDELAYDISPATCHER = 100; // Random delay range for event dispatching.
uint8_t MAXCARPERMISSION = 3; // Maximum number of cars permitted per mission.
uint32_t DISSIMULATINTIME = 5000; // Simulation time for the dispatcher in milliseconds.
uint32_t DEPSIMULATIONTIME = 6000; // Simulation time for the departments in milliseconds.
uint8_t MAXQUEUENUM = 100; // Maximum number of elements each queue can hold.
uint32_t TIMEFORMISSION_MS = 300; // Time allocated for each mission in milliseconds.
volatile uint8_t ENDDISFLAG = 0; // Flag indicating if dispatcher simulation has ended.
volatile uint8_t ENDDEPFLAG = 0; // Flag indicating if department simulation has ended.
uint8_t policeVehiclesAvailable = 3; // Number of available police vehicles for missions.
uint8_t fireVehiclesAvailable = 3; // available fire.
uint8_t ambulanceVehiclesAvailable = 3; // available ambulance.
uint8_t coronaVehiclesAvailable = 3; // // available corona.

QueueHandle_t policeQueue;
QueueHandle_t fireQueue;
QueueHandle_t ambulanceQueue;
QueueHandle_t coronaQueue;

SemaphoreHandle_t policeQueueMutex;
SemaphoreHandle_t fireQueueMutex;
SemaphoreHandle_t ambulanceQueueMutex;
SemaphoreHandle_t coronaQueueMutex;
SemaphoreHandle_t printMutex;



void initMessage(char *TaskName);
static void Message(void *pvParameters);
void SIMULATIONCOUNTINGDOWN(void *pvParameters);
void SIMULATIONCOUNTINGDOWN2(void *pvParameters);
void initCOUNTING(char* TaskName,uint32_t msTime);
void initCOUNTING2(char* TaskName,uint32_t msTime);
void initDispatcherMutexQueue(void);
/*!
 * @brief Entry point for the application.
 *
 * This function initializes a simulation of task dispatching and processing. The simulation
 * involves a dispatcher assigning random tasks to departments at irregular intervals. Each
 * department then processes these tasks over a fixed duration specified by TIMEFORMISSION_MS.
 * Priority is reflected in the TIMEFORMISSION_MS for each department, the formula is TIMEFORMISSION_MS * priority for each task.
 * Based on input parameters, the simulation predicts the number of tasks completed, partially
 * completed, and completely missed by the departments.
 *
 * @return Returns 0 upon successful completion.
 */
int main(void)
{
    /* Init board hardware. */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    initDispatcherMutexQueue();
	srand(1234);
    initMessage("BeginSimuTask");
    initCOUNTING("DispatcherTaskTime", DISSIMULATINTIME);
    initCOUNTING2("DepartmentsTaskTime",DEPSIMULATIONTIME);
    createDepartments();
    createDispatcherTasks();
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Initializes a printing task.
 *
 * This function creates a FreeRTOS task for printing messages using the provided
 * task name and a predefined stack size.
 *
 * @param TaskName Pointer to a character array representing the name of the task.
 */
void initMessage(char *TaskName){

    if (xTaskCreate(Message, TaskName, configMINIMAL_STACK_SIZE+100, NULL, Message_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
}
/*!
 * @brief Initializes a countdown simulation task.
 *
 * This function creates a FreeRTOS task for simulating a countdown using the provided
 * task name and a specified time in milliseconds.
 *
 * @param TaskName Pointer to a character array representing the name of the task.
 * @param msTime Time in milliseconds for the countdown simulation.
 */
void initCOUNTING(char* TaskName,uint32_t msTime) {

    if (xTaskCreate(SIMULATIONCOUNTINGDOWN, TaskName, configMINIMAL_STACK_SIZE, (void*)msTime, Counting_PRIORITY, NULL) != pdPASS) {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
}
/*!
 * @brief Initializes a second countdown simulation task.
 *
 * This function creates a FreeRTOS task for simulating a countdown (version 2) using
 * the provided task name and a specified time in milliseconds.
 *
 * @param TaskName Pointer to a character array representing the name of the task.
 * @param msTime Time in milliseconds for the countdown simulation.
 */
void initCOUNTING2(char* TaskName,uint32_t msTime) {

    if (xTaskCreate(SIMULATIONCOUNTINGDOWN2, TaskName, configMINIMAL_STACK_SIZE, (void*)msTime, Counting_PRIORITY, NULL) != pdPASS) {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
}
/*!
 * @brief Prints the begin message repeatedly.
 *
 * This function is a FreeRTOS task that prints "simulation begins." indefinitely
 * and suspends itself.
 *
 * @param pvParameters Pointer to task parameters.
 */
static void Message(void *pvParameters)
{
    for (;;)
    {
        PRINTF("simulation begins.\r\n");
        vTaskSuspend(NULL);
    }
}
/*!
 * @brief Counts down a specified time and sets a flag when complete.
 *
 * This function is a FreeRTOS task that counts down the specified time in milliseconds
 * using `vTaskDelay`, and then sets a flag (`ENDDISFLAG`) to indicate completion.
 *
 * @param pvParameters Pointer to the time in milliseconds (cast to void pointer).
 */
void SIMULATIONCOUNTINGDOWN(void *pvParameters) {
    int msTime = (int)pvParameters; // Cast pvParameters back to int

    vTaskDelay(pdMS_TO_TICKS(msTime)); // Use msTime in vTaskDelay
    ENDDISFLAG = 1; // Assuming ENDSIMULATION is a global flag
    vTaskDelete(NULL);
}
/*!
 * @brief Counts down a specified time and sets a flag when complete.
 *
 * This function is a FreeRTOS task that counts down the specified time in milliseconds
 * using `vTaskDelay`, and then sets a flag (`ENDDEPFLAG`) to indicate completion.
 *
 * @param pvParameters Pointer to the time in milliseconds (cast to void pointer).
 */
void SIMULATIONCOUNTINGDOWN2(void *pvParameters) {
    int msTime = (int)pvParameters; // Cast pvParameters back to int

    vTaskDelay(pdMS_TO_TICKS(msTime)); // Use msTime in vTaskDelay
    ENDDEPFLAG = 1; // Assuming ENDSIMULATION is a global flag
    vTaskDelete(NULL);
}
/*!
 * @brief Initializes queues and semaphores for dispatcher tasks.
 *
 * This function initializes queues for police, fire, ambulance, and corona dispatching,
 * and creates corresponding mutexes to protect access to these queues. It also creates
 * a mutex (`printMutex`) for printing operations.
 */
void initDispatcherMutexQueue(void) {
    // Create queues
    policeQueue = xQueueCreate(MAXQUEUENUM, sizeof(uint8_t));
    fireQueue = xQueueCreate(MAXQUEUENUM, sizeof(uint8_t));
    ambulanceQueue = xQueueCreate(MAXQUEUENUM, sizeof(uint8_t));
    coronaQueue = xQueueCreate(MAXQUEUENUM, sizeof(uint8_t));

    // Create mutexes for each queue
    policeQueueMutex = xSemaphoreCreateMutex();
    fireQueueMutex = xSemaphoreCreateMutex();
    ambulanceQueueMutex = xSemaphoreCreateMutex();
    coronaQueueMutex = xSemaphoreCreateMutex();
    printMutex = xSemaphoreCreateMutex();

    if (policeQueueMutex == NULL || fireQueueMutex == NULL || ambulanceQueueMutex == NULL || coronaQueueMutex == NULL || printMutex == NULL ) {
        PRINTF("Failed to create one or more mutexes.\n");
    }
}
