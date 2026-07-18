#ifndef __HRC_READ_H
#define __HRC_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hrc_cfg.h"

#define HRC_CMD_READ                         0x03U

#define HRC_READ_UNIT_NUM                    65U
#define HRC_READ_RESULT_BYTE_NUM             9U
#define HRC_READ_EN_CFG_FIRST_ADDR           16U
#define HRC_READ_EN_CFG_LAST_ADDR            24U
#define HRC_READ_TIMING_CFG0_ADDR             24U
#define HRC_READ_TIMING_CFG1_ADDR             25U
#define HRC_READ_WAIT_CFG_ADDR                26U

#define HRC_READ_EN_UNIT0_MASK                0x01U
#define HRC_READ_DEVELOP_MASK                 0x0EU
#define HRC_READ_DEVELOP_SHIFT                1U
#define HRC_READ_PRE_MASK                     0xF0U
#define HRC_READ_PRE_SHIFT                    4U
#define HRC_READ_SENS_MASK                    0x07U
#define HRC_READ_SENS_SHIFT                   0U
#define HRC_READ_RES_TRIM_MASK                0xF8U
#define HRC_READ_RES_TRIM_SHIFT               3U
#define HRC_READ_WAIT_CYCLE_MASK              0x07U
#define HRC_READ_WAIT_CYCLE_SHIFT             0U

#define HRC_READ_DEFAULT_DEVELOP_SEL          0U
#define HRC_READ_DEFAULT_PRE_SEL              0U
#define HRC_READ_DEFAULT_SENS_SEL             0U
#define HRC_READ_DEFAULT_RES_TRIM              0U
#define HRC_READ_DEFAULT_WAIT_CYCLE            0U

#define HRC_READ_MODE_ROW                      0U
#define HRC_READ_MODE_SINGLE                   1U
#define HRC_READ_ADAPTIVE_WL_DISABLE           0U
#define HRC_READ_ADAPTIVE_WL_ENABLE            1U

typedef struct
{
  uint8_t unit_enable[HRC_READ_RESULT_BYTE_NUM];
  uint8_t develop_sel;
  uint8_t pre_sel;
  uint8_t sens_sel;
  uint8_t read_res_trim;
  uint8_t read_wait_cycle;
} HRC_ReadConfigTypeDef;

void HRC_ReadInitConfig(HRC_ReadConfigTypeDef *config);
HRC_StatusTypeDef HRC_ReadSetUnitEnable(HRC_ReadConfigTypeDef *config,
                                        uint8_t unit_addr,
                                        uint8_t enable);
uint8_t HRC_ReadGetUnitBit(const uint8_t *unit_bits, uint8_t unit_addr);
HRC_StatusTypeDef HRC_ReadConfigure(const HRC_ReadConfigTypeDef *config);
HRC_StatusTypeDef HRC_ReadSingle(uint8_t adaptive_wl,
                                 uint8_t row_addr,
                                 uint8_t unit_sel_addr,
                                 uint8_t cell_sel,
                                 uint8_t read_wait_cycle,
                                 uint8_t *result);
HRC_StatusTypeDef HRC_ReadRow(uint8_t adaptive_wl,
                              uint8_t row_addr,
                              uint8_t cell_sel,
                              uint8_t read_wait_cycle,
                              uint8_t *unit_results);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_READ_H */
