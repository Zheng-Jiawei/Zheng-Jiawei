# HRC 文档行为模型与 STM32 联合仿真

本目录把 HRC 技术文档中的数字端口协议实现为独立 C 行为模型，并通过 GPIO/HAL mock 直接运行工程中的生产代码：

- `Core/Src/hrc_bus.c`
- `Core/Src/hrc_cfg.c`
- `Core/Src/hrc_test.c`

串口格式化不参与 HRC 时序，HostTest 使用轻量 `pc_comm_host.c` 记录 ADC/OCTDC 结果，避免 Keil 指令集模拟器在 `vsnprintf` 上消耗大量时间。板端 `pc_comm.c` 未修改。

## 当前模型覆盖范围

- 异步低有效复位：`VALID_OUT=0`、`DATA_OUT=0xA5`
- `CMD/DATA_IN` 在前一 `CLK` 下降沿后准备，并保持到下一上升沿采样
- IDLE 与连续两个空闲周期 + 一个保护周期
- `WRITE_CFG`：单寄存器、45 寄存器全写、cycle 47/48 尾周期
- `READ_CFG`：单寄存器、45 寄存器全读、`VALID_OUT` 时序
- `ADC_TEST`：锁定状态、异步输出、D0 为 MSB 的 6 位反序、IDLE 手动退出
- `OCTDC_TEST`：锁定状态、异步输出、IDLE 手动退出

其余运算类 opcode 尚未建模；若执行，`unsupported_commands` 会递增，不能被误判为通过。

文档只规定 ADC/OCTDC 输出为异步信号，没有给出数值完成时间。当前测试把 ADC 延迟设为 3000 ns、OCTDC 延迟设为 2000 ns，这两个值是可替换的仿真参数，不是芯片保证值。

## 一键运行

环境要求：Keil MDK/ARMCC5、Cortex-M4 Simulator、Python 3。

在仓库项目目录执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\HostTest\run_host_test.ps1
```

脚本会：

1. 用 Keil 完整重编译独立 `HRC_HostTest` 目标；
2. 从 MAP 文件提取 trace、cycle、summary 的实际地址并生成调试脚本；
3. 无界面运行 Cortex-M4 Simulator；
4. 将目标内存导出并解码成 VCD/CSV；
5. 独立核对输入下降沿准备/上升沿采样、复位、单/全表配置读写和 ADC/OCTDC 时序。

若 Keil 或 Python 不在默认位置，可使用：

```powershell
.\HostTest\run_host_test.ps1 -Uv4Path 'D:\path\UV4.exe' -PythonPath 'C:\path\python.exe'
```

运行前请关闭其他 µVision 会话，避免 Keil 把批处理命令转发给已有窗口。

## 输出文件

- `results/hrc_simulation.vcd`：标准数字波形，可用 GTKWave 等工具打开
- `results/hrc_trace.csv`：所有信号变化事件
- `results/hrc_cycles.csv`：每个 CLK 下降沿的周期快照
- `results/hrc_summary.json`：机器可读断言及独立时序核对结果
- `results/hrc_summary.txt`：人可读摘要
- `results/hrc_test.log`：精简测试日志

`hrc_trace.csv` 和 VCD 会把完整的下一拍 `CMD/DATA_IN` 记录在下降沿；
独立解码器还会检查输入只在 `CLK=0` 时变化，并在下一上升沿前满足建立时间。

修改 ADC/OCTDC 输入值或延迟，可编辑 `src/test_main.c` 中的：

```c
HRC_Model_SetAdcCode(42U);
HRC_Model_SetOctdc(1U);
HRC_Model_SetAdcLatencyNs(3000UL);
HRC_Model_SetOctdcLatencyNs(2000UL);
```

若后续提供实测 HRC 波形，可把测得的完成延迟和输出序列作为新的 stimulus/expectation 接入该 runner；不需要原芯片 RTL 源码。只有需要复现未公开的内部状态或模拟电路细节时，才需要更底层的设计资料。
