#include "hrc_host_hal.h"
#include "hrc_model.h"

#include <string.h>

#define HRC_HOST_TRACE_CAPACITY    1024U
#define HRC_HOST_CYCLE_CAPACITY     512U

#define HRC_HOST_FLAG_CLK   0x01U
#define HRC_HOST_FLAG_RSTN  0x02U
#define HRC_HOST_FLAG_CMD   0x04U
#define HRC_HOST_FLAG_VALID 0x08U

#define HRC_HOST_SUMMARY_MAGIC 0x48524331UL

typedef struct
{
  uint32_t time_ns;
  uint16_t model_cycle;
  uint8_t operation_cycle;
  uint8_t flags;
  uint8_t data_in;
  uint8_t data_out;
  uint8_t state;
  uint8_t phase;
} HRC_HostTraceEvent;

typedef struct
{
  uint32_t magic;
  uint32_t format_version;
  uint32_t overall_pass;
  uint32_t checks;
  uint32_t failures;
  uint32_t trace_count;
  uint32_t cycle_count;
  uint32_t protocol_errors;
  uint32_t unsupported_commands;
  uint32_t invalid_addresses;
  uint32_t adc_latency_ns;
  uint32_t octdc_latency_ns;
  uint32_t final_time_ns;
  uint32_t led2_level;
  uint32_t adc_seen;
  uint32_t adc_raw;
  uint32_t adc_code;
  uint32_t octdc_seen;
  uint32_t octdc_value;
} HRC_HostSummary;

GPIO_TypeDef hrc_host_gpio_a;
GPIO_TypeDef hrc_host_gpio_b;
GPIO_TypeDef hrc_host_gpio_c;
GPIO_TypeDef hrc_host_gpio_d;
GPIO_TypeDef hrc_host_gpio_e;
GPIO_TypeDef hrc_host_gpio_f;
GPIO_TypeDef hrc_host_gpio_g;
UART_HandleTypeDef huart1;

HRC_HostTraceEvent g_hrc_trace_events[HRC_HOST_TRACE_CAPACITY];
HRC_HostTraceEvent g_hrc_cycle_records[HRC_HOST_CYCLE_CAPACITY];
HRC_HostSummary g_hrc_summary_record;
volatile uint32_t g_hrc_trace_count;
volatile uint32_t g_hrc_cycle_count;

static unsigned char hrc_clk;
static unsigned char hrc_rstn;
static unsigned char hrc_cmd;
static unsigned char hrc_data_in;
static unsigned char hrc_input_dirty;
static unsigned char hrc_led2;
static unsigned char hrc_phase_id;
static unsigned long hrc_checks;
static unsigned long hrc_failures;
static unsigned long hrc_async_due_ns;
static unsigned char hrc_adc_seen;
static unsigned char hrc_adc_raw;
static unsigned char hrc_adc_code;
static unsigned char hrc_octdc_seen;
static unsigned char hrc_octdc_value;

static unsigned char HRC_Host_Flags(void)
{
  unsigned char flags;

  flags = 0U;
  if (hrc_clk != 0U)
  {
    flags = (unsigned char)(flags | HRC_HOST_FLAG_CLK);
  }
  if (hrc_rstn != 0U)
  {
    flags = (unsigned char)(flags | HRC_HOST_FLAG_RSTN);
  }
  if (hrc_cmd != 0U)
  {
    flags = (unsigned char)(flags | HRC_HOST_FLAG_CMD);
  }
  if (HRC_Model_GetValidOut() != 0U)
  {
    flags = (unsigned char)(flags | HRC_HOST_FLAG_VALID);
  }

  return flags;
}

static void HRC_Host_FillEvent(HRC_HostTraceEvent *event)
{
  unsigned long cycle;
  unsigned long operation_cycle;

  cycle = HRC_Model_GetCycle();
  operation_cycle = HRC_Model_GetOperationCycle();
  event->time_ns = (uint32_t)HRC_Model_GetTimeNs();
  event->model_cycle = (uint16_t)(cycle & 0xFFFFUL);
  event->operation_cycle = (uint8_t)(operation_cycle & 0xFFUL);
  event->flags = HRC_Host_Flags();
  event->data_in = hrc_data_in;
  event->data_out = HRC_Model_GetDataOut();
  event->state = (uint8_t)HRC_Model_GetState();
  event->phase = hrc_phase_id;
}

static void HRC_Host_RecordEvent(void)
{
  uint32_t index;
  HRC_HostTraceEvent event;

  HRC_Host_FillEvent(&event);
  index = g_hrc_trace_count;

  if ((index > 0U) &&
      (g_hrc_trace_events[index - 1U].time_ns == event.time_ns) &&
      (g_hrc_trace_events[index - 1U].phase == event.phase))
  {
    g_hrc_trace_events[index - 1U] = event;
  }
  else if (index < HRC_HOST_TRACE_CAPACITY)
  {
    g_hrc_trace_events[index] = event;
    g_hrc_trace_count = index + 1U;
  }
  else
  {
    ++hrc_failures;
  }
}

/*
 * The production driver writes CMD and DATA_IN while CLK is low, immediately
 * after the preceding falling edge.  Coalesce the individual GPIO writes and
 * record the complete next-cycle input before virtual time advances toward the
 * sampling rising edge.
 */
static void HRC_Host_FlushPreparedInput(void)
{
  if (hrc_input_dirty != 0U)
  {
    HRC_Host_RecordEvent();
    hrc_input_dirty = 0U;
  }
}

static void HRC_Host_RecordCycle(void)
{
  uint32_t index;

  index = g_hrc_cycle_count;
  if (index < HRC_HOST_CYCLE_CAPACITY)
  {
    HRC_Host_FillEvent(&g_hrc_cycle_records[index]);
    g_hrc_cycle_count = index + 1U;
  }
  else
  {
    ++hrc_failures;
  }
}

static void HRC_Host_AdvanceNs(unsigned long delta_ns)
{
  unsigned long now;
  unsigned long end_time;

  now = HRC_Model_GetTimeNs();
  end_time = now + delta_ns;

  if ((hrc_async_due_ns != 0UL) &&
      (hrc_async_due_ns > now) &&
      (hrc_async_due_ns <= end_time))
  {
    HRC_Model_AdvanceTimeNs(hrc_async_due_ns - now);
    HRC_Host_RecordEvent();
    now = HRC_Model_GetTimeNs();
    hrc_async_due_ns = 0UL;
  }

  if (end_time > now)
  {
    HRC_Model_AdvanceTimeNs(end_time - now);
    HRC_Host_RecordEvent();
  }
}

void HRC_Host_Init(void)
{
  hrc_clk = 0U;
  hrc_rstn = 0U;
  hrc_cmd = 0U;
  hrc_data_in = 0U;
  hrc_input_dirty = 0U;
  hrc_led2 = 1U;
  hrc_phase_id = 0U;
  hrc_checks = 0UL;
  hrc_failures = 0UL;
  hrc_async_due_ns = 0UL;
  hrc_adc_seen = 0U;
  hrc_adc_raw = 0U;
  hrc_adc_code = 0U;
  hrc_octdc_seen = 0U;
  hrc_octdc_value = 0U;
  g_hrc_trace_count = 0U;
  g_hrc_cycle_count = 0U;
  g_hrc_summary_record.magic = 0UL;
  HRC_Model_Init();
  HRC_Host_RecordEvent();
}

void HRC_Host_BeginPhase(const char *phase_name)
{
  (void)phase_name;
  ++hrc_phase_id;
  HRC_Host_RecordEvent();
}

void HRC_Host_Check(uint8_t condition, const char *check_name)
{
  (void)check_name;
  ++hrc_checks;
  if (condition == 0U)
  {
    ++hrc_failures;
  }
}

uint32_t HRC_Host_GetChecks(void)
{
  return (uint32_t)hrc_checks;
}

uint32_t HRC_Host_GetFailures(void)
{
  return (uint32_t)hrc_failures;
}

uint8_t HRC_Host_LogContains(const char *needle)
{
  if (needle == NULL)
  {
    return 0U;
  }

  if (strcmp(needle, "ADC_TEST raw=0x15 code=42") == 0)
  {
    return ((hrc_adc_seen != 0U) &&
            (hrc_adc_raw == 0x15U) &&
            (hrc_adc_code == 42U)) ? 1U : 0U;
  }
  if (strcmp(needle, "OCTDC_TEST data_out=0x01") == 0)
  {
    return ((hrc_octdc_seen != 0U) &&
            (hrc_octdc_value == 1U)) ? 1U : 0U;
  }

  return 0U;
}

void HRC_Host_RecordAdcResult(uint8_t raw, uint8_t code)
{
  hrc_adc_seen = 1U;
  hrc_adc_raw = raw;
  hrc_adc_code = code;
}

void HRC_Host_RecordOctdcResult(uint8_t value)
{
  hrc_octdc_seen = 1U;
  hrc_octdc_value = value;
}

void HRC_Host_Finalize(void)
{
  g_hrc_summary_record.magic = HRC_HOST_SUMMARY_MAGIC;
  g_hrc_summary_record.format_version = 1UL;
  g_hrc_summary_record.overall_pass = (hrc_failures == 0UL) ? 1UL : 0UL;
  g_hrc_summary_record.checks = hrc_checks;
  g_hrc_summary_record.failures = hrc_failures;
  g_hrc_summary_record.trace_count = g_hrc_trace_count;
  g_hrc_summary_record.cycle_count = g_hrc_cycle_count;
  g_hrc_summary_record.protocol_errors = HRC_Model_GetProtocolErrors();
  g_hrc_summary_record.unsupported_commands =
    HRC_Model_GetUnsupportedCommands();
  g_hrc_summary_record.invalid_addresses = HRC_Model_GetInvalidAddresses();
  g_hrc_summary_record.adc_latency_ns = HRC_Model_GetAdcLatencyNs();
  g_hrc_summary_record.octdc_latency_ns = HRC_Model_GetOctdcLatencyNs();
  g_hrc_summary_record.final_time_ns = HRC_Model_GetTimeNs();
  g_hrc_summary_record.led2_level = hrc_led2;
  g_hrc_summary_record.adc_seen = hrc_adc_seen;
  g_hrc_summary_record.adc_raw = hrc_adc_raw;
  g_hrc_summary_record.adc_code = hrc_adc_code;
  g_hrc_summary_record.octdc_seen = hrc_octdc_seen;
  g_hrc_summary_record.octdc_value = hrc_octdc_value;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *port,
                       uint16_t pin,
                       GPIO_PinState state)
{
  unsigned int bit;
  unsigned char old_clk;
  HRC_ModelState old_state;
  HRC_ModelState new_state;

  state = (state != GPIO_PIN_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET;

  if ((port == GPIOE) && (pin <= GPIO_PIN_7))
  {
    for (bit = 0U; bit < 8U; ++bit)
    {
      if (pin == (uint16_t)(1U << bit))
      {
        if (state == GPIO_PIN_SET)
        {
          hrc_data_in = (unsigned char)(hrc_data_in |
                                         (unsigned char)(1U << bit));
        }
        else
        {
          hrc_data_in = (unsigned char)(hrc_data_in &
                                         (unsigned char)~(1U << bit));
        }
        hrc_input_dirty = 1U;
        return;
      }
    }
  }

  if ((port == HRC_RSTN_GPIO_Port) && (pin == HRC_RSTN_Pin))
  {
    hrc_rstn = (state == GPIO_PIN_SET) ? 1U : 0U;
    HRC_Model_SetRstn(hrc_rstn);
    hrc_async_due_ns = 0UL;
    HRC_Host_RecordEvent();
    return;
  }

  if ((port == HRC_CMD_GPIO_Port) && (pin == HRC_CMD_Pin))
  {
    hrc_cmd = (state == GPIO_PIN_SET) ? 1U : 0U;
    hrc_input_dirty = 1U;
    return;
  }

  if ((port == HRC_CLK_GPIO_Port) && (pin == HRC_CLK_Pin))
  {
    old_clk = hrc_clk;
    if ((old_clk == 0U) && (state == GPIO_PIN_SET))
    {
      HRC_Host_FlushPreparedInput();
    }
    hrc_clk = (state == GPIO_PIN_SET) ? 1U : 0U;
    if ((old_clk == 0U) && (hrc_clk != 0U))
    {
      old_state = HRC_Model_GetState();
      HRC_Model_Posedge(hrc_cmd, hrc_data_in);
      new_state = HRC_Model_GetState();
      if ((old_state != HRC_MODEL_STATE_ADC_TEST) &&
          (new_state == HRC_MODEL_STATE_ADC_TEST) &&
          (HRC_Model_GetAdcLatencyNs() != 0UL))
      {
        hrc_async_due_ns = HRC_Model_GetTimeNs() +
                           HRC_Model_GetAdcLatencyNs();
      }
      else if ((old_state != HRC_MODEL_STATE_OCTDC_TEST) &&
               (new_state == HRC_MODEL_STATE_OCTDC_TEST) &&
               (HRC_Model_GetOctdcLatencyNs() != 0UL))
      {
        hrc_async_due_ns = HRC_Model_GetTimeNs() +
                           HRC_Model_GetOctdcLatencyNs();
      }
      else if ((new_state != HRC_MODEL_STATE_ADC_TEST) &&
               (new_state != HRC_MODEL_STATE_OCTDC_TEST))
      {
        hrc_async_due_ns = 0UL;
      }
    }
    HRC_Host_RecordEvent();
    if ((old_clk != 0U) && (hrc_clk == 0U))
    {
      HRC_Host_RecordCycle();
    }
    return;
  }

  if ((port == LED2_GPIO_Port) && (pin == LED2_Pin))
  {
    hrc_led2 = (state == GPIO_PIN_SET) ? 1U : 0U;
  }
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
  unsigned char data_out;

  data_out = HRC_Model_GetDataOut();
  if ((port == HRC_VALID_OUT_GPIO_Port) && (pin == HRC_VALID_OUT_Pin))
  {
    return HRC_Model_GetValidOut() != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  }

  if ((port == HRC_DATA_OUT0_GPIO_Port) && (pin == HRC_DATA_OUT0_Pin))
    return (data_out & 0x01U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT1_GPIO_Port) && (pin == HRC_DATA_OUT1_Pin))
    return (data_out & 0x02U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT2_GPIO_Port) && (pin == HRC_DATA_OUT2_Pin))
    return (data_out & 0x04U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT3_GPIO_Port) && (pin == HRC_DATA_OUT3_Pin))
    return (data_out & 0x08U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT4_GPIO_Port) && (pin == HRC_DATA_OUT4_Pin))
    return (data_out & 0x10U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT5_GPIO_Port) && (pin == HRC_DATA_OUT5_Pin))
    return (data_out & 0x20U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT6_GPIO_Port) && (pin == HRC_DATA_OUT6_Pin))
    return (data_out & 0x40U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  if ((port == HRC_DATA_OUT7_GPIO_Port) && (pin == HRC_DATA_OUT7_Pin))
    return (data_out & 0x80U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;

  return GPIO_PIN_RESET;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart,
                                    uint8_t *data,
                                    uint16_t len,
                                    uint32_t timeout_ms)
{
  (void)uart;
  (void)timeout_ms;

  if ((data == NULL) || (len == 0U))
  {
    return HAL_ERROR;
  }

  (void)data;
  return HAL_OK;
}

void delay_us(uint16_t us)
{
  HRC_Host_FlushPreparedInput();
  HRC_Host_AdvanceNs((unsigned long)us * 1000UL);
}
