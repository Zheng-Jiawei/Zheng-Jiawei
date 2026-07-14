#ifndef __HRC_CFG_H
#define __HRC_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hrc_bus.h"

#define HRC_CFG_REG_NUM             45U
#define HRC_CFG_DEFAULT_TIMEOUT_MS  10U

void HRC_CFG_ClearShadow(void);
void HRC_CFG_SetShadow(uint8_t addr, uint8_t value);
uint8_t HRC_CFG_GetShadow(uint8_t addr);
uint8_t *HRC_CFG_GetShadowTable(void);

HRC_StatusTypeDef HRC_WriteCfgSingle(uint8_t addr, uint8_t value);
HRC_StatusTypeDef HRC_ReadCfgSingle(uint8_t addr, uint8_t *value);
HRC_StatusTypeDef HRC_WriteCfgTotal(const uint8_t *cfg);
HRC_StatusTypeDef HRC_ReadCfgTotal(uint8_t *cfg);
HRC_StatusTypeDef HRC_CFG_VerifyTotal(const uint8_t *expect);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_CFG_H */
