#ifndef HRC_MODEL_H
#define HRC_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Stand-alone, single-instance behavioral model of the HRC digital port.
 *
 * The model deliberately does not include STM32 or production-driver headers.
 * A host GPIO shim should call HRC_Model_Posedge() whenever the emulated CLK
 * changes from low to high, and HRC_Model_SetRstn() whenever RSTN changes.
 */

#define HRC_MODEL_CFG_COUNT 45U
#define HRC_MODEL_DEFAULT_ADC_LATENCY_NS   3000UL
#define HRC_MODEL_DEFAULT_OCTDC_LATENCY_NS 2000UL

typedef enum
{
  HRC_MODEL_STATE_RESET = 0,
  HRC_MODEL_STATE_IDLE,
  HRC_MODEL_STATE_WRITE_CFG_MODE,
  HRC_MODEL_STATE_WRITE_CFG_SINGLE_ADDR,
  HRC_MODEL_STATE_WRITE_CFG_SINGLE_VALUE,
  HRC_MODEL_STATE_WRITE_CFG_SINGLE_DONE,
  HRC_MODEL_STATE_WRITE_CFG_TOTAL,
  HRC_MODEL_STATE_READ_CFG_MODE,
  HRC_MODEL_STATE_READ_CFG_SINGLE_ADDR,
  HRC_MODEL_STATE_READ_CFG_SINGLE_DONE,
  HRC_MODEL_STATE_READ_CFG_TOTAL,
  HRC_MODEL_STATE_ADC_TEST,
  HRC_MODEL_STATE_OCTDC_TEST
} HRC_ModelState;

/* Initialize the model in asserted-reset state (RSTN=0). */
void HRC_Model_Init(void);

/*
 * Drive the active-low asynchronous reset input.
 * Assertion immediately forces VALID_OUT=0 and DATA_OUT=0xA5.
 * After release, the first rising CLK edge returns/advances the model to IDLE.
 */
void HRC_Model_SetRstn(unsigned char rstn);

/*
 * Supply the logical six-bit ADC result (0..63).  HRC pin 0 is the ADC MSB,
 * so the model reverses the six bits when presenting DATA_OUT[5:0].  While
 * ADC_TEST is active, a new value appears after the configured model latency.
 */
void HRC_Model_SetAdcCode(unsigned char code);

/* Supply the OCTDC output level; it appears asynchronously on bit 0. */
void HRC_Model_SetOctdc(unsigned char level);

/*
 * Configure simulation-only settling delays.  The HRC document specifies
 * asynchronous outputs but does not guarantee these numerical delays.
 * A zero delay makes the corresponding output change immediately.
 */
void HRC_Model_SetAdcLatencyNs(unsigned long latency_ns);
void HRC_Model_SetOctdcLatencyNs(unsigned long latency_ns);
unsigned long HRC_Model_GetAdcLatencyNs(void);
unsigned long HRC_Model_GetOctdcLatencyNs(void);

/* Advance monotonic simulation time and publish any due async result. */
void HRC_Model_AdvanceTimeNs(unsigned long delta_ns);

/* Sample CMD and DATA_IN on a rising CLK edge. */
void HRC_Model_Posedge(unsigned char cmd, unsigned char data_in);

unsigned char HRC_Model_GetValidOut(void);
unsigned char HRC_Model_GetDataOut(void);
HRC_ModelState HRC_Model_GetState(void);
const char *HRC_Model_StateName(HRC_ModelState state);

/* Total sampled rising edges since the most recent reset assertion. */
unsigned long HRC_Model_GetCycle(void);

/* Operation-relative cycle: command edge is cycle 0, next edge is cycle 1. */
unsigned long HRC_Model_GetOperationCycle(void);

unsigned long HRC_Model_GetTimeNs(void);
unsigned long HRC_Model_GetProtocolErrors(void);
unsigned long HRC_Model_GetUnsupportedCommands(void);
unsigned long HRC_Model_GetInvalidAddresses(void);
void HRC_Model_ClearErrorCounters(void);

/* Invalid addresses return zero and do not alter diagnostic counters. */
unsigned char HRC_Model_GetCfg(unsigned char address);

#ifdef __cplusplus
}
#endif

#endif /* HRC_MODEL_H */
