/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, M203_DATA_IN2_Pin|M203_DATA_IN3_Pin|M203_DATA_IN4_Pin|M203_DATA_IN5_Pin
                          |M203_DATA_IN6_Pin|M203_DATA_IN7_Pin|M203_RSTN_Pin|M203_CMD_Pin
                          |M203_CLK_Pin|CONVST_Pin|M203_DATA_IN0_Pin|M203_DATA_IN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, EN_1_1V_Pin|EN_2_5V_Pin|GWL2_A0_Pin|GWL2_A1_Pin
                          |GSL3_EN_Pin|GSL3_A0_Pin|HW_RNGSEL0_Pin|HW_RNGSEL1_Pin
                          |GSL1_A0_Pin|GSL1_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, EN_3V_Pin|FPGA_IO_Pin|GWL2_EN_Pin|GSL3_A1_Pin
                          |GSL2_A1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, NRST_Pin|ADC_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, DAC_CS3_Pin|DAC_CS2_Pin|DAC_CS1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ADC_CHSEL2_Pin|ADC_CHSEL1_Pin|ADC_CHSEL0_Pin|BURST_Pin
                          |GBL1_A0_Pin|GBL1_EN_Pin|GBL1_A1_Pin|GBL2_A1_Pin
                          |GBL2_A0_Pin|GBL2_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ADC_CS_Pin|RD_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, SEQEN_Pin|GSL3_ADC_X2_SEL_Pin|GSL1_A1_Pin|GSL2_A0_Pin
                          |GSL2_EN_Pin|GBL3_A1_Pin|GBL3_A0_Pin|GBL3_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GSL1_ADC_X0_SEL_Pin|GSL2_ADC_X1_SEL_Pin|GWL1_A1_Pin|GWL1_A0_Pin
                          |GWL1_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : M203_DATA_IN2_Pin M203_DATA_IN3_Pin M203_DATA_IN4_Pin M203_DATA_IN5_Pin
                           M203_DATA_IN6_Pin M203_DATA_IN7_Pin M203_RSTN_Pin M203_CMD_Pin
                           M203_CLK_Pin DAC_CS3_Pin DAC_CS2_Pin DAC_CS1_Pin
                           M203_DATA_IN0_Pin M203_DATA_IN1_Pin */
  GPIO_InitStruct.Pin = M203_DATA_IN2_Pin|M203_DATA_IN3_Pin|M203_DATA_IN4_Pin|M203_DATA_IN5_Pin
                          |M203_DATA_IN6_Pin|M203_DATA_IN7_Pin|M203_RSTN_Pin|M203_CMD_Pin
                          |M203_CLK_Pin|DAC_CS3_Pin|DAC_CS2_Pin|DAC_CS1_Pin
                          |M203_DATA_IN0_Pin|M203_DATA_IN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : EN_1_1V_Pin EN_2_5V_Pin GWL2_A0_Pin GWL2_A1_Pin
                           GSL3_EN_Pin GSL3_A0_Pin HW_RNGSEL0_Pin HW_RNGSEL1_Pin
                           GSL1_A0_Pin GSL1_EN_Pin */
  GPIO_InitStruct.Pin = EN_1_1V_Pin|EN_2_5V_Pin|GWL2_A0_Pin|GWL2_A1_Pin
                          |GSL3_EN_Pin|GSL3_A0_Pin|HW_RNGSEL0_Pin|HW_RNGSEL1_Pin
                          |GSL1_A0_Pin|GSL1_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PG_1_1V_Pin */
  GPIO_InitStruct.Pin = PG_1_1V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PG_1_1V_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PG_1_1VF3_Pin PG_3V_Pin */
  GPIO_InitStruct.Pin = PG_1_1VF3_Pin|PG_3V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : EN_3V_Pin FPGA_IO_Pin GWL2_EN_Pin GSL3_A1_Pin
                           LED1_Pin LED2_Pin LED3_Pin GSL2_A1_Pin */
  GPIO_InitStruct.Pin = EN_3V_Pin|FPGA_IO_Pin|GWL2_EN_Pin|GSL3_A1_Pin
                          |LED1_Pin|LED2_Pin|LED3_Pin|GSL2_A1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : NRST_Pin ADC_RESET_Pin GSL3_ADC_X2_SEL_Pin GSL1_A1_Pin
                           GSL2_A0_Pin GSL2_EN_Pin GBL3_A1_Pin GBL3_A0_Pin
                           GBL3_EN_Pin */
  GPIO_InitStruct.Pin = NRST_Pin|ADC_RESET_Pin|GSL3_ADC_X2_SEL_Pin|GSL1_A1_Pin
                          |GSL2_A0_Pin|GSL2_EN_Pin|GBL3_A1_Pin|GBL3_A0_Pin
                          |GBL3_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY2_Pin KEY1_Pin */
  GPIO_InitStruct.Pin = KEY2_Pin|KEY1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PG_3_3V_Pin */
  GPIO_InitStruct.Pin = PG_3_3V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PG_3_3V_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CONVST_Pin */
  GPIO_InitStruct.Pin = CONVST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CONVST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BUSY_Pin */
  GPIO_InitStruct.Pin = BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ADC_CHSEL2_Pin ADC_CHSEL1_Pin ADC_CHSEL0_Pin ADC_CS_Pin
                           RD_Pin */
  GPIO_InitStruct.Pin = ADC_CHSEL2_Pin|ADC_CHSEL1_Pin|ADC_CHSEL0_Pin|ADC_CS_Pin
                          |RD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BURST_Pin GBL1_A0_Pin GBL1_EN_Pin GBL1_A1_Pin
                           GBL2_A1_Pin GBL2_A0_Pin GBL2_EN_Pin */
  GPIO_InitStruct.Pin = BURST_Pin|GBL1_A0_Pin|GBL1_EN_Pin|GBL1_A1_Pin
                          |GBL2_A1_Pin|GBL2_A0_Pin|GBL2_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DB15_Pin DB14_Pin DB13_Pin DB12_Pin
                           DB11_Pin DB10_Pin DB9_Pin DB8_Pin
                           DB7_Pin DB6_Pin DB5_Pin DB4_Pin
                           DB3_Pin DB2_Pin DB1_Pin DB0_Pin */
  GPIO_InitStruct.Pin = DB15_Pin|DB14_Pin|DB13_Pin|DB12_Pin
                          |DB11_Pin|DB10_Pin|DB9_Pin|DB8_Pin
                          |DB7_Pin|DB6_Pin|DB5_Pin|DB4_Pin
                          |DB3_Pin|DB2_Pin|DB1_Pin|DB0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : SEQEN_Pin */
  GPIO_InitStruct.Pin = SEQEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SEQEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GSL1_ADC_X0_SEL_Pin GSL2_ADC_X1_SEL_Pin GWL1_A1_Pin GWL1_A0_Pin
                           GWL1_EN_Pin */
  GPIO_InitStruct.Pin = GSL1_ADC_X0_SEL_Pin|GSL2_ADC_X1_SEL_Pin|GWL1_A1_Pin|GWL1_A0_Pin
                          |GWL1_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
