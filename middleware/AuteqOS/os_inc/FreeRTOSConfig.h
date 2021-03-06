#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

//#include "cmsis.h"
/* Here is a good place to include header files that are required across
 your application. */

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

extern volatile unsigned long ulHighFrequencyTimerTicks;
/* ulHighFrequencyTimerTicks is already being incremented at 20KHz.  Just set
its value back to 0. */
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ( ulHighFrequencyTimerTicks = 0UL )
#define portGET_RUN_TIME_COUNTER_VALUE()	ulHighFrequencyTimerTicks

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      120000000 //204000000
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    10
#define configMINIMAL_STACK_SIZE                256
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 1024 * 1024 ) ) //40k
#define configMAX_TASK_NAME_LEN                 15
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               64
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configCHECK_FOR_STACK_OVERFLOW          1
#define configUSE_MALLOC_FAILED_HOOK            1

#ifndef NDEBUG
/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS				1
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1
#define configGENERATE_RUN_TIME_STATS_USE_TICKS   1
#define configUSE_HEAP_SCHEME							4
#define configRECORD_STACK_HIGH_ADDRESS			1
#else
/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS				0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configGENERATE_RUN_TIME_STATS_USE_TICKS   1
#endif

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               5
#define configTIMER_QUEUE_LENGTH                128
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE * 8

// Cortex-M specific definitions.
#ifdef __NVIC_PRIO_BITS
//__BVIC_PRIO_BITS will be specified when CMSIS is being used.
#define configPRIO_BITS         __NVIC_PRIO_BITS
#else
#define configPRIO_BITS         3		 //8 priority levels
#endif

//#define configPRIO_BITS         5		/* 32 priority levels */

/* The lowest interrupt priority that can be used in a call to a "set priority"
 function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   ((1 << configPRIO_BITS) - 1)

/* The highest interrupt priority that can be used by any interrupt service
 routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
 INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
 PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

/* Interrupt nesting behaviour configuration. */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Priority 5, or 160 as only the top three bits are implemented. */
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_API_CALL_INTERRUPT_PRIORITY   ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Priorities passed to NVIC_SetPriority() do not require shifting as the
 function does the shifting itself.  Note these priorities need to be equal to
 or lower than configMAX_SYSCALL_INTERRUPT_PRIORITY - therefore the numeric
 value needs to be equal to or greater than 5 (on the Cortex-M3 the lower the
 numeric value the higher the interrupt priority). */

/* Define to trap errors during development. */
//extern void onAssert__(char const *file, unsigned line);
//#define configASSERT( x )     if( ( x ) == 0 ) onAssert__( __FILE__, __LINE__ )
#define configASSERT( x ) if( ( x ) == pdFALSE ) { taskDISABLE_INTERRUPTS(); \
          for( ;; ); }

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle  1
#define INCLUDE_pcTaskGetTaskName               0
#define INCLUDE_eTaskGetState                   0
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_pxTaskGetStackStart             (1 && configUSE_SEGGER_SYSTEM_VIEWER_HOOKS)
/* A header file that defines trace macro can be included here. */

#endif /* FREERTOS_CONFIG_H */
