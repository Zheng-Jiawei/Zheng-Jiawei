#ifndef __ADC_USER_H
#define __ADC_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void ADC_Init(void);
void ADC_SwitchChannel(uint8_t channel);
int16_t ADC_Read_data(uint8_t Ch);
void ADC_Read_AB(uint8_t Ch, int16_t *pA, int16_t *pB);
void ADC_Read_AllChannels(uint16_t *adc_result);
int16_t Read_16BitData_Direct(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_USER_H */
