#include "pc_comm.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>

#define PC_COMM_TIMEOUT_MS      100U
#define PC_PRINTF_BUF_SIZE      160U

void PC_Printf(const char *fmt, ...)
{
  char buffer[PC_PRINTF_BUF_SIZE];
  int len;
  va_list args;

  va_start(args, fmt);
  len = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  if (len <= 0)
  {
    return;
  }

  if ((uint32_t)len > sizeof(buffer))
  {
    len = sizeof(buffer);
  }

  PC_SendBytes((const uint8_t *)buffer, (uint16_t)len);
}

void PC_SendBytes(const uint8_t *buf, uint16_t len)
{
  if ((buf == NULL) || (len == 0U))
  {
    return;
  }

  (void)HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, PC_COMM_TIMEOUT_MS);
}

void PC_SendCfgTable(const uint8_t *cfg, uint8_t len)
{
  uint8_t i;

  if (cfg == NULL)
  {
    return;
  }

  for (i = 0U; i < len; i++)
  {
    PC_Printf("CFG[%02u]=0x%02X\r\n", (unsigned int)i, (unsigned int)cfg[i]);
  }
}

void PC_SendADCData(const uint8_t *data, uint16_t len)
{
  uint16_t i;

  if (data == NULL)
  {
    return;
  }

  for (i = 0U; i < len; i++)
  {
    PC_Printf("%u,%u\r\n", (unsigned int)i, (unsigned int)data[i]);
  }
}
