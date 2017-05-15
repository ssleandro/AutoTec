
#DEBUG configuration

__USE_LPCOPEN                           // Use in case of LPC MCU
__USE_WATCHDOG                          // Activate Watchdog
configUSE_SEGGER_SYSTEM_VIEWER_HOOKS=1  // Activate SEGGER HOOKS
USE_SYSVIEW                             // USE SYSTEMVIEWER
TEST_BOARD                              // MYID Test board configuration
CHIP_LPC43XX                            // Use for LPC43XX
CORE_M4                                 // Indicate ARM CORE
NDEBUG                                  // DEBUG Flag

#RELEASE configuration

__USE_LPCOPEN                           // Use in case of LPC MCU
__USE_WATCHDOG                          // Activate Watchdog
configUSE_SEGGER_SYSTEM_VIEWER_HOOKS=0  // Activate SEGGER HOOKS
TDA_BOARD                               // TDA final board configuration
CHIP_LPC43XX                            // Use for LPC43XX
CORE_M4                                 // Indicate ARM CORE
