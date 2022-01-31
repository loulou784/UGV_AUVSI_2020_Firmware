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

//CommManager available command
#define BNO055DATA_CMD 0x01
#define BNO055DATA_LEN (sizeof(oBNO055Data_t))

#define CAMM8QDATA_CMD 0x02
#define CAMM8QDATA_LEN (sizeof(oCAMM8QData_t))

#define BME280DATA_CMD 0x03
#define BME280DATA_LEN (sizeof(oBME280Data_t))

#define RAWDATA_CMD 0x04
#define RAWDATA_LEN (sizeof(oRawData_t))

#define VEHICULEDATA_CMD 0x05
#define VEHICULEDATA_LEN (sizeof(oVehiculeData_t))

#define CONFIGDATA_CMD 0x06
#define CONFIGDATA_LEN (sizeof(oConfig_t))

#define HEARTBEAT_CMD 0x07
#define HEARTBEAT_LEN (sizeof(oHeartbeatData_t))

#define MOTORSPEED_CMD 0x07
#define MOTORSPEED_LEN (sizeof(oMotorSpeedData_t))

#define READPARAM_CMD 0x08
#define READPARAM_LEN (0x00)

#define SETMODE_CMD 0x09
#define SETMODE_LEN (0x01)

#define SETESTOP_CMD 0x0A
#define SETESTOP_LEN (0x01)

#define SETRESETALT_CMD 0x0B
#define SETRESETALT_LEN (0x01)

#define SETRELEASE_CMD 0x0C
#define SETRELEASE_LEN (0x01)

//A changer selon le VID du vehicule
#define VID   	   0x01				// Le vehicule ID
#define START      0x16				// Le start byte
#define TELEM_UART &huart2

#include "stm32l4xx_hal.h"
#include "usart.h"
#include "DataTypes.h"
#include "usbd_cdc_if.h"

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
