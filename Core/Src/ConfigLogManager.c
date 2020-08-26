
#include "ConfigLogManager.h"

float ConfigLogManagerReadFloatFromSD(FIL *fp, uint8_t *pu8Key, float fDefaultVal) {
    uint8_t buf[100];
    uint8_t u8Key[50];
    uint8_t u8Val[50];

    f_lseek(fp, 0);

    while (1) {
        if (f_gets(buf, 100, fp) == NULL) break;
        if(2 == sscanf(buf, "%[^'=']=%[^';']", u8Key, u8Val)) {
            if(strcmp(pu8Key, u8Key) == 0) {
                return atof(u8Val);
            }
        }
    }
    return fDefaultVal;
}

float ConfigLogManagerReadUInt16FromSD(FIL *fp, uint8_t *pu8Key, uint16_t fDefaultVal) {
    uint8_t buf[100];
    uint8_t u8Key[50];
    uint8_t u8Val[50];

    f_lseek(fp, 0);

    while (1) {
        if (f_gets(buf, 100, fp) == NULL) break;
        if(2 == sscanf(buf, "%[^'=']=%[^';']", u8Key, u8Val)) {
            if(strcmp(pu8Key, u8Key) == 0) {
                return atoi(u8Val);
            }
        }
    }
    return fDefaultVal;
}

bool ConfigLogManagerReadBoolFromSD(FIL *fp, uint8_t *pu8Key, bool bDefaultVal) {
    uint8_t buf[100];
    uint8_t u8Key[50];
    uint8_t u8Val[50];

    f_lseek(fp, 0);

    while (1) {
        if (f_gets(buf, 100, fp) == NULL) break;
        if(2 == sscanf(buf, "%[^'=']=%[^';']", u8Key, u8Val)) {
            if(strcmp(pu8Key, u8Key) == 0) {
                return (atoi(u8Val) <= 0) ? false : true;
            }
        }
    }
    return bDefaultVal;
}

void ConfigLogManagerReadConfigFileSD(uint8_t *pu8fileName, oConfig_t *pConfig) {
	FIL fp;
	f_open(&fp, pu8fileName, FA_READ);


    pConfig->fReleaseAltitude = ConfigLogManagerReadFloatFromSD(&fp, "fReleaseAltitude", 1.0);
    pConfig->fArmingAltitude = ConfigLogManagerReadFloatFromSD(&fp, "fArmingAltitude", 12.0);
    pConfig->fHardReleaseAltitude = ConfigLogManagerReadFloatFromSD(&fp, "fHardReleaseAltitude", 8.0);
    pConfig->fSoftReleaseAltitude = ConfigLogManagerReadFloatFromSD(&fp, "fSoftReleaseAltitude", 2.0);
    pConfig->fTargetRadius = ConfigLogManagerReadFloatFromSD(&fp, "fTargetRadius", 1.0);
    pConfig->fTargetLat = ConfigLogManagerReadFloatFromSD(&fp, "fTargetLat", 0.0);
    pConfig->fTargetLong = ConfigLogManagerReadFloatFromSD(&fp, "fTargetLong", 0.0);

    pConfig->u16HardReleaseCountdown = ConfigLogManagerReadUInt16FromSD(&fp, "u16HardReleaseCountdown", 60000);
    pConfig->u16SoftReleaseCountdown = ConfigLogManagerReadUInt16FromSD(&fp, "u16SoftReleaseCountdown", 6000);
    pConfig->u16MaxSpeed = ConfigLogManagerReadUInt16FromSD(&fp, "u16MaxSpeed", 4096);
    pConfig->u16HeartbeatTransmitInterval = ConfigLogManagerReadUInt16FromSD(&fp, "u16HeartbeatTransmitInterval", 1000);
    pConfig->u16SensorTransmitInterval = ConfigLogManagerReadUInt16FromSD(&fp, "u16SensorTransmitInterval", 250);
    pConfig->u8VID = ConfigLogManagerReadUInt16FromSD(&fp, "u8VID", 36);

    pConfig->bSound = ConfigLogManagerReadBoolFromSD(&fp, "bSound", 0);
    pConfig->bExternalTelemetry = ConfigLogManagerReadBoolFromSD(&fp, "bExternalTelemetry", 0);

    f_close(&fp);
}

void ConfigLogManagerWriteKeyValuePairFloatToSD(FIL *fp, uint8_t *pu8Key, float fValue) {
    uint8_t u8ValStr[12];
	gcvt(fValue,10,u8ValStr);
	u8ValStr[11] = '\0';
    f_printf(fp, "%s=%s;\n", pu8Key, u8ValStr);
}

void ConfigLogManagerOverwriteFileSD(uint8_t *pu8fileName, oConfig_t *pConfig) {
	FIL fp;
    f_open(&fp, pu8fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fReleaseAltitude", pConfig->fReleaseAltitude);
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fArmingAltitude", pConfig->fArmingAltitude);
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fHardReleaseAltitude", pConfig->fHardReleaseAltitude);
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fSoftReleaseAltitude", pConfig->fSoftReleaseAltitude );
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fTargetRadius", pConfig->fTargetRadius);
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fTargetLat", pConfig->fTargetLat);
    ConfigLogManagerWriteKeyValuePairFloatToSD(&fp, "fTargetLong", pConfig->fTargetLong);

    f_printf(&fp, "u16HardReleaseCountdown=%d;\n", pConfig->u16HardReleaseCountdown);
    f_printf(&fp, "u16SoftReleaseCountdown=%d;\n", pConfig->u16SoftReleaseCountdown);
    f_printf(&fp, "u16MaxSpeed=%d;\n", pConfig->u16MaxSpeed);
    f_printf(&fp, "u16HeartbeatTransmitInterval=%d;\n", pConfig->u16HeartbeatTransmitInterval);
    f_printf(&fp, "u16SensorTransmitInterval=%d;\n", pConfig->u16SensorTransmitInterval);
    f_printf(&fp, "u8VID=%d;\n", pConfig->u8VID);

    f_printf(&fp, "bSound=%d;\n", pConfig->bSound);
    f_printf(&fp, "bExternalTelemetry=%d;\n", pConfig->bExternalTelemetry);

    f_close(&fp);
}
