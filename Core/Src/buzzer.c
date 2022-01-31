/*
 * buzzer.c
 *
 *  Created on: Mar 10, 2020
 *      Author: alexmorin
 */

#include "buzzer.h"

// Private variable
volatile uint16_t *pu16SongMelody;
volatile uint8_t *pu8SongTone;
volatile uint16_t u16Index;
volatile uint16_t u16Length;
volatile bool bIsPlaying;
volatile uint8_t bState;
volatile bool bRepeat;
volatile float fSongSpeed;
bool bBuzzerEnabled = true;


/*
 * Plays the desired melody
 * *pMelody: pointer to a uint16_t array of melody, must be same size as u16Len
 * *pTone: pointer to a uint8_t array of tone, must be same size as u16Len
 * u16Len: uint16_t that specifies the length of pTone and pMelody array
 * fSpeed: Speed divider of the song, below 1 song is gonna be faster, above will slow it down
 */
void buzzerPlaySong(uint16_t *pMelody, uint8_t *pTone, uint16_t u16Len, float fSpeed) {

	if(bBuzzerEnabled == false) {
		return;
	}

	if(pMelody == NULL || pTone == NULL || u16Len == 0 || fSpeed <= 0) {
		return;
	}

	bIsPlaying = true;
	pu16SongMelody = pMelody;
	pu8SongTone = pTone;
	bState = true;
	u16Index = 0;
	fSongSpeed = fSpeed;
	u16Length = u16Len;
	bRepeat = false;

	// Start 1ms Timer interrupt
	HAL_TIM_PWM_Start(PWM_TIMER, PWM_TIMER_CH);
	HAL_TIM_Base_Start_IT(DELAY_TIMER);
}

/*
 * Stops the currently playing song, resets repeat
 */
void buzzerStopSong() {
	//Set duty cycle to 0%
	__HAL_TIM_SET_COMPARE(PWM_TIMER, PWM_TIMER_CH, 0);
	HAL_TIM_PWM_Stop(PWM_TIMER, PWM_TIMER_CH);

	//Start 1ms Timer interrupt
	HAL_TIM_Base_Stop_IT(DELAY_TIMER);

	bIsPlaying = false;
	bRepeat = false;
}

/*
 * Plays current song indefinetly
 * bOnReapeat: enables repeat when true
 */
void buzzerEnableRepeat(bool bOnRepeat) {
	bRepeat = bOnRepeat;
}

/*
 * Returns true if the song is playing, false otherwise
 */
bool buzzerIsCurrentlyPlaying() {
	return bIsPlaying;
}

/*
 * Call this function in the TIMER ISR of DELAY_TIMER, otherwise it wont work
 */
void buzzerPlayNoteITCallback() {
	if((u16Index == u16Length)) {
		if(bRepeat == true) {
	   		u16Index = 0;
	   	} else {
	   		buzzerStopSong();
	   		return;
	   	}
	}

	if(bState == true) {
		// Play a frequency

		// Set PWM duty cycle to 50%
		__HAL_TIM_SET_COMPARE(PWM_TIMER, PWM_TIMER_CH, 500);

		//Set required PWM frequency
		if(pu16SongMelody[u16Index] != 0) {
			int freq = (80000000/pu16SongMelody[u16Index])/1000;
			__HAL_TIM_SET_PRESCALER(PWM_TIMER, freq);
		} else {
			__HAL_TIM_SET_COMPARE(PWM_TIMER, PWM_TIMER_CH, 0);
		}

		__HAL_TIM_SET_COUNTER(DELAY_TIMER, 0);
		__HAL_TIM_SET_AUTORELOAD(DELAY_TIMER, (uint32_t)(10000 / pu8SongTone[u16Index]));

		bState = false;
	} else if (bState == false) {
		// Play a silence

		//Set PWM duty cycle to 0%
		__HAL_TIM_SET_COMPARE(PWM_TIMER, PWM_TIMER_CH, 0);

		__HAL_TIM_SET_COUNTER(DELAY_TIMER, 0);
		__HAL_TIM_SET_AUTORELOAD(DELAY_TIMER, (uint32_t)(10000 / pu8SongTone[u16Index] / fSongSpeed));

		u16Index++;

		bState = true;
	}
}

/*
 * Disbale audio completly
 * bEnable: Disable all audio generation when false
 */
void buzzerSetEnable(bool bEnable) {
	if(bIsPlaying && !bEnable) {
		buzzerStopSong();
	}
	bBuzzerEnabled = bEnable;
}
