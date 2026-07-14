/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32g4xx_hal.h"

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
extern uint8_t FPGA_rx_flag;

extern uint8_t FPGA_tx_buff[1024];
extern uint8_t FPGA_buff[256];
extern uint16_t ADC_data[1024];

extern uint8_t Reset_flag;

extern uint16_t FPGA_Row_start;
extern uint16_t FPGA_RowS;

extern uint8_t FPGA_Col_Card;
extern uint8_t Rows_and_cols_flag;

extern uint8_t FPGA_Row_Reg_start;
extern uint8_t FPGA_Row_Reg_end;

extern uint16_t FPGA_Col_start;
extern uint16_t FPGA_ColS;

extern uint8_t Valid_data_byte;
extern uint8_t FPGA_Read_num;

extern uint16_t rec_index;
extern uint16_t Dispose_index;
extern uint8_t Dispose_read_flag;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HRC_DATA_IN2_Pin GPIO_PIN_2
#define HRC_DATA_IN2_GPIO_Port GPIOE
#define HRC_DATA_IN3_Pin GPIO_PIN_3
#define HRC_DATA_IN3_GPIO_Port GPIOE
#define HRC_DATA_IN4_Pin GPIO_PIN_4
#define HRC_DATA_IN4_GPIO_Port GPIOE
#define HRC_DATA_IN5_Pin GPIO_PIN_5
#define HRC_DATA_IN5_GPIO_Port GPIOE
#define HRC_DATA_IN6_Pin GPIO_PIN_6
#define HRC_DATA_IN6_GPIO_Port GPIOE
#define PG_1_1V_Pin GPIO_PIN_14
#define PG_1_1V_GPIO_Port GPIOC
#define EN_2_5V_Pin GPIO_PIN_15
#define EN_2_5V_GPIO_Port GPIOC
#define PG_1_1VF3_Pin GPIO_PIN_3
#define PG_1_1VF3_GPIO_Port GPIOF
#define EN_3V_Pin GPIO_PIN_4
#define EN_3V_GPIO_Port GPIOF
#define PG_3V_Pin GPIO_PIN_5
#define PG_3V_GPIO_Port GPIOF
#define FPGA_IO_Pin GPIO_PIN_9
#define FPGA_IO_GPIO_Port GPIOF
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
#define HRC_DATA_OUT0_Pin GPIO_PIN_0
#define HRC_DATA_OUT0_GPIO_Port GPIOC
#define HRC_DATA_OUT1_Pin GPIO_PIN_1
#define HRC_DATA_OUT1_GPIO_Port GPIOC
#define HRC_DATA_OUT2_Pin GPIO_PIN_2
#define HRC_DATA_OUT2_GPIO_Port GPIOC
#define HRC_DATA_OUT3_Pin GPIO_PIN_3
#define HRC_DATA_OUT3_GPIO_Port GPIOC
#define HRC_DATA_OUT4_Pin GPIO_PIN_2
#define HRC_DATA_OUT4_GPIO_Port GPIOF
#define KEY2_Pin GPIO_PIN_3
#define KEY2_GPIO_Port GPIOA
#define KEY1_Pin GPIO_PIN_4
#define KEY1_GPIO_Port GPIOA
#define PG_3_3V_Pin GPIO_PIN_2
#define PG_3_3V_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOF
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOF
#define HRC_DATA_IN7_Pin GPIO_PIN_7
#define HRC_DATA_IN7_GPIO_Port GPIOE
#define HRC_RSTN_Pin GPIO_PIN_8
#define HRC_RSTN_GPIO_Port GPIOE
#define HRC_CMD_Pin GPIO_PIN_9
#define HRC_CMD_GPIO_Port GPIOE
#define HRC_CLK_Pin GPIO_PIN_10
#define HRC_CLK_GPIO_Port GPIOE
#define DAC_CS1_Pin GPIO_PIN_13
#define DAC_CS1_GPIO_Port GPIOE
#define CONVST_Pin GPIO_PIN_14
#define CONVST_GPIO_Port GPIOE
#define BUSY_Pin GPIO_PIN_15
#define BUSY_GPIO_Port GPIOE
#define ADC_CHSEL2_Pin GPIO_PIN_10
#define ADC_CHSEL2_GPIO_Port GPIOB
#define ADC_CHSEL1_Pin GPIO_PIN_11
#define ADC_CHSEL1_GPIO_Port GPIOB
#define ADC_CHSEL0_Pin GPIO_PIN_12
#define ADC_CHSEL0_GPIO_Port GPIOB
#define ADC_CS_Pin GPIO_PIN_13
#define ADC_CS_GPIO_Port GPIOB
#define RD_Pin GPIO_PIN_14
#define RD_GPIO_Port GPIOB
#define BURST_Pin GPIO_PIN_15
#define BURST_GPIO_Port GPIOB
#define DB15_Pin GPIO_PIN_8
#define DB15_GPIO_Port GPIOD
#define DB14_Pin GPIO_PIN_9
#define DB14_GPIO_Port GPIOD
#define DB13_Pin GPIO_PIN_10
#define DB13_GPIO_Port GPIOD
#define DB12_Pin GPIO_PIN_11
#define DB12_GPIO_Port GPIOD
#define DB11_Pin GPIO_PIN_12
#define DB11_GPIO_Port GPIOD
#define DB10_Pin GPIO_PIN_13
#define DB10_GPIO_Port GPIOD
#define DB9_Pin GPIO_PIN_14
#define DB9_GPIO_Port GPIOD
#define DB8_Pin GPIO_PIN_15
#define DB8_GPIO_Port GPIOD
#define HW_RNGSEL0_Pin GPIO_PIN_6
#define HW_RNGSEL0_GPIO_Port GPIOC
#define HW_RNGSEL1_Pin GPIO_PIN_7
#define HW_RNGSEL1_GPIO_Port GPIOC
#define SEQEN_Pin GPIO_PIN_0
#define SEQEN_GPIO_Port GPIOG
#define ADC_RESET_Pin GPIO_PIN_1
#define ADC_RESET_GPIO_Port GPIOG
#define GSL1_A1_Pin GPIO_PIN_4
#define GSL1_A1_GPIO_Port GPIOG
#define GSL1_A0_Pin GPIO_PIN_8
#define GSL1_A0_GPIO_Port GPIOC
#define GSL1_EN_Pin GPIO_PIN_9
#define GSL1_EN_GPIO_Port GPIOC
#define GSL1_ADC_X0_SEL_Pin GPIO_PIN_8
#define GSL1_ADC_X0_SEL_GPIO_Port GPIOA
#define GWL1_A1_Pin GPIO_PIN_10
#define GWL1_A1_GPIO_Port GPIOA
#define GWL1_A0_Pin GPIO_PIN_11
#define GWL1_A0_GPIO_Port GPIOA
#define GWL1_EN_Pin GPIO_PIN_12
#define GWL1_EN_GPIO_Port GPIOA
#define HRC_DATA_OUT5_Pin GPIO_PIN_7
#define HRC_DATA_OUT5_GPIO_Port GPIOG
#define HRC_DATA_OUT6_Pin GPIO_PIN_8
#define HRC_DATA_OUT6_GPIO_Port GPIOG
#define HRC_DATA_OUT7_Pin GPIO_PIN_9
#define HRC_DATA_OUT7_GPIO_Port GPIOG
#define DB7_Pin GPIO_PIN_0
#define DB7_GPIO_Port GPIOD
#define DB6_Pin GPIO_PIN_1
#define DB6_GPIO_Port GPIOD
#define DB5_Pin GPIO_PIN_2
#define DB5_GPIO_Port GPIOD
#define DB4_Pin GPIO_PIN_3
#define DB4_GPIO_Port GPIOD
#define DB3_Pin GPIO_PIN_4
#define DB3_GPIO_Port GPIOD
#define DB2_Pin GPIO_PIN_5
#define DB2_GPIO_Port GPIOD
#define DB1_Pin GPIO_PIN_6
#define DB1_GPIO_Port GPIOD
#define DB0_Pin GPIO_PIN_7
#define DB0_GPIO_Port GPIOD
#define GBL1_A0_Pin GPIO_PIN_3
#define GBL1_A0_GPIO_Port GPIOB
#define GBL1_EN_Pin GPIO_PIN_4
#define GBL1_EN_GPIO_Port GPIOB
#define GBL1_A1_Pin GPIO_PIN_5
#define GBL1_A1_GPIO_Port GPIOB
#define HRC_VALID_OUT_Pin GPIO_PIN_9
#define HRC_VALID_OUT_GPIO_Port GPIOB
#define HRC_DATA_IN0_Pin GPIO_PIN_0
#define HRC_DATA_IN0_GPIO_Port GPIOE
#define HRC_DATA_IN1_Pin GPIO_PIN_1
#define HRC_DATA_IN1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
void delay_us(uint16_t nus);
uint16_t CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN);


void SPI3_DMA_Reset(void);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
