# STM32_G474_Project Code Overview

## 1. Project Summary

这个工程运行在 `STM32G474` 上，整体作用可以概括为：

1. 通过 `SPI3 + DMA` 与 FPGA 通信，接收 FPGA 下发的配置命令。
2. 根据命令配置片上外部控制网络，其中核心控制逻辑在 `M203.c`。
3. 驱动 DAC 和并口 ADC，完成目标节点的电压配置与采样。
4. 将采样结果重新封装成带 CRC 的数据帧，再通过 `SPI3` 回传给 FPGA。
5. 使用 `FreeRTOS` 将“通信解析”和“采样处理”拆成独立任务执行。

从功能上看，这份代码更像一个“阵列读写控制中间层”：

- FPGA 负责下发控制意图和读取请求。
- STM32 负责执行底层时序、阵列寄存器配置、ADC 采样和结果打包。
- `M203` 相关函数负责把“哪一行、哪一列、接哪一路”真正落到 GPIO 时序上。

## 2. Core File Roles

工程里和业务最相关的文件主要有下面几类：

### `Core/Src/main.c`

负责系统启动和底层驱动函数，主要内容包括：

- HAL、时钟、GPIO、DMA、SPI、USART 初始化
- 上电后对 M203、GBL/GWL/GSL、DAC、ADC 的初始化
- SPI3 DMA 启动
- ADC 读取函数
- CRC 计算函数
- `HAL_SPI_TxRxCpltCallback()` 回调

### `Core/Src/app_freertos.c`

负责 FreeRTOS 任务创建与业务调度，是系统运行时逻辑的核心：

- `StartDefaultTask()`：LED 心跳任务
- `StartTask02()`：解析 FPGA 指令帧
- `StartTask03()`：执行阵列配置、ADC 采样、结果打包和回传准备
- `StartTask04()`、`StartTask05()`：当前为空任务占位

### `Core/Src/M203.c`

负责 M203 相关控制时序和阵列开关配置，包括：

- `Data_in()` / `Clk()` / `Cmd()` / `Rstn()`：底层 GPIO 位控制
- `Set_Sl_*()` / `Set_Bl_*()` / `Set_Wl_*()`：列线、位线、字线配置
- `GBL_select()` / `GWL_select()` / `GSL_select()`：模拟路径选择
- `Set_Bl_single_rows()`：根据接收到的位图数据批量写行寄存器
- `Set_Mux_Reg()`：多列模式下的复用器配置

### 其他外设文件

- `spi.c`：`SPI1` 和 `SPI3` 配置
- `gpio.c`：GPIO 默认电平与方向配置
- `dma.c`：DMA1 通道中断配置
- `stm32g4xx_it.c`：SPI3 和 DMA 中断入口
- `main.h`：全局共享变量声明、引脚定义、公共函数声明

## 3. System Startup Flow

系统上电后的主流程在 `main.c` 中，大致顺序如下：

1. `HAL_Init()`
2. `SystemClock_Config()`
3. 初始化 GPIO、DMA、SPI3、SPI1、USART1
4. 立即启动一次 `SPI3` 的 DMA 收发，准备接收 FPGA 命令
5. 延时后执行硬件初始化：
   - `M203_Init()`
   - `GBL_GWL_GSL_Init()`
   - 选择默认通路 `GBL1`、`GWL2`
   - 配置 DAC 输出电压
   - `ADC_Init()`
6. 调用 `MX_FREERTOS_Init()` 创建任务
7. `osKernelStart()` 启动调度器

这说明工程设计上采用的是：

- 启动阶段完成硬件静态初始化
- 运行阶段由 FreeRTOS 任务处理 FPGA 请求

## 4. Runtime Architecture

运行时最重要的是两个任务和一个 SPI 完成回调之间的配合。

### 4.1 SPI 接收回调

`main.c` 中的 `HAL_SPI_TxRxCpltCallback()` 在 `SPI3` 完成一次 DMA 收发后触发，主要做三件事：

1. 将 `FPGA_rx_buff` 的内容复制到 `FPGA_buff`
2. 如果当前帧不是 `0x03`，继续启动下一次固定长度的 `SPI3 DMA` 接收
3. 置位 `FPGA_rx_flag = 1`，通知任务层有新命令到了

可以理解为：中断只做“收包”和“投递事件”，真正的业务解析放到任务里完成。

### 4.2 `StartTask02()`：协议解析任务

这个任务轮询 `FPGA_rx_flag`。收到完整数据后，会依次检查：

- 帧头 `0x55 0xAA`
- 帧尾 `0xAA 0x55`
- CRC16 校验
- 功能码

然后根据 `FPGA_buff[3]` 的功能码执行不同逻辑：

#### `0x01` 单列模式参数配置

作用：

- 指定起始行、行数
- 指定起始列、列数
- 选择 GSL 和模拟开关路径
- 设置单列对应的 SL/WL
- 计算每次下发的有效位图字节数 `Valid_data_byte = (FPGA_RowS + 7) / 8`

这是后续接收行配置位图的准备阶段。

#### `0x02` 行配置位图数据

作用：

- 接收每一批行连接数据
- 将有效载荷复制到 `Rec_data[]`
- 累加 `rec_index`

这里的 `Rec_data` 可以理解为“待写入 BL 寄存器的位图缓存”。

#### `0x03` 数据传输结束

作用：

- 置位 `Dispose_read_flag = 1`

这表示：配置数据已经收全，可以开始真正的阵列写入和 ADC 读取。

#### `0x04` 复位并开始下一轮

作用：

- 执行 `Rst(0)`
- 清空本轮配置状态和缓存
- 拉低 `FPGA_IO`

它相当于一次完整事务结束后的软件复位入口。

#### `0x05` 多行多列模式配置

作用：

- 配置起始行、行数
- 设置列卡号 `FPGA_Col_Card`
- 设置本次实际读取通道数 `FPGA_Read_num`
- 调用 `Set_Mux_Reg()` 配置复用器
- 切换 GSL 模拟路径
- 对 16 组列执行 `Set_Wl_single()`
- 置位 `Rows_and_cols_flag = 1`

这说明代码支持两种工作模式：

- 单行/单列读
- 多行/多列批量读

### 4.3 `StartTask03()`：执行与回传任务

这个任务依赖 `Dispose_read_flag` 构成状态机：

- `0`：空闲
- `1`：开始执行阵列配置并采样
- `2`：采样完成，开始打包回传帧
- `3`：回传 DMA 已启动，通知 FPGA 可来取数
- `4`：等待下一轮命令或复位

#### 当 `Dispose_read_flag == 1`

任务根据 `Rows_and_cols_flag` 分成两种路径：

##### 模式 A：单行单列

流程：

1. 按批次调用 `Set_Bl_single_rows()`，把 `Rec_data` 中的位图写入 BL 寄存器
2. 延时等待模拟链路稳定
3. 调用 `ADC_Read_data(0)` 读取 1 路结果
4. 保存到 `ADC_data[]`

##### 模式 B：多行多列

流程：

1. 同样按批次调用 `Set_Bl_single_rows()`
2. 每次调用 `ADC_Read_AllChannels()` 一次性读出 16 路
3. 只取前 `FPGA_Read_num` 个结果存入 `ADC_data[]`

完成后统一把 `Dispose_read_flag` 置为 `2`。

#### 当 `Dispose_read_flag == 2`

任务开始组织回传数据帧：

- 帧头：`0x55 0xAA`
- 长度：`total_adc_points * 2`
- 功能码：`0x03`
- 数据区：所有 ADC 结果，按高字节、低字节顺序发送
- CRC：对 `[2, buff_pos-1]` 区间计算
- 帧尾：`0xAA 0x55`

随后调用：

`HAL_SPI_TransmitReceive_DMA(&hspi3, FPGA_tx_buff, FPGA_rx_buff, buff_pos);`

这表示回传长度不是固定 20 字节，而是根据采样点数动态变化。

#### 当 `Dispose_read_flag == 3`

代码将 `FPGA_IO` 拉高，通知 FPGA 当前结果帧已经准备好，可以读取。

## 5. Data Flow

这份代码的核心数据流可以总结为：

1. FPGA 发送参数帧或位图帧给 STM32
2. SPI3 DMA 回调把数据搬进 `FPGA_buff`
3. `StartTask02()` 解析命令并缓存配置数据
4. 收到结束命令后，`StartTask03()` 根据 `Rec_data` 配置阵列
5. STM32 读取 ADC，结果写入 `ADC_data`
6. STM32 将 `ADC_data` 打包进 `FPGA_tx_buff`
7. SPI3 DMA 将结果帧回传给 FPGA
8. `FPGA_IO` 拉高，作为“数据就绪”通知信号

## 6. Important Shared Variables

这些全局变量定义在 `main.c`，并通过 `main.h` 暴露给任务和驱动模块：

- `FPGA_tx_buff[1024]`：发给 FPGA 的回传缓存
- `FPGA_rx_buff[256]`：SPI DMA 的接收缓存
- `FPGA_buff[256]`：任务层解析用缓存
- `Rec_data[1024]`：从 FPGA 收到的位图配置数据
- `BL_Reg_data[256]`：BL 寄存器镜像
- `ADC_data[1024]`：采样结果缓存

状态变量：

- `FPGA_rx_flag`：收到新帧
- `Dispose_read_flag`：处理状态机
- `Rows_and_cols_flag`：单列模式 / 多列模式标志
- `rec_index`：已接收位图数据长度
- `Dispose_index`：当前处理到 `Rec_data` 的位置
- `Valid_data_byte`：当前一批行配置所需字节数
- `FPGA_Row_start` / `FPGA_RowS`：起始行与行数
- `FPGA_Col_start` / `FPGA_ColS`：起始列与列数
- `FPGA_Col_Card`：列卡编号
- `FPGA_Read_num`：多列模式下实际读取的 ADC 点数

## 7. M203 and Analog Control Logic

`M203.c` 是这份工程最“硬件相关”的部分。

### 7.1 M203 基础控制

这些函数直接通过 GPIO 模拟控制总线：

- `Data_in()`：输出 8 位数据
- `Clk()`：产生时钟变化
- `Cmd()`：切换命令/数据阶段
- `Rstn()`：控制复位脚

上层所有 `Set_*` 函数，本质上都是用这些基础动作拼出来的时序。

### 7.2 行列线配置

主要包括：

- `Set_Sl_single()`：设置某列 SL
- `Set_Bl_single()`：设置某行 BL
- `Set_Wl_single()`：设置某列 WL
- `Set_Bl_single_rows()`：按位图批量更新一段行的 BL 配置

其中 `Set_Bl_single_rows()` 非常关键，它完成了：

1. 从 `Rec_data` 中逐位取出某一行的控制值
2. 将控制值映射成 2-bit 的寄存器编码
3. 写入 `BL_Reg_data`
4. 调用 `Set_Bl_Reg()` 把寄存器镜像真正写到器件

所以它是“软件位图”到“硬件寄存器”的桥梁。

### 7.3 模拟开关路径

`GBL_select()`、`GWL_select()`、`GSL_select()` 负责选择不同模拟通路。

从代码看，设计上区分了：

- GBL
- GWL
- GSL

并且支持不同的输入/输出模式切换，例如：

- GSL 输出电压给 ADC 检测
- GSL 通过 DAC 输入电压

## 8. ADC and DAC Logic

### DAC

`DAC_update()` 通过 `SPI1` 给 3 片 DAC 下发 16-bit 控制字：

- `CS_num` 选择第几片 DAC
- `Ch` 选择该片 DAC 的通道
- `data` 为 12-bit 输出码值

启动时已经给部分通道写入默认电压，例如：

- WL 约 1.1V
- BL 约 0.2V

### ADC

ADC 采用并口读取方式，相关流程在 `main.c`：

- `ADC_Init()`：复位并配置量程、通道组和控制引脚
- `ADC_Read_data()`：读取单组通道中的 A 路结果
- `ADC_Read_AB()`：同时读出 A/B 两路
- `ADC_Read_AllChannels()`：遍历 8 个通道组，得到 16 路数据
- `Read_16BitData_Direct()`：从 `GPIOD->IDR` 直接取并口并重组位序

这里的采样实现很偏底层，强调的是严格控制：

- `CONVST`
- `BUSY`
- `CS`
- `RD`
- 通道选择引脚 `CHSEL`

## 9. FreeRTOS Role in This Project

FreeRTOS 在这个工程里主要不是为了复杂调度，而是为了把任务职责分开：

- `defaultTask`：系统心跳，方便判断程序是否仍在运行
- `myTask02`：通信解析
- `myTask03`：采样执行和回传
- `myTask04` / `myTask05`：预留扩展

当前实现没有使用队列、信号量、互斥锁，而是主要依赖全局标志位协作。这种方式简单直接，但也意味着后续扩展时要特别关注并发访问时机。

## 10. Protocol Summary

根据 `app_freertos.c` 现有实现，可以把 FPGA 与 STM32 之间的协议概括为：

### 接收帧基本格式

- 帧头：`0x55 0xAA`
- 长度：`FPGA_buff[2]`
- 功能码：`FPGA_buff[3]`
- 数据区：从 `FPGA_buff[4]` 开始
- CRC16：倒数第 4、3 字节
- 帧尾：`0xAA 0x55`

### 功能码定义

- `0x01`：配置单列模式参数
- `0x02`：发送行位图数据
- `0x03`：位图数据发送结束，开始执行采样
- `0x04`：复位状态，准备下一轮
- `0x05`：配置多行多列模式参数

### 回传帧

STM32 回传帧的功能码也使用 `0x03`，其中数据区是 ADC 采样结果，每个点占 2 字节。

## 11. How to Read This Code Efficiently

如果你后面要继续读这份工程，建议顺序如下：

1. 先读 `main.c`
   目标：弄清楚硬件启动顺序、SPI/ADC/DAC 初始化、全局变量来源。
2. 再读 `app_freertos.c`
   目标：弄清楚一帧命令从接收到执行再到回传的完整流程。
3. 最后读 `M203.c`
   目标：把“抽象命令”对应到“具体 GPIO 时序和寄存器写法”。

按照这个顺序，理解成本会低很多。

## 12. Current Characteristics and Notes

从当前代码状态看，有几个明显特点：

- 接收命令帧默认按固定 20 字节处理。
- 回传 ADC 结果帧长度是动态变化的。
- `StartTask04()`、`StartTask05()` 和 3 个软件定时器目前基本未使用。
- 任务之间主要靠全局变量和标志位配合，没有使用 RTOS 队列。
- 代码已经具备“单列读”和“多列批量读”两种模式。

如果后面要继续完善，这几个方向通常最值得优先关注：

- 给协议和状态机补充更明确的注释
- 增加异常帧处理和超时处理
- 统一 DMA 接收长度与协议格式
- 给关键全局变量建立更清晰的生命周期说明

## 13. One-Sentence Summary

这份代码的本质，是一个运行在 `STM32G474` 上的“FPGA 命令执行器”：它接收 FPGA 下发的阵列配置请求，驱动 M203/模拟开关/DAC/ADC 完成一次硬件测量，再把结果打包回传给 FPGA。
