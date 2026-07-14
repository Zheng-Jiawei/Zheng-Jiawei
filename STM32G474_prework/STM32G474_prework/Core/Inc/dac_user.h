#ifndef __DAC_USER_H
#define __DAC_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void DAC_update(uint8_t CS_num, uint8_t Ch, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* __DAC_USER_H */
