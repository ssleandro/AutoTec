/****************************************************************************
 * Title                 :   iso11783 Source File
 * Filename              :   iso11783.c
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
 *  01/07/16   1.0.0    Henrique Reis    iso11783.c created.
 *
 *****************************************************************************/
/** @file   iso11783.c
 *  @brief
 *
 */
#include "M2G_app.h"
#include "isobus_core.h"
#include "iso11783.h"

#if defined(DUAL_ECUs)
//	Used to address claim message
uint8_t NAME[] = { 0xD5, 0x55, 0x35, 0x04, 0x00, 0x84, 0x09, 0x20 };
#else
//  Used to address claim message
uint8_t NAME[] =
{	0x55, 0x55, 0x35, 0x04, 0x00, 0x84, 0x09, 0x20};
#endif

uint32_t wTransfTimestamp;                  // Needs to be global
const uint8_t* pbTransferBuffer;            // Needs to be global...
uint32_t wTransfSentBytes = 0;              // Total number of sent bytes... Needs to be global...
uint32_t tTransfBufferSize;                 // Needs to be global

extern osMessageQId WriteQ;

/*****************************************************
 * Global variables
 *****************************************************/
ISOBUSMsg pISOMsg;         // Holds the actual message

/* *
 * Get the ID of CAN frame from PGN, source address, destination address and priority
 * */
uint32_t ISO_vGetID (uint32_t pgn, uint32_t sa, uint32_t da, uint32_t prio)
{
	return ((((pgn | da) << 8) | sa) | (prio << 26));
}

void ISO_vSendAddressClaimed (void)
{

	(pISOMsg.frame).id = ISO_vGetID(ADDRESS_CLAIM_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = NAME[0];
	pISOMsg.B2 = NAME[1];
	pISOMsg.B3 = NAME[2];
	pISOMsg.B4 = NAME[3];
	pISOMsg.B5 = NAME[4];
	pISOMsg.B6 = NAME[5];
	pISOMsg.B7 = NAME[6];
	pISOMsg.B8 = NAME[7];

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendCommandedAddress (void)
{

	(pISOMsg.frame).id = ISO_vGetID(TP_CONN_MANAGE_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	/*	TRANSPORT PROTOCOL - CONNECTION MANAGEMENT	*/
	pISOMsg.B1 = 32;
	pISOMsg.B2 = 0x09;		// Number of bytes (9)
	pISOMsg.B3 = 0x00;		// 9 bytes
	pISOMsg.B4 = 0x02;		// Number of packets
	pISOMsg.B5 = RESERVED_BYTE;		// Reserved
	pISOMsg.B6 = 0xD8;		// PGN packed message (LSB)
	pISOMsg.B7 = 0xFE;		// Commanded-address PGN 65240 = 0x00FED8
	pISOMsg.B8 = 0x00;		// (MSB)  

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);

	(pISOMsg.frame).id = ISO_vGetID(TP_DATA_TRANSFER_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	/*	TRANSPORT PROTOCOL - DATA TRANSFER	*/
	pISOMsg.B1 = 0x01;
	pISOMsg.B2 = NAME[0];
	pISOMsg.B3 = NAME[1];
	pISOMsg.B4 = NAME[2];
	pISOMsg.B5 = NAME[3];
	pISOMsg.B6 = NAME[4];
	pISOMsg.B7 = NAME[5];
	pISOMsg.B8 = NAME[6];

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);

	(pISOMsg.frame).id = ISO_vGetID(TP_DATA_TRANSFER_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = 0x02;
	pISOMsg.B2 = NAME[7];
	pISOMsg.B3 = M2G_SOURCE_ADDRESS;
	pISOMsg.B4 = RESERVED_BYTE;
	pISOMsg.B5 = RESERVED_BYTE;
	pISOMsg.B6 = RESERVED_BYTE;
	pISOMsg.B7 = RESERVED_BYTE;
	pISOMsg.B8 = RESERVED_BYTE;

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendWorkingSetMaster (void)
{

	(pISOMsg.frame).id = ISO_vGetID(WORKING_SET_MASTER_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = 0x01;              // Number of members in a WS
	pISOMsg.B2 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B3 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B4 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B5 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B6 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B7 = RESERVED_BYTE;     // RESERVED_BYTE         
	pISOMsg.B8 = RESERVED_BYTE;     // RESERVED_BYTE         

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendGetHardware (void)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_GET_HARDWARE;
	pISOMsg.B2 = RESERVED_BYTE;
	pISOMsg.B3 = RESERVED_BYTE;
	pISOMsg.B4 = RESERVED_BYTE;
	pISOMsg.B5 = RESERVED_BYTE;
	pISOMsg.B6 = RESERVED_BYTE;
	pISOMsg.B7 = RESERVED_BYTE;
	pISOMsg.B8 = RESERVED_BYTE;

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendWorkingSetMaintenance (bool bInitiatingWS)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	// Working set maintenance ISO 11783-6    
	pISOMsg.B1 = FUNC_WS_MAINTENANCE;       // VT function
	pISOMsg.B2 = bInitiatingWS;             // Initiating working set maintenance
	pISOMsg.B3 = 0x03;                      // Compliant with VT version 3
	pISOMsg.B4 = RESERVED_BYTE;             // RESERVED BYTE             
	pISOMsg.B5 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B6 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B7 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B8 = RESERVED_BYTE;             // RESERVED BYTE

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendWSMaintenancePoolSent (void)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	// Working set maintenance ISO 11783-6
	// Send after object pool is sent
	pISOMsg.B1 = FUNC_WS_MAINTENANCE;       // VT function
	pISOMsg.B2 = RESERVED_BYTE;             // Initiating working set maintenance
	pISOMsg.B3 = RESERVED_BYTE;             // Compliant with VT version 3
	pISOMsg.B4 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B5 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B6 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B7 = RESERVED_BYTE;             // RESERVED BYTE
	pISOMsg.B8 = RESERVED_BYTE;             // RESERVED BYTE

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendGetMemory (uint32_t wMemoryRequired)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_GET_MEMORY;                   // VT funtion
	pISOMsg.B2 = RESERVED_BYTE;                     // Reserved bytes
	pISOMsg.B3 = wMemoryRequired & 0xFF;            // Memory Required (LSB)
	pISOMsg.B4 = (wMemoryRequired >> 8) & 0xFF;     // Memory Required
	pISOMsg.B5 = (wMemoryRequired >> 16) & 0xFF;    // Memory Required
	pISOMsg.B6 = (wMemoryRequired >> 24) & 0xFF;    // Memory Required (MSB)
	pISOMsg.B7 = RESERVED_BYTE;                     // Reserved bytes
	pISOMsg.B8 = RESERVED_BYTE;                     // Reserved bytes

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendRequest (uint32_t wPGNRequested)
{
	/*  Request + 3bytes Data (PGN)  */

	(pISOMsg.frame).id = ISO_vGetID(REQUEST_PGN_MSG_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 3;

	pISOMsg.B1 = wPGNRequested & 0xFF;              // PGN being requested (LSB)
	pISOMsg.B2 = (wPGNRequested >> 8) & 0xFF;       // PGN being requested
	pISOMsg.B3 = (wPGNRequested >> 16) & 0xFF;      // PGN being requested (MSB)

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendProprietaryA (void)
{

	(pISOMsg.frame).id = ISO_vGetID(PROPRIETARY_A_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = 0x00;
	pISOMsg.B2 = 0x00;
	pISOMsg.B3 = 0x00;
	pISOMsg.B4 = 0x00;
	pISOMsg.B5 = 0x00;
	pISOMsg.B6 = 0x00;
	pISOMsg.B7 = 0x00;
	pISOMsg.B8 = 0x00;

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendStoreVersion (uint64_t lVersionLabel)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_STORE_VERSION;
	pISOMsg.B2 = lVersionLabel & 0xFF;
	pISOMsg.B3 = ((lVersionLabel >> 8) & 0xFF);
	pISOMsg.B4 = ((lVersionLabel >> 16) & 0xFF);
	pISOMsg.B5 = ((lVersionLabel >> 24) & 0xFF);
	pISOMsg.B6 = ((lVersionLabel >> 32) & 0xFF);
	pISOMsg.B7 = ((lVersionLabel >> 40) & 0xFF);
	pISOMsg.B8 = ((lVersionLabel >> 48) & 0xFF);

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendLoadVersion (uint64_t lVersionLabel)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_LOAD_VERSION;
	pISOMsg.B2 = lVersionLabel & 0xFF;
	pISOMsg.B3 = ((lVersionLabel >> 8) & 0xFF);
	pISOMsg.B4 = ((lVersionLabel >> 16) & 0xFF);
	pISOMsg.B5 = ((lVersionLabel >> 24) & 0xFF);
	pISOMsg.B6 = ((lVersionLabel >> 32) & 0xFF);
	pISOMsg.B7 = ((lVersionLabel >> 40) & 0xFF);
	pISOMsg.B8 = ((lVersionLabel >> 48) & 0xFF);

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendGetNumberSoftKeys (void)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_GET_NUMBER_SOFT_KEYS;
	pISOMsg.B2 = RESERVED_BYTE;
	pISOMsg.B3 = RESERVED_BYTE;
	pISOMsg.B4 = RESERVED_BYTE;
	pISOMsg.B5 = RESERVED_BYTE;
	pISOMsg.B6 = RESERVED_BYTE;
	pISOMsg.B7 = RESERVED_BYTE;
	pISOMsg.B8 = RESERVED_BYTE;

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

void ISO_vSendEndObjectPool (void)
{

	(pISOMsg.frame).id = ISO_vGetID(ECU_TO_VT_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	pISOMsg.B1 = FUNC_OBJECT_POOL_END;
	pISOMsg.B2 = RESERVED_BYTE;
	pISOMsg.B3 = RESERVED_BYTE;
	pISOMsg.B4 = RESERVED_BYTE;
	pISOMsg.B5 = RESERVED_BYTE;
	pISOMsg.B6 = RESERVED_BYTE;
	pISOMsg.B7 = RESERVED_BYTE;
	pISOMsg.B8 = RESERVED_BYTE;

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

/* *
 * Makes an request to VT to send the object pool
 * opSize: size of object pool
 * nPackets: number of packets necessary to send the object pool
 * */
void ISO_vSendRequestToSend (uint32_t dBytesToSend)
{

	uint32_t nPackets;
	uint8_t bProtocol =
			(dBytesToSend <= TRANSPORT_PROTOCOL_MAX_POOLSIZE) ? TRANSPORT_PROTOCOL : EXTENDED_TRANSPORT_PROTOCOL;

	switch (bProtocol)
	{
		case TRANSPORT_PROTOCOL:
		{
			(pISOMsg.frame).id = ISO_vGetID(TP_CONN_MANAGE_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
			pISOMsg.DLC = 8;

			nPackets = (uint32_t)ceil(dBytesToSend / 7.0);

			// Send TP_CM_RTS (ISO 11783-3)
			pISOMsg.B1 = TP_CM_RTS;					        // Control Byte = 16 (TP.CM_RTS)
			pISOMsg.B2 = dBytesToSend & 0xFF;			    // Total message size (LSB)
			pISOMsg.B3 = ((dBytesToSend >> 8) & 0xFF);		// Total message size (MSB)
			pISOMsg.B4 = (uint8_t)nPackets;                	// Total number of packets
			pISOMsg.B5 = 0xFF;			   					// Maximum number of packets that can be sent in response to one CTS (FF = no limit)
			pISOMsg.B6 = ECU_TO_VT_PGN & 0xFF;              // PGN of requested information (LSB)
			pISOMsg.B7 = ((ECU_TO_VT_PGN >> 8) & 0xFF);     // PGN of requested information (Second byte)
			pISOMsg.B8 = ((ECU_TO_VT_PGN >> 16) & 0xFF);    // PGN of requested information (MSB)

			PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
			break;
		}
		case EXTENDED_TRANSPORT_PROTOCOL:
		{
			(pISOMsg.frame).id = ISO_vGetID(ETP_CONN_MANAGE_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
			pISOMsg.DLC = 8;

			// Send ETP_CM_RTS (ISO 11783-3)
			pISOMsg.B1 = ETP_CM_RTS;				        // Control Byte = 20 (ETP.CM_RTS)
			pISOMsg.B2 = (dBytesToSend & 0xFF);			    // Total message size (LSB)
			pISOMsg.B3 = ((dBytesToSend >> 8) & 0xFF);		    // Total message size
			pISOMsg.B4 = ((dBytesToSend >> 16) & 0xFF);	    // Total message size
			pISOMsg.B5 = ((dBytesToSend >> 24) & 0xFF);        // Total message size (MSB)
			pISOMsg.B6 = ECU_TO_VT_PGN & 0xFF;              // PGN of requested information (LSB)
			pISOMsg.B7 = ((ECU_TO_VT_PGN >> 8) & 0xFF);     // PGN of requested information (Second byte)
			pISOMsg.B8 = ((ECU_TO_VT_PGN >> 16) & 0xFF);    // PGN of requested information (MSB)

			PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
			break;
		}
		default:
			break;
	}
}

void ISO_vSendETP_CM_DPO (uint8_t bNumPackets, uint32_t wDataPacketOffset)
{
	(pISOMsg.frame).id = ISO_vGetID(ETP_CONN_MANAGE_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
	pISOMsg.DLC = 8;

	// wDataPacketOffset is always 1 less than bytes 3 to 5 of the ETP.CM_CTS message
	wDataPacketOffset -= 1;

	pISOMsg.B1 = ETP_CM_DPO;                                // Control Byte = 22 (ETP.CM_DPO)
	pISOMsg.B2 = bNumPackets;                               // Number of packets to which to apply the offset (1 to 255)
	pISOMsg.B3 = (wDataPacketOffset & 0xFF);                // Data packet offset (0 to n) (LSB)
	pISOMsg.B4 = ((wDataPacketOffset & 0xFF00) >> 8);       // Data packet offset (0 to n)
	pISOMsg.B5 = ((wDataPacketOffset & 0xFF0000) >> 16);    // Data packet offset (0 to n) (MSB)
	pISOMsg.B6 = ECU_TO_VT_PGN & 0xFF;                      // PGN of requested information (LSB)
	pISOMsg.B7 = ((ECU_TO_VT_PGN >> 8) & 0xFF);             // PGN of requested information (Second byte)
	pISOMsg.B8 = ((ECU_TO_VT_PGN >> 16) & 0xFF);            // PGN of requested information (MSB)

	PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
}

/* *
 * Send the object pool to VT
 * nPackets: number of packets necessary to send the object pool
 * */
uint8_t ISO_vInitPointersToTranfer (const uint8_t* pbAuxPointer, uint32_t tBufferSize)
{

	if ((pbAuxPointer == NULL) || (tBufferSize == 0))
	{
		return -1;
	}

	pbTransferBuffer = &pbAuxPointer[0];
	tTransfBufferSize = tBufferSize;
	wTransfSentBytes = 0;
	wTransfTimestamp = 1;

	return 0;
}

void ISO_vSendBytesToVT (uint8_t bNumPacketsToSend, uint32_t wNextPacketNumber, uint8_t bProtocol)
{
	uint8_t bIterator;
	uint16_t bSentPackets;                   // Number of packets sent in this CTS message

	switch (bProtocol)
	{
		case TRANSPORT_PROTOCOL:
		{
			(pISOMsg.frame).id = ISO_vGetID(TP_DATA_TRANSFER_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
			pISOMsg.DLC = 8;
			bSentPackets = 1;

			if (wNextPacketNumber == ((wTransfSentBytes / 7) + 1))
			{
				while ((wTransfSentBytes < tTransfBufferSize) && (bSentPackets <= bNumPacketsToSend))
				{
					if ((tTransfBufferSize - wTransfSentBytes) <= 7)
					{
						// Mark the first byte with the timestamp
						pISOMsg.B1 = bSentPackets;

						for (bIterator = 1; bIterator < (tTransfBufferSize - wTransfSentBytes); bIterator++)
						{
							(pISOMsg.frame).data[bIterator] = *(pbTransferBuffer++);
						}
						for (; bIterator < 8; bIterator++)      // Complete the frame with 0xFF
						{
							(pISOMsg.frame).data[bIterator] = 0xFF;
						}
						bSentPackets++;
						wTransfSentBytes += (tTransfBufferSize - wTransfSentBytes);
					}
					else
					{
						// Mark the first byte with the timestamp
						pISOMsg.B1 = bSentPackets;

						for (bIterator = 1; bIterator < 8; bIterator++)
						{
							(pISOMsg.frame).data[bIterator] = *(pbTransferBuffer++);
						}
						bSentPackets++;
						wTransfSentBytes += 7;
					}
					// Send ISOBUSMsg to Write Thread
					PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
				}
			}
			break;
		}
		case EXTENDED_TRANSPORT_PROTOCOL:
		{
			(pISOMsg.frame).id = ISO_vGetID(ETP_DATA_TRANSFER_PGN, M2G_SOURCE_ADDRESS, DESTINATION_ADDRESS, PRIORITY_6);
			pISOMsg.DLC = 8;
			bSentPackets = 1;

			if (wNextPacketNumber == ((wTransfSentBytes / 7) + 1))
			{
				while ((wTransfSentBytes < tTransfBufferSize) && (bSentPackets <= bNumPacketsToSend))
				{
					if ((tTransfBufferSize - wTransfSentBytes) <= 7)
					{
						// Mark the first byte with the timestamp
						pISOMsg.B1 = bSentPackets;

						for (bIterator = 1; bIterator < (tTransfBufferSize - wTransfSentBytes); bIterator++)
						{
							(pISOMsg.frame).data[bIterator] = *(pbTransferBuffer++);
						}
						for (; bIterator < 8; bIterator++)      // Complete the frame with 0xFF
						{
							(pISOMsg.frame).data[bIterator] = 0xFF;
						}
						bSentPackets++;
						wTransfSentBytes += (tTransfBufferSize - wTransfSentBytes);
					}
					else
					{
						// Mark the first byte with the timestamp
						pISOMsg.B1 = bSentPackets;

						for (bIterator = 1; bIterator < 8; bIterator++)
						{
							(pISOMsg.frame).data[bIterator] = *(pbTransferBuffer++);
						}
						bSentPackets++;
						wTransfSentBytes += 7;
					}
					// Send ISOBUSMsg to Write Thread
					PUT_LOCAL_QUEUE(WriteQ, pISOMsg, osWaitForever);
				}
			}
			break;
		}
		default:
			break;
	}
}
