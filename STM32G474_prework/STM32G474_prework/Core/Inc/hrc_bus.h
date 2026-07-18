#ifndef __HRC_BUS_H
#define __HRC_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HRC_HOST_TEST
#include "hrc_host_hal.h"
#else
#include "main.h"
#endif

#define HRC_DATA_OUT_WIDTH          8U
#define HRC_CLK_LOW_US              1U
#define HRC_CLK_HIGH_US             1U
#define HRC_IDLE_CONFIRM_CYCLES     2U
#define HRC_IDLE_GUARD_CYCLES       1U
#define HRC_DEFAULT_TIMEOUT_CYCLES  128U

#define HRC_CMD_IDLE                0x00U
#define HRC_CMD_RW_SINGLE           0x01U
#define HRC_CMD_RW_MULTIPLE         0x02U
#define HRC_CMD_OCTDC_TEST          0x0CU
#define HRC_CMD_ADC_TEST            0x0DU
#define HRC_CMD_WRITE_CFG           0x0EU
#define HRC_CMD_READ_CFG            0x0FU

typedef enum
{
  HRC_OK = 0U,
  HRC_TIMEOUT = 1U,
  HRC_INVALID_PARAM = 2U,
  HRC_PROTOCOL_ERROR = 3U,
  HRC_VERIFY_FAILED = 4U
} HRC_StatusTypeDef;

void HRC_Bus_InitDefault(void);
void HRC_SetDataIn(uint8_t data);
void HRC_SetCmd(uint8_t level);
void HRC_SetRstn(uint8_t level);
void HRC_ClockPulse(void);
void HRC_ClockPulseAndRead(uint8_t *valid_out, uint8_t *data_out);
void HRC_TransferCycle(uint8_t cmd,
                       uint8_t data_in,
                       uint8_t *valid_out,
                       uint8_t *data_out);
void HRC_ClockCycles(uint16_t n);
void HRC_SendCommand(uint8_t cmd);
void HRC_SendData(uint8_t data);
uint8_t HRC_ReadDataOut(void);
uint8_t HRC_ReadValidOut(void);
HRC_StatusTypeDef HRC_WaitIdle(uint32_t max_cycles);

#ifdef __cplusplus
}
#endif

#endif /* __HRC_BUS_H */
