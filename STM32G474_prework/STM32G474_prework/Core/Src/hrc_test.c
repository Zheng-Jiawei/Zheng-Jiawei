#include "hrc_test.h"
#include "pc_comm.h"

static uint8_t hrc_adc_samples[HRC_ADC_SAMPLE_MAX];
static uint8_t hrc_adc_test_active = 0U;
static uint8_t hrc_octdc_test_active = 0U;

#define HRC_OCTDC_DELAY_MASK       0x70U
#define HRC_OCTDC_DELAY_SHIFT      4U
#define HRC_OCTDC_RES_MASK         0x1FU
#define HRC_RW_MODE_NONE           0U
#define HRC_RW_MODE_SINGLE         1U
#define HRC_RW_MODE_MULTIPLE       2U
#define HRC_RW_SINGLE_PROGRESS_ROW   0x10U
#define HRC_RW_SINGLE_PROGRESS_CELL  0x11U
#define HRC_RW_SINGLE_PROGRESS_UNIT  0x12U
#define HRC_RW_SINGLE_PROGRESS_READY 0x13U
#define HRC_RW_MULTI_PROGRESS_CELL   0x20U
#define HRC_RW_MULTI_PROGRESS_UNIT   0x21U
#define HRC_RW_MULTI_PROGRESS_READY  0x23U

static uint8_t hrc_rw_active_mode = HRC_RW_MODE_NONE;

static void HRC_RW_SetActiveMode(uint8_t mode)
{
  /*
   * 功能：更新片外读写活动模式，并同步控制低电平点亮的LED3。
   * 输入参数：
   *   mode：片外读写模式；有效值为HRC_RW_MODE_NONE、HRC_RW_MODE_SINGLE
   *         或HRC_RW_MODE_MULTIPLE，无单位。
   * 返回值：无。
   */
  hrc_rw_active_mode = mode;
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,
                    (mode == HRC_RW_MODE_NONE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void HRC_SetTestLed(uint8_t passed)
{
  /* The board initializes LEDs high; LED2 is therefore treated as active-low. */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin,
                    (passed != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
static uint8_t HRC_BitReverse6(uint8_t value)
{
  uint8_t result = 0U;
  uint8_t i;

  value &= 0x3FU;
  for (i = 0U; i < 6U; i++)
  {
    if (value & (1U << i))
    {
      result |= (uint8_t)(1U << (5U - i));
    }
  }

  return result;
}

/*
 * HRC samples CMD and DATA_IN on CLK rising edges.  Prepare the next command
 * immediately after the preceding falling edge, then keep it stable until the
 * next rising edge.
 */
/*初始化测试程序
测试流程为：
1. LED2 熄灭。
2. 初始化 HRC 总线，使 RSTN=0。
3. 发送 8 个干净的 CLK 周期。
4. 检查复位态：VALID_OUT=0 且 DATA_OUT=0xA5。
5. 释放复位：RSTN=1。
6. 每产生一个 CLK 周期后读取一次输出，确认 VALID_OUT=0、DATA_OUT=0x00 连续满足 2 个时钟周期。
7. 全部通过后点亮 LED2，并通过串口输出 INITIAL_TEST passed；任一步失败则保持 LED2 熄灭，并打印失败时的 VALID、DATA 值。
*/
HRC_StatusTypeDef HRC_Test_Initial(void)
{
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  HRC_SetTestLed(0U);
  HRC_Bus_InitDefault();
  HRC_ClockCycles(HRC_RESET_CLOCK_CYCLES - 1U);
  HRC_ClockPulseAndRead(&valid_out, &data_out);

  if ((valid_out != 0U) || (data_out != 0xA5U))
  {
    PC_Printf("INITIAL_TEST reset-state failed: VALID=%u DATA=0x%02X\r\n",
              (unsigned int)valid_out,
              (unsigned int)data_out);
    return HRC_VERIFY_FAILED;
  }

  HRC_SetRstn(1U);
  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    PC_Printf("INITIAL_TEST idle-state failed: VALID=%u DATA=0x%02X\r\n",
              (unsigned int)HRC_ReadValidOut(),
              (unsigned int)HRC_ReadDataOut());
    return status;
  }

  HRC_SetTestLed(1U);
  PC_Printf("INITIAL_TEST passed\r\n");
  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_CfgDefault(void)
{
  uint8_t cfg[HRC_CFG_REG_NUM];
  uint8_t i;
  HRC_StatusTypeDef status;

  status = HRC_ReadCfgTotal(cfg);
  if (status != HRC_OK)
  {
    PC_Printf("CFG_DEFAULT failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  PC_SendCfgTable(cfg, HRC_CFG_REG_NUM);
  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    if (cfg[i] != 0x00U)
    {
      PC_Printf("CFG_DEFAULT failed: addr=%u expected=0x00 actual=0x%02X\r\n",
                (unsigned int)i,
                (unsigned int)cfg[i]);
      return HRC_VERIFY_FAILED;
    }
  }

  PC_Printf("CFG_DEFAULT passed: 45 registers are 0x00\r\n");
  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_CfgSingle(uint8_t addr, uint8_t value)
{
  uint8_t readback = 0U;
  HRC_StatusTypeDef status;

  status = HRC_WriteCfgSingle(addr, value);
  PC_Printf("WRITE_CFG addr=%u value=0x%02X status=%u\r\n",
            (unsigned int)addr,
            (unsigned int)value,
            (unsigned int)status);

  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_ReadCfgSingle(addr, &readback);
  PC_Printf("READ_CFG addr=%u expected=0x%02X actual=0x%02X status=%u\r\n",
            (unsigned int)addr,
            (unsigned int)value,
            (unsigned int)readback,
            (unsigned int)status);

  if (status != HRC_OK)
  {
    return status;
  }

  if (readback != value)
  {
    PC_Printf("CFG_SINGLE failed\r\n");
    return HRC_VERIFY_FAILED;
  }

  PC_Printf("CFG_SINGLE passed\r\n");
  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_CfgTotal(void)
{
  uint8_t write_cfg[HRC_CFG_REG_NUM];
  uint8_t read_cfg[HRC_CFG_REG_NUM];
  uint8_t i;
  HRC_StatusTypeDef status;

  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    write_cfg[i] = ((i & 1U) == 0U) ? 0x55U : 0xAAU;
  }

  status = HRC_WriteCfgTotal(write_cfg);
  PC_Printf("WRITE_CFG total status=%u\r\n", (unsigned int)status);
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_ReadCfgTotal(read_cfg);
  PC_Printf("READ_CFG total status=%u\r\n", (unsigned int)status);
  if (status != HRC_OK)
  {
    return status;
  }

  PC_SendCfgTable(read_cfg, HRC_CFG_REG_NUM);
  for (i = 0U; i < HRC_CFG_REG_NUM; i++)
  {
    if (read_cfg[i] != write_cfg[i])
    {
      PC_Printf("CFG_TOTAL failed: addr=%u expected=0x%02X actual=0x%02X\r\n",
                (unsigned int)i,
                (unsigned int)write_cfg[i],
                (unsigned int)read_cfg[i]);
      return HRC_VERIFY_FAILED;
    }
  }

  PC_Printf("CFG_TOTAL passed: 45 registers matched\r\n");
  return HRC_OK;
}

static HRC_StatusTypeDef HRC_ADC_ReadStableRaw(uint8_t *raw)
{
  /* 功能：重复读取异步ADC输出，确认高两位为0且低六位至少连续两次保持一致。 */
  uint8_t previous;
  uint8_t current;
  uint8_t read_count;

  if (raw == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  previous = HRC_ReadDataOut();
  if ((previous & 0xC0U) != 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  for (read_count = 1U; read_count < HRC_ADC_STABLE_MAX_READS; read_count++)
  {
    delay_us(HRC_ADC_STABLE_INTERVAL_US);
    current = HRC_ReadDataOut();
    if ((current & 0xC0U) != 0U)
    {
      return HRC_PROTOCOL_ERROR;
    }

    if (current == previous)
    {
      *raw = current & 0x3FU;
      return HRC_OK;
    }

    previous = current;
  }

  return HRC_TIMEOUT;
}

HRC_StatusTypeDef HRC_ADC_TestStart(void)
{
  /* 功能：确认HRC处于IDLE后发送ADC_TEST指令，使内部ADC开始一次量化。 */
  HRC_StatusTypeDef status;

  if (hrc_adc_test_active != 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_SendCommand(HRC_CMD_ADC_TEST);
  hrc_adc_test_active = 1U;
  return HRC_OK;
}

HRC_StatusTypeDef HRC_ADC_ReadResult(uint16_t conversion_wait_us,
                                     uint8_t *raw,
                                     uint8_t *code)
{
  /* 功能：等待可调转换时间，读取稳定的ADC原始值并完成六位码序反转，
	raw是直接从 DATA_OUT[5:0] 读取到的6-bit数据，
	code 是对 raw 的低6位进行bit reverse之后得到的结果。 */
  HRC_StatusTypeDef status;

  if ((raw == NULL) || (code == NULL))
  {
    return HRC_INVALID_PARAM;
  }

  if (hrc_adc_test_active == 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  if (conversion_wait_us > 0U)
  {
    delay_us(conversion_wait_us);
  }

  status = HRC_ADC_ReadStableRaw(raw);
  if (status != HRC_OK)
  {
    return status;
  }

  *code = HRC_BitReverse6(*raw);
  return HRC_OK;
}

HRC_StatusTypeDef HRC_ADC_TestStop(void)
{
  /* 功能：向锁定在ADC_TEST状态的HRC发送IDLE指令，并确认芯片重新进入IDLE。 */
  HRC_StatusTypeDef status;

  if (hrc_adc_test_active == 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  HRC_SendCommand(HRC_CMD_IDLE);
  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    hrc_adc_test_active = 0U;
  }

  return status;
}

static HRC_StatusTypeDef HRC_ADC_CaptureOne(uint16_t conversion_wait_us,
                                             uint8_t *raw,
                                             uint8_t *code)
{
  /* 功能：完整执行一次ADC_TEST启动、结果读取和IDLE退出流程。 */
  HRC_StatusTypeDef read_status;
  HRC_StatusTypeDef stop_status;

  read_status = HRC_ADC_TestStart();
  if (read_status != HRC_OK)
  {
    return read_status;
  }

  read_status = HRC_ADC_ReadResult(conversion_wait_us, raw, code);
  stop_status = HRC_ADC_TestStop();

  if (read_status != HRC_OK)
  {
    return read_status;
  }

  return stop_status;
}

HRC_StatusTypeDef HRC_Test_ADC_Single(uint16_t conversion_wait_us,
                                      uint8_t *code)
{
  /* 功能：执行一次ADC_TEST，打印原始六位数据、反转后码值和可调等待时间。 */
  uint8_t raw = 0U;
  HRC_StatusTypeDef status;

  if (code == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_ADC_CaptureOne(conversion_wait_us, &raw, code);
  if (status != HRC_OK)
  {
    PC_Printf("ADC_TEST single failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  PC_Printf("ADC_TEST single passed: wait_us=%u raw=0x%02X code=%u\r\n",
            (unsigned int)conversion_wait_us,
            (unsigned int)raw,
            (unsigned int)(*code));
  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_ADC_Continuous(uint16_t sample_count,
                                          uint16_t conversion_wait_us)
{
  /* 功能：连续完成指定次数ADC_TEST采样，先存入数组，结束后统一通过串口发送。 */
  uint16_t i;
  uint8_t raw;
  uint8_t code;
  HRC_StatusTypeDef status;

  if ((sample_count == 0U) || (sample_count > HRC_ADC_SAMPLE_MAX))
  {
    return HRC_INVALID_PARAM;
  }

  for (i = 0U; i < sample_count; i++)
  {
    status = HRC_ADC_CaptureOne(conversion_wait_us, &raw, &code);
    if (status != HRC_OK)
    {
      PC_Printf("ADC_TEST continuous failed: index=%u status=%u\r\n",
                (unsigned int)i,
                (unsigned int)status);
      return status;
    }

    hrc_adc_samples[i] = code;
  }

  PC_Printf("ADC_DATA_BEGIN count=%u wait_us=%u\r\n",
            (unsigned int)sample_count,
            (unsigned int)conversion_wait_us);
  PC_SendADCData(hrc_adc_samples, sample_count);
  PC_Printf("ADC_DATA_END\r\n");
  return HRC_OK;
}

static HRC_StatusTypeDef HRC_OCTDC_WriteField(uint8_t addr,
                                               uint8_t mask,
                                               uint8_t shift,
                                               uint8_t field_value)
{
  /* 功能：读改写指定CFG字段，并通过单地址读回确认字段写入正确。 */
  uint8_t current;
  uint8_t write_value;
  uint8_t readback;
  uint8_t encoded_value;
  HRC_StatusTypeDef status;

  status = HRC_ReadCfgSingle(addr, &current);
  if (status != HRC_OK)
  {
    return status;
  }

  encoded_value = (uint8_t)((field_value << shift) & mask);
  write_value = (uint8_t)((current & (uint8_t)(~mask)) | encoded_value);

  status = HRC_WriteCfgSingle(addr, write_value);
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_ReadCfgSingle(addr, &readback);
  if (status != HRC_OK)
  {
    return status;
  }

  return ((readback & mask) == encoded_value) ? HRC_OK : HRC_VERIFY_FAILED;
}

HRC_StatusTypeDef HRC_OCTDC_ConfigTrim(uint8_t delay_trim,
                                       uint8_t bl_res_trim,
                                       uint8_t ref_res_trim)
{
  /* 功能：配置并验证OCTDC延时、BL侧电阻和REF侧电阻三个TRIM字段。 */
  HRC_StatusTypeDef status;

  if ((delay_trim > 7U) || (bl_res_trim > 31U) || (ref_res_trim > 31U))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_OCTDC_WriteField(HRC_OCTDC_DELAY_CFG_ADDR,
                                HRC_OCTDC_DELAY_MASK,
                                HRC_OCTDC_DELAY_SHIFT,
                                delay_trim);
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_OCTDC_WriteField(HRC_OCTDC_BL_RES_CFG_ADDR,
                                HRC_OCTDC_RES_MASK,
                                0U,
                                bl_res_trim);
  if (status != HRC_OK)
  {
    return status;
  }

  return HRC_OCTDC_WriteField(HRC_OCTDC_REF_RES_CFG_ADDR,
                              HRC_OCTDC_RES_MASK,
                              0U,
                              ref_res_trim);
}

static HRC_StatusTypeDef HRC_OCTDC_ReadStableRaw(uint8_t *raw)
{
  /* 功能：重复读取异步OCTDC输出，确认高七位为0且输出至少连续两次保持一致。 */
  uint8_t previous;
  uint8_t current;
  uint8_t read_count;

  if (raw == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  previous = HRC_ReadDataOut();
  if ((previous & 0xFEU) != 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  for (read_count = 1U; read_count < HRC_OCTDC_STABLE_MAX_READS; read_count++)
  {
    delay_us(HRC_OCTDC_STABLE_INTERVAL_US);
    current = HRC_ReadDataOut();
    if ((current & 0xFEU) != 0U)
    {
      return HRC_PROTOCOL_ERROR;
    }

    if (current == previous)
    {
      *raw = current;
      return HRC_OK;
    }

    previous = current;
  }

  return HRC_TIMEOUT;
}

HRC_StatusTypeDef HRC_OCTDC_TestStart(void)
{
  /* 功能：确认HRC处于IDLE后发送OCTDC_TEST指令，使OCTDC开始一次读取。 */
  HRC_StatusTypeDef status;

  if (hrc_octdc_test_active != 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_SendCommand(HRC_CMD_OCTDC_TEST);
  hrc_octdc_test_active = 1U;
  return HRC_OK;
}

HRC_StatusTypeDef HRC_OCTDC_ReadResult(uint16_t conversion_wait_us,
                                       uint8_t *raw,
                                       uint8_t *result)
{
  /* 功能：等待可调OCTDC读取时间，稳定采集原始输出并提取最低位结果。 */
  HRC_StatusTypeDef status;

  if ((raw == NULL) || (result == NULL))
  {
    return HRC_INVALID_PARAM;
  }

  if (hrc_octdc_test_active == 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  if (conversion_wait_us > 0U)
  {
    delay_us(conversion_wait_us);
  }

  status = HRC_OCTDC_ReadStableRaw(raw);
  if (status != HRC_OK)
  {
    return status;
  }

  *result = *raw & 0x01U;
  return HRC_OK;
}

HRC_StatusTypeDef HRC_OCTDC_TestStop(void)
{
  /* 功能：向锁定在OCTDC_TEST状态的HRC发送IDLE指令，并确认重新进入IDLE。 */
  HRC_StatusTypeDef status;

  if (hrc_octdc_test_active == 0U)
  {
    return HRC_PROTOCOL_ERROR;
  }

  HRC_SendCommand(HRC_CMD_IDLE);
  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    hrc_octdc_test_active = 0U;
  }

  return status;
}

static HRC_StatusTypeDef HRC_OCTDC_CaptureOne(uint16_t conversion_wait_us,
                                               uint8_t *raw,
                                               uint8_t *result)
{
  /* 功能：完整执行一次OCTDC_TEST启动、异步结果读取和IDLE退出流程。 */
  HRC_StatusTypeDef read_status;
  HRC_StatusTypeDef stop_status;

  read_status = HRC_OCTDC_TestStart();
  if (read_status != HRC_OK)
  {
    return read_status;
  }

  read_status = HRC_OCTDC_ReadResult(conversion_wait_us, raw, result);
  stop_status = HRC_OCTDC_TestStop();

  if (read_status != HRC_OK)
  {
    return read_status;
  }

  return stop_status;
}

HRC_StatusTypeDef HRC_Test_OCTDC_Single(uint8_t delay_trim,
                                        uint8_t bl_res_trim,
                                        uint8_t ref_res_trim,
                                        uint16_t conversion_wait_us,
                                        uint8_t *result)
{
  /* 功能：配置OCTDC三个TRIM字段，执行单次读取并对比电阻关系对应的期望结果。 */
  uint8_t raw = 0U;
  uint8_t expected_result;
  HRC_StatusTypeDef status;

  if (result == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_OCTDC_ConfigTrim(delay_trim, bl_res_trim, ref_res_trim);
  if (status != HRC_OK)
  {
    PC_Printf("OCTDC_TEST config failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  status = HRC_OCTDC_CaptureOne(conversion_wait_us, &raw, result);
  if (status != HRC_OK)
  {
    PC_Printf("OCTDC_TEST capture failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  expected_result = (bl_res_trim < ref_res_trim) ? 1U : 0U;
  PC_Printf("OCTDC_TEST delay=%u bl=%u ref=%u wait_us=%u raw=0x%02X expected=%u actual=%u\r\n",
            (unsigned int)delay_trim,
            (unsigned int)bl_res_trim,
            (unsigned int)ref_res_trim,
            (unsigned int)conversion_wait_us,
            (unsigned int)raw,
            (unsigned int)expected_result,
            (unsigned int)(*result));

  if (*result != expected_result)
  {
    PC_Printf("OCTDC_TEST failed\r\n");
    return HRC_VERIFY_FAILED;
  }

  PC_Printf("OCTDC_TEST passed\r\n");
  return HRC_OK;
}

static HRC_StatusTypeDef HRC_RW_CheckProgress(uint8_t valid_out,
                                               uint8_t data_out,
                                               uint8_t expected_data)
{
  /*
   * 功能：检查片外读写指令某个操作周期结束时的进度输出是否符合预期。
   * 输入参数：
   *   valid_out：本周期下降沿采集到的VALID_OUT，进度状态要求为0。
   *   data_out：本周期下降沿采集到的8-bit DATA_OUT进度码。
   *   expected_data：当前周期期望出现的8-bit DATA_OUT进度码。
   * 返回值：HRC_OK表示进度正确；HRC_PROTOCOL_ERROR表示VALID或DATA不符合协议。
   */
  if ((valid_out != 0U) || (data_out != expected_data))
  {
    return HRC_PROTOCOL_ERROR;
  }

  return HRC_OK;
}

static HRC_StatusTypeDef HRC_RW_AbortCommand(HRC_StatusTypeDef failure_status)
{
  /*
   * 功能：片外读写指令发送过程中出现错误时，尝试发送IDLE恢复芯片状态。
   * 输入参数：
   *   failure_status：触发恢复流程的原始错误状态，恢复完成后仍返回该状态。
   * 返回值：始终返回failure_status；若IDLE恢复失败，会额外通过串口报告恢复状态。
   */
  HRC_StatusTypeDef idle_status;

  HRC_SendCommand(HRC_CMD_IDLE);
  idle_status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (idle_status != HRC_OK)
  {
    PC_Printf("RW command abort failed: idle_status=%u\r\n",
              (unsigned int)idle_status);
  }

  HRC_RW_SetActiveMode(HRC_RW_MODE_NONE);
  return failure_status;
}

HRC_StatusTypeDef HRC_RW_MultipleConfigRows(const uint8_t *row_bitmap)
{
  /*
   * 功能：配置R/W_multiple使用的128-bit ROW寄存器，并逐字节读回验证。
   * 输入参数：
   *   row_bitmap：指向16字节ROW位图，row_bitmap[n]对应CFG[n]；每个bit为1表示打开对应行。
   *               数组必须至少包含16字节且不能为NULL，row0对应byte0 bit0，row127对应byte15 bit7。
   * 返回值：HRC_OK表示配置及读回正确；也可能返回参数、协议、超时或验证错误。
   */
  uint8_t cfg[HRC_CFG_REG_NUM];
  uint8_t readback[HRC_CFG_REG_NUM];
  uint8_t i;
  HRC_StatusTypeDef status;

  if (row_bitmap == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_ReadCfgTotal(cfg);
  if (status != HRC_OK)
  {
    return status;
  }

  for (i = 0U; i < HRC_RW_ROW_CFG_REG_NUM; i++)
  {
    cfg[i] = row_bitmap[i];
  }

  status = HRC_WriteCfgTotal(cfg);
  if (status != HRC_OK)
  {
    return status;
  }

  status = HRC_ReadCfgTotal(readback);
  if (status != HRC_OK)
  {
    return status;
  }

  for (i = 0U; i < HRC_RW_ROW_CFG_REG_NUM; i++)
  {
    if (readback[i] != row_bitmap[i])
    {
      PC_Printf("RW multiple ROW verify failed: cfg=%u expected=0x%02X actual=0x%02X\r\n",
                (unsigned int)i,
                (unsigned int)row_bitmap[i],
                (unsigned int)readback[i]);
      return HRC_VERIFY_FAILED;
    }
  }

  return HRC_OK;
}

HRC_StatusTypeDef HRC_RW_SingleStart(uint8_t row_addr,
                                     uint8_t cell_sel,
                                     uint8_t unit_sel_addr)
{
  /*
   * 功能：发送R/W_single指令和行列地址，验证DATA_OUT依次为0x10、0x11、0x12、0x13。
   *       返回HRC_OK后芯片保持片外连接状态，不会自动发送IDLE。
   * 输入参数：
   *   row_addr：行地址，范围0～127，对应DATA_IN[6:0]。
   *   cell_sel：unit内cell编号，范围0～3，对应DATA_IN[1:0]。
   *   unit_sel_addr：unit列地址，范围0～64，对应DATA_IN[6:0]。
   * 返回值：HRC_OK表示地址配置完成且进入0x13就绪状态；也可能返回参数、超时或协议错误。
   */
  uint8_t valid_out;
  uint8_t data_out;
  uint16_t col_addr;
  HRC_StatusTypeDef status;

  if ((row_addr > 127U) || (cell_sel > 3U) || (unit_sel_addr > 64U))
  {
    return HRC_INVALID_PARAM;
  }

  if (hrc_rw_active_mode != HRC_RW_MODE_NONE)
  {
    return HRC_PROTOCOL_ERROR;
  }

  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(1U, HRC_CMD_RW_SINGLE, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, 0x00U);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, row_addr, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_SINGLE_PROGRESS_ROW);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, cell_sel, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_SINGLE_PROGRESS_CELL);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, unit_sel_addr, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_SINGLE_PROGRESS_UNIT);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_SINGLE_PROGRESS_READY);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_RW_SetActiveMode(HRC_RW_MODE_SINGLE);
  col_addr = (uint16_t)(((uint16_t)unit_sel_addr << 2U) | cell_sel);
  PC_Printf("RW_SINGLE ready: row=%u cell=%u unit=%u col=%u DATA=0x%02X\r\n",
            (unsigned int)row_addr,
            (unsigned int)cell_sel,
            (unsigned int)unit_sel_addr,
            (unsigned int)col_addr,
            (unsigned int)data_out);
  return HRC_OK;
}

HRC_StatusTypeDef HRC_RW_MultipleStart(uint8_t cell_sel,
                                       uint8_t unit_sel_addr)
{
  /*
   * 功能：发送R/W_multiple指令和列地址，验证DATA_OUT依次为0x20、0x21、0x23。
   *       调用前必须已配置ROW寄存器；返回HRC_OK后保持片外连接状态，不自动发送IDLE。
   * 输入参数：
   *   cell_sel：unit内cell编号，范围0～3，对应DATA_IN[1:0]。
   *   unit_sel_addr：unit列地址，范围0～64，对应DATA_IN[6:0]。
   * 返回值：HRC_OK表示列地址配置完成且进入0x23就绪状态；也可能返回参数、超时或协议错误。
   */
  uint8_t valid_out;
  uint8_t data_out;
  uint16_t col_addr;
  HRC_StatusTypeDef status;

  if ((cell_sel > 3U) || (unit_sel_addr > 64U))
  {
    return HRC_INVALID_PARAM;
  }

  if (hrc_rw_active_mode != HRC_RW_MODE_NONE)
  {
    return HRC_PROTOCOL_ERROR;
  }

  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(1U, HRC_CMD_RW_MULTIPLE, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, 0x00U);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, cell_sel, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_MULTI_PROGRESS_CELL);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, unit_sel_addr, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_MULTI_PROGRESS_UNIT);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  status = HRC_RW_CheckProgress(valid_out, data_out, HRC_RW_MULTI_PROGRESS_READY);
  if (status != HRC_OK)
  {
    return HRC_RW_AbortCommand(status);
  }

  HRC_RW_SetActiveMode(HRC_RW_MODE_MULTIPLE);
  col_addr = (uint16_t)(((uint16_t)unit_sel_addr << 2U) | cell_sel);
  PC_Printf("RW_MULTIPLE ready: cell=%u unit=%u col=%u DATA=0x%02X\r\n",
            (unsigned int)cell_sel,
            (unsigned int)unit_sel_addr,
            (unsigned int)col_addr,
            (unsigned int)data_out);
  return HRC_OK;
}

HRC_StatusTypeDef HRC_RW_Stop(void)
{
  /*
   * 功能：片外GBL/GSL读写脉冲结束后发送IDLE，使芯片退出R/W_single或R/W_multiple状态。
   * 输入参数：无。
   * 返回值：HRC_OK表示已确认回到IDLE；HRC_PROTOCOL_ERROR表示当前没有活动读写状态；也可能超时。
   */
  HRC_StatusTypeDef status;

  if (hrc_rw_active_mode == HRC_RW_MODE_NONE)
  {
    return HRC_PROTOCOL_ERROR;
  }

  HRC_SendCommand(HRC_CMD_IDLE);
  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status == HRC_OK)
  {
    PC_Printf("RW mode stopped: previous_mode=%u\r\n",
              (unsigned int)hrc_rw_active_mode);
    HRC_RW_SetActiveMode(HRC_RW_MODE_NONE);
  }

  return status;
}

HRC_StatusTypeDef HRC_Test_RW_SingleDefault(void)
{
  /*
   * 功能：使用hrc_test.h中的默认行、cell和unit地址启动R/W_single测试并保持连接。
   * 输入参数：无，地址由HRC_RW_SINGLE_DEFAULT_*宏配置。
   * 返回值：与HRC_RW_SingleStart一致；成功后必须在片外脉冲结束时调用HRC_RW_Stop。
   */
  if ((HRC_RW_SINGLE_DEFAULT_ROW_ADDR > 127U) ||
      (HRC_RW_SINGLE_DEFAULT_CELL_SEL > 3U) ||
      (HRC_RW_SINGLE_DEFAULT_UNIT_ADDR > 64U))
  {
    return HRC_INVALID_PARAM;
  }

  return HRC_RW_SingleStart((uint8_t)HRC_RW_SINGLE_DEFAULT_ROW_ADDR,
                            (uint8_t)HRC_RW_SINGLE_DEFAULT_CELL_SEL,
                            (uint8_t)HRC_RW_SINGLE_DEFAULT_UNIT_ADDR);
}

HRC_StatusTypeDef HRC_Test_RW_MultipleDefault(void)
{
  /*
   * 功能：仅打开头文件指定的默认行，配置ROW寄存器后启动R/W_multiple测试并保持连接。
   * 输入参数：无，行、cell和unit地址由HRC_RW_MULTIPLE_DEFAULT_*宏配置。
   * 返回值：HRC_OK表示ROW配置完成并进入0x23状态；成功后片外脉冲结束时必须调用HRC_RW_Stop。
   */
  uint8_t row_bitmap[HRC_RW_ROW_CFG_REG_NUM] = {0U};
  uint16_t row_addr = HRC_RW_MULTIPLE_DEFAULT_ROW_ADDR;
  HRC_StatusTypeDef status;

  if ((row_addr > 127U) ||
      (HRC_RW_MULTIPLE_DEFAULT_CELL_SEL > 3U) ||
      (HRC_RW_MULTIPLE_DEFAULT_UNIT_ADDR > 64U))
  {
    return HRC_INVALID_PARAM;
  }

  row_bitmap[row_addr >> 3U] = (uint8_t)(1U << (row_addr & 0x07U));
  status = HRC_RW_MultipleConfigRows(row_bitmap);
  if (status != HRC_OK)
  {
    PC_Printf("RW_MULTIPLE default ROW config failed: status=%u\r\n",
              (unsigned int)status);
    return status;
  }

  return HRC_RW_MultipleStart((uint8_t)HRC_RW_MULTIPLE_DEFAULT_CELL_SEL,
                              (uint8_t)HRC_RW_MULTIPLE_DEFAULT_UNIT_ADDR);
}

static HRC_StatusTypeDef HRC_Test_ReadPrepareConfig(HRC_ReadConfigTypeDef *config,
                                                     uint8_t enable_all_units,
                                                     uint8_t single_unit_addr)
{
  /*
   * 功能：生成片上READ测试配置，填入头文件指定的TRIM值，并按测试模式设置EN_config。
   * 输入参数：config为待填充配置结构体指针，不能为NULL；enable_all_units取0或1，1表示使能全部65个unit；
   *           single_unit_addr范围0至64，仅在enable_all_units为0时表示唯一使能的unit，无单位。
   * 输出参数：config返回完整READ配置，unit_enable中的bit n对应unit n。
   * 返回值：HRC_OK表示配置生成成功，HRC_INVALID_PARAM表示参数或头文件宏超出范围。
   */
  uint8_t unit_addr;
  HRC_StatusTypeDef status;

  if ((config == NULL) || (enable_all_units > 1U) ||
      (single_unit_addr >= HRC_READ_UNIT_NUM) ||
      (HRC_READ_TEST_DEVELOP_SEL > 7U) ||
      (HRC_READ_TEST_PRE_SEL > 15U) ||
      (HRC_READ_TEST_SENS_SEL > 7U) ||
      (HRC_READ_TEST_RES_TRIM > 31U) ||
      (HRC_READ_TEST_WAIT_CYCLE > 7U))
  {
    return HRC_INVALID_PARAM;
  }

  HRC_ReadInitConfig(config);
  config->develop_sel = HRC_READ_TEST_DEVELOP_SEL;
  config->pre_sel = HRC_READ_TEST_PRE_SEL;
  config->sens_sel = HRC_READ_TEST_SENS_SEL;
  config->read_res_trim = HRC_READ_TEST_RES_TRIM;
  config->read_wait_cycle = HRC_READ_TEST_WAIT_CYCLE;

  if (enable_all_units != 0U)
  {
    for (unit_addr = 0U; unit_addr < HRC_READ_UNIT_NUM; unit_addr++)
    {
      status = HRC_ReadSetUnitEnable(config, unit_addr, 1U);
      if (status != HRC_OK)
      {
        return status;
      }
    }
  }
  else
  {
    status = HRC_ReadSetUnitEnable(config, single_unit_addr, 1U);
    if (status != HRC_OK)
    {
      return status;
    }
  }

  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_ReadSingleDefault(void)
{
  /*
   * 功能：配置并执行一次默认行、unit和cell的片上单bit读取，通过串口发送地址和SA结果。
   * 输入参数：无；行、unit、cell、自适应WL、TRIM和等待周期由hrc_test.h中的HRC_READ_*测试宏设置。
   * 输出参数：无；串口输出READ_SINGLE_RESULT以及读取到的0或1。
   * 返回值：HRC_OK表示配置、READ协议、结果接收和IDLE确认成功；也可能返回参数、协议、超时或验证错误。
   */
  HRC_ReadConfigTypeDef config;
  uint8_t result = 0U;
  HRC_StatusTypeDef status;

  if ((HRC_READ_SINGLE_TEST_ROW_ADDR > 127U) ||
      (HRC_READ_SINGLE_TEST_UNIT_ADDR > 64U) ||
      (HRC_READ_SINGLE_TEST_CELL_SEL > 3U) ||
      (HRC_READ_TEST_ADAPTIVE_WL > 1U))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_Test_ReadPrepareConfig(&config, 0U,
                                      (uint8_t)HRC_READ_SINGLE_TEST_UNIT_ADDR);
  if (status == HRC_OK)
  {
    status = HRC_ReadConfigure(&config);
  }
  if (status == HRC_OK)
  {
    status = HRC_ReadSingle((uint8_t)HRC_READ_TEST_ADAPTIVE_WL,
                            (uint8_t)HRC_READ_SINGLE_TEST_ROW_ADDR,
                            (uint8_t)HRC_READ_SINGLE_TEST_UNIT_ADDR,
                            (uint8_t)HRC_READ_SINGLE_TEST_CELL_SEL,
                            (uint8_t)HRC_READ_TEST_WAIT_CYCLE,
                            &result);
  }

  if (status != HRC_OK)
  {
    PC_Printf("READ_SINGLE failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  PC_Printf("READ_SINGLE_RESULT row=%u unit=%u cell=%u adaptive=%u result=%u\r\n",
            (unsigned int)HRC_READ_SINGLE_TEST_ROW_ADDR,
            (unsigned int)HRC_READ_SINGLE_TEST_UNIT_ADDR,
            (unsigned int)HRC_READ_SINGLE_TEST_CELL_SEL,
            (unsigned int)HRC_READ_TEST_ADAPTIVE_WL,
            (unsigned int)result);
  return HRC_OK;
}

HRC_StatusTypeDef HRC_Test_ReadRowDefault(void)
{
  /*
   * 功能：配置并读取默认行中所有使能unit的指定cell，通过串口逐unit发送65bit SA结果。
   * 输入参数：无；行、cell、自适应WL、是否使能全部unit、TRIM和等待周期由hrc_test.h中的测试宏设置。
   * 输出参数：无；串口在READ_ROW_DATA_BEGIN与READ_ROW_DATA_END之间输出“unit,result”。
   * 返回值：HRC_OK表示配置、9周期结果接收和IDLE确认成功；也可能返回参数、协议、超时或验证错误。
   */
  HRC_ReadConfigTypeDef config;
  uint8_t unit_results[HRC_READ_RESULT_BYTE_NUM];
  uint8_t unit_addr;
  HRC_StatusTypeDef status;

  if ((HRC_READ_ROW_TEST_ROW_ADDR > 127U) ||
      (HRC_READ_ROW_TEST_CELL_SEL > 3U) ||
      (HRC_READ_ROW_TEST_ENABLE_ALL_UNITS > 1U) ||
      (HRC_READ_TEST_ADAPTIVE_WL > 1U))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_Test_ReadPrepareConfig(&config,
                                      (uint8_t)HRC_READ_ROW_TEST_ENABLE_ALL_UNITS,
                                      (uint8_t)HRC_READ_SINGLE_TEST_UNIT_ADDR);
  if (status == HRC_OK)
  {
    status = HRC_ReadConfigure(&config);
  }
  if (status == HRC_OK)
  {
    status = HRC_ReadRow((uint8_t)HRC_READ_TEST_ADAPTIVE_WL,
                         (uint8_t)HRC_READ_ROW_TEST_ROW_ADDR,
                         (uint8_t)HRC_READ_ROW_TEST_CELL_SEL,
                         (uint8_t)HRC_READ_TEST_WAIT_CYCLE,
                         unit_results);
  }

  if (status != HRC_OK)
  {
    PC_Printf("READ_ROW failed: status=%u\r\n", (unsigned int)status);
    return status;
  }

  PC_Printf("READ_ROW_DATA_BEGIN row=%u cell=%u adaptive=%u\r\n",
            (unsigned int)HRC_READ_ROW_TEST_ROW_ADDR,
            (unsigned int)HRC_READ_ROW_TEST_CELL_SEL,
            (unsigned int)HRC_READ_TEST_ADAPTIVE_WL);
  for (unit_addr = 0U; unit_addr < HRC_READ_UNIT_NUM; unit_addr++)
  {
    PC_Printf("%u,%u\r\n",
              (unsigned int)unit_addr,
              (unsigned int)HRC_ReadGetUnitBit(unit_results, unit_addr));
  }
  PC_Printf("READ_ROW_DATA_END\r\n");
  return HRC_OK;
}
