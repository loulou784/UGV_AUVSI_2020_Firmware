/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TELEM_MUX_Pin GPIO_PIN_1
#define TELEM_MUX_GPIO_Port GPIOA
#define TELEM_USART2_TX_Pin GPIO_PIN_2
#define TELEM_USART2_TX_GPIO_Port GPIOA
#define TELEM_USART2_RX_Pin GPIO_PIN_3
#define TELEM_USART2_RX_GPIO_Port GPIOA
#define DROP_CTRL_Pin GPIO_PIN_4
#define DROP_CTRL_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_0
#define LED_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_1
#define SD_CS_GPIO_Port GPIOB
#define BUZZER_TIM1_CH1_Pin GPIO_PIN_8
#define BUZZER_TIM1_CH1_GPIO_Port GPIOA
#define RIGHT_TIM2_CH1_Pin GPIO_PIN_15
#define RIGHT_TIM2_CH1_GPIO_Port GPIOA
#define LEFT_TIM2_CH2_Pin GPIO_PIN_3
#define LEFT_TIM2_CH2_GPIO_Port GPIOB
#define RIGHT_MOTOR_DIR_Pin GPIO_PIN_4
#define RIGHT_MOTOR_DIR_GPIO_Port GPIOB
#define LEFT_MOTOR_DIR_Pin GPIO_PIN_5
#define LEFT_MOTOR_DIR_GPIO_Port GPIOB
#define GPS_USART1_TX_Pin GPIO_PIN_6
#define GPS_USART1_TX_GPIO_Port GPIOB
#define GPS_USART1_RX_Pin GPIO_PIN_7
#define GPS_USART1_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
