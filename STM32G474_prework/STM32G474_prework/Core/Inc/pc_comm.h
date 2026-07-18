#ifndef __PC_COMM_H
#define __PC_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HRC_HOST_TEST
#include "hrc_host_hal.h"
#else
#include "main.h"
#endif

void PC_Printf(const char *fmt, ...);
void PC_SendBytes(const uint8_t *buf, uint16_t len);
void PC_SendCfgTable(const uint8_t *cfg, uint8_t len);
void PC_SendADCData(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __PC_COMM_H */
