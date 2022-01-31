/*
 * commManager.c
 *
 *  Created on: Sep 13, 2019
 *      Author: Alex Morin
 */

#include <commManager.h>

#define FSM_STATE_HEADER  0x00
#define FSM_STATE_VID     0x01
#define FSM_STATE_CMD     0x02
#define FSM_STATE_PLL     0x03
#define FSM_STATE_PL      0x04
#define FSM_STATE_CRC     0x05

//Local variables
oCommManager_t oCommManager;
UART_HandleTypeDef* pUartHandle;

/**
  * @brief  Calculate a CRC16 on a buffer
  * @param  pu8Data: Pointers to our data for CRC calculation
  * @param  u16Length: Length of buffer to calculate our CRC16 on
  * @retval the calculated CRC16
  */
uint16_t CommManagerCRC16(uint8_t* pu8Data, uint16_t u16Length) {
    uint8_t i;
    uint16_t u16crc = 0xffff;

    if(pu8Data == NULL) {
    	return 0;
    }

    while (u16Length--) {
    	u16crc ^= *(unsigned char *)pu8Data++ << 8;
        for (i=0; i < 8; i++)
        	u16crc = u16crc & 0x8000 ? (u16crc << 1) ^ 0x1021 : u16crc << 1;
    }
    return u16crc & 0xffff;
}


/**
  * @brief  Format and send return message containing a payload
  * @param  u8CMD: The cmd code value
  * @param  pu8Payload: Pointer to a uint8_t buffer containing data to send
  * @param  u16PayloadLength: uint16_t for the buffer length
  */
void CommManagerSendPacket(uint8_t u8CMD, uint8_t* pu8Payload, uint16_t u16PayloadLength) {
	uint8_t u8Reply[256];
	uint16_t u16CalculatedCRC16;

    if(pu8Payload == NULL && u16PayloadLength != 0) {
    	return;
    }

	u8Reply[0] = START;					//START
	u8Reply[1] = VID;					//VID
	u8Reply[2] = u8CMD;					//CMD
	u8Reply[3] = u16PayloadLength;		//PLL
	if(u16PayloadLength != 0) {
		memcpy(&u8Reply[4], pu8Payload, u16PayloadLength);	//PL
	}
	u16CalculatedCRC16 = CommManagerCRC16(u8Reply, 4 + u16PayloadLength);
	u8Reply[4 + u16PayloadLength] = (uint8_t)(u16CalculatedCRC16 >> 8);		//CRC part 1
	u8Reply[5 + u16PayloadLength] = (uint8_t)u16CalculatedCRC16;			//CRC part 2

	HAL_UART_Transmit(pUartHandle, u8Reply, 6 + u16PayloadLength, 25);
	HAL_UART_Transmit(pUartHandle, '\n\r', 2, 25);

	CDC_Transmit_FS(u8Reply, 6 + u16PayloadLength);
	CDC_Transmit_FS('\n\r', 2);
}

/**
  * @brief  Check if message are contained inside a circular DMA buffer, the erase the message in the buffer
  * @param  pu8Data: Pointer to a size 256 uint8_t buffer
  * @param  pu16Length: Pointer to a uint16_t for the buffer length
  * @retval 1 if a new message has been received, 0 otherwise
  */
uint8_t CommManagerProcessBuffer(uint8_t* pu8Data, uint16_t* pu16Length) {
	uint16_t u16Index;
	uint16_t u16CircularBufferIndex;
	uint16_t u16RegularIndex;
	uint16_t u16CalculatedCRC16;
	uint8_t u8Data = oCommManager.u8DMABuffer[oCommManager.u16Counter];

	if(pu8Data == NULL || pu16Length == NULL) {
			return 0;
	}

	switch(oCommManager.u8State) {
		case FSM_STATE_HEADER:
			if(u8Data == START) {
				oCommManager.u8State = FSM_STATE_VID;
				oCommManager.u8CounterCRC = 2;
				oCommManager.u8PayloadLength = 0;
				oCommManager.u16StartLocation = oCommManager.u16Counter;
			}
		break;
		case FSM_STATE_VID:
			if(u8Data == FSM_STATE_VID) {
				oCommManager.u8State = FSM_STATE_CMD;
			} else {
				oCommManager.u8State = FSM_STATE_HEADER;
				//We restart one char after the startLocation
				oCommManager.u16Counter = ++oCommManager.u16StartLocation % oCommManager.u16DMABufferLength;
				return 0;
			}
		break;
		case FSM_STATE_CMD:
			oCommManager.u8State = FSM_STATE_PLL;
		break;
		case FSM_STATE_PLL:
			oCommManager.u8PayloadLength = u8Data;
			if(oCommManager.u8PayloadLength > 0) {
				oCommManager.u8State = FSM_STATE_PL;
			} else {
				oCommManager.u8State = FSM_STATE_CRC;
			}
		break;
		case FSM_STATE_PL:
			if(oCommManager.u8PayloadLength > 0) {
				oCommManager.u8PayloadLength--;
				if(oCommManager.u8PayloadLength == 0) {
					oCommManager.u8State = FSM_STATE_CRC;
				}
			}

		break;
		case FSM_STATE_CRC:
			if(oCommManager.u8CounterCRC > 0) {
				oCommManager.u8CounterCRC--;
				if(oCommManager.u8CounterCRC == 0) {
					//We have a complete message
					oCommManager.u16EndLocation = oCommManager.u16Counter++;

					if(oCommManager.u16StartLocation > oCommManager.u16EndLocation) {
						//Circular buffer copy
						u16Index = oCommManager.u16StartLocation;
						u16RegularIndex = 0;
						while(u16Index % oCommManager.u16DMABufferLength != (oCommManager.u16EndLocation + 1) % oCommManager.u16DMABufferLength) {
							u16CircularBufferIndex = u16Index % oCommManager.u16DMABufferLength;
							oCommManager.u8LastDataReceived[u16RegularIndex] = oCommManager.u8DMABuffer[u16CircularBufferIndex];
							u16Index++;
							u16RegularIndex++;
						}
						oCommManager.u16DataLength = u16RegularIndex;
					} else {
						//Regular buffer copy
						u16RegularIndex = 0;
						for(u16Index = oCommManager.u16StartLocation; u16Index <= oCommManager.u16EndLocation; u16Index++) {
							oCommManager.u8LastDataReceived[u16RegularIndex] = oCommManager.u8DMABuffer[u16Index];
							u16RegularIndex++;
						}
						oCommManager.u16DataLength = u16RegularIndex;
					}

					u16CalculatedCRC16 = CommManagerCRC16(oCommManager.u8LastDataReceived, oCommManager.u16DataLength);

					if(u16CalculatedCRC16 == 0x00) {
						//We found a valid data packet
						oCommManager.u8State = FSM_STATE_HEADER;
						CommManagerEraseBufferPart(oCommManager.u16StartLocation, oCommManager.u16EndLocation);

						memcpy(pu8Data, oCommManager.u8LastDataReceived, oCommManager.u16DataLength);
						*pu16Length = oCommManager.u16DataLength;
						return 1;
					} else {
						//It is invalid
						oCommManager.u8State = FSM_STATE_HEADER;
						oCommManager.u16Counter = ++oCommManager.u16StartLocation % oCommManager.u16DMABufferLength;
						return 0;
					}
				}
			}
		break;
	}

	//We increment our counter while keeping it circular
	oCommManager.u16Counter = ++oCommManager.u16Counter % oCommManager.u16DMABufferLength;
	return 0;
}

/**
  * @brief  Erase part of a circular buffer
  * @param  u16FromIndex: Start index of the circular buffer
  * @param  u16ToIndex: End index of the circular buffer
  */
void CommManagerEraseBufferPart(uint16_t u16FromIndex, uint16_t u16ToIndex) {
	uint16_t u16Index;
    uint16_t u16RegularIndex;
    uint16_t u16CircularBufferIndex;

    if(u16FromIndex > u16ToIndex) {
		//Circular buffer copy
		u16Index = u16FromIndex;
		u16RegularIndex = 0;
		while(u16Index % oCommManager.u16DMABufferLength != (u16ToIndex + 1) % oCommManager.u16DMABufferLength) {
			u16CircularBufferIndex = u16Index % oCommManager.u16DMABufferLength;
			oCommManager.u8DMABuffer[u16CircularBufferIndex] = 0x00;
			u16Index++;
			u16RegularIndex++;
		}
		oCommManager.u16DataLength = u16RegularIndex;
	} else {
		//Regular buffer copy
		u16RegularIndex = 0;
		for(u16Index = u16FromIndex; u16Index <= u16ToIndex; u16Index++) {
			oCommManager.u8DMABuffer[u16Index] = 0x00;
			u16RegularIndex++;
		}
		oCommManager.u16DataLength = u16RegularIndex;
	}
}

void CommManagerSendHeartbeat(uint32_t u32Time) {
	oHeartbeatData_t oHS;
	oHS.u32Timestamp = u32Time;

	CommManagerSendPacket(HEARTBEAT_CMD, &oHS, sizeof(oHS));
}

void CommManagerSendRawSensorData(oRawData_t *rawData) {
	CommManagerSendPacket(RAWDATA_CMD, (uint8_t *) rawData, sizeof(*rawData));
}

void CommManagerSendConfigData(oConfig_t *configData) {
	CommManagerSendPacket(CONFIGDATA_CMD, (uint8_t *) configData, sizeof(*configData));
}

CommManagerSendVehiculeData(oVehiculeData_t *vehiculeData) {
	CommManagerSendPacket(VEHICULEDATA_CMD, (uint8_t *) vehiculeData, sizeof(*vehiculeData));
}

/**
  * @brief  Initialise the communication module
  */
void CommManagerInit(UART_HandleTypeDef* pUart) {
	memset(&oCommManager, 0, sizeof(oCommManager));

	oCommManager.u16Counter = 0;
	oCommManager.u16StartLocation = 0;
	oCommManager.u8PayloadLength = 0;
	oCommManager.u16EndLocation = 0;
	oCommManager.u8State = FSM_STATE_HEADER;
	oCommManager.u16DMABufferLength = 512;
	oCommManager.u8CounterCRC = 0;
	oCommManager.u16DataLength = 0;
	oCommManager.u8ReceivedInit = 0;
	pUartHandle = pUart;

	HAL_UART_Receive_DMA(pUartHandle, (uint8_t*)oCommManager.u8DMABuffer, oCommManager.u16DMABufferLength);
}
