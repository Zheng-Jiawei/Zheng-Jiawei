#include "hrc_cfg.h"
#include <string.h>

static uint8_t hrc_cfg_shadow[HRC_CFG_REG_NUM];

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
  if (addr >= HRC_CFG_REG_NUM)
  {
    return HRC_INVALID_PARAM;
  }

  HRC_SendCommand(HRC_CMD_WRITE_CFG);
  HRC_SendData(addr);
  HRC_SendData(value);
  hrc_cfg_shadow[addr] = value;

  return HRC_WaitIdle(HRC_CFG_DEFAULT_TIMEOUT_MS);
}

HRC_StatusTypeDef HRC_ReadCfgSingle(uint8_t addr, uint8_t *value)
{
  HRC_StatusTypeDef status;

  if ((addr >= HRC_CFG_REG_NUM) || (value == NULL))
  {
    return HRC_INVALID_PARAM;
  }

  HRC_SendCommand(HRC_CMD_READ_CFG);
  HRC_SendData(addr);

  status = HRC_WaitValid(HRC_CFG_DEFAULT_TIMEOUT_MS);
  if (status != HRC_OK)
  {
    return status;
  }

  *value = HRC_ReadDataOut();
  return HRC_WaitIdle(HRC_CFG_DEFAULT_TIMEOUT_MS);
}

HRC_StatusTypeDef HRC_WriteCfgTotal(const uint8_t *cfg)
{
  uint8_t i;
  HRC_StatusTypeDef status;

  if (cfg == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    status = HRC_WriteCfgSingle(i, cfg[i]);
    if (status != HRC_OK)
    {
      return status;
    }
  }

  return HRC_OK;
}

HRC_StatusTypeDef HRC_ReadCfgTotal(uint8_t *cfg)
{
  uint8_t i;
  HRC_StatusTypeDef status;

  if (cfg == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    status = HRC_ReadCfgSingle(i, &cfg[i]);
    if (status != HRC_OK)
    {
      return status;
    }
  }

  return HRC_OK;
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
      return HRC_INVALID_PARAM;
    }
  }

  return HRC_OK;
}
