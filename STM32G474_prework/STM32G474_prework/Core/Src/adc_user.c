#include "adc_user.h"

void ADC_Init(void)
{
  HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);

  HAL_GPIO_WritePin(GPIOC, HW_RNGSEL0_Pin | HW_RNGSEL1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SEQEN_GPIO_Port, SEQEN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(4);
}

void ADC_SwitchChannel(uint8_t channel)
{
  if (channel > 7)
  {
    channel = 0;
  }

  HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, (channel & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, (channel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int16_t ADC_Read_data(uint8_t Ch)
{
  int16_t adc_data1 = 0;

  HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, (Ch & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, (Ch & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, (Ch & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  delay_us(1);

  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

  while (HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

  while (HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
  delay_us(1);
  adc_data1 = Read_16BitData_Direct();
  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
  delay_us(1);

  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
  delay_us(1);
  (void)Read_16BitData_Direct();
  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

  return adc_data1;
}

void ADC_Read_AB(uint8_t Ch, int16_t *pA, int16_t *pB)
{
  ADC_SwitchChannel(Ch);
  delay_us(1);

  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

  while (HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

  while (HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
  delay_us(1);
  *pA = Read_16BitData_Direct();
  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
  delay_us(1);

  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
  delay_us(1);
  *pB = Read_16BitData_Direct();
  HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
}

void ADC_Read_AllChannels(uint16_t *adc_result)
{
  int16_t A, B;
  uint8_t ch;

  for (ch = 0; ch < 8; ch++)
  {
    ADC_Read_AB(ch, &A, &B);

    adc_result[ch] = A;
    adc_result[15 - ch] = B;
  }
}

int16_t Read_16BitData_Direct(void)
{
  int32_t port_value = GPIOD->IDR;
  int16_t data = 0;

  data |= ((port_value >> 7) & 0x01) << 0;
  data |= ((port_value >> 6) & 0x01) << 1;
  data |= ((port_value >> 5) & 0x01) << 2;
  data |= ((port_value >> 4) & 0x01) << 3;
  data |= ((port_value >> 3) & 0x01) << 4;
  data |= ((port_value >> 2) & 0x01) << 5;
  data |= ((port_value >> 1) & 0x01) << 6;
  data |= ((port_value >> 0) & 0x01) << 7;

  data |= ((port_value >> 15) & 0x01) << 8;
  data |= ((port_value >> 14) & 0x01) << 9;
  data |= ((port_value >> 13) & 0x01) << 10;
  data |= ((port_value >> 12) & 0x01) << 11;
  data |= ((port_value >> 11) & 0x01) << 12;
  data |= ((port_value >> 10) & 0x01) << 13;
  data |= ((port_value >> 9) & 0x01) << 14;
  data |= ((port_value >> 8) & 0x01) << 15;

  return data;
}
