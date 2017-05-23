/****************************************************************************
* Title                 :   sensor_core Source File
* Filename              :   sensor_core.c
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
*    Date    Version     Author                         Description
*  01/07/16   1.0.0  Henrique Reis     sensor_core.c created
*
*****************************************************************************/
/** @file
 *  @brief
 *
 *
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "M2G_app.h"
#include "sensor_core.h"
#include "sensor_management.h"
#include "debug_tool.h"
#include "../../sensor/config/sensor_config.h"
#include "sensor_ThreadControl.h"
#include <stdlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
//!< MACRO to define the size of SENSOR queue
#define QUEUE_SIZEOFSENSOR 5

#define THIS_MODULE MODULE_SENSOR

#define ENABLE_PS9_PORT 1
#define ENABLE_PS9_PIN 17
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static eAPPError_s eError;                   		//!< Error variable

osThreadId xRecvThreadId;               // Holds the RecvThread ID
osThreadId xManagementThreadId;         // Holds the WriteThread ID
osThreadId xAqrRegThreadId;              // Holds the AqrRegThread ID

// Pin to enable/disable sensor power source
gpio_config_s sEnablePS9;

//#define ENABLE_PS9 GPIO_vClear(&sEnablePS9)     // Enable sensor power source
//#define DISABLE_PS9 GPIO_vSet(&sEnablePS9)      // Disable sensor power source

/******************************************************************************
* Module Variable Definitions from MPA2500 (can.h)
*******************************************************************************/
// CAN control structure
CAN_tsCtrlMPA CAN_sCtrlMPA;

// CAN network status
CAN_tsStatus CAN_sStatus;

// Holds the status of system...
extern osFlagsGroupId UOS_sFlagSis;

// Holds the status of CAN interface
//osFlagsGroupId CAN_psFlagEnl;

// Holds the status of CAN application and Auteq protocol
osFlagsGroupId CAN_psFlagApl;

// Variable from UOS_sConfiguracao.sMonitor.bMonitorArea
uint8_t bSENMonitorArea;

bool bSENWaitForConfig;

/******************************************************************************
* Module Broker Queue
*******************************************************************************/
DECLARE_QUEUE(SensorQueue, QUEUE_SIZEOFSENSOR);     //!< Declaration of Interface Queue
CREATE_SIGNATURE(SensorControl);                    //!< Signature Declarations
CREATE_SIGNATURE(SensorDiagnostic);                 //!< Signature Declarations
CREATE_SIGNATURE(SensorGPS);                      	//!< Signature Declarations
CREATE_SIGNATURE(SensorAcquireg);                   //!< Signature Declarations
CREATE_CONTRACT(Sensor);                            //!< Create contract for sensor msg publication

/******************************************************************************
* Local messages queue
*******************************************************************************/
CREATE_LOCAL_QUEUE(SensorPublishQ, canMSGStruct_s, 10);
CREATE_LOCAL_QUEUE(SensorWriteQ, canMSGStruct_s, 10);

/******************************************************************************
* Module Threads
*******************************************************************************/
#define X(a, b, c, d, e, f) {.thisThread.name = a, .thisThread.stacksize = b, .thisThread.tpriority = c, .thisThread.pthread = d, .thisModule = e, .thisWDTPosition = f},
Threads_t THREADS_THISTHREAD[] = {
    SENSOR_MODULES
};
#undef X

volatile uint8_t WATCHDOG_FLAG_ARRAY[sizeof(THREADS_THISTHREAD) / sizeof(THREADS_THISTHREAD[0])];   //!< Threads Watchdog flag holder

//Thread Control
WATCHDOG_CREATE(SENPUB);                                   //!< WDT pointer flag
WATCHDOG_CREATE(SENRCV);                                   //!< WDT pointer flag
WATCHDOG_CREATE(SENWRT);                                   //!< WDT pointer flag
WATCHDOG_CREATE(SENMGT);                                   //!< WDT pointer flag
uint8_t bSENPUBThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bSENRCVThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bSENWRTThreadArrayPosition = 0;                    //!< Thread position in array
uint8_t bSENMGTThreadArrayPosition = 0;                    //!< Thread position in array

peripheral_descriptor_p pSENSORHandle = NULL;          //!< ISO Handler

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void SEN_vTimerCallbackPnP(void const *arg);
void SEN_vTimerCallbackPnPTimeout(void const *arg);
void SEN_vTimerCallbackReadSensorsTimeout(void const *arg);
void SEN_vTimerCallbackConfigSensorsTimeout(void const *arg);

/******************************************************************************
* Module timers
*******************************************************************************/
CREATE_TIMER(PnPTimer, SEN_vTimerCallbackPnP);
CREATE_TIMER(PnPTimeoutTimer, SEN_vTimerCallbackPnPTimeout);
CREATE_TIMER(ReadSensorsTimeoutTimer, SEN_vTimerCallbackReadSensorsTimeout);
CREATE_TIMER(ConfigSensorsTimeoutTimer, SEN_vTimerCallbackConfigSensorsTimeout);

/******************************************************************************
* Function Definitions
*******************************************************************************/
uint8_t * SEN_WDTData(uint8_t * pbNumberOfThreads)
{
    *pbNumberOfThreads = ((sizeof(WATCHDOG_FLAG_ARRAY) / sizeof(WATCHDOG_FLAG_ARRAY[0]) - 0)); //-1 = remove core thread from list, -0 = keep it
    return (uint8_t*)WATCHDOG_FLAG_ARRAY;
}

inline void SEN_vDetectThread(thisWDTFlag* flag, uint8_t* bPosition, void* pFunc)
{
    *bPosition = 0;
    while (THREADS_THREAD(*bPosition) != (os_pthread)pFunc)
    {
        (*bPosition)++;
    }
    *flag = (uint8_t*)&WATCHDOG_FLAGPOS(THREADS_WDT_POSITION(*bPosition));
}

#ifndef UNITY_TEST
/******************************************************************************
* Function : SEN_vCreateThread(const Threads_t sSensorThread )
*//**
* \b Description:
*
* Function used to create threads.
*
* PRE-CONDITION: None
*
* POST-CONDITION: Threads created
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* osThreadDef_t sKEYBackThread;
*
* sKEYBackThread.name = "KEYBackThread";
* sKEYBackThread.stacksize = 500;
* sKEYBackThread.tpriority = osPriorityNormal;
* sKEYBackThread.pthread = SEN_vKEYBackLightThread;
*
* osThreadCreate(&sKEYBackThread, NULL);
~~~~~~~~~~~~~~~
*
* @see ISO_vCreateThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*******************************************************************************/
static void SEN_vCreateThread(const Threads_t sThread )
{
    osThreadId xThreads = osThreadCreate(&sThread.thisThread, (void*)osThreadGetId());
    
    ASSERT(xThreads != NULL);
    if (sThread.thisModule != 0)
    {
        osSignalWait(sThread.thisModule, osWaitForever); //wait for broker initialization
    }
}

/******************************************************************************
* Function : SEN_eReceivePooling(void)
*//**
* \b Description:
*
* This is the main routine of the SEN_vSensorPublishThread task. It performs a call to the
* device read function and check if there is a received message on the buffer. The received
* message is then published to the SENSOR mod topic.
*
* PRE-CONDITION: none
*
* POST-CONDITION: none
*
* @return Number of bytes received by the device and published to the topic.
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Called from SEN_vDiagnosticPublishThread
~~~~~~~~~~~~~~~
*
* @see SEN_vSensorThread, DIG_vSensorPublishThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 04/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/

uint32_t SEN_eReceivePooling(void)
{
    /* Check if the receive buffer has some data */
    uint8_t abTempBuffer[256];
    uint32_t wReceiveLenght = DEV_read(pSENSORHandle, abTempBuffer, 256);
    
    if (wReceiveLenght)
    {
        /* Publish the array at the DIAGNOSTIC topic */
        MESSAGE_HEADER(Sensor, wReceiveLenght, 1, MT_ARRAYBYTE);
        MESSAGE_PAYLOAD(Sensor) = (void*) abTempBuffer;
        PUBLISH(CONTRACT(Sensor), 0);
    }    
    return wReceiveLenght;
}
#endif

/******************************************************************************
* Function : SEN_eInitSensorPublisher(void)
*//**
* \b Description:
*
* This routine prepares the contract and message that the SEN_vSensorPublishThread thread
* will publish to the broker.
*
* PRE-CONDITION: none
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Called from SEN_vSensorPublishThread
~~~~~~~~~~~~~~~
*
* @see SEN_vSensorThread, SEN_vSensorPublishThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
eAPPError_s SEN_eInitSensorPublisher(void)
{
    /* Check if handler is already enabled */
    if (pSENSORHandle == NULL)
    {
        return APP_ERROR_ERROR;
    }
    
    //Prepare Default Contract/Message
    MESSAGE_HEADER(Sensor, 1, SENSOR_DEFAULT_MSGSIZE, MT_ARRAYBYTE);
    CONTRACT_HEADER(Sensor, 1, THIS_MODULE, TOPIC_SENSOR);
    
    return APP_ERROR_SUCCESS;
}

PubMessage sSensorPubMsg;

/******************************************************************************
* Function : SEN_vSensorPublishThread(void const *argument)
*//**
* \b Description:
*
* This is a thread of the Sensor module. It will poll the receive buffer of the device
* and in case of any incoming message, it will publish on the SENSOR topic.
*
* PRE-CONDITION: Sensor core initialized, interface enabled.
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Created from SEN_vSensorThread,
~~~~~~~~~~~~~~~
*
* @see SEN_vSensorThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
#ifndef UNITY_TEST
void SEN_vSensorPublishThread(void const *argument)
{    
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Sensor Publish Thread Created");
#endif
    
    SEN_vDetectThread(&WATCHDOG(SENPUB), &bSENPUBThreadArrayPosition, (void*)SEN_vSensorPublishThread);
    WATCHDOG_STATE(SENPUB, WDT_ACTIVE);
        
    osThreadId xDiagMainID = (osThreadId) argument;
    osSignalSet(xDiagMainID, THREADS_RETURN_SIGNAL(bSENPUBThreadArrayPosition));//Task created, inform core
    osThreadSetPriority(NULL, osPriorityLow);
    
    canMSGStruct_s* recv;
    uint8_t abTempBuffer[8];
    uint8_t bIterator = 0;
    uint32_t dValorFlag;
    osStatus status;

    (void)abTempBuffer;
    (void)bIterator;

    INITIALIZE_LOCAL_QUEUE(SensorPublishQ);   //!< Initialise message queue to publish thread

    SEN_eInitSensorPublisher();

    while(1)
    {
//        /* Pool the device waiting for */
//        WATCHDOG_STATE(SENPUB, WDT_SLEEP);
//        osEvent evtPub = RECEIVE_LOCAL_QUEUE(SensorPublishQ, osWaitForever);   // Wait
//        WATCHDOG_STATE(SENPUB, WDT_ACTIVE);
//
//        if(evtPub.status == osEventMessage)
//        {
//            // recv = (canMSGStruct_s*) evtPub.value.p;
//
//            // /* Publish the array at the SENSOR topic */
//            // MESSAGE_HEADER(Sensor, SENSOR_DEFAULT_MSGSIZE, 1, MT_ARRAYBYTE);
//            // CONTRACT_HEADER(Sensor, 1, THIS_MODULE, TOPIC_BUZZER);
//            // MESSAGE_PAYLOAD(Sensor) = (void*) recv->data;
//            // PUBLISH(CONTRACT(Sensor), 0);
//        }
        WATCHDOG_STATE(SENPUB, WDT_SLEEP);
        dValorFlag = osFlagWait (CAN_psFlagApl,
                                (CAN_APL_FLAG_TODOS_SENS_RESP_PNP |
                                 CAN_APL_FLAG_DET_NOVO_SENSOR |
                                 CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP |
                                 CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP |
                                 CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP |
                                 CAN_APL_FLAG_SENSOR_NAO_RESPONDEU |
                                 CAN_APL_FLAG_DET_SENSOR_RECONECTADO |
                                 CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO |
                                 CAN_APL_FLAG_CFG_SENSOR_RESPONDEU),
                                 true, false, osWaitForever);
        WATCHDOG_STATE(SENPUB, WDT_ACTIVE);

        status = WAIT_MUTEX(CAN_MTX_sBufferListaSensores, osWaitForever);
        ASSERT(status == osOK);

        if ((dValorFlag & CAN_APL_FLAG_TODOS_SENS_RESP_PNP) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_TODOS_SENS_RESP_PNP;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_DET_NOVO_SENSOR) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_DET_NOVO_SENSOR;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_DADOS_TODOS_SENSORES_RESP;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_PARAMETROS_TODOS_SENS_RESP;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_VERSAO_SW_TODOS_SENS_RESP;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_SENSOR_NAO_RESPONDEU) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_SENSOR_NAO_RESPONDEU;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_DET_SENSOR_RECONECTADO) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_DET_SENSOR_RECONECTADO;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_NENHUM_SENSOR_CONECTADO;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }
        if ((dValorFlag & CAN_APL_FLAG_CFG_SENSOR_RESPONDEU) > 0)
        {
            sSensorPubMsg.dEvent = CAN_APL_FLAG_CFG_SENSOR_RESPONDEU;
            sSensorPubMsg.vPayload = (void*) &CAN_sCtrlLista;
            MESSAGE_PAYLOAD(Sensor) = (void*) &sSensorPubMsg;
            PUBLISH(CONTRACT(Sensor), 0);
        }

        status = RELEASE_MUTEX(CAN_MTX_sBufferListaSensores);
        ASSERT(status == osOK);

    }
    osThreadTerminate(NULL);
}
#else
void SEN_vSensorPublishThread(void const *argument){}
#endif

/******************************************************************************
* Function : SEN_vInitDeviceLayer(uint32_t wSelectedInterface)
*//**
* \b Description:
*
* This function will init the M2GSENSORCOMM instance used by the Sensor module.
* The function parameter indicates the selected interface which will output the
* messages.
*
* PRE-CONDITION: Sensor core initialized
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
*
~~~~~~~~~~~~~~~
*
* @see 
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 04/04/2016 </td><td> 1.0.0            </td><td> TP       </td><td> Interface Created </td></tr>
* </table><br><br>
* <hr>
*
*******************************************************************************/
eAPPError_s SEN_vInitDeviceLayer(uint32_t wSelectedInterface)
{
    sEnablePS9.vpPrivateData = NULL;
    sEnablePS9.bDefaultOutputHigh = true;
    sEnablePS9.eDirection = GPIO_OUTPUT;
    sEnablePS9.ePull = GPIO_PULLDOWN;
    sEnablePS9.bMPort = ENABLE_PS9_PORT;
    sEnablePS9.bMPin = ENABLE_PS9_PIN;

    // Initialize enable PS9 structure
    GPIO_eInit(&sEnablePS9);
    // Enable power source 9V3
    ENABLE_PS9;

	/*Prepare the device */
    pSENSORHandle = DEV_open(PERIPHERAL_M2GSENSORCOMM);
    ASSERT(pSENSORHandle != NULL);
    
    /* Set device output to CAN peripheral */
    eError = (eAPPError_s) DEV_ioctl(pSENSORHandle, IOCTL_M2GSENSORCOMM_SET_ACTIVE, (void*)&wSelectedInterface);
    ASSERT(eError == APP_ERROR_SUCCESS);
    
    /* Configurations for CAN multipacket communications */
    if (wSelectedInterface == SENSOR_DEV_CAN)
    {
        /* Enables the reception of all frames with extended id number (29 bits) */
        uint32_t wCANInitID = 0xFFFFFFFF;
        eError = (eAPPError_s) DEV_ioctl(pSENSORHandle, IOCTL_M2GSENSORCOMM_ADD_ALL_CAN_ID, (void*)&wCANInitID);
        ASSERT(eError == APP_ERROR_SUCCESS);
    }
    return eError;
}

void SEN_vIdentifyEvent(contract_s* contract)
{
    osStatus status;

    switch(contract->eOrigin)
    {
        case MODULE_CONTROL:
        {
            bSENWaitForConfig = true;
            break;
        }
        case MODULE_ACQUIREG:
        {
            if((((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->dEvent) == CAN_APL_FLAG_FINISH_INSTALLATION)
            {
                // Send final handshake messages to sensors...


                uint32_t dFlagsSis = osFlagGet(UOS_sFlagSis);

                //-----------------------------------------------------------------------------
                //Envia Parâmetros para sensores de semente
                uint8_t abParametros[6];

                CAN_sParametrosSensor.wMinimo = 600; //us
                CAN_sParametrosSensor.wMaximo = 20000; //us
                CAN_sParametrosSensor.wMaximoDuplo = 20000; //us

                memcpy (abParametros, &CAN_sParametrosSensor, sizeof(CAN_sParametrosSensor));

                //Envia parâmetros para os sensores de semente
                SEN_vSensorsParameters ( CAN_APL_CMD_PARAMETROS_SENSOR, CAN_APL_LINHA_TODAS,
                                         CAN_APL_SENSOR_SEMENTE, abParametros, CAN_MSG_DLC_6);
                //-----------------------------------------------------------------------------
                //Envia solicitação de Versão dos sensores, se já obteve a versão , não envia a solicitação
                if (!(dFlagsSis & UOS_SIS_FLAG_VERSAO_SW_OK))
                {
                    SEN_vGetVersion ();
                }

                //-----------------------------------------------------------------------------
                //Envia Parâmetros para sensores de semente com algoritimo para tratar duplos
                uint8_t abParametrosExt[8];

                CAN_sParametrosExtended.bMinimo = 6;    //( x 100us )
                CAN_sParametrosExtended.bDuplo = 200;   //( x 100us )
                CAN_sParametrosExtended.bMaximo = 200;   //( x 100us )
                CAN_sParametrosExtended.bFreqAmostra = 4; //( x 10 Amostras/seg )
                CAN_sParametrosExtended.bNAmostra = 250;   //( Amostras )
                CAN_sParametrosExtended.bPasso = 20;    //( x 10us )
                CAN_sParametrosExtended.bDivPicoOffsetMinimo = 0x58; //( DivPico = 8 / OffsetMinimo = 5 (-) )
                CAN_sParametrosExtended.bOffsetDuploOffsetTriplo = 0x40; //( OffsetDuplo = 0 / OffsetTriplo = 4 (-) )

                memcpy (abParametrosExt, &CAN_sParametrosExtended, sizeof(CAN_tsParametrosExtended));

                //Envia parâmetros para os sensores de semente
                SEN_vSensorsParameters (CAN_APL_CMD_PARAMETROS_EXTENDED, CAN_APL_LINHA_TODAS,
                                        CAN_APL_SENSOR_SEMENTE, abParametrosExt, CAN_MSG_DLC_8);
            }
            break;
        }
        case MODULE_GPS:
        {
            if(((((PubMessage*)(GET_MESSAGE(contract)->pvMessage))->dEvent) & GPS_FLAG_METRO) > 0)
            {
                // TODO: Se nao esta em monitor de area nao pede a leitura dos sensores e nao esta em modo instalacao
                SEN_vReadDataFromSensors();
            }
            break;
        }
        default:
            break;
    }
}


/* ************************* Main thread ************************************ */
#ifndef UNITY_TEST
void SEN_vSensorThread (void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Isobus Thread Created");
#endif

    osStatus status;

    /* Init the module queue - structure that receive data from broker */
    INITIALIZE_QUEUE(SensorQueue);

    /* Init M2GSENSORCOMM device for output */
    SEN_vInitDeviceLayer(SENSOR_INITIAL_IO_IFACE);
    
    //Flag to interface CAN status and Auteq protocol
    memset(&CAN_sCtrlMPA, 0x00, sizeof(CAN_sCtrlMPA));
    
    //Inicializa a estrutura de controle de enlace da interface CAN
    CAN_sCtrlMPA.sCtrlEnl.psFlagEnl     = &xRecvThreadId;            // SensorRecv Thread ID
    CAN_sCtrlMPA.sCtrlEnl.dEventosEnl   = CAN_ENL_FLAG_NENHUM;
    
    //Inicializa a estrutura de controle de aplicacao da interface CAN
    CAN_sCtrlMPA.sCtrlApl.psFlagApl              = &CAN_psFlagApl;                 // TODO: Test with osFlags... SensorManagement Thread ID
    CAN_sCtrlMPA.sCtrlApl.dEventosApl            = CAN_APL_FLAG_NENHUM;            // TODO: Test with osFlags... SensorManagement Thread ID
    CAN_sCtrlMPA.sCtrlApl.dSensSementeConectados = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dSensAduboConectados   = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dSensDigitalConectados = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dRespostaSemente       = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dRespostaAdubo         = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dRespostaDigital       = CAN_APL_FLAG_NENHUM;
    
    CAN_sCtrlMPA.sCtrlApl.dSensSementeConectadosExt = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dSensAduboConectadosExt   = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dRespostaSementeExt       = CAN_APL_FLAG_NENHUM;
    CAN_sCtrlMPA.sCtrlApl.dRespostaAduboExt         = CAN_APL_FLAG_NENHUM;
    
    CAN_sCtrlMPA.sCtrlApl.dTicksTimeoutPnP       = CAN_wTICKS_TIMEOUT_PNP;
    CAN_sCtrlMPA.sCtrlApl.dTicksTimeoutConfigura = CAN_wTICKS_TIMEOUT_CFG;
    CAN_sCtrlMPA.sCtrlApl.dTicksTimeoutComando   = CAN_wTICKS_TIMEOUT_TESTE;//CAN_wTICKS_TIMEOUT;

    // Flags to receive the control module interface status and errors
//    status = osFlagGroupCreate(&UOS_sFlagSis);
//    ASSERT(status == osOK);

    // Flags to indicate the CAN application status and treatment of Auteq protocol
    status = osFlagGroupCreate(&CAN_psFlagApl);
    ASSERT(status == osOK);

    // Create a Mutex to access sensor buffer list on CAN network
    INITIALIZE_MUTEX(CAN_MTX_sBufferListaSensores);

    // Create a Mutex to access sensor file list on CAN network

    // Create and start CAN application thread

    // System timers allocation to timeout control
    // PnP command
    CAN_sCtrlMPA.sCtrlApl.dTicksCmdPnP = CAN_wTICKS_PRIMEIRO_CMD_PNP;
    INITIALIZE_TIMER(PnPTimer, osTimerPeriodic);
//    INITIALIZE_TIMER(PnPTimer, osTimerOnce);
    CAN_sCtrlMPA.sCtrlApl.wTimerCmdPnP = PnPTimer;
    // PnP command timeout
    INITIALIZE_TIMER(PnPTimeoutTimer, osTimerPeriodic);
    CAN_sCtrlMPA.sCtrlApl.wTimerTimeoutPnP = PnPTimeoutTimer;
    // Read sensor timeout
    INITIALIZE_TIMER(ReadSensorsTimeoutTimer, osTimerOnce);
    CAN_sCtrlMPA.sCtrlApl.wTimerTimeoutComando = ReadSensorsTimeoutTimer;
    // Config sensor timeout
    INITIALIZE_TIMER(ConfigSensorsTimeoutTimer, osTimerPeriodic);
    CAN_sCtrlMPA.sCtrlApl.wTimerTimeoutConfigura = ConfigSensorsTimeoutTimer;

    /* Prepare the signature - struture that notify the broker about subscribers */
    // TODO: just for sensor application test
    SIGNATURE_HEADER(SensorControl, THIS_MODULE, TOPIC_CONTROL, SensorQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(SensorControl), 0) == osOK);

    SIGNATURE_HEADER(SensorAcquireg, THIS_MODULE, TOPIC_ACQUIREG, SensorQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(SensorAcquireg), 0) == osOK);

    SIGNATURE_HEADER(SensorGPS, THIS_MODULE, TOPIC_GPS, SensorQueue);
    ASSERT(SUBSCRIBE(SIGNATURE(SensorGPS), 0) == osOK);

    //Create subthreads
    uint8_t bNumberOfThreads = 0;
    while(THREADS_THREAD(bNumberOfThreads) != NULL)
    {
        SEN_vCreateThread(THREADS_THISTHREAD[bNumberOfThreads++]);
    }

    /* Inform Main thread that initialization was a success */
    osThreadId xMainFromIsobusID = (osThreadId) argument;
    osSignalSet(xMainFromIsobusID, MODULE_SENSOR);

    // TODO: Wait for system is ready to work event

    /* Start the main functions of the application */
    while (1)
    {
        /* Blocks until any message is published on any topic */
        WATCHDOG_FLAG_ARRAY[0] = WDT_SLEEP;
        osEvent evt = RECEIVE(SensorQueue, osWaitForever);
        WATCHDOG_FLAG_ARRAY[0] = WDT_ACTIVE;
        if (evt.status == osEventMessage)
        {
            SEN_vIdentifyEvent(GET_CONTRACT(evt));
        }
    }    
    /* Unreachable */
    osThreadSuspend(NULL);
}
#else
void SEN_vSensorThread (void const *argument){}
#endif

/*****************************************************************************
*
*  static void CAN_vIdentificaMensagem( void )
*
*  Description : Identifica a mensagem recebida pelo controlador CAN
*  Parameters: nenhum
*  Return   : nenhum
*
*******************************************************************************/
static void SEN_vIdentifyMessage( canMSGStruct_s* sRcvMsg )
{
    osFlags flags;
    uint32_t wLastSig;
    CAN_tsIDAuteq    sIDAuteq;
    CAN_tsCtrlApl    *psCtrlApl;

    // Pointer to application control structure
    psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;
    
    // Received message
    CAN_sCtrlMPA.sMensagemRecebida = *sRcvMsg;

    // Filter only 29 bits from identifier field
    sIDAuteq.dID = 0x1FFFFFFF & CAN_sCtrlMPA.sMensagemRecebida.id;

    // Each message has meaning associated with identifier field, so test it!
    switch ( sIDAuteq.sID_bits.bComando )
    {
        case CAN_APL_RESP_PNP :
            // TODO: Is necessary verify if it's an data frame?
            // Increment PnP answer counter
            //CAN_bNumRespostasPNP += 1;

            // Answer to PnP command received
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_RESP_PNP);

            break;
        case CAN_APL_RESP_LEITURA_DADOS :

            // Received an answer to an data read command message
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS);

            break;
        case CAN_APL_LEITURA_VELOCIDADE :

            // Received an answer to an speed read command message
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE);

            break;
        case CAN_APL_RESP_CONFIGURA_SENSOR :

            // Received an answer to an configure sensor command message
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_RESP_CONFIGURACAO);

            break;
        case CAN_APL_RESP_PARAMETROS_SENSOR :
        case CAN_APL_RESP_PARAMETROS_EXTENDED:

            // Received an answer to an parameters command message
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_RESP_PARAMETROS);

            break;
        case CAN_APL_RESP_VERSAO_SW_SENSOR :

            // Received an answer to an sensor software version command message
            flags = osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR);

            break;
        default :
            break;
    } // End of switch ( sIDAuteq.sID_bits.bComando )
} // End of function

/******************************************************************************
* Function : SEN_vSensorRecvThread(void const *argument)
*//**
* \b Description:
*
* This is a thread of the Sensor module. It will poll the receive buffer of the device
* and in case of any incoming message, it will send to the responsible thread on module.
*
* PRE-CONDITION: Sensor core initialized, interface enabled.
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Created from SEN_vSensorThread,
~~~~~~~~~~~~~~~
*
* @see SEN_vSensorThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
#ifndef UNITY_TEST
void SEN_vSensorRecvThread(void const *argument)
{
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Sensor Recv Thread Created");
#endif
    
    SEN_vDetectThread(&WATCHDOG(SENRCV), &bSENRCVThreadArrayPosition, (void*)SEN_vSensorRecvThread);
    WATCHDOG_STATE(SENRCV, WDT_ACTIVE);
        
    osThreadId xSenMainID = (osThreadId) argument;
    osSignalSet(xSenMainID, THREADS_RETURN_SIGNAL(bSENRCVThreadArrayPosition));//Task created, inform core
//    osThreadSetPriority(NULL, osPriorityLow);
    
    xRecvThreadId = osThreadGetId();

    canMSGStruct_s asPayload[32];   //!< Buffer to hold the contract and message data
    
    osEvent evt;
    uint8_t bIterator;
    uint8_t bRecvMessages = 0;		//!< Lenght (messages) received

    WATCHDOG_STATE(SENRCV, WDT_SLEEP);
    osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
    WATCHDOG_STATE(SENRCV, WDT_ACTIVE);
    
    while(1)
    {
        /* Pool the device waiting for */
        WATCHDOG_STATE(SENRCV, WDT_ACTIVE);
        osDelay(250); // Wait
        
        // Use an osDelayUntil function instead osDelay function

        bRecvMessages = DEV_read(pSENSORHandle, &asPayload[0], sizeof(asPayload));
        
        if(bRecvMessages)
        {
            for(bIterator = 0; bIterator < bRecvMessages; bIterator++)
            {
                SEN_vIdentifyMessage(&asPayload[bIterator]);
                WATCHDOG_STATE(SENRCV, WDT_SLEEP);
                // Wait until XXX_Thread reads the received message
                evt = osSignalWait(CAN_APL_FLAG_MENSAGEM_LIDA, osWaitForever);
                ASSERT(evt.status == osEventSignal);
                WATCHDOG_STATE(SENRCV, WDT_ACTIVE);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void SEN_vSensorRecvThread(void const *argument){}
#endif

/******************************************************************************
* Function : ISO_vIsobusWriteThread(void const *argument)
*//**
* \b Description:
*
* This is a thread of the Isobus module. It will write the incoming messages to the device.
*
* PRE-CONDITION: Isobus core initialized, interface enabled.
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Created from ISO_vIsobusThread,
~~~~~~~~~~~~~~~
*
* @see ISO_vIsobusThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
#ifndef UNITY_TEST
void SEN_vSensorWriteThread(void const *argument)
{    
#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Sensor Write Thread Created");
#endif
    
    SEN_vDetectThread(&WATCHDOG(SENWRT), &bSENWRTThreadArrayPosition, (void*)SEN_vSensorWriteThread);
    WATCHDOG_STATE(SENWRT, WDT_ACTIVE);
        
    osThreadId xSenMainID = (osThreadId) argument;
    osSignalSet(xSenMainID, THREADS_RETURN_SIGNAL(bSENWRTThreadArrayPosition));//Task created, inform core
//    osThreadSetPriority(NULL, osPriorityLow);
    
    eAPPError_s eError;
    canMSGStruct_s* recv;
    
    INITIALIZE_LOCAL_QUEUE(SensorWriteQ);
    
    WATCHDOG_STATE(SENWRT, WDT_SLEEP);
    osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
    WATCHDOG_STATE(SENWRT, WDT_ACTIVE);

    while(1)
    {
        WATCHDOG_STATE(SENWRT, WDT_SLEEP);
        osEvent evtPub = RECEIVE_LOCAL_QUEUE(SensorWriteQ, osWaitForever);   // Wait
        WATCHDOG_STATE(SENWRT, WDT_ACTIVE);
        
        if(evtPub.status == osEventMessage)
        {
            recv = (canMSGStruct_s*) evtPub.value.p;
            
            eError = (eAPPError_s) DEV_ioctl(pSENSORHandle, IOCTL_M2GSENSORCOMM_CHANGE_SEND_ID, (void*)&recv->id);
            ASSERT(eError == APP_ERROR_SUCCESS);

            // wSendCANID changed
            if(eError == APP_ERROR_SUCCESS)
            {
                DEV_write(pSENSORHandle, &(recv->data[0]), recv->dlc);
            }
        }
    }
    osThreadTerminate(NULL);
}
#else
void SEN_vSensorWriteThread(void const *argument){}
#endif

//#define PnP_MESSAGE
#if defined(PnP_MESSAGE)
canMSGStruct_s pMsg;
#endif

void SEN_vTimerCallbackPnP(void const *arg)
{
    osFlags flags;
    CAN_tsCtrlApl    *psCtrlApl;
    psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;
    
    osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_TMR_CMD_PNP);
}

void SEN_vTimerCallbackPnPTimeout(void const *arg)
{
    uint32_t wLastSig;
    CAN_tsCtrlApl    *psCtrlApl;
    psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;
    
    osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_PNP_TIMEOUT);
}

void SEN_vTimerCallbackReadSensorsTimeout(void const *arg)
{
    uint32_t wLastSig;
    CAN_tsCtrlApl    *psCtrlApl;
    psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;
    
    osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_COMANDO_TIMEOUT);
}

void SEN_vTimerCallbackConfigSensorsTimeout(void const *arg)
{
    uint32_t wLastSig;
    CAN_tsCtrlApl    *psCtrlApl;
    psCtrlApl = &CAN_sCtrlMPA.sCtrlApl;
    
    osFlagSet(*(psCtrlApl->psFlagApl), CAN_APL_FLAG_COMANDO_CONFIGURA);
}

/******************************************************************************
* Function : SEN_vSensorPublishThread(void const *argument)
*//**
* \b Description:
*
* This is a thread of the Sensor module. It will poll the receive buffer of the device
* and in case of any incoming message, it will send to the responsible thread on module.
*
* PRE-CONDITION: Sensor core initialized, interface enabled.
*
* POST-CONDITION: none
*
* @return     void
*
* \b Example
~~~~~~~~~~~~~~~{.c}
* //Created from SEN_vSensorThread,
~~~~~~~~~~~~~~~
*
* @see SEN_vSensorThread
*
* <br><b> - HISTORY OF CHANGES - </b>
*
*
*******************************************************************************/
#ifndef UNITY_TEST
void SEN_vSensorNetworkManagementThread(void const *argument)
{
    osEvent evt;
    osStatus status;
    CAN_tsCtrlApl* psApl;

#ifdef configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
    SEGGER_SYSVIEW_Print("Sensor Management Thread Created");
#endif

    SEN_vDetectThread(&WATCHDOG(SENMGT), &bSENMGTThreadArrayPosition, (void*)SEN_vSensorNetworkManagementThread);
    WATCHDOG_STATE(SENMGT, WDT_ACTIVE);

    osThreadId xSenMainID = (osThreadId) argument;
    osSignalSet(xSenMainID, THREADS_RETURN_SIGNAL(bSENMGTThreadArrayPosition));//Task created, inform core
//    osThreadSetPriority(NULL, osPriorityLow);

    // Gets the Recv Thread ID
    xManagementThreadId = osThreadGetId();
    
    psApl = &CAN_sCtrlMPA.sCtrlApl;
    
    // Restart PnP timer
    status = START_TIMER(PnPTimer, psApl->dTicksCmdPnP);
    ASSERT(status == osOK);

    // Wait for IHM start...
    
    // Wait for receive an configuration flag
    WATCHDOG_STATE(SENMGT, WDT_SLEEP);
    osFlagWait(UOS_sFlagSis, UOS_SIS_FLAG_SIS_OK, false, false, osWaitForever);
    WATCHDOG_STATE(SENMGT, WDT_ACTIVE);

    while(1)
    {
        WATCHDOG_STATE(SENMGT, WDT_SLEEP);
        psApl->dEventosApl |= osFlagWait(*(psApl->psFlagApl),
                                             (CAN_APL_FLAG_TMR_CMD_PNP            | \
                                              CAN_APL_FLAG_MSG_RESP_PNP           | \
                                              CAN_APL_FLAG_MSG_RESP_LEITURA_DADOS | \
                                              CAN_APL_FLAG_MSG_LEITURA_VELOCIDADE | \
                                              CAN_APL_FLAG_MSG_RESP_PARAMETROS    | \
                                              CAN_APL_FLAG_MSG_RESP_VERSAO_SW_SENSOR | \
                                              CAN_APL_FLAG_MSG_RESP_CONFIGURACAO  | \
                                              CAN_APL_FLAG_PNP_TIMEOUT            | \
                                              CAN_APL_FLAG_COMANDO_TIMEOUT        | \
                                              CAN_APL_FLAG_COMANDO_CONFIGURA),
                                             true, false, osWaitForever);
        WATCHDOG_STATE(SENMGT, WDT_ACTIVE);

        // Manage network...
        SEN_vManagementNetwork();
    }
    osThreadTerminate(NULL);
}
#else
void SEN_vSensorNetworkManagementThread(void const *argument){}
#endif

