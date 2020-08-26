/*
 * commManager.h
 *
 *  Created on: Sep 13, 2019
 *      Author: Alex Morin
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include "DataTypes.h"
#include "commManager.h"
#include "ConfigLogManager.h"
#include "bno055.h"
#include "bmp280.h"
#include "GPS.h"
#include "buzzer.h"

#include <stdint.h>
#include <stdbool.h>

#include "usart.h"
#include "fatfs.h"
#include "i2c.h"
#include "main.h"

#define SENSORUPDATE_MS 20

void ApplicationInit();
void ApplicationTask();

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_H_ */
