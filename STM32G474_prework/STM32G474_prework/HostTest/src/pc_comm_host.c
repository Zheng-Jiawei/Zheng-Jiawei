#include "pc_comm.h"
#include "hrc_host_hal.h"

#include <stdarg.h>
#include <string.h>

void PC_Printf(const char *fmt, ...)
{
  va_list args;
  unsigned int first;
  unsigned int second;

  if (fmt == NULL)
  {
    return;
  }

  va_start(args, fmt);
  if (strncmp(fmt, "ADC_TEST raw=", 13U) == 0)
  {
    first = va_arg(args, unsigned int);
    second = va_arg(args, unsigned int);
    HRC_Host_RecordAdcResult((uint8_t)first, (uint8_t)second);
  }
  else if (strncmp(fmt, "OCTDC_TEST data_out=", 20U) == 0)
  {
    first = va_arg(args, unsigned int);
    HRC_Host_RecordOctdcResult((uint8_t)first);
  }
  va_end(args);
}

void PC_SendBytes(const uint8_t *buf, uint16_t len)
{
  (void)buf;
  (void)len;
}

void PC_SendCfgTable(const uint8_t *cfg, uint8_t len)
{
  (void)cfg;
  (void)len;
}

void PC_SendADCData(const uint8_t *data, uint16_t len)
{
  (void)data;
  (void)len;
}
