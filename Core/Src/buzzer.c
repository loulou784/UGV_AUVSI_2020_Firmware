/*
 * buzzer.c
 *
 *  Created on: Mar 10, 2020
 *      Author: alexmorin
 */

#include "buzzer.h"

uint16_t *pu16SongMelody;
uint8_t *pu8SongTone;
uint16_t u16Index;
uint16_t u16Length;
bool bIsPlaying;
bool bState;
bool bRepeat;
float fSongSpeed;

void playNoteIT();

void playSongIT(uint16_t *pMelody, uint8_t *pTone, uint16_t u16Len, float fSpeed) {
	bIsPlaying = true;
	pu16SongMelody = pMelody;
	pu8SongTone = pTone;
	bState = 0;
	u16Index = 0;
	fSongSpeed = fSpeed;
	u16Length = u16Len;
	bRepeat = false;

	//Start 1ms Timer interrupt
	//__HAL_TIM_SET_COUNTER(&htim16, )
	HAL_TIM_Base_Start_IT(&htim16);

	playNoteIT();
}

void stopSongIT() {
	//Set duty cycle to 0%
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

	//Start 1ms Timer interrupt
	HAL_TIM_Base_Stop_IT(&htim16);

	bIsPlaying = false;
}

void playNoteIT() {
	if(bState == true) {
		//We play a note...
		bState = false;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);

		//Set required frequency
		if(pu16SongMelody[u16Index] != 0) {
			int freq = (80000000/pu16SongMelody[u16Index])/1000;
			__HAL_TIM_SET_PRESCALER(&htim1, freq);
		} else {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		}

		__HAL_TIM_SET_AUTORELOAD(&htim16, (10000 / pu8SongTone[u16Index]));

	} else {
		//We add a delay
		bState = true;

		//Set duty cycle to 0%
	    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
	    __HAL_TIM_SET_AUTORELOAD(&htim16, (10000 / pu8SongTone[u16Index] * fSongSpeed) + 1);
	    u16Index++;

	    if(!(u16Index < u16Length)) {
	    	if(bRepeat == true) {
	    		u16Index = 0;
	    	} else {
	    		stopSongIT();
	    	}
	    }
	}
}

void enableRepeat(bool bOnRepeat) {
	bRepeat = bOnRepeat;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == htim16.Instance)
    {
    	playNoteIT();
    }
}
