#include "hrc_model.h"

/* HRC command values are intentionally local to this independent model. */
#define HRC_MODEL_OPCODE_IDLE       0x00U
#define HRC_MODEL_OPCODE_OCTDC      0x0CU
#define HRC_MODEL_OPCODE_ADC        0x0DU
#define HRC_MODEL_OPCODE_WRITE_CFG  0x0EU
#define HRC_MODEL_OPCODE_READ_CFG   0x0FU

#define HRC_MODEL_RESET_SIGNATURE   0xA5U
#define HRC_MODEL_WRITE_MODE_CODE   0xE0U
#define HRC_MODEL_WRITE_ADDR_CODE   0xE1U
#define HRC_MODEL_WRITE_VALUE_CODE  0xE2U
#define HRC_MODEL_READ_MODE_CODE    0xF0U

typedef struct
{
  unsigned char reset_n;
  unsigned char valid_out;
  unsigned char data_out;
  unsigned char adc_code;
  unsigned char octdc_level;
  unsigned char cfg[HRC_MODEL_CFG_COUNT];
  unsigned char cfg_index;
  unsigned char cfg_address;
  unsigned char adc_pending;
  unsigned char octdc_pending;
  HRC_ModelState state;
  unsigned long time_ns;
  unsigned long adc_latency_ns;
  unsigned long octdc_latency_ns;
  unsigned long adc_ready_time_ns;
  unsigned long octdc_ready_time_ns;
  unsigned long cycle;
  unsigned long operation_cycle;
  unsigned long protocol_errors;
  unsigned long unsupported_commands;
  unsigned long invalid_addresses;
} HRC_ModelInstance;

static HRC_ModelInstance hrc_model;

static void HRC_Model_ClearCfg(void)
{
  unsigned int i;

  for (i = 0U; i < HRC_MODEL_CFG_COUNT; ++i)
  {
    hrc_model.cfg[i] = 0U;
  }
}

static unsigned char HRC_Model_Reverse6(unsigned char value)
{
  unsigned char result;
  unsigned int i;

  value = (unsigned char)(value & 0x3FU);
  result = 0U;
  for (i = 0U; i < 6U; ++i)
  {
    if ((value & (unsigned char)(1U << i)) != 0U)
    {
      result = (unsigned char)(result | (unsigned char)(1U << (5U - i)));
    }
  }

  return result;
}

static void HRC_Model_SetIdleOutput(void)
{
  hrc_model.valid_out = 0U;
  hrc_model.data_out = 0U;
}

static void HRC_Model_EnterIdle(void)
{
  hrc_model.state = HRC_MODEL_STATE_IDLE;
  hrc_model.cfg_index = 0U;
  hrc_model.cfg_address = 0U;
  hrc_model.adc_pending = 0U;
  hrc_model.octdc_pending = 0U;
  hrc_model.operation_cycle = 0U;
  HRC_Model_SetIdleOutput();
}

static void HRC_Model_ScheduleAdc(void)
{
  hrc_model.adc_pending = 1U;
  hrc_model.adc_ready_time_ns = hrc_model.time_ns + hrc_model.adc_latency_ns;
  if (hrc_model.adc_latency_ns == 0UL)
  {
    hrc_model.data_out = HRC_Model_Reverse6(hrc_model.adc_code);
    hrc_model.adc_pending = 0U;
  }
}

static void HRC_Model_ScheduleOctdc(void)
{
  hrc_model.octdc_pending = 1U;
  hrc_model.octdc_ready_time_ns = hrc_model.time_ns + hrc_model.octdc_latency_ns;
  if (hrc_model.octdc_latency_ns == 0UL)
  {
    hrc_model.data_out = hrc_model.octdc_level;
    hrc_model.octdc_pending = 0U;
  }
}

static void HRC_Model_BeginCommand(unsigned char opcode)
{
  HRC_Model_SetIdleOutput();
  hrc_model.operation_cycle = 0U;
  hrc_model.cfg_index = 0U;

  switch (opcode)
  {
    case HRC_MODEL_OPCODE_IDLE:
      HRC_Model_EnterIdle();
      break;

    case HRC_MODEL_OPCODE_WRITE_CFG:
      hrc_model.state = HRC_MODEL_STATE_WRITE_CFG_MODE;
      break;

    case HRC_MODEL_OPCODE_READ_CFG:
      hrc_model.state = HRC_MODEL_STATE_READ_CFG_MODE;
      break;

    case HRC_MODEL_OPCODE_ADC:
      hrc_model.state = HRC_MODEL_STATE_ADC_TEST;
      HRC_Model_ScheduleAdc();
      break;

    case HRC_MODEL_OPCODE_OCTDC:
      hrc_model.state = HRC_MODEL_STATE_OCTDC_TEST;
      HRC_Model_ScheduleOctdc();
      break;

    default:
      ++hrc_model.unsupported_commands;
      HRC_Model_EnterIdle();
      break;
  }
}

static unsigned char HRC_Model_RequireDataCycle(unsigned char cmd)
{
  if (cmd != 0U)
  {
    ++hrc_model.protocol_errors;
    return 0U;
  }

  ++hrc_model.operation_cycle;
  return 1U;
}

void HRC_Model_Init(void)
{
  hrc_model.reset_n = 0U;
  hrc_model.valid_out = 0U;
  hrc_model.data_out = HRC_MODEL_RESET_SIGNATURE;
  hrc_model.adc_code = 0U;
  hrc_model.octdc_level = 0U;
  hrc_model.cfg_index = 0U;
  hrc_model.cfg_address = 0U;
  hrc_model.adc_pending = 0U;
  hrc_model.octdc_pending = 0U;
  hrc_model.state = HRC_MODEL_STATE_RESET;
  hrc_model.time_ns = 0UL;
  hrc_model.adc_latency_ns = HRC_MODEL_DEFAULT_ADC_LATENCY_NS;
  hrc_model.octdc_latency_ns = HRC_MODEL_DEFAULT_OCTDC_LATENCY_NS;
  hrc_model.adc_ready_time_ns = 0UL;
  hrc_model.octdc_ready_time_ns = 0UL;
  hrc_model.cycle = 0UL;
  hrc_model.operation_cycle = 0UL;
  hrc_model.protocol_errors = 0UL;
  hrc_model.unsupported_commands = 0UL;
  hrc_model.invalid_addresses = 0UL;
  HRC_Model_ClearCfg();
}

void HRC_Model_SetRstn(unsigned char rstn)
{
  rstn = (rstn != 0U) ? 1U : 0U;

  if (rstn == 0U)
  {
    if (hrc_model.reset_n != 0U)
    {
      hrc_model.cycle = 0UL;
    }

    hrc_model.reset_n = 0U;
    hrc_model.valid_out = 0U;
    hrc_model.data_out = HRC_MODEL_RESET_SIGNATURE;
    hrc_model.cfg_index = 0U;
    hrc_model.cfg_address = 0U;
    hrc_model.adc_pending = 0U;
    hrc_model.octdc_pending = 0U;
    hrc_model.operation_cycle = 0UL;
    hrc_model.state = HRC_MODEL_STATE_RESET;
    HRC_Model_ClearCfg();
  }
  else
  {
    hrc_model.reset_n = 1U;
    /* DATA_OUT remains 0xA5 until a rising edge samples the release. */
  }
}

void HRC_Model_SetAdcCode(unsigned char code)
{
  hrc_model.adc_code = (unsigned char)(code & 0x3FU);
  if ((hrc_model.reset_n != 0U) &&
      (hrc_model.state == HRC_MODEL_STATE_ADC_TEST))
  {
    hrc_model.valid_out = 0U;
    HRC_Model_ScheduleAdc();
  }
}

void HRC_Model_SetOctdc(unsigned char level)
{
  hrc_model.octdc_level = (level != 0U) ? 1U : 0U;
  if ((hrc_model.reset_n != 0U) &&
      (hrc_model.state == HRC_MODEL_STATE_OCTDC_TEST))
  {
    hrc_model.valid_out = 0U;
    HRC_Model_ScheduleOctdc();
  }
}

void HRC_Model_SetAdcLatencyNs(unsigned long latency_ns)
{
  hrc_model.adc_latency_ns = latency_ns;
}

void HRC_Model_SetOctdcLatencyNs(unsigned long latency_ns)
{
  hrc_model.octdc_latency_ns = latency_ns;
}

unsigned long HRC_Model_GetAdcLatencyNs(void)
{
  return hrc_model.adc_latency_ns;
}

unsigned long HRC_Model_GetOctdcLatencyNs(void)
{
  return hrc_model.octdc_latency_ns;
}

void HRC_Model_AdvanceTimeNs(unsigned long delta_ns)
{
  hrc_model.time_ns += delta_ns;

  if ((hrc_model.adc_pending != 0U) &&
      (hrc_model.state == HRC_MODEL_STATE_ADC_TEST) &&
      (hrc_model.time_ns >= hrc_model.adc_ready_time_ns))
  {
    hrc_model.data_out = HRC_Model_Reverse6(hrc_model.adc_code);
    hrc_model.adc_pending = 0U;
  }

  if ((hrc_model.octdc_pending != 0U) &&
      (hrc_model.state == HRC_MODEL_STATE_OCTDC_TEST) &&
      (hrc_model.time_ns >= hrc_model.octdc_ready_time_ns))
  {
    hrc_model.data_out = hrc_model.octdc_level;
    hrc_model.octdc_pending = 0U;
  }
}

void HRC_Model_Posedge(unsigned char cmd, unsigned char data_in)
{
  cmd = (cmd != 0U) ? 1U : 0U;

  if (hrc_model.reset_n == 0U)
  {
    ++hrc_model.cycle;
    hrc_model.state = HRC_MODEL_STATE_RESET;
    hrc_model.valid_out = 0U;
    hrc_model.data_out = HRC_MODEL_RESET_SIGNATURE;
    return;
  }

  ++hrc_model.cycle;

  /* A released reset is recognized synchronously at this rising edge. */
  if (hrc_model.state == HRC_MODEL_STATE_RESET)
  {
    HRC_Model_EnterIdle();
  }

  switch (hrc_model.state)
  {
    case HRC_MODEL_STATE_IDLE:
      HRC_Model_SetIdleOutput();
      if (cmd != 0U)
      {
        if ((data_in & 0xF0U) != 0U)
        {
          ++hrc_model.protocol_errors;
        }
        else
        {
          HRC_Model_BeginCommand((unsigned char)(data_in & 0x0FU));
        }
      }
      break;

    case HRC_MODEL_STATE_WRITE_CFG_MODE:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        hrc_model.valid_out = 0U;
        hrc_model.data_out = HRC_MODEL_WRITE_MODE_CODE;
        hrc_model.cfg_index = 0U;
        if ((data_in & 0x01U) != 0U)
        {
          hrc_model.state = HRC_MODEL_STATE_WRITE_CFG_TOTAL;
        }
        else
        {
          hrc_model.state = HRC_MODEL_STATE_WRITE_CFG_SINGLE_ADDR;
        }
      }
      break;

    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_ADDR:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        if (data_in >= HRC_MODEL_CFG_COUNT)
        {
          ++hrc_model.invalid_addresses;
          HRC_Model_EnterIdle();
        }
        else
        {
          hrc_model.cfg_address = data_in;
          hrc_model.valid_out = 0U;
          hrc_model.data_out = HRC_MODEL_WRITE_ADDR_CODE;
          hrc_model.state = HRC_MODEL_STATE_WRITE_CFG_SINGLE_VALUE;
        }
      }
      break;

    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_VALUE:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        hrc_model.cfg[hrc_model.cfg_address] = data_in;
        hrc_model.valid_out = 0U;
        hrc_model.data_out = HRC_MODEL_WRITE_VALUE_CODE;
        hrc_model.state = HRC_MODEL_STATE_WRITE_CFG_SINGLE_DONE;
      }
      break;

    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_DONE:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        HRC_Model_EnterIdle();
      }
      break;

    case HRC_MODEL_STATE_WRITE_CFG_TOTAL:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        hrc_model.valid_out = 0U;
        if (hrc_model.cfg_index < HRC_MODEL_CFG_COUNT)
        {
          hrc_model.cfg[hrc_model.cfg_index] = data_in;
          hrc_model.data_out = hrc_model.cfg_index;
          ++hrc_model.cfg_index;
        }
        else if (hrc_model.cfg_index == HRC_MODEL_CFG_COUNT)
        {
          /* Cycle 47 holds address 44; DATA_IN is ignored. */
          hrc_model.data_out = (unsigned char)(HRC_MODEL_CFG_COUNT - 1U);
          ++hrc_model.cfg_index;
        }
        else
        {
          /* Cycle 48 enters IDLE. */
          HRC_Model_EnterIdle();
        }
      }
      break;

    case HRC_MODEL_STATE_READ_CFG_MODE:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        hrc_model.valid_out = 0U;
        hrc_model.data_out = HRC_MODEL_READ_MODE_CODE;
        hrc_model.cfg_index = 0U;
        if ((data_in & 0x01U) != 0U)
        {
          hrc_model.state = HRC_MODEL_STATE_READ_CFG_TOTAL;
        }
        else
        {
          hrc_model.state = HRC_MODEL_STATE_READ_CFG_SINGLE_ADDR;
        }
      }
      break;

    case HRC_MODEL_STATE_READ_CFG_SINGLE_ADDR:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        if (data_in >= HRC_MODEL_CFG_COUNT)
        {
          ++hrc_model.invalid_addresses;
          HRC_Model_EnterIdle();
        }
        else
        {
          hrc_model.cfg_address = data_in;
          hrc_model.valid_out = 1U;
          hrc_model.data_out = hrc_model.cfg[data_in];
          hrc_model.state = HRC_MODEL_STATE_READ_CFG_SINGLE_DONE;
        }
      }
      break;

    case HRC_MODEL_STATE_READ_CFG_SINGLE_DONE:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        HRC_Model_EnterIdle();
      }
      break;

    case HRC_MODEL_STATE_READ_CFG_TOTAL:
      if (HRC_Model_RequireDataCycle(cmd) != 0U)
      {
        if (hrc_model.cfg_index < HRC_MODEL_CFG_COUNT)
        {
          hrc_model.valid_out = 1U;
          hrc_model.data_out = hrc_model.cfg[hrc_model.cfg_index];
          ++hrc_model.cfg_index;
        }
        else if (hrc_model.cfg_index == HRC_MODEL_CFG_COUNT)
        {
          /* Cycle 47 deasserts VALID; DATA_OUT is unspecified, so hold it. */
          hrc_model.valid_out = 0U;
          ++hrc_model.cfg_index;
        }
        else
        {
          /* Cycle 48 enters IDLE. */
          HRC_Model_EnterIdle();
        }
      }
      break;

    case HRC_MODEL_STATE_ADC_TEST:
      hrc_model.valid_out = 0U;
      if (cmd != 0U)
      {
        if (data_in == HRC_MODEL_OPCODE_IDLE)
        {
          HRC_Model_EnterIdle();
        }
        else
        {
          ++hrc_model.protocol_errors;
        }
      }
      break;

    case HRC_MODEL_STATE_OCTDC_TEST:
      hrc_model.valid_out = 0U;
      if (cmd != 0U)
      {
        if (data_in == HRC_MODEL_OPCODE_IDLE)
        {
          HRC_Model_EnterIdle();
        }
        else
        {
          ++hrc_model.protocol_errors;
        }
      }
      break;

    case HRC_MODEL_STATE_RESET:
    default:
      ++hrc_model.protocol_errors;
      HRC_Model_EnterIdle();
      break;
  }
}

unsigned char HRC_Model_GetValidOut(void)
{
  return hrc_model.valid_out;
}

unsigned char HRC_Model_GetDataOut(void)
{
  return hrc_model.data_out;
}

HRC_ModelState HRC_Model_GetState(void)
{
  return hrc_model.state;
}

const char *HRC_Model_StateName(HRC_ModelState state)
{
  switch (state)
  {
    case HRC_MODEL_STATE_RESET:                  return "RESET";
    case HRC_MODEL_STATE_IDLE:                   return "IDLE";
    case HRC_MODEL_STATE_WRITE_CFG_MODE:         return "WRITE_CFG_MODE";
    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_ADDR:  return "WRITE_CFG_SINGLE_ADDR";
    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_VALUE: return "WRITE_CFG_SINGLE_VALUE";
    case HRC_MODEL_STATE_WRITE_CFG_SINGLE_DONE:  return "WRITE_CFG_SINGLE_DONE";
    case HRC_MODEL_STATE_WRITE_CFG_TOTAL:        return "WRITE_CFG_TOTAL";
    case HRC_MODEL_STATE_READ_CFG_MODE:          return "READ_CFG_MODE";
    case HRC_MODEL_STATE_READ_CFG_SINGLE_ADDR:   return "READ_CFG_SINGLE_ADDR";
    case HRC_MODEL_STATE_READ_CFG_SINGLE_DONE:   return "READ_CFG_SINGLE_DONE";
    case HRC_MODEL_STATE_READ_CFG_TOTAL:         return "READ_CFG_TOTAL";
    case HRC_MODEL_STATE_ADC_TEST:               return "ADC_TEST";
    case HRC_MODEL_STATE_OCTDC_TEST:             return "OCTDC_TEST";
    default:                                     return "UNKNOWN";
  }
}

unsigned long HRC_Model_GetCycle(void)
{
  return hrc_model.cycle;
}

unsigned long HRC_Model_GetOperationCycle(void)
{
  return hrc_model.operation_cycle;
}

unsigned long HRC_Model_GetTimeNs(void)
{
  return hrc_model.time_ns;
}

unsigned long HRC_Model_GetProtocolErrors(void)
{
  return hrc_model.protocol_errors;
}

unsigned long HRC_Model_GetUnsupportedCommands(void)
{
  return hrc_model.unsupported_commands;
}

unsigned long HRC_Model_GetInvalidAddresses(void)
{
  return hrc_model.invalid_addresses;
}

void HRC_Model_ClearErrorCounters(void)
{
  hrc_model.protocol_errors = 0UL;
  hrc_model.unsupported_commands = 0UL;
  hrc_model.invalid_addresses = 0UL;
}

unsigned char HRC_Model_GetCfg(unsigned char address)
{
  if (address < HRC_MODEL_CFG_COUNT)
  {
    return hrc_model.cfg[address];
  }

  return 0U;
}
