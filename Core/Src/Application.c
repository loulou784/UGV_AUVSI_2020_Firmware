/*
 * Application.c
 *
 *  Created on: Mar 4, 2020
 *      Author: alexmorin
 */

#include "Application.h"
#include "song.h"

#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

float CalculateAltitude(float fStartPressure, float fCurrentPressure);
float calculateBearing(float lat, float lon, float lat2, float lon2);
float calculateDistanceBetween(double lat1, double long1, double lat2, double long2);

#define MOTORSPEED_FAILSAFE_INTERVAL 150

bool bSDPresent;
uint32_t u32SensorLastUpdate;
uint32_t u32SensorTransmitLastUpdate;
uint32_t u32HeartBeatLastUpdate;
uint32_t u32MotorTeleopLastUpdate;
uint32_t u32VehiculeDataLastUpdate;
FATFS FatFs;

oConfig_t configData;
oVehiculeData_t vehiculeData;
oMotorSpeedData_t motorSpeedData;
BMP280_HandleTypedef bmp280;
oRawData_t rawSensorData;

uint8_t u8CommRetVal;
uint16_t u16CommLength = 0;
uint8_t u8CommReturnArray[256];

volatile bool bNewADCValueAvailable;
volatile uint32_t u32RawADCValue;

void ApplicationInit() {
	bSDPresent = false;
	u32SensorLastUpdate = 0;
	u32SensorTransmitLastUpdate = 0;
	u8CommRetVal = 0;
	u32HeartBeatLastUpdate = 0;
	memset(&configData, 0, sizeof(configData));
	memset(&vehiculeData, 0, sizeof(vehiculeData));
	memset(&rawSensorData, 0, sizeof(rawSensorData));
	memset(&motorSpeedData, 0, sizeof(motorSpeedData));
	bNewADCValueAvailable = false;
	u32RawADCValue = 0;

	// Mount SD Card
	if(FR_OK == f_mount(&FatFs, "", 1)) {
		bSDPresent = true;
	}

	// Try reading param if sd present else using default params
	ConfigLogManagerReadConfigFileSD("config.txt", &configData);

	// Unmount SD Card
	if(bSDPresent == true) {
		f_mount(NULL, "", 0);
	}

	// Initialize buzzer pwm
	HAL_TIM_PWM_Init(&htim1);

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
	GPS_Init();

	// Initialize Telemetry
	CommManagerInit(&huart2);

	// Use the Right telemetry
	if(configData.bExternalTelemetry == false) {
		HAL_GPIO_WritePin(TELEM_MUX_GPIO_Port, TELEM_MUX_Pin, GPIO_PIN_RESET);
	} else {
		HAL_GPIO_WritePin(TELEM_MUX_GPIO_Port, TELEM_MUX_Pin, GPIO_PIN_SET);
	}

	// Initialize motor PWM
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	setMotorSpeed(0, 0, 0, 0);

	HAL_ADC_Start_IT(&hadc1);

	// Get current average pressure as sealevel
	vehiculeData.fStartPressure = 0.0f;
	float fTemperature, fPressure, fHumidity;
	for(int i = 0; i < 100; i++) {
		bmp280_read_float(&bmp280, &fTemperature, &fPressure, &fHumidity);
		vehiculeData.fStartPressure += fPressure / 100.0f;
		HAL_Delay(20);
	}
	vehiculeData.fStartPressure /= 100.0f;

	// Initialize audio
	buzzerPlaySong(&successMelody[0], &successTone[0], sizeof(successTone), 2);
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
		rawSensorData.BME280Data.fPressure = fPressure / 100.0f;	// Convert to hPa
		rawSensorData.BME280Data.fHumidity = fHumidity;


		//Time to update vehiculeData
		if(bNewADCValueAvailable == true) {
			vehiculeData.u32Battery = (u32RawADCValue*3300/4096) * 6;	// Simplified Voltage divider
			bNewADCValueAvailable = false;
			HAL_ADC_Start_IT(&hadc1); // Restart the ADC
		}

		rawSensorData.CAMM8QData.fLatitude = GPS.GPGGA.LatitudeDecimal;
		rawSensorData.CAMM8QData.fLongitude = GPS.GPGGA.LongitudeDecimal;
		rawSensorData.CAMM8QData.fLatitude = 45.330444;
		rawSensorData.CAMM8QData.fLongitude = -71.980835;
		rawSensorData.CAMM8QData.fMSLAlt = GPS.GPGGA.MSL_Altitude;
		rawSensorData.CAMM8QData.u8Fix = GPS.GPGGA.PositionFixIndicator;
		rawSensorData.CAMM8QData.u8NumSat = GPS.GPGGA.SatellitesUsed;

	    vehiculeData.fBearing = calculateBearing(rawSensorData.CAMM8QData.fLatitude, rawSensorData.CAMM8QData.fLongitude, configData.fTargetLat, configData.fTargetLong);
	    vehiculeData.fSpeed = 0.0f;
	    vehiculeData.fTargetDistance = calculateDistanceBetween(rawSensorData.CAMM8QData.fLatitude, rawSensorData.CAMM8QData.fLongitude, configData.fTargetLat, configData.fTargetLong);
	    vehiculeData.fAltitude = CalculateAltitude(vehiculeData.fStartPressure, rawSensorData.BME280Data.fPressure);
	    vehiculeData.u32Time = HAL_GetTick();

		u32SensorLastUpdate = HAL_GetTick();
	}

	// Process telemetry data as fast as possible
	u8CommRetVal = CommManagerProcessBuffer(u8CommReturnArray, &u16CommLength);
	if (u8CommRetVal) {
		uint8_t u8CMD = u8CommReturnArray[2];
		uint8_t u8Len = u8CommReturnArray[3];

		if(u8CMD == READPARAM_CMD) {
			if(u8Len == READPARAM_LEN) {
				CommManagerSendConfigData(&configData);
			}
		} else if(u8CMD == SETMODE_CMD) {
			if(u8Len == SETMODE_LEN) {
				vehiculeData.u8Mode = u8CommReturnArray[4];
			}
		} else if(u8CMD == SETESTOP_CMD) {
			if(u8Len == SETESTOP_LEN) {
				vehiculeData.u8EStop = u8CommReturnArray[4];
			}
		} else if(u8CMD == SETRESETALT_CMD) {
			if(u8Len == SETRESETALT_LEN) {
				// TODO: Reset altitude offset
			}
		} else if(u8CMD == SETRELEASE_CMD) {
			if(u8Len == SETRELEASE_LEN) {
				// TODO: Trigger UGV Release
			}
		} else if(u8CMD == MOTORSPEED_CMD) {
			if(u8Len == MOTORSPEED_LEN) {
				memcpy(&motorSpeedData, &u8CommReturnArray[4], MOTORSPEED_LEN);

				vehiculeData.u16LeftSpeed = motorSpeedData.u16LeftSpeed;
				vehiculeData.u8LeftDirection = motorSpeedData.u8LeftDirection;
				vehiculeData.u16RightSpeed = motorSpeedData.u16RightSpeed;
				vehiculeData.u8RightDirection = motorSpeedData.u8RightDirection;

				u32MotorTeleopLastUpdate = HAL_GetTick();
			}
		}
	}

	if(vehiculeData.u8EStop == 0) {
		if(vehiculeData.u8Mode == MODE_TELEOP) {
			// We are in Teleop
			if(HAL_GetTick() - u32MotorTeleopLastUpdate < MOTORSPEED_FAILSAFE_INTERVAL) {
				// Teleop data received and valid
				setMotorSpeed(motorSpeedData.u16LeftSpeed, motorSpeedData.u8LeftDirection, motorSpeedData.u16RightSpeed, motorSpeedData.u8RightDirection);
			} else {
				// Teleop data expired
				setMotorSpeed(0, 0, 0, 0);
			}
		} else if(vehiculeData.u8Mode == MODE_AUTO) {
			// We are in auto
			// TODO: Run auto code
		}

	} else {
		setMotorSpeed(0, 0, 0, 0);
	}

	if(HAL_GetTick() - u32HeartBeatLastUpdate > configData.u16HeartbeatTransmitInterval) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		CommManagerSendHeartbeat(HAL_GetTick());
		u32HeartBeatLastUpdate = HAL_GetTick();
	}

	if(HAL_GetTick() - u32VehiculeDataLastUpdate > 500) {
		CommManagerSendVehiculeData(&vehiculeData);
		u32VehiculeDataLastUpdate = HAL_GetTick();
	}

	if(HAL_GetTick() - u32SensorTransmitLastUpdate > configData.u16SensorTransmitInterval) {
		CommManagerSendRawSensorData(&rawSensorData);
		u32SensorTransmitLastUpdate = HAL_GetTick();
	}

	GPS_Process();
}

// Right is channel 1
void setMotorSpeed(uint16_t u16LeftSpeed, uint8_t u8LeftDirection, uint16_t u16RightSpeed, uint8_t u8RightDirection) {
	// TODO: Set timers accordingly;
	if(u16LeftSpeed == 0) {
		TIM2->CCR2 = 0;
	}

	if(u16LeftSpeed > 4096) {
		u16LeftSpeed = 4096;
	}

	if(u8LeftDirection == 0) {
		TIM2->CCR2 = (u16LeftSpeed * 65535) / 4096;
		HAL_GPIO_WritePin(LEFT_MOTOR_DIR_GPIO_Port, LEFT_MOTOR_DIR_Pin, GPIO_PIN_RESET);
	} else if(u8LeftDirection == 1) {
		TIM2->CCR2 = 65535 - ((u16LeftSpeed * 65535) / 4096);
		HAL_GPIO_WritePin(LEFT_MOTOR_DIR_GPIO_Port, LEFT_MOTOR_DIR_Pin, GPIO_PIN_SET);
	}


	if(u16RightSpeed == 0) {
		TIM2->CCR1 = 0;
	}

	if(u16RightSpeed > 4096) {
		u16RightSpeed = 4096;
	}

	if(u8RightDirection == 0) {
		TIM2->CCR1 = (u16RightSpeed * 65535) / 4096;
		HAL_GPIO_WritePin(RIGHT_MOTOR_DIR_GPIO_Port, RIGHT_MOTOR_DIR_Pin, GPIO_PIN_RESET);
	} else if(u8RightDirection == 1) {
		TIM2->CCR1 = 65535 - ((u16RightSpeed * 65535) / 4096);
		HAL_GPIO_WritePin(RIGHT_MOTOR_DIR_GPIO_Port, RIGHT_MOTOR_DIR_Pin, GPIO_PIN_SET);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	GPS_CallBack();
}


/*
 * Both pressure in hPa
 */
float CalculateAltitude(float fStartPressure, float fCurrentPressure) {
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
  return 44330.0 * (1.0 - pow(fCurrentPressure / fStartPressure, 0.1903));
}

// https://gis.stackexchange.com/questions/252672/calculate-bearing-between-two-decimal-gps-coordinates-arduino-c
float calculateBearing(float lat, float lon, float lat2, float lon2) {
  float teta1 = radians(lat);
  float teta2 = radians(lat2);
  float delta2 = radians(lon2 - lon);

  float y = sin(delta2) * cos(teta2);
  float x = cos(teta1) * sin(teta2) - sin(teta1) * cos(teta2) * cos(delta2);
  float brng = atan2(y, x);
  brng = degrees(brng);
  brng = (((int)brng + 360) % 360);

  return brng;
}

float calculateDistanceBetween(double lat1, double long1, double lat2, double long2) {
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1-long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return (float)(delta * 6372795);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	u32RawADCValue = HAL_ADC_GetValue(AdcHandle);
    bNewADCValueAvailable = true;
}
