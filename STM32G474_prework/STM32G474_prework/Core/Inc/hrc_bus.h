#ifndef __HRC_BUS_H
#define __HRC_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define HRC_DATA_OUT_WIDTH          8U

#define HRC_CMD_IDLE                0x00U
#define HRC_CMD_ADC_TEST            0x0DU

/* TODO: Fill these opcodes from the HRC digital protocol document. */
#define HRC_CMD_WRITE_CFG           0x00U
#define HRC_CMD_READ_CFG            0x00U
#define HRC_CMD_OCTDC_TEST          0x00U

typedef enum
{
  HRC_OK = 0U,
  HRC_TIMEOUT = 1U,
  HRC_INVALID_PARAM = 2U
} HRC_StatusTypeDef;

void HRC_Bus_InitDefault(void);
void HRC_SetDataIn(uint8_t data);
void HRC_SetCmd(uint8_t level);
void HRC_SetRstn(uint8_t level);
void HRC_ClockPulse(void);
void HRC_ClockPulseAndRead(uint8_t *valid_out, uint8_t *data_out);
void HRC_ClockCycles(uint16_t n);
void HRC_SendCommand(uint8_t cmd);
void HRC_SendData(uint8_t data);
uint8_t HRC_ReadDataOut(void);
uint8_t HRC_ReadValidOut(void);
HRC_StatusTypeDef HRC_WaitIdle(uint32_t timeout_ms);
HRC_StatusTypeDef HRC_WaitValid(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_BUS_H */
