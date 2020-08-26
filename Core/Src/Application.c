/*
 * Application.c
 *
 *  Created on: Mar 4, 2020
 *      Author: alexmorin
 */

#include "Application.h"
#include "song.h"

bool bSDPresent;
uint32_t u32SensorLastUpdate;
uint32_t u32SensorTransmitLastUpdate;
uint32_t u32HeartBeatLastUpdate;
FATFS FatFs;

oConfig_t vehiculeConfig;
BMP280_HandleTypedef bmp280;

oRawData_t rawSensorData;

uint8_t u8CommRetVal;
uint16_t u16CommLength = 0;
uint8_t u8CommReturnArray[256];

void ApplicationInit() {
	bSDPresent = false;
	u32SensorLastUpdate = 0;
	u32SensorTransmitLastUpdate = 0;
	u8CommRetVal = 0;
	u32HeartBeatLastUpdate = 0;
	memset(&vehiculeConfig, 0, sizeof(vehiculeConfig));
	memset(&rawSensorData, 0, sizeof(rawSensorData));

	// Mount SD Card
	if(FR_OK == f_mount(&FatFs, "", 1)) {
		bSDPresent = true;
	}

	// Try reading param if sd present else using default params
	ConfigLogManagerReadConfigFileSD("config.txt", &vehiculeConfig);

	// Unmount SD Card
	if(bSDPresent == true) {
		f_mount(NULL, "", 0);
	}

	// Initialize buzzer pwm
	HAL_TIM_PWM_Init(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	// Initialize BNO055
	bno055_assignI2C(&hi2c1);
	bno055_setup();
	bno055_setExternalCrystalUse(true);
	bno055_setOperationModeNDOF();

	// Initialize BME280
	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = &hi2c1;
	bmp280_init(&bmp280, &bmp280.params);

	// Initialize GPS
	//GPS_Init();

	// Initialize Telemetry
	CommManagerInit(&huart2);

	// Initialize audio
	//playSongIT(&takeOnMeMelody[0], &takeOnMeTone[0], sizeof(takeOnMeTone), 0.5);
	//enableRepeat(true);
}

void ApplicationTask() {


	//Sensor data update loop
	if(HAL_GetTick() - u32SensorLastUpdate > SENSORUPDATE_MS) {
		// Reading BNO055
		bno055_vector_t v = bno055_getVectorEuler();
		bno055_calibration_state_t s = bno055_getCalibrationState();
		rawSensorData.BNO055Data.dPitch = v.z;
		rawSensorData.BNO055Data.dRoll = v.y;
		rawSensorData.BNO055Data.dYaw = v.x;

		rawSensorData.BNO055Data.u8AccCal = s.accel;
		rawSensorData.BNO055Data.u8GyrCal = s.gyro;
		rawSensorData.BNO055Data.u8MagCal = s.mag;
		rawSensorData.BNO055Data.u8SysCal = s.sys;

		// Reading BME280
		float fTemperature, fPressure, fHumidity;
		bmp280_read_float(&bmp280, &fTemperature, &fPressure, &fHumidity);
		rawSensorData.BME280Data.fTemperature = fTemperature;
		rawSensorData.BME280Data.fPressure = fPressure;
		rawSensorData.BME280Data.fHumidity = fHumidity;

		u32SensorLastUpdate = HAL_GetTick();
	}

	// Process telemetry data as fast as possible
	u8CommRetVal = CommManagerProcessBuffer(u8CommReturnArray, &u16CommLength);
	if (u8CommRetVal) {
		uint8_t u8CMD = u8CommReturnArray[2];
		uint8_t u8Len = u8CommReturnArray[3];

		if(u8CMD == CMD_READPARAM) {
			CommManagerSendConfigData(&vehiculeConfig);
		}
	}

	if(HAL_GetTick() - u32HeartBeatLastUpdate > vehiculeConfig.u16HeartbeatTransmitInterval) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		CommManagerSendHeartbeat(HAL_GetTick());

		u32HeartBeatLastUpdate = HAL_GetTick();
	}

	if(HAL_GetTick() - u32SensorTransmitLastUpdate > vehiculeConfig.u16SensorTransmitInterval) {
		CommManagerSendRawSensorData(&rawSensorData);
		u32SensorTransmitLastUpdate = HAL_GetTick();
	}

	//GPS_Process();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	GPS_CallBack();
}
