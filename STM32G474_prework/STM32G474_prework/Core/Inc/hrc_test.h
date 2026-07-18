#ifndef __HRC_TEST_H
#define __HRC_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hrc_bus.h"
#include "hrc_cfg.h"
#include "hrc_read.h"

#define HRC_ADC_SAMPLE_MAX                  4096U
#define HRC_ADC_DEFAULT_CONVERSION_WAIT_US  1U
#define HRC_ADC_STABLE_INTERVAL_US          1U
#define HRC_ADC_STABLE_MAX_READS            4U
#define HRC_OCTDC_DEFAULT_WAIT_US           10U
#define HRC_OCTDC_STABLE_INTERVAL_US        1U
#define HRC_OCTDC_STABLE_MAX_READS          4U
#define HRC_OCTDC_DELAY_CFG_ADDR            42U
#define HRC_OCTDC_BL_RES_CFG_ADDR           43U
#define HRC_OCTDC_REF_RES_CFG_ADDR          44U
#define HRC_OCTDC_DEFAULT_DELAY_TRIM        0U
#define HRC_OCTDC_DEFAULT_BL_RES_TRIM       0U
#define HRC_OCTDC_DEFAULT_REF_RES_TRIM      31U
#define HRC_RW_ROW_CFG_REG_NUM              16U
#define HRC_RW_SINGLE_DEFAULT_ROW_ADDR      0U
#define HRC_RW_SINGLE_DEFAULT_CELL_SEL      0U
#define HRC_RW_SINGLE_DEFAULT_UNIT_ADDR     0U
#define HRC_RW_MULTIPLE_DEFAULT_ROW_ADDR    0U
#define HRC_RW_MULTIPLE_DEFAULT_CELL_SEL    0U
#define HRC_RW_MULTIPLE_DEFAULT_UNIT_ADDR   0U
#define HRC_RESET_CLOCK_CYCLES              8U
#define HRC_CFG_SINGLE_TEST_ADDR            0U
#define HRC_CFG_SINGLE_TEST_VALUE           0x5AU
#define HRC_READ_TEST_ADAPTIVE_WL           HRC_READ_ADAPTIVE_WL_DISABLE
#define HRC_READ_TEST_DEVELOP_SEL           HRC_READ_DEFAULT_DEVELOP_SEL
#define HRC_READ_TEST_PRE_SEL               HRC_READ_DEFAULT_PRE_SEL
#define HRC_READ_TEST_SENS_SEL              HRC_READ_DEFAULT_SENS_SEL
#define HRC_READ_TEST_RES_TRIM              HRC_READ_DEFAULT_RES_TRIM
#define HRC_READ_TEST_WAIT_CYCLE            HRC_READ_DEFAULT_WAIT_CYCLE
#define HRC_READ_SINGLE_TEST_ROW_ADDR       0U
#define HRC_READ_SINGLE_TEST_UNIT_ADDR      0U
#define HRC_READ_SINGLE_TEST_CELL_SEL       0U
#define HRC_READ_ROW_TEST_ROW_ADDR          0U
#define HRC_READ_ROW_TEST_CELL_SEL          0U
#define HRC_READ_ROW_TEST_ENABLE_ALL_UNITS  1U

HRC_StatusTypeDef HRC_Test_Initial(void);
HRC_StatusTypeDef HRC_Test_CfgDefault(void);
HRC_StatusTypeDef HRC_Test_CfgSingle(uint8_t addr, uint8_t value);
HRC_StatusTypeDef HRC_Test_CfgTotal(void);
HRC_StatusTypeDef HRC_ADC_TestStart(void);
HRC_StatusTypeDef HRC_ADC_ReadResult(uint16_t conversion_wait_us,
                                     uint8_t *raw,
                                     uint8_t *code);
HRC_StatusTypeDef HRC_ADC_TestStop(void);
HRC_StatusTypeDef HRC_Test_ADC_Single(uint16_t conversion_wait_us,
                                      uint8_t *code);
HRC_StatusTypeDef HRC_Test_ADC_Continuous(uint16_t sample_count,
                                          uint16_t conversion_wait_us);
HRC_StatusTypeDef HRC_OCTDC_ConfigTrim(uint8_t delay_trim,
                                       uint8_t bl_res_trim,
                                       uint8_t ref_res_trim);
HRC_StatusTypeDef HRC_OCTDC_TestStart(void);
HRC_StatusTypeDef HRC_OCTDC_ReadResult(uint16_t conversion_wait_us,
                                       uint8_t *raw,
                                       uint8_t *result);
HRC_StatusTypeDef HRC_OCTDC_TestStop(void);
HRC_StatusTypeDef HRC_Test_OCTDC_Single(uint8_t delay_trim,
                                        uint8_t bl_res_trim,
                                        uint8_t ref_res_trim,
                                        uint16_t conversion_wait_us,
                                        uint8_t *result);
HRC_StatusTypeDef HRC_RW_MultipleConfigRows(const uint8_t *row_bitmap);
HRC_StatusTypeDef HRC_RW_SingleStart(uint8_t row_addr,
                                     uint8_t cell_sel,
                                     uint8_t unit_sel_addr);
HRC_StatusTypeDef HRC_RW_MultipleStart(uint8_t cell_sel,
                                       uint8_t unit_sel_addr);
HRC_StatusTypeDef HRC_RW_Stop(void);
HRC_StatusTypeDef HRC_Test_RW_SingleDefault(void);
HRC_StatusTypeDef HRC_Test_RW_MultipleDefault(void);
HRC_StatusTypeDef HRC_Test_ReadSingleDefault(void);
HRC_StatusTypeDef HRC_Test_ReadRowDefault(void);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_TEST_H */
