#include "hrc_host_hal.h"
#include "hrc_model.h"
#include "hrc_test.h"

volatile unsigned long g_hrc_test_failures;
volatile unsigned long g_hrc_protocol_errors;
volatile unsigned long g_hrc_sim_complete;

__attribute__((noinline)) void HRC_Sim_TestComplete(void)
{
  g_hrc_sim_complete = 1UL;
}

static void HRC_CheckCfgPattern(void)
{
  unsigned int i;
  unsigned char expected;

  for (i = 0U; i < HRC_MODEL_CFG_COUNT; ++i)
  {
    expected = ((i & 1U) == 0U) ? 0x55U : 0xAAU;
    HRC_Host_Check(HRC_Model_GetCfg((unsigned char)i) == expected,
                   "CFG_TOTAL model register value");
  }
}

int main(void)
{
  HRC_StatusTypeDef status;
  unsigned char value;

  HRC_Host_Init();
  HRC_Model_SetAdcCode(42U);
  HRC_Model_SetOctdc(1U);
  HRC_Model_SetAdcLatencyNs(3000UL);
  HRC_Model_SetOctdcLatencyNs(2000UL);

  HRC_Host_BeginPhase("INITIAL_RESET_IDLE");
  status = HRC_Test_Initial();
  HRC_Host_Check(status == HRC_OK, "HRC_Test_Initial status");
  HRC_Host_Check(HRC_Model_GetState() == HRC_MODEL_STATE_IDLE,
                 "INITIAL final state IDLE");

  HRC_Host_BeginPhase("CFG_DEFAULT_READ_TOTAL");
  status = HRC_Test_CfgDefault();
  HRC_Host_Check(status == HRC_OK, "HRC_Test_CfgDefault status");

  HRC_Host_BeginPhase("CFG_SINGLE_WRITE_READ");
  status = HRC_Test_CfgSingle(HRC_CFG_SINGLE_TEST_ADDR,
                              HRC_CFG_SINGLE_TEST_VALUE);
  HRC_Host_Check(status == HRC_OK, "HRC_Test_CfgSingle status");
  HRC_Host_Check(HRC_Model_GetCfg(HRC_CFG_SINGLE_TEST_ADDR) ==
                   HRC_CFG_SINGLE_TEST_VALUE,
                 "CFG_SINGLE model register value");

  HRC_Host_BeginPhase("CFG_TOTAL_WRITE_READ");
  status = HRC_Test_CfgTotal();
  HRC_Host_Check(status == HRC_OK, "HRC_Test_CfgTotal status");
  HRC_CheckCfgPattern();

  HRC_Host_BeginPhase("ADC_ASYNC_LOCKED");
  HRC_Test_ADC_Single();
  HRC_Host_Check(HRC_Host_LogContains("ADC_TEST raw=0x15 code=42"),
                 "ADC physical bit order and logical code");
  HRC_Host_Check(HRC_Model_GetState() == HRC_MODEL_STATE_IDLE,
                 "ADC manual IDLE command");

  HRC_Host_BeginPhase("OCTDC_ASYNC_LOCKED");
  HRC_Test_OCTDC();
  HRC_Host_Check(HRC_Host_LogContains("OCTDC_TEST data_out=0x01"),
                 "OCTDC asynchronous result");
  HRC_Host_Check(HRC_Model_GetState() == HRC_MODEL_STATE_IDLE,
                 "OCTDC manual IDLE command");

  HRC_Host_BeginPhase("DRIVER_PARAMETER_GUARDS");
  value = 0U;
  HRC_Host_Check(HRC_WriteCfgSingle(HRC_MODEL_CFG_COUNT, 0x12U) ==
                   HRC_INVALID_PARAM,
                 "WRITE_CFG rejects address 45");
  HRC_Host_Check(HRC_ReadCfgSingle(HRC_MODEL_CFG_COUNT, &value) ==
                   HRC_INVALID_PARAM,
                 "READ_CFG rejects address 45");
  HRC_Host_Check(HRC_ReadCfgSingle(0U, NULL) == HRC_INVALID_PARAM,
                 "READ_CFG rejects NULL output");
  HRC_Host_Check(HRC_WriteCfgTotal(NULL) == HRC_INVALID_PARAM,
                 "WRITE_CFG total rejects NULL input");

  HRC_Host_Check(HRC_Model_GetProtocolErrors() == 0UL,
                 "No model protocol errors");
  HRC_Host_Check(HRC_Model_GetUnsupportedCommands() == 0UL,
                 "No unsupported opcode executed");
  HRC_Host_Check(HRC_Model_GetInvalidAddresses() == 0UL,
                 "Driver blocked invalid addresses before the bus");
  HRC_Host_Check((HRC_Model_GetValidOut() == 0U) &&
                   (HRC_Model_GetDataOut() == 0U),
                 "Final output is IDLE signature");

  HRC_Host_Finalize();
  g_hrc_test_failures = (unsigned long)HRC_Host_GetFailures();
  g_hrc_protocol_errors = HRC_Model_GetProtocolErrors();
  HRC_Sim_TestComplete();

  for (;;)
  {
    /* The debugger initialization script exits after the completion hook. */
  }
}
