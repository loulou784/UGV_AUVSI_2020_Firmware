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

// Device Config
#define PWM_TIMER    (&htim1)
#define PWM_TIMER_CH (TIM_CHANNEL_1)
#define DELAY_TIMER  (&htim16)

void buzzerSetEnable(bool bEnable);
void buzzerPlaySong(uint16_t *pMelody, uint8_t *pTone, uint16_t u16Len, float fSpeed);
void buzzerStopSong();
void buzzerEnableRepeat(bool bOnRepeat);
void buzzerPlayNoteITCallback();
bool buzzerIsCurrentlyPlaying();

#endif /* INC_BUZZER_H_ */
