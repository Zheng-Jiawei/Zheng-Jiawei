#include "hrc_cfg.h"
#include <string.h>

#define HRC_WRITE_PROGRESS_MODE     0xE0U
#define HRC_WRITE_PROGRESS_ADDR     0xE1U
#define HRC_WRITE_PROGRESS_VALUE    0xE2U
#define HRC_READ_PROGRESS_MODE      0xF0U

static uint8_t hrc_cfg_shadow[HRC_CFG_REG_NUM];

static HRC_StatusTypeDef HRC_CFG_CheckOutput(uint8_t valid_out,
                                             uint8_t data_out,
                                             uint8_t expected_data)
{
  if ((valid_out != 0U) || (data_out != expected_data))
  {
    return HRC_PROTOCOL_ERROR;
  }

  return HRC_OK;
}

static HRC_StatusTypeDef HRC_CFG_BeginCommand(uint8_t command,
                                               uint8_t total_mode,
                                               uint8_t expected_progress)
{
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  status = HRC_WaitIdle(HRC_CFG_IDLE_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(1U, command, &valid_out, &data_out);
  status = HRC_CFG_CheckOutput(valid_out, data_out, 0x00U);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(0U, total_mode, &valid_out, &data_out);
  return HRC_CFG_CheckOutput(valid_out, data_out, expected_progress);
}

void HRC_CFG_ClearShadow(void)
{
  memset(hrc_cfg_shadow, 0, sizeof(hrc_cfg_shadow));
}

void HRC_CFG_SetShadow(uint8_t addr, uint8_t value)
{
  if (addr < HRC_CFG_REG_NUM)
  {
    hrc_cfg_shadow[addr] = value;
  }
}

uint8_t HRC_CFG_GetShadow(uint8_t addr)
{
  if (addr < HRC_CFG_REG_NUM)
  {
    return hrc_cfg_shadow[addr];
  }

  return 0U;
}

uint8_t *HRC_CFG_GetShadowTable(void)
{
  return hrc_cfg_shadow;
}

HRC_StatusTypeDef HRC_WriteCfgSingle(uint8_t addr, uint8_t value)
{
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  if (addr >= HRC_CFG_REG_NUM)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_CFG_BeginCommand(HRC_CMD_WRITE_CFG,
                                0x00U,
                                HRC_WRITE_PROGRESS_MODE);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(0U, addr, &valid_out, &data_out);
  status = HRC_CFG_CheckOutput(valid_out,
                               data_out,
                               HRC_WRITE_PROGRESS_ADDR);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(0U, value, &valid_out, &data_out);
  status = HRC_CFG_CheckOutput(valid_out,
                               data_out,
                               HRC_WRITE_PROGRESS_VALUE);
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_WaitIdle(HRC_CFG_IDLE_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    hrc_cfg_shadow[addr] = value;
  }

  return status;
}

HRC_StatusTypeDef HRC_ReadCfgSingle(uint8_t addr, uint8_t *value)
{
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  if ((addr >= HRC_CFG_REG_NUM) || (value == NULL))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_CFG_BeginCommand(HRC_CMD_READ_CFG,
                                0x00U,
                                HRC_READ_PROGRESS_MODE);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(0U, addr, &valid_out, &data_out);
  if (valid_out == 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  *value = data_out;
  status = HRC_WaitIdle(HRC_CFG_IDLE_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    hrc_cfg_shadow[addr] = data_out;
  }

  return status;
}

HRC_StatusTypeDef HRC_WriteCfgTotal(const uint8_t *cfg)
{
  uint8_t i;
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  if (cfg == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_CFG_BeginCommand(HRC_CMD_WRITE_CFG,
                                0x01U,
                                HRC_WRITE_PROGRESS_MODE);
  if (status != HRC_OK)
  {
    return status;
  }

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    HRC_TransferCycle(0U, cfg[i], &valid_out, &data_out);
    status = HRC_CFG_CheckOutput(valid_out, data_out, i);
    if (status != HRC_OK)
    {
      return status;
    }
  }

  /* Cycle 47: CFG_regs[44] remains on DATA_OUT; DATA_IN is ignored. */
  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  status = HRC_CFG_CheckOutput(valid_out,
                               data_out,
                               (uint8_t)(HRC_CFG_REG_NUM - 1U));
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_WaitIdle(HRC_CFG_IDLE_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    memcpy(hrc_cfg_shadow, cfg, HRC_CFG_REG_NUM);
  }

  return status;
}

HRC_StatusTypeDef HRC_ReadCfgTotal(uint8_t *cfg)
{
  uint8_t i;
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  if (cfg == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_CFG_BeginCommand(HRC_CMD_READ_CFG,  //HRC_CMD_READ_CFG = 0x0F：READ_CFG 指令码。
                                0x01U,		//0x01：选择全表读取模式。
                                HRC_READ_PROGRESS_MODE); //HRC_READ_PROGRESS_MODE = 0xF0：预期的指令进度输出
  if (status != HRC_OK)
  {
    return status;
  }

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
    if (valid_out == 0U)
    {
      return HRC_PROTOCOL_ERROR;
    }

    cfg[i] = data_out;
  }

  /* Cycle 47: VALID_OUT goes low; the chip enters IDLE next cycle. */
  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  if (valid_out != 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  status = HRC_WaitIdle(HRC_CFG_IDLE_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    memcpy(hrc_cfg_shadow, cfg, HRC_CFG_REG_NUM);
  }

  return status;
}

HRC_StatusTypeDef HRC_CFG_VerifyTotal(const uint8_t *expect)
{
  uint8_t i;
  uint8_t readback[HRC_CFG_REG_NUM];
  HRC_StatusTypeDef status;

  if (expect == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_ReadCfgTotal(readback);
  if (status != HRC_OK)
  {
    return status;
  }

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    if (readback[i] != expect[i])
    {
      return HRC_VERIFY_FAILED;
    }
  }

  return HRC_OK;
}
