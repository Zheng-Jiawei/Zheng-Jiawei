#include "hrc_bus.h"

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} HRC_GpioPinDef;

static const HRC_GpioPinDef hrc_data_in_pins[8] =
{
  {HRC_DATA_IN0_GPIO_Port, HRC_DATA_IN0_Pin},
  {HRC_DATA_IN1_GPIO_Port, HRC_DATA_IN1_Pin},
  {HRC_DATA_IN2_GPIO_Port, HRC_DATA_IN2_Pin},
  {HRC_DATA_IN3_GPIO_Port, HRC_DATA_IN3_Pin},
  {HRC_DATA_IN4_GPIO_Port, HRC_DATA_IN4_Pin},
  {HRC_DATA_IN5_GPIO_Port, HRC_DATA_IN5_Pin},
  {HRC_DATA_IN6_GPIO_Port, HRC_DATA_IN6_Pin},
  {HRC_DATA_IN7_GPIO_Port, HRC_DATA_IN7_Pin}
};

static const HRC_GpioPinDef hrc_data_out_pins[8] =
{
  {HRC_DATA_OUT0_GPIO_Port, HRC_DATA_OUT0_Pin},
  {HRC_DATA_OUT1_GPIO_Port, HRC_DATA_OUT1_Pin},
  {HRC_DATA_OUT2_GPIO_Port, HRC_DATA_OUT2_Pin},
  {HRC_DATA_OUT3_GPIO_Port, HRC_DATA_OUT3_Pin},
  {HRC_DATA_OUT4_GPIO_Port, HRC_DATA_OUT4_Pin},
  {HRC_DATA_OUT5_GPIO_Port, HRC_DATA_OUT5_Pin},
  {HRC_DATA_OUT6_GPIO_Port, HRC_DATA_OUT6_Pin},
  {HRC_DATA_OUT7_GPIO_Port, HRC_DATA_OUT7_Pin}
};

void HRC_Bus_InitDefault(void)
{
  HRC_SetDataIn(0x00U);
  HRC_SetCmd(0U);
  HRC_SetRstn(0U);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_RESET);
}

void HRC_SetDataIn(uint8_t data)
{
  uint8_t i;

  for (i = 0U; i < 8U; i++)
  {
    HAL_GPIO_WritePin(hrc_data_in_pins[i].port,
                      hrc_data_in_pins[i].pin,
                      (data & (1U << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

void HRC_SetCmd(uint8_t level)
{
  HAL_GPIO_WritePin(HRC_CMD_GPIO_Port, HRC_CMD_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HRC_SetRstn(uint8_t level)
{
  HAL_GPIO_WritePin(HRC_RSTN_GPIO_Port, HRC_RSTN_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HRC_ClockPulse(void)
{
  delay_us(HRC_CLK_LOW_US);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_SET);
  delay_us(HRC_CLK_HIGH_US);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_RESET);
}

/* Generate one clock and sample the HRC output at the falling edge. */
void HRC_ClockPulseAndRead(uint8_t *valid_out, uint8_t *data_out)
{
  delay_us(HRC_CLK_LOW_US);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_SET);
  delay_us(HRC_CLK_HIGH_US);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_RESET);

  *valid_out = HRC_ReadValidOut();
  *data_out = HRC_ReadDataOut();
}

void HRC_TransferCycle(uint8_t cmd,
                       uint8_t data_in,
                       uint8_t *valid_out,
                       uint8_t *data_out)
{
  HRC_SetCmd(cmd);
  HRC_SetDataIn(data_in);
  HRC_ClockPulseAndRead(valid_out, data_out);
}

void HRC_ClockCycles(uint16_t n)
{
  uint16_t i;

  for (i = 0U; i < n; i++)
  {
    HRC_ClockPulse();
  }
}

void HRC_SendCommand(uint8_t cmd)
{
  HRC_SetCmd(1U);
  HRC_SetDataIn(cmd);
  HRC_ClockPulse();
}

void HRC_SendData(uint8_t data)
{
  HRC_SetCmd(0U);
  HRC_SetDataIn(data);
  HRC_ClockPulse();
}

uint8_t HRC_ReadDataOut(void)
{
  uint8_t data = 0U;
  uint8_t i;

  for (i = 0U; i < HRC_DATA_OUT_WIDTH; i++)
  {
    if (HAL_GPIO_ReadPin(hrc_data_out_pins[i].port, hrc_data_out_pins[i].pin) == GPIO_PIN_SET)
    {
      data |= (uint8_t)(1U << i);
    }
  }

  return data;
}

uint8_t HRC_ReadValidOut(void)
{
  return (HAL_GPIO_ReadPin(HRC_VALID_OUT_GPIO_Port, HRC_VALID_OUT_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

HRC_StatusTypeDef HRC_WaitIdle(uint32_t max_cycles)
{
  uint32_t cycle;
  uint8_t idle_count = 0U;
  uint8_t valid_out;
  uint8_t data_out;

  for (cycle = 0U; cycle < max_cycles; cycle++)
  {
    HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);

    if ((valid_out == 0U) && (data_out == 0x00U))
    {
      idle_count++;
      if (idle_count >= HRC_IDLE_CONFIRM_CYCLES)
      {
        HRC_ClockCycles(HRC_IDLE_GUARD_CYCLES);
        return HRC_OK;
      }
    }
    else
    {
      idle_count = 0U;
    }
  }

  return HRC_TIMEOUT;
}
