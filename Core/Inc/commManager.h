/*
 * commManager.h
 *
 *  Created on: Sep 13, 2019
 *      Author: Alex Morin
 */

#ifndef COMMMANAGER_H_
#define COMMMANAGER_H_
#ifdef __cplusplus
 extern "C" {
#endif

#define CMD_HEARTBEAT 0x04
#define CMD_SPEED     0x06
#define CMD_RAWSENSOR 0x08
#define CMD_CONFIG    0x10
#define CMD_READPARAM 0x12

//A changer selon le VID du vehicule
#define VID   	   0x01				// Le vehicule ID
#define START      0x16				// Le start byte
#define TELEM_UART &huart2

#include "stm32l4xx_hal.h"
#include "usart.h"
#include "DataTypes.h"

typedef struct {
	uint8_t u8LastDataReceived[256];
	uint8_t u8ReceivedInit;
	uint8_t u8DMABuffer[512];
	uint8_t u8State;
	uint8_t u8PayloadLength;

	uint16_t u16StartLocation;
	uint16_t u16EndLocation;
	uint16_t u16DataLength;
	uint16_t u16Counter;
	uint16_t u16DMABufferLength;
	uint16_t u8CounterCRC;
} oCommManager_t, *poCommManager_t;

uint8_t CommManagerProcessBuffer(uint8_t* pu8Data, uint16_t* pu16Length);

void CommManagerInit(UART_HandleTypeDef* pUartHandle);

void CommManagerProcessMessage(uint8_t *pu8Data, uint16_t *pu16Length);

void CommManagerSendPacket(uint8_t u8CMD, uint8_t* pu8Payload, uint16_t u16PayloadLength);

void CommManagerSendHeartbeat(uint32_t u32Time);

void CommManagerSendRawSensorData(oRawData_t *rawData);

void CommManagerSendConfigData(oConfig_t *configData);

#ifdef __cplusplus
}
#endif

#endif /* COMMMANAGER_H_ */
