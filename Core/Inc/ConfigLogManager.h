/*
 * ConfigLogManager.h
 *
 *  Created on: Mar 2, 2020
 *      Author: alexmorin
 */

#ifndef INC_CONFIGLOGMANAGER_H_
#define INC_CONFIGLOGMANAGER_H_

#include "DataTypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "fatfs.h"

float ConfigLogManagerReadFloatFromSD(FIL *fp, uint8_t *pu8Key, float fDefaultVal);
float ConfigLogManagerReadUInt16FromSD(FIL *fp, uint8_t *pu8Key, uint16_t fDefaultVal);
bool ConfigLogManagerReadBoolFromSD(FIL *fp, uint8_t *pu8Key, bool bDefaultVal);
void ConfigLogManagerReadConfigFileSD(uint8_t *pu8fileName, oConfig_t *pConfig);
void ConfigLogManagerOverwriteFileSD(uint8_t *pu8fileName, oConfig_t *pConfig);
void ConfigLogManagerWriteKeyValuePairFloatToSD(FIL *fp, uint8_t *pu8Key, float fValue);

#endif /* INC_CONFIGLOGMANAGER_H_ */
