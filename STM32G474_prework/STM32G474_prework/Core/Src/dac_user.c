#include "dac_user.h"
#include "spi.h"

void DAC_update(uint8_t CS_num, uint8_t Ch, uint16_t data)
{
  uint8_t Tx_buf[2] = {0x00, 0x00};
  uint16_t cmd = 0x8000 | (Ch << 12) | (data & 0x0FFF);
  GPIO_TypeDef *cs_port = NULL;
  uint16_t cs_pin = 0U;

  if (CS_num == 1)
  {
    cs_port = DAC_CS1_GPIO_Port;
    cs_pin = DAC_CS1_Pin;
  }
/*  else if (CS_num == 2)
  {
    cs_port = DAC_CS2_GPIO_Port;
    cs_pin = DAC_CS2_Pin;
  }
  else if (CS_num == 3)
  {
    cs_port = DAC_CS3_GPIO_Port;
    cs_pin = DAC_CS3_Pin;
  }
*/
  else
  {
    return;
  }

  Tx_buf[0] = (uint8_t)(cmd >> 8);
  Tx_buf[1] = (uint8_t)(cmd & 0xFF);

  HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, Tx_buf, 2, 0xFFF);
  HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
}
