#include "hrc_test.h"
#include "pc_comm.h"

static uint8_t hrc_adc_samples[HRC_ADC_SAMPLE_MAX];

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
