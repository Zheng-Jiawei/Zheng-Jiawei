#include "hrc_test.h"
#include "pc_comm.h"

static uint8_t hrc_adc_samples[HRC_ADC_SAMPLE_MAX];

static void HRC_SetTestLed(uint8_t passed)
{
  /* The board initializes LEDs high; LED2 is therefore treated as active-low. */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin,
                    (passed != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
//
static HRC_StatusTypeDef HRC_WaitIdleByClock(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t idle_count = 0U;
    uint8_t valid_out;
    uint8_t data_out;

    while ((HAL_GetTick() - start) <= timeout_ms)
    {
        HRC_ClockPulseAndRead(&valid_out, &data_out);

        if ((valid_out == 0U) && (data_out == 0x00U))
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

static uint8_t HRC_BitReverse6(uint8_t value)
{
  uint8_t result = 0U;
  uint8_t i;

  value &= 0x3FU;
  for (i = 0U; i < 6U; i++)
  {
    if (value & (1U << i))
    {
      result |= (uint8_t)(1U << (5U - i));
    }
  }

  return result;
}
/*初始化测试程序
测试流程为：
1. LED2 熄灭。
2. 初始化 HRC 总线，使 RSTN=0。
3. 发送 8 个干净的 CLK 周期。
4. 检查复位态：VALID_OUT=0 且 DATA_OUT=0xA5。
5. 释放复位：RSTN=1。
6. 每产生一个 CLK 周期后读取一次输出，确认 VALID_OUT=0、DATA_OUT=0x00 连续满足 2 个时钟周期。
7. 全部通过后点亮 LED2，并通过串口输出 INITIAL_TEST passed；任一步失败则保持 LED2 熄灭，并打印失败时的 VALID、DATA 值。
*/
void HRC_Test_Initial(void)
{
  HRC_StatusTypeDef status;

  HRC_SetTestLed(0U);
  HRC_Bus_InitDefault();
  HRC_ClockCycles(8U);

  if ((HRC_ReadValidOut() != 0U) || (HRC_ReadDataOut() != 0xA5U))
  {
    PC_Printf("INITIAL_TEST reset-state failed: VALID=%u DATA=0x%02X\r\n",
              (unsigned int)HRC_ReadValidOut(),
              (unsigned int)HRC_ReadDataOut());
    return;
  }

  HRC_SetRstn(1U);
  status = HRC_WaitIdleByClock(HRC_INITIAL_TEST_TIMEOUT_MS);
  if (status != HRC_OK)
  {
    PC_Printf("INITIAL_TEST idle-state failed: VALID=%u DATA=0x%02X\r\n",
              (unsigned int)HRC_ReadValidOut(),
              (unsigned int)HRC_ReadDataOut());
    return;
  }

  HRC_SetTestLed(1U);
  PC_Printf("INITIAL_TEST passed\r\n");
}

void HRC_Test_CfgDefault(void)
{
  uint8_t cfg[HRC_CFG_REG_NUM];
  HRC_StatusTypeDef status;

  status = HRC_ReadCfgTotal(cfg);
  PC_Printf("CFG default read status=%u\r\n", (unsigned int)status);
  if (status == HRC_OK)
  {
    PC_SendCfgTable(cfg, HRC_CFG_REG_NUM);
  }
}

void HRC_Test_CfgSingle(uint8_t addr, uint8_t value)
{
  uint8_t readback = 0U;
  HRC_StatusTypeDef status;

  status = HRC_WriteCfgSingle(addr, value);
  PC_Printf("WRITE_CFG addr=%u value=0x%02X status=%u\r\n",
            (unsigned int)addr,
            (unsigned int)value,
            (unsigned int)status);

  if (status != HRC_OK)
  {
    return;
  }

  status = HRC_ReadCfgSingle(addr, &readback);
  PC_Printf("READ_CFG addr=%u value=0x%02X status=%u\r\n",
            (unsigned int)addr,
            (unsigned int)readback,
            (unsigned int)status);
}

void HRC_Test_CfgTotal(void)
{
  HRC_StatusTypeDef status;
  uint8_t *cfg = HRC_CFG_GetShadowTable();

  status = HRC_WriteCfgTotal(cfg);
  PC_Printf("WRITE_CFG total status=%u\r\n", (unsigned int)status);
  if (status != HRC_OK)
  {
    return;
  }

  status = HRC_CFG_VerifyTotal(cfg);
  PC_Printf("VERIFY_CFG total status=%u\r\n", (unsigned int)status);
}

void HRC_Test_ADC_Single(void)
{
  uint8_t raw;
  uint8_t code;

  HRC_SendCommand(HRC_CMD_ADC_TEST);
  delay_us(HRC_ADC_TCONV_MARGIN_US);
  raw = HRC_ReadDataOut() & 0x3FU;
  code = HRC_BitReverse6(raw);
  HRC_SendCommand(HRC_CMD_IDLE);
  (void)HRC_WaitIdle(HRC_CFG_DEFAULT_TIMEOUT_MS);

  PC_Printf("ADC_TEST raw=0x%02X code=%u\r\n",
            (unsigned int)raw,
            (unsigned int)code);
}

void HRC_Test_ADC_Continuous(uint16_t sample_count)
{
  uint16_t i;

  if (sample_count > HRC_ADC_SAMPLE_MAX)
  {
    sample_count = HRC_ADC_SAMPLE_MAX;
  }

  for (i = 0U; i < sample_count; i++)
  {
    HRC_SendCommand(HRC_CMD_ADC_TEST);
    delay_us(HRC_ADC_TCONV_MARGIN_US);
    hrc_adc_samples[i] = HRC_BitReverse6(HRC_ReadDataOut() & 0x3FU);
    HRC_SendCommand(HRC_CMD_IDLE);
    (void)HRC_WaitIdle(HRC_CFG_DEFAULT_TIMEOUT_MS);
  }

  PC_Printf("ADC continuous samples=%u\r\n", (unsigned int)sample_count);
  PC_SendADCData(hrc_adc_samples, sample_count);
}

void HRC_Test_OCTDC(void)
{
  HRC_SendCommand(HRC_CMD_OCTDC_TEST);
  delay_us(HRC_ADC_TCONV_MARGIN_US);
  PC_Printf("OCTDC_TEST data_out=0x%02X\r\n", (unsigned int)HRC_ReadDataOut());
  HRC_SendCommand(HRC_CMD_IDLE);
  (void)HRC_WaitIdle(HRC_CFG_DEFAULT_TIMEOUT_MS);
}
