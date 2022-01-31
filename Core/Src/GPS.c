#include "GPS.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

GPS_t GPS;

uint8_t strTok(uint8_t *u8InputString, uint8_t **u8ModifiedString, uint8_t *u8Delimiter, char* u8Token, uint8_t u8MaxTokenSize);
uint8_t NMEAValidateCRC(uint8_t *u8InputString);
//##################################################################################################################
double convertDegMinToDecDeg (float degMin)
{
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  return decDeg;
}
//##################################################################################################################
void	GPS_Init(void)
{
	GPS.rxIndex=0;
	HAL_UART_Receive_IT(&_GPS_USART,&GPS.rxTmp,1);
}
//##################################################################################################################
void	GPS_CallBack(void)
{
	GPS.LastTime=HAL_GetTick();
	if(GPS.rxIndex < sizeof(GPS.rxBuffer)-2)
	{
		GPS.rxBuffer[GPS.rxIndex] = GPS.rxTmp;
		GPS.rxIndex++;
	}
	HAL_UART_Receive_IT(&_GPS_USART,&GPS.rxTmp,1);
}
//##################################################################################################################
void	GPS_Process(void)
{
	if( (HAL_GetTick()-GPS.LastTime>50) && (GPS.rxIndex>0))
	{
		char	*str;
		#if (_GPS_DEBUG==1)
		printf("%s",GPS.rxBuffer);
		#endif
		str=strstr((char*)GPS.rxBuffer,"$GNGGA,");

	    if(str != NULL) {
	    	str[strcspn(str, "\r\n")] = '\0';       // Trim String to remove \n\r
	        if(NMEAValidateCRC(str)) {
	        	memset(&GPS.GPGGA,0,sizeof(GPS.GPGGA));
	            uint8_t *outStr = 0;
	            uint8_t u8Token[128];
	            uint8_t tokenCounter = 0;
	            while(strTok(str, &outStr, ",", u8Token, sizeof(u8Token))) {

	                // Split each token into the structre
	                switch(tokenCounter) {
	                    case 1:
	                        sscanf(u8Token,"%2d%2d%2d.%2d",&GPS.GPGGA.UTC_Hour,&GPS.GPGGA.UTC_Min,&GPS.GPGGA.UTC_Sec,&GPS.GPGGA.UTC_MicroSec);
	                        break;
	                    case 2:
	                    	GPS.GPGGA.Latitude = atof(u8Token);
	                        break;
	                    case 3:
	                    	GPS.GPGGA.NS_Indicator = u8Token[0];
	                        break;
	                    case 4:
	                    	GPS.GPGGA.Longitude = atof(u8Token);
	                        break;
	                    case 5:
	                    	GPS.GPGGA.EW_Indicator = u8Token[0];
	                        break;
	                    case 6:
	                    	GPS.GPGGA.PositionFixIndicator = atoi(u8Token);
	                        break;
	                    case 7:
	                    	GPS.GPGGA.SatellitesUsed = atoi(u8Token);
	                        break;
	                    case 8:
	                    	GPS.GPGGA.HDOP = atof(u8Token);
	                        break;
	                    case 9:
	                    	GPS.GPGGA.MSL_Altitude = atof(u8Token);
	                        break;
	                    case 10:
	                    	GPS.GPGGA.MSL_Units = u8Token[0];
	                        break;
	                    case 11:
	                    	GPS.GPGGA.Geoid_Separation = atof(u8Token);
	                        break;
	                    case 12:
	                    	GPS.GPGGA.Geoid_Units = u8Token[0];
	                        break;
	                    case 13:
	                    	GPS.GPGGA.AgeofDiffCorr = atoi(u8Token);
	                        break;
	                }
	                str = &(*outStr);
	                tokenCounter++;
	            }
				if(GPS.GPGGA.NS_Indicator==0)
					GPS.GPGGA.NS_Indicator='-';
				if(GPS.GPGGA.EW_Indicator==0)
					GPS.GPGGA.EW_Indicator='-';
				if(GPS.GPGGA.Geoid_Units==0)
					GPS.GPGGA.Geoid_Units='-';
				if(GPS.GPGGA.MSL_Units==0)
					GPS.GPGGA.MSL_Units='-';

				GPS.GPGGA.LatitudeDecimal=convertDegMinToDecDeg(GPS.GPGGA.Latitude) * (GPS.GPGGA.NS_Indicator == 'S' ? -1 : 1);
				GPS.GPGGA.LongitudeDecimal=convertDegMinToDecDeg(GPS.GPGGA.Longitude) * (GPS.GPGGA.EW_Indicator == 'W' ? -1 : 1);
	        }
	    }
		memset(GPS.rxBuffer,0,sizeof(GPS.rxBuffer));
		GPS.rxIndex=0;
	}
	HAL_UART_Receive_IT(&_GPS_USART,&GPS.rxTmp,1);
}


uint8_t NMEAValidateCRC(uint8_t *u8InputString) {
    uint16_t i = 0;
    uint8_t u8CalculatedChecksum = 0;
    uint8_t u8ContainedChecksum = (uint8_t)strtol(&u8InputString[strlen(u8InputString) - 2], NULL, 16);

    for(i = 1; i < strlen(u8InputString) - 3; i++) {
        u8CalculatedChecksum = u8CalculatedChecksum ^ u8InputString[i];
    }

    return (u8CalculatedChecksum == u8ContainedChecksum);
}

/*
 * https://stackoverflow.com/questions/3375530/c-parse-empty-tokens-from-a-string-with-strtok/55544721#55544721
 */
uint8_t strTok(uint8_t *u8InputString, uint8_t **u8ModifiedString, uint8_t *u8Delimiter, char* u8Token, uint8_t u8MaxTokenSize) {
    uint8_t* u8DelimiterFound = 0;
    int itokenLenght = 0;

    if(u8InputString == NULL || u8InputString == NULL || u8InputString == NULL || u8Delimiter == NULL) {
        return 0;
    }

    if(!u8InputString) return (char*) 0;

    u8DelimiterFound = strstr((char*)u8InputString, u8Delimiter);

    if(u8DelimiterFound){
        itokenLenght = u8DelimiterFound-u8InputString;
    } else {
        itokenLenght = strlen(u8InputString);
    }

    if(itokenLenght > u8MaxTokenSize) {
        itokenLenght = u8MaxTokenSize;
    }

    memcpy(u8Token, u8InputString, itokenLenght);
    u8Token[itokenLenght] = '\0';

    *u8ModifiedString = u8DelimiterFound ? u8DelimiterFound + strlen(u8Delimiter) : (uint8_t *)0;

    return 1;
}
//##################################################################################################################
