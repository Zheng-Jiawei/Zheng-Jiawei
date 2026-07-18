#include "hrc_read.h"
#include <string.h>

#define HRC_READ_PROGRESS_ADAPTIVE  0x30U
#define HRC_READ_PROGRESS_ROW       0x31U
#define HRC_READ_PROGRESS_MODE      0x32U
#define HRC_READ_PROGRESS_UNIT      0x33U
#define HRC_READ_PROGRESS_CELL      0x34U
#define HRC_READ_PROGRESS_START     0x35U
#define HRC_READ_PROGRESS_WAIT      0x36U

static HRC_StatusTypeDef HRC_ReadCheckProgress(uint8_t valid_out,
                                                uint8_t data_out,
                                                uint8_t expected_data)
{
  /*
   * 功能：检查READ指令某个操作周期结束时的进度输出。
   * 输入参数：valid_out为下降沿读取的VALID_OUT，进度阶段应为0；data_out为8-bit进度码；
   *           expected_data为期望的8-bit进度码，三者均无单位。
   * 输出参数：无。
   * 返回值：HRC_OK表示输出符合协议，HRC_PROTOCOL_ERROR表示VALID_OUT或DATA_OUT不正确。
   */
  if ((valid_out != 0U) || (data_out != expected_data))
  {
    return HRC_PROTOCOL_ERROR;
  }

  return HRC_OK;
}

static HRC_StatusTypeDef HRC_ReadAbort(HRC_StatusTypeDef failure_status)
{
  /*
   * 功能：READ指令发生协议错误时发送IDLE，并尝试恢复芯片状态。
   * 输入参数：failure_status为触发恢复的原始HRC状态码，无单位。
   * 输出参数：无。
   * 返回值：返回原始failure_status，便于调用者保留首个失败原因。
   */
  HRC_SendCommand(HRC_CMD_IDLE);
  (void)HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  return failure_status;
}

static HRC_StatusTypeDef HRC_ReadTransferAndCheck(uint8_t data_in,
                                                   uint8_t expected_data)
{
  /*
   * 功能：发送一个CMD=0的数据操作周期，并检查下降沿采集到的READ进度码。
   * 输入参数：data_in为本周期送入DATA_IN的8-bit数据；expected_data为本周期期望读取的8-bit进度码，均无单位。
   * 输出参数：无。
   * 返回值：HRC_OK表示进度正确，否则返回HRC_PROTOCOL_ERROR。
   */
  uint8_t valid_out;
  uint8_t data_out;

  HRC_TransferCycle(0U, data_in, &valid_out, &data_out);
  return HRC_ReadCheckProgress(valid_out, data_out, expected_data);
}

static HRC_StatusTypeDef HRC_ReadBegin(uint8_t adaptive_wl,
                                       uint8_t row_addr,
                                       uint8_t read_mode,
                                       uint8_t unit_sel_addr,
                                       uint8_t cell_sel,
                                       uint8_t read_wait_cycle)
{
  /*
   * 功能：发送READ指令全部输入字段，检查0x30至0x35，并等待配置的额外SA读取周期。
   * 输入参数：adaptive_wl取0或1，表示关闭或开启自适应关WL；row_addr范围0至127；
   *           read_mode取HRC_READ_MODE_ROW或HRC_READ_MODE_SINGLE；unit_sel_addr范围0至64，整行模式忽略；
   *           cell_sel范围0至3；read_wait_cycle范围0至7，单位为操作周期。
   * 输出参数：无。
   * 返回值：HRC_OK表示下一周期应开始输出读取结果；也可能返回参数、超时或协议错误。
   */
  uint8_t valid_out;
  uint8_t data_out;
  uint8_t wait_index;
  HRC_StatusTypeDef status;

  if ((adaptive_wl > 1U) || (row_addr > 127U) ||
      (read_mode > HRC_READ_MODE_SINGLE) || (unit_sel_addr > 64U) ||
      (cell_sel > 3U) || (read_wait_cycle > 7U))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(1U, HRC_CMD_READ, &valid_out, &data_out);
  status = HRC_ReadCheckProgress(valid_out, data_out, 0x00U);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck(adaptive_wl, HRC_READ_PROGRESS_ADAPTIVE);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck(row_addr, HRC_READ_PROGRESS_ROW);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck(read_mode, HRC_READ_PROGRESS_MODE);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck((read_mode == HRC_READ_MODE_SINGLE) ? unit_sel_addr : 0U,
                                    HRC_READ_PROGRESS_UNIT);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck(cell_sel, HRC_READ_PROGRESS_CELL);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  status = HRC_ReadTransferAndCheck(0x00U, HRC_READ_PROGRESS_START);
  if (status != HRC_OK)
  {
    return HRC_ReadAbort(status);
  }

  for (wait_index = 0U; wait_index < read_wait_cycle; wait_index++)
  {
    status = HRC_ReadTransferAndCheck(0x00U, HRC_READ_PROGRESS_WAIT);
    if (status != HRC_OK)
    {
      return HRC_ReadAbort(status);
    }
  }

  return HRC_OK;
}

void HRC_ReadInitConfig(HRC_ReadConfigTypeDef *config)
{
  /*
   * 功能：清零65个unit使能位，并填入表7给出的READ寄存器默认值。
   * 输入参数：config指向待初始化的HRC_ReadConfigTypeDef结构体，不能为NULL。
   * 输出参数：config返回初始化后的配置；unit_enable共9字节，bit n对应unit n。
   * 返回值：无；config为NULL时不执行操作。
   */
  if (config == NULL)
  {
    return;
  }

  memset(config, 0, sizeof(*config));
  config->develop_sel = HRC_READ_DEFAULT_DEVELOP_SEL;
  config->pre_sel = HRC_READ_DEFAULT_PRE_SEL;
  config->sens_sel = HRC_READ_DEFAULT_SENS_SEL;
  config->read_res_trim = HRC_READ_DEFAULT_RES_TRIM;
  config->read_wait_cycle = HRC_READ_DEFAULT_WAIT_CYCLE;
}

HRC_StatusTypeDef HRC_ReadSetUnitEnable(HRC_ReadConfigTypeDef *config,
                                        uint8_t unit_addr,
                                        uint8_t enable)
{
  /*
   * 功能：在自然顺序的65-bit使能位图中开启或关闭一个unit。
   * 输入参数：config为READ配置结构体指针，不能为NULL；unit_addr范围0至64；enable取0关闭、1开启，无单位。
   * 输出参数：更新config->unit_enable中对应unit的bit。
   * 返回值：HRC_OK表示设置成功，HRC_INVALID_PARAM表示指针、unit地址或enable无效。
   */
  uint8_t mask;

  if ((config == NULL) || (unit_addr >= HRC_READ_UNIT_NUM) || (enable > 1U))
  {
    return HRC_INVALID_PARAM;
  }

  mask = (uint8_t)(1U << (unit_addr & 0x07U));
  if (enable != 0U)
  {
    config->unit_enable[unit_addr >> 3U] |= mask;
  }
  else
  {
    config->unit_enable[unit_addr >> 3U] &= (uint8_t)(~mask);
  }

  return HRC_OK;
}

uint8_t HRC_ReadGetUnitBit(const uint8_t *unit_bits, uint8_t unit_addr)
{
  /*
   * 功能：从自然顺序的65-bit位图中取得指定unit的一位状态或读取结果。
   * 输入参数：unit_bits指向至少9字节的位图，bit n对应unit n；unit_addr范围0至64，无单位。
   * 输出参数：无。
   * 返回值：返回0或1；指针为NULL或unit地址无效时返回0。
   */
  if ((unit_bits == NULL) || (unit_addr >= HRC_READ_UNIT_NUM))
  {
    return 0U;
  }

  return (uint8_t)((unit_bits[unit_addr >> 3U] >> (unit_addr & 0x07U)) & 0x01U);
}

HRC_StatusTypeDef HRC_ReadConfigure(const HRC_ReadConfigTypeDef *config)
{
  /*
   * 功能：配置并读回验证READ所需的EN_config、模拟时序TRIM、参考电阻TRIM和等待周期。
   * 输入参数：config为READ配置结构体指针；develop_sel范围0至7，pre_sel范围0至15，sens_sel范围0至7，
   *           read_res_trim范围0至31，read_wait_cycle范围0至7，均无单位；unit_enable bit n对应unit n。
   * 输出参数：无。
   * 返回值：HRC_OK表示CFG16至CFG26配置和读回一致；也可能返回参数、协议、超时或验证错误。
   */
  uint8_t cfg[HRC_CFG_REG_NUM];
  uint8_t readback[HRC_CFG_REG_NUM];
  uint8_t unit_addr;
  uint8_t cfg_addr;
  uint8_t cfg_mask;
  HRC_StatusTypeDef status;

  if ((config == NULL) || (config->develop_sel > 7U) ||
      (config->pre_sel > 15U) || (config->sens_sel > 7U) ||
      (config->read_res_trim > 31U) || (config->read_wait_cycle > 7U) ||
      ((config->unit_enable[8] & 0xFEU) != 0U))
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_ReadCfgTotal(cfg);
  if (status != HRC_OK)
  {
    return status;
  }

  memset(&cfg[HRC_READ_EN_CFG_FIRST_ADDR], 0,
         HRC_READ_EN_CFG_LAST_ADDR - HRC_READ_EN_CFG_FIRST_ADDR + 1U);

  for (unit_addr = 1U; unit_addr < HRC_READ_UNIT_NUM; unit_addr++)
  {
    if (HRC_ReadGetUnitBit(config->unit_enable, unit_addr) != 0U)
    {
      cfg_addr = (uint8_t)(HRC_READ_EN_CFG_FIRST_ADDR + ((unit_addr - 1U) >> 3U));
      cfg_mask = (uint8_t)(1U << ((unit_addr - 1U) & 0x07U));
      cfg[cfg_addr] |= cfg_mask;
    }
  }

  cfg[HRC_READ_TIMING_CFG0_ADDR] =
      (uint8_t)((HRC_ReadGetUnitBit(config->unit_enable, 0U) != 0U ? HRC_READ_EN_UNIT0_MASK : 0U) |
                ((config->develop_sel << HRC_READ_DEVELOP_SHIFT) & HRC_READ_DEVELOP_MASK) |
                ((config->pre_sel << HRC_READ_PRE_SHIFT) & HRC_READ_PRE_MASK));
  cfg[HRC_READ_TIMING_CFG1_ADDR] =
      (uint8_t)(((config->sens_sel << HRC_READ_SENS_SHIFT) & HRC_READ_SENS_MASK) |
                ((config->read_res_trim << HRC_READ_RES_TRIM_SHIFT) & HRC_READ_RES_TRIM_MASK));
  cfg[HRC_READ_WAIT_CFG_ADDR] =
      (uint8_t)((cfg[HRC_READ_WAIT_CFG_ADDR] & (uint8_t)(~HRC_READ_WAIT_CYCLE_MASK)) |
                ((config->read_wait_cycle << HRC_READ_WAIT_CYCLE_SHIFT) & HRC_READ_WAIT_CYCLE_MASK));

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

  for (cfg_addr = HRC_READ_EN_CFG_FIRST_ADDR;
       cfg_addr <= HRC_READ_WAIT_CFG_ADDR;
       cfg_addr++)
  {
    if (readback[cfg_addr] != cfg[cfg_addr])
    {
      return HRC_VERIFY_FAILED;
    }
  }

  return HRC_OK;
}

HRC_StatusTypeDef HRC_ReadSingle(uint8_t adaptive_wl,
                                 uint8_t row_addr,
                                 uint8_t unit_sel_addr,
                                 uint8_t cell_sel,
                                 uint8_t read_wait_cycle,
                                 uint8_t *result)
{
  /*
   * 功能：执行一次指定行、unit和cell的片上SA单bit读取，并确认芯片自动返回IDLE。
   * 输入参数：adaptive_wl取0或1；row_addr范围0至127；unit_sel_addr范围0至64；cell_sel范围0至3；
   *           read_wait_cycle范围0至7，单位为操作周期，必须与CFG26中设置一致。
   * 输出参数：result不能为NULL，成功时返回SA读取结果0或1。
   * 返回值：HRC_OK表示读取及IDLE确认成功；也可能返回参数、协议、超时或验证错误。
   */
  uint8_t valid_out;
  uint8_t data_out;
  HRC_StatusTypeDef status;

  if (result == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  status = HRC_ReadBegin(adaptive_wl, row_addr, HRC_READ_MODE_SINGLE,
                         unit_sel_addr, cell_sel, read_wait_cycle);
  if (status != HRC_OK)
  {
    return status;
  }

  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  if ((valid_out != 1U) || ((data_out & 0xFEU) != 0U))
  {
    return HRC_ReadAbort(HRC_PROTOCOL_ERROR);
  }

  *result = data_out & 0x01U;
  return HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
}

HRC_StatusTypeDef HRC_ReadRow(uint8_t adaptive_wl,
                              uint8_t row_addr,
                              uint8_t cell_sel,
                              uint8_t read_wait_cycle,
                              uint8_t *unit_results)
{
  /*
   * 功能：读取指定行中所有使能unit的指定cell，将芯片输出的9个周期重组为自然顺序65-bit结果。
   * 输入参数：adaptive_wl取0或1；row_addr范围0至127；cell_sel范围0至3；read_wait_cycle范围0至7，
   *           单位为操作周期且必须与CFG26设置一致。
   * 输出参数：unit_results指向至少9字节数组，成功时bit n对应unit n的SA读取结果，未使能unit应为0。
   * 返回值：HRC_OK表示65bit结果接收并确认IDLE成功；也可能返回参数、协议、超时或验证错误。
   */
  uint8_t valid_out;
  uint8_t data_out;
  uint8_t group;
  uint8_t bit_index;
  uint8_t unit_addr;
  HRC_StatusTypeDef status;

  if (unit_results == NULL)
  {
    return HRC_INVALID_PARAM;
  }

  memset(unit_results, 0, HRC_READ_RESULT_BYTE_NUM);
  status = HRC_ReadBegin(adaptive_wl, row_addr, HRC_READ_MODE_ROW,
                         0U, cell_sel, read_wait_cycle);
  if (status != HRC_OK)
  {
    return status;
  }

  for (group = 0U; group < 8U; group++)
  {
    HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
    if (valid_out != 1U)
    {
      return HRC_ReadAbort(HRC_PROTOCOL_ERROR);
    }

    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
      unit_addr = (uint8_t)(1U + group * 8U + bit_index);
      if ((data_out & (1U << bit_index)) != 0U)
      {
        unit_results[unit_addr >> 3U] |= (uint8_t)(1U << (unit_addr & 0x07U));
      }
    }
  }

  HRC_TransferCycle(0U, 0x00U, &valid_out, &data_out);
  if ((valid_out != 1U) || ((data_out & 0xFEU) != 0U))
  {
    return HRC_ReadAbort(HRC_PROTOCOL_ERROR);
  }

  unit_results[0] |= data_out & 0x01U;
  return HRC_WaitIdle(HRC_DEFAULT_TIMEOUT_CYCLES);
}
