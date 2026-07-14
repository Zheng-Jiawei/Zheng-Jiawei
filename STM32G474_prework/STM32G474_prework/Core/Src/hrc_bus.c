#include "hrc_bus.h"

/* Prefer CubeMX HRC_* labels.  The fallback aliases keep this file buildable
 * before main.h is regenerated from the updated .ioc.
 */
#if !defined(HRC_DATA_IN0_Pin) && defined(M203_DATA_IN0_Pin)
#define HRC_DATA_IN0_Pin             M203_DATA_IN0_Pin
#define HRC_DATA_IN0_GPIO_Port       M203_DATA_IN0_GPIO_Port
#define HRC_DATA_IN1_Pin             M203_DATA_IN1_Pin
#define HRC_DATA_IN1_GPIO_Port       M203_DATA_IN1_GPIO_Port
#define HRC_DATA_IN2_Pin             M203_DATA_IN2_Pin
#define HRC_DATA_IN2_GPIO_Port       M203_DATA_IN2_GPIO_Port
#define HRC_DATA_IN3_Pin             M203_DATA_IN3_Pin
#define HRC_DATA_IN3_GPIO_Port       M203_DATA_IN3_GPIO_Port
#define HRC_DATA_IN4_Pin             M203_DATA_IN4_Pin
#define HRC_DATA_IN4_GPIO_Port       M203_DATA_IN4_GPIO_Port
#define HRC_DATA_IN5_Pin             M203_DATA_IN5_Pin
#define HRC_DATA_IN5_GPIO_Port       M203_DATA_IN5_GPIO_Port
#define HRC_DATA_IN6_Pin             M203_DATA_IN6_Pin
#define HRC_DATA_IN6_GPIO_Port       M203_DATA_IN6_GPIO_Port
#define HRC_DATA_IN7_Pin             M203_DATA_IN7_Pin
#define HRC_DATA_IN7_GPIO_Port       M203_DATA_IN7_GPIO_Port
#endif

#if !defined(HRC_RSTN_Pin) && defined(M203_RSTN_Pin)
#define HRC_RSTN_Pin                 M203_RSTN_Pin
#define HRC_RSTN_GPIO_Port           M203_RSTN_GPIO_Port
#endif

#if !defined(HRC_CMD_Pin) && defined(M203_CMD_Pin)
#define HRC_CMD_Pin                  M203_CMD_Pin
#define HRC_CMD_GPIO_Port            M203_CMD_GPIO_Port
#endif

#if !defined(HRC_CLK_Pin) && defined(M203_CLK_Pin)
#define HRC_CLK_Pin                  M203_CLK_Pin
#define HRC_CLK_GPIO_Port            M203_CLK_GPIO_Port
#endif

#if !defined(HRC_DATA_OUT0_Pin) && defined(GWL2_A0_Pin)
#define HRC_DATA_OUT0_Pin            GWL2_A0_Pin
#define HRC_DATA_OUT0_GPIO_Port      GWL2_A0_GPIO_Port
#define HRC_DATA_OUT1_Pin            GWL2_A1_Pin
#define HRC_DATA_OUT1_GPIO_Port      GWL2_A1_GPIO_Port
#define HRC_DATA_OUT2_Pin            GSL3_EN_Pin
#define HRC_DATA_OUT2_GPIO_Port      GSL3_EN_GPIO_Port
#define HRC_DATA_OUT3_Pin            GSL3_A0_Pin
#define HRC_DATA_OUT3_GPIO_Port      GSL3_A0_GPIO_Port
#define HRC_DATA_OUT4_Pin            GSL3_A1_Pin
#define HRC_DATA_OUT4_GPIO_Port      GSL3_A1_GPIO_Port
#define HRC_DATA_OUT5_Pin            GBL3_A1_Pin
#define HRC_DATA_OUT5_GPIO_Port      GBL3_A1_GPIO_Port
#define HRC_DATA_OUT6_Pin            GBL3_A0_Pin
#define HRC_DATA_OUT6_GPIO_Port      GBL3_A0_GPIO_Port
#define HRC_DATA_OUT7_Pin            GBL3_EN_Pin
#define HRC_DATA_OUT7_GPIO_Port      GBL3_EN_GPIO_Port
#endif

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
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_SET);
  delay_us(1U);
  HAL_GPIO_WritePin(HRC_CLK_GPIO_Port, HRC_CLK_Pin, GPIO_PIN_RESET);
  delay_us(1U);
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
#if defined(HRC_VALID_OUT_Pin) && defined(HRC_VALID_OUT_GPIO_Port)
  return (HAL_GPIO_ReadPin(HRC_VALID_OUT_GPIO_Port, HRC_VALID_OUT_Pin) == GPIO_PIN_SET) ? 1U : 0U;
#else
  return 0U;
#endif
}

HRC_StatusTypeDef HRC_WaitIdle(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();
  uint8_t idle_count = 0U;

  while ((HAL_GetTick() - start) <= timeout_ms)
  {
    if ((HRC_ReadValidOut() == 0U) && (HRC_ReadDataOut() == 0x00U))
    {
      idle_count++;
      if (idle_count >= 2U)
      {
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

HRC_StatusTypeDef HRC_WaitValid(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  while ((HAL_GetTick() - start) <= timeout_ms)
  {
    if (HRC_ReadValidOut() != 0U)
    {
      return HRC_OK;
    }
  }

  return HRC_TIMEOUT;
}
