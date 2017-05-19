/****************************************************************************
* Title                 :   iso11783 Include File
* Filename              :   iso11783.h
* Author                :   Henrique Reis
* Origin Date           :   01 de jul de 2016
* Version               :   1.0.0
* Compiler              :   GCC 5.4 2016q2 / ICCARM 7.40.3.8938
* Target                :   LPC43XX M4
* Notes                 :   Qualicode Machine Technologies
*
* THIS SOFTWARE IS PROVIDED BY AUTEQ TELEMATICA "AS IS" AND ANY EXPRESSED
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL AUTEQ TELEMATICA OR ITS CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
*
*    Date    Version        Author         Description
*  01/07/16   1.0.0    Henrique Reis    iso11783.h created.
*
*****************************************************************************/
/** @file   iso11783.h
*  @brief
*
*/
#ifndef ISO_PGN_H__
#define ISO_PGN_H__

#define DUAL_ECUs
#if defined(DUAL_ECUs)
#define M2G_SOURCE_ADDRESS 0x87
#else
#define M2G_SOURCE_ADDRESS 0x81
#endif
#define DESTINATION_ADDRESS 0x26
#define VT_ADDRESS 0x26
#define BROADCAST_ADDRESS 0xFF
#define PRIORITY 0x06

#define TRANSPORT_PROTOCOL 0
#define TRANSPORT_PROTOCOL_MAX_POOLSIZE 1785
#define EXTENDED_TRANSPORT_PROTOCOL 1
#define EXTENDED_TRANSPORT_PROTOCOL_MAX_POOLSIZE 117440505
#define EXTENDED_TRANSPORT_PROTOCOL_MAX_PACKETS 16777215

/* ********************************************************
* Define PGN codes of ISO 11783 messages
***********************************************************/
// <DESTINATION> PGNs
#define CLIENT_TO_FS_PGN        0x00AA00LU
#define FS_TO_GLOBAL_PGN        0x00ABFFLU
#define FS_TO_CLIENT_PGN        0x00AB00LU
#define GUIDANCE_MACHINE_STATUS 0x00AC00LU
#define GUIDANCE_SYSTEM_CMD     0x00AD00LU
#define ETP_DATA_TRANSFER_PGN   0x00C700LU
#define ETP_CONN_MANAGE_PGN     0x00C800LU
#define PROCESS_DATA_PGN        0x00CB00LU
#define CAB_MESSAGE_1_PGN       0x00E000LU
#define VT_TO_GLOBAL_PGN        0x00E6FFLU
#define VT_TO_ECU_PGN           0x00E600LU
#define ECU_TO_VT_PGN           0x00E700LU
#define ECU_TO_GLOBAL_PGN       0x00E7FFLU
/* we're NOT using ACK/NACK to Global address, we're directing directly to the sender! */
#define ACKNOWLEDGEMENT_PGN     0x00E800LU
#define REQUEST_PGN_MSG_PGN     0x00EA00LU
#define TP_DATA_TRANSFER_PGN    0x00EB00LU
#define TP_CONN_MANAGE_PGN      0x00EC00LU
#define ADDRESS_CLAIM_PGN       0x00EE00LU
#define PROPRIETARY_A_PGN       0x00EF00LU
#define PROPRIETARY_A2_PGN      0x01EF00LU

// <NO DESTINATION> PGNs
#define ELECTRONIC_ENGINE_CONTROLLER_1_PGN     0x00F004LU
#define ELECTRONIC_TRANSMISSION_CONTROLLER_2_PGN 0x00F005LU
#define ELECTRONIC_AXLE_CONTROLLER_1_PGN       0x00F006LU
#define SELECTED_SPEED_PGN                     0x00F022LU
#define PRODUCT_IDENTIFICATION_PGN             0x00FC8DLU
#define CONTROL_FUNCTION_FUNCTIONALITIES_PGN   0x00FC8ELU
#define ALL_IMPLEMENTS_STOP_OPERATIONS_SWITCH_STATE_PGN 0x00FD02LU
#define ECU_DIAGNOSTIC_PROTOCOL_PGN            0x00FD32LU
#define ISOBUS_CERTIFICATION_PGN               0x00FD42LU
#define SELECTED_SPEED_CMD                     0x00FD43LU
#define ECU_IDENTIFICATION_INFORMATION_PGN     0x00FDC5LU
#define OPERATORS_EXTERNAL_LIGHT_CONTROLS_MESSAGE_PGN 0x00FDCCLU
#define BASIC_JOYSTICK_MESSAGE_1_PGN           0x00FDD6LU
#define TRACTOR_FACILITIES_PGN                 0x00FE09LU
#define WORKING_SET_MEMBER_PGN                 0x00FE0CLU
#define WORKING_SET_MASTER_PGN                 0x00FE0DLU
#define LANGUAGE_PGN                           0x00FE0FLU
#define AUX_VALVE_0_ESTIMATED_FLOW             0x00FE10LU
#define AUX_VALVE_1_ESTIMATED_FLOW             0x00FE11LU
#define AUX_VALVE_2_ESTIMATED_FLOW             0x00FE12LU
#define AUX_VALVE_3_ESTIMATED_FLOW             0x00FE13LU
#define AUX_VALVE_4_ESTIMATED_FLOW             0x00FE14LU
#define AUX_VALVE_5_ESTIMATED_FLOW             0x00FE15LU
#define AUX_VALVE_6_ESTIMATED_FLOW             0x00FE16LU
#define AUX_VALVE_7_ESTIMATED_FLOW             0x00FE17LU
#define AUX_VALVE_8_ESTIMATED_FLOW             0x00FE18LU
#define AUX_VALVE_9_ESTIMATED_FLOW             0x00FE19LU
#define AUX_VALVE_10_ESTIMATED_FLOW            0x00FE1ALU
#define AUX_VALVE_11_ESTIMATED_FLOW            0x00FE1BLU
#define AUX_VALVE_12_ESTIMATED_FLOW            0x00FE1CLU
#define AUX_VALVE_13_ESTIMATED_FLOW            0x00FE1DLU
#define AUX_VALVE_14_ESTIMATED_FLOW            0x00FE1ELU
#define AUX_VALVE_15_ESTIMATED_FLOW            0x00FE1FLU
#define AUX_VALVE_0_MEASURED_FLOW              0x00FE20LU
#define AUX_VALVE_1_MEASURED_FLOW              0x00FE21LU
#define AUX_VALVE_2_MEASURED_FLOW              0x00FE22LU
#define AUX_VALVE_3_MEASURED_FLOW              0x00FE23LU
#define AUX_VALVE_4_MEASURED_FLOW              0x00FE24LU
#define AUX_VALVE_5_MEASURED_FLOW              0x00FE25LU
#define AUX_VALVE_6_MEASURED_FLOW              0x00FE26LU
#define AUX_VALVE_7_MEASURED_FLOW              0x00FE27LU
#define AUX_VALVE_8_MEASURED_FLOW              0x00FE28LU
#define AUX_VALVE_9_MEASURED_FLOW              0x00FE29LU
#define AUX_VALVE_10_MEASURED_FLOW             0x00FE2ALU
#define AUX_VALVE_11_MEASURED_FLOW             0x00FE2BLU
#define AUX_VALVE_12_MEASURED_FLOW             0x00FE2CLU
#define AUX_VALVE_13_MEASURED_FLOW             0x00FE2DLU
#define AUX_VALVE_14_MEASURED_FLOW             0x00FE2ELU
#define AUX_VALVE_15_MEASURED_FLOW             0x00FE2FLU
#define AUX_VALVE_0_COMMAND                    0x00FE30LU
#define AUX_VALVE_1_COMMAND                    0x00FE31LU
#define AUX_VALVE_2_COMMAND                    0x00FE32LU
#define AUX_VALVE_3_COMMAND                    0x00FE33LU
#define AUX_VALVE_4_COMMAND                    0x00FE34LU
#define AUX_VALVE_5_COMMAND                    0x00FE35LU
#define AUX_VALVE_6_COMMAND                    0x00FE36LU
#define AUX_VALVE_7_COMMAND                    0x00FE37LU
#define AUX_VALVE_8_COMMAND                    0x00FE38LU
#define AUX_VALVE_9_COMMAND                    0x00FE39LU
#define AUX_VALVE_10_COMMAND                   0x00FE3ALU
#define AUX_VALVE_11_COMMAND                   0x00FE3BLU
#define AUX_VALVE_12_COMMAND                   0x00FE3CLU
#define AUX_VALVE_13_COMMAND                   0x00FE3DLU
#define AUX_VALVE_14_COMMAND                   0x00FE3ELU
#define AUX_VALVE_15_COMMAND                   0x00FE3FLU
#define LIGHTING_DATA_PGN                      0x00FE40LU
#define LIGHTING_COMMAND_PGN                   0x00FE41LU
#define HITCH_PTO_COMMANDS                     0x00FE42LU
#define REAR_PTO_STATE_PGN                     0x00FE43LU
#define FRONT_PTO_STATE_PGN                    0x00FE44LU
#define REAR_HITCH_STATE_PGN                   0x00FE45LU
#define FRONT_HITCH_STATE_PGN                  0x00FE46LU
#define MAINTAIN_POWER_REQUEST_PGN             0x00FE47LU
#define WHEEL_BASED_SPEED_DIST_PGN             0x00FE48LU
#define GROUND_BASED_SPEED_DIST_PGN            0x00FE49LU
#define VEHICLE_FLUIDS_PGN                     0x00FE68LU
#define ELECTRONIC_TRANSMISSION_CONTROLLER_5_PGN 0x00FEC3LU
#define ACTIVE_DIAGNOSTIC_TROUBLE_CODES_PGN    0x00FECALU
#define PREVIOUSLY_ACTIVE_DIAGNOSTIC_TROUBLE_CODES_PGN 0x00FECBLU
#define DIAGNOSTIC_DATA_CLEAR_PGN              0x00FECCLU
#define SOFTWARE_IDENTIFICATION_PGN            0x00FEDALU
#define IDLE_OPERATION_PGN                     0x00FEDCLU
#define VEHICLE_DISTANCE_PGN                   0x00FEE0LU
#define SHUTDOWN_PGN                           0x00FEE4LU
#define ENGINE_HOURS_REVOLUTIONS_PGN           0x00FEE5LU
#define TIME_DATE_PGN                          0x00FEE6LU
#define VEHICLE_HOURS_PGN                      0x00FEE7LU
#define VEHICLE_DIRECTION_SPEED_PGN            0x00FEE8LU
#define FUEL_CONSUMPTION_LIQUID_PGN            0x00FEE9LU
#define VEHICLE_WEIGHT_PGN                     0x00FEEALU
#define ENGINE_TEMPERATURE_1_PGN               0x00FEEELU
#define ENGINE_FLUID_LEVEL_PRESSURE_1_PGN      0x00FEEFLU
#define POWER_TAKEOFF_INFORMATION_PGN          0x00FEF0LU
#define CRUISE_CONTROL_VEHICLE_SPEED_PGN       0x00FEF1LU
#define FUEL_ECONOMY_LIQUID_PGN                0x00FEF2LU
#define VEHICLE_POSITION_PGN                   0x00FEF3LU
#define AMBIENT_CONDITIONS_PGN                 0x00FEF5LU
#define VEHICLE_ELECTRICAL_POWER_PGN           0x00FEF7LU
#define TRANSMISSION_FLUIDS_PGN                0x00FEF8LU
#define DASH_DISPLAY_PGN                       0x00FEFCLU
// PROPRIETARY_B_PGN ranges from 0x00FF00 to 0x00FFFF !
#define PROPRIETARY_B_PGN                      0x00FF00LU

// <NMEA 2000 GPS Messages>
#define NMEA_GPS_POSITION_RAPID_UPDATE_PGN        0x01F801LU
#define NMEA_GPS_COG_SOG_RAPID_UPDATE_PGN         0x01F802LU
#define NMEA_GPS_HIGH_PRECISION_RAPID_UPDATE_PGN  0x01F803LU
#define NMEA_GPS_POSITION_DATA_PGN                0x01F805LU
#define NMEA_GPS_TIME_DATE_PGN                    0x01F809LU
#define NMEA_GPS_CROSS_TRACK_ERROR_PGN            0x01F903LU
#define NMEA_GPS_DIRECTION_DATA_PGN               0x01FE11LU
#define NMEA_GNSS_PSEUDORANGE_NOISE_STATISTICS    0x01FA06LU

// VT_TO_ECU/ECU_TO_VT functions
#define FUNC_OBJECT_POOL_TRANSF 0x11
#define FUNC_OBJECT_POOL_END 0x12
#define FUNC_GET_MEMORY 0xC0
#define FUNC_GET_NUMBER_SOFT_KEYS 0xC2
#define FUNC_GET_TEXT_FONT_DATA 0xC3
#define FUNC_GET_HARDWARE 0xC7
#define FUNC_GET_VERSIONS 0xDF
#define FUNC_STORE_VERSION 0xD0
#define FUNC_LOAD_VERSION 0xD1
#define FUNC_DELETE_VERSION 0xD2
#define FUNC_HIDE_SHOW_OBJECT 0xA0
#define FUNC_ENABLE_DISABLE_OBJECT 0xA1
#define FUNC_SELECT_INPUT_OBJECT 0xA2
#define FUNC_ESC_COMMAND 0x92
#define FUNC_CONTROL_AUDIO_SIGNAL 0xA3
#define FUNC_SET_AUDIO_VOLUME 0xA4
#define FUNC_CHANGE_CHILD_LOCATION 0xA5
#define FUNC_CHANGE_CHILD_POSITION 0xB4
#define FUNC_CHANGE_SIZE 0xA6
#define FUNC_CHANGE_BACK_COLOUR 0xA7
#define FUNC_CHANGE_NUMERIC_VALUE 0xA8
#define FUNC_CHANGE_STRING_VALUE 0xB3
#define FUNC_CHANGE_END_POINT 0xA9
#define FUNC_CHANGE_FONT_ATTRIBUTES 0xAA
#define FUNC_CHANGE_LINE_ATTRIBUTES 0xAB
#define FUNC_CHANGE_FILL_ATTRIBUTES 0xAC
#define FUNC_CHANGE_ACTIVE_MASK 0xAD
#define FUNC_CHANGE_SOFT_KEY_MASK 0xAE
#define FUNC_CHANGE_ATTRIBUTE 0xAF
#define FUNC_CHANGE_PRIORITY 0xB0
#define FUNC_CHANGE_LIST_ITEM 0xB1
#define FUNC_DELETE_OBJECT_POOL 0xB2
#define FUNC_VT_STATUS 0xFE
#define FUNC_WS_MAINTENANCE 0xFF
#define FUNC_SOFT_KEY_ACTIVATION 0x00
#define FUNC_BUTTON_ACTIVATION 0x01
#define FUNC_POINTING_EVENT 0x02
#define FUNC_VT_SELECT_INP_OBJECT 0x03
#define FUNC_VT_ESC 0x04
#define FUNC_VT_CHANGE_NUMERIC_VALUE 0x05
#define FUNC_VT_CHANGE_ACTIVE_MASK 0x06
#define FUNC_VT_CHANGE_SOFT_KEY_MASK 0x07
#define FUNC_VT_CHANGE_STRING_VALUE 0x08
#define FUNC_AUX_ASSIG_TYPE_1 0x20
#define FUNC_AUX_INP_TYPE_1_STATUS 0x21
#define FUNC_AUX_ASSIG_TYPE_2 0x24
#define FUNC_PREFERRED_ASSIG 0x22
#define FUNC_AUX_INP_TYPE_2_STATUS 0x26
#define FUNC_AUX_INP_TYPE_2_MAINTENANCE 0x23
#define FUNC_AUX_INP_STATUS_TYPE_2_ENABLE 0x25

#define RESERVED_BYTE 0xFF

// Transport protocol message types
#define TP_CM_RTS        0x10
#define TP_CM_CTS        0x11
#define TP_EndofMsgACK   0x13
#define TP_Conn_Abort    0xFF
#define TP_BAM           0x20

// Extended transport protocol message types
#define ETP_CM_RTS        0x14
#define ETP_CM_CTS        0x15
#define ETP_CM_DPO        0x16
#define ETP_CM_EOMA          0x17
#define ETP_Conn_Abort    0xFF

/******************************************************************************
* Typedefs
*******************************************************************************/
typedef union{
    struct {
        uint8_t SA : 8;			// Source address according to ISO 11783
        uint8_t PS : 8;			// PDU Specific according to ISO 11783, if <DESTINATION PGN> is the destination address of received message
        uint8_t PF : 8;			// PDU Format according to ISO 11783
        uint8_t DP : 1;			// Data Page according to ISO 11783
        uint8_t R1 : 1;			// Reserved Bit according to ISO 11783
        uint8_t P  : 3;         // Priority Bit according to ISO 11783
        uint8_t    : 3;
        uint32_t DLC;           // Message data length
        uint8_t B1 : 8;         // Message data byte 1
        uint8_t B2 : 8;         // Message data byte 2
        uint8_t B3 : 8;         // Message data byte 3
        uint8_t B4 : 8;         // Message data byte 4
        uint8_t B5 : 8;         // Message data byte 5
        uint8_t B6 : 8;         // Message data byte 6
        uint8_t B7 : 8;         // Message data byte 7
        uint8_t B8 : 8;         // Message data byte 8
    };
    canMSGStruct_s frame;
} ISOBUSMsg;

typedef struct{
    uint8_t bActiveWSM   : 8;     // Source address of active working set master
    uint16_t wMaskActWS  : 16;    // Object ID of visible Data/Alarm Mask of the active working set
    uint16_t wSoftActWS  : 16;    // Object ID of visible Soft Key Mask of the active working set
    uint8_t bBusyUpdMask : 1;     // VT is busy updating visible mask
    uint8_t bBusySavMem  : 1;     // VT is busy saving data to non-volatile memory
    uint8_t bBusyExcCmd  : 1;     // VT is busy executing a command
    uint8_t bBusyExcMac  : 1;     // VT is busy executing a macro
    uint8_t bBusyParsOP  : 1;     // VT is busy parsing an object pool
    uint8_t bBusyReserv  : 1;     // Reserved, set to 0
    uint8_t bAuxCtrlAct  : 1;     // Auxiliary controls learn mode active
    uint8_t bVTOutMemory : 1;     // VT is out of memory
    uint8_t bVTFuncCode  : 8;     // VT function code of current command being executed
} VTStatus;
    
/*      Function prototypes    */
void ISO_vSendAddressClaimed(void);
void ISO_vSendCommandedAddress(void);
void ISO_vSendRequest(uint32_t);
void ISO_vSendWorkingSetMaster(void);
void ISO_vSendGetHardware(void);
void ISO_vSendWorkingSetMaintenance(bool);
void ISO_vSendWSMaintenancePoolSent(void);
void ISO_vSendGetMemory(uint32_t);
void ISO_vSendGetNumberSoftKeys(void);
void ISO_vSendRequestToSend(void);
void ISO_vSendProprietaryA(void);
void ISO_vSendLoadVersion(uint64_t);
void ISO_vSendStoreVersion(uint64_t);
void ISO_vSendETP_CM_DPO(uint8_t, uint32_t);
uint8_t ISO_vInitPointersToTranfer(const uint8_t*, uint32_t);
void ISO_vSendObjectPool(uint8_t, uint32_t, uint8_t);
void ISO_vSendEndObjectPool(void);

#endif
