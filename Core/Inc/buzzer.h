/*
 * buzzer.h
 *
 *  Created on: Mar 10, 2020
 *      Author: alexmorin
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include <stdint.h>
#include <stdbool.h>
#include "tim.h"

void tone(uint32_t u32Frequency, uint16_t u16Duration);
void playSong(uint16_t *pMelody, uint8_t *pTone, uint16_t u16Length, float fSpeed);

void playSongIT(uint16_t *pMelody, uint8_t *pTone, uint16_t u16Len, float fSpeed);
void stopSongIT();
void enableRepeat(bool bOnRepeat);

#endif /* INC_BUZZER_H_ */
