#ifndef __HRC_TEST_H
#define __HRC_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hrc_cfg.h"

#define HRC_ADC_SAMPLE_MAX          4096U
#define HRC_ADC_TCONV_MARGIN_US     10U

void HRC_Test_CfgDefault(void);
void HRC_Test_CfgSingle(uint8_t addr, uint8_t value);
void HRC_Test_CfgTotal(void);
void HRC_Test_ADC_Single(void);
void HRC_Test_ADC_Continuous(uint16_t sample_count);
void HRC_Test_OCTDC(void);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_TEST_H */
