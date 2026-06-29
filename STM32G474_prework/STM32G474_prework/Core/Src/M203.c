#include "M203.h"
#include <math.h>




void Data_in(uint8_t s1)
{
	// 定义引脚端口和引脚号
    GPIO_TypeDef* GPIO_PORT = GPIOE;
    
    // Bit7 -> PE7
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_7, (s1 & (1 << 7)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit6 -> PE6
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_6, (s1 & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit5 -> PE5
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_5, (s1 & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit4 -> PE4
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_4, (s1 & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit3 -> PE3
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_3, (s1 & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit2 -> PE2
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_2, (s1 & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit1 -> PE1
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_1, (s1 & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // Bit0 -> PE0
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN_0, (s1 & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	
}


void Clk(uint8_t s1)
{
    // 定义目标引脚：PE10
    GPIO_TypeDef* GPIO_PORT = GPIOE;
    uint16_t GPIO_PIN = GPIO_PIN_10;
    
    // 步骤1：将s1的bit2位输出到PE10
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, (s1 & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // 步骤2：将s1的bit1位输出到PE10
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, (s1 & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // 步骤3：将s1的bit0位输出到PE10
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, (s1 & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


void Cmd(uint8_t s1)
{
    // 定义目标引脚：PE9
    GPIO_TypeDef* GPIO_PORT = GPIOE;
    uint16_t GPIO_PIN = GPIO_PIN_9;
    
    // 提取s1的bit0位，控制PE9电平
    // (s1 & (1 << 0)) 非0表示bit0为1，设置高电平；否则设置低电平
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, (s1 & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Rstn(uint8_t s1)
{
    // 定义目标引脚：PE8
    GPIO_TypeDef* GPIO_PORT = GPIOE;
    uint16_t GPIO_PIN = GPIO_PIN_8;
    
    // 提取s1的bit0位，控制PE8电平
    // (s1 & (1 << 0)) 非0则bit0为1 → PE8置高；否则置低
    HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, (s1 & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Rst(uint8_t s1)  //命令复位命令
{
		Rstn(0x01);
		
		Cmd(0x01);
		Data_in(0x01);
		Clk(0x02);
		
		Cmd(0x00);
		Data_in(0x00);
		Clk(0x02);
}	

void M203_Init(void)
{
		HAL_GPIO_WritePin(EN_1_1V_GPIO_Port, EN_1_1V_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(EN_2_5V_GPIO_Port, EN_2_5V_Pin, GPIO_PIN_SET);
		delay_us(60000);
		Rst(0);
}  


void Set_Sl_all(uint8_t s1)
{
		Rstn(0x01);
		//接收指令
		Cmd(0x01);
		Data_in(0x02);
		Clk(0x02);
		
		//执行set
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
		
		//read接收指令
		Cmd(0x01);
		Data_in(0x08);
		Clk(0x02);
		
		//执行读取
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
}

void Set_Bl_all(uint8_t s1)
{
		Rstn(0x01);
		
		//接收指令
		Cmd(0x01);
		Data_in(0x04);
		Clk(0x02);
		
		//执行set
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
		
		//read接收指令
		Cmd(0x01);
		Data_in(0x0A);
		Clk(0x02);
		
		//执行读取
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
}

void Set_Wl_all(uint8_t s1)
{
		Rstn(0x01);
		
		//接收指令
		Cmd(0x01);
		Data_in(0x03);
		Clk(0x02);
		
		//执行set
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
		
		//read接收指令
		Cmd(0x01);
		Data_in(0x09);
		Clk(0x05);
		
		//执行读取
		Cmd(0x00);
		Data_in(s1);
		Clk(0x02);
}

void Set_Sl_single(uint16_t Col, uint8_t GSL_num)  //Col是多少列，GBL_num是连接哪个GSL（0-GS4 OUT,1-GSL2，2-GSL1，3-GSL3）
{
	uint8_t s1;
	uint8_t s2;
	
	s1 = Col/4;
	s2 = (pow(4,(Col%4)))*GSL_num;
	
	Rstn(0x01);
	
	//接收指令
	Cmd(0x01);
	Data_in(0x05);
	Clk(0x02);
	
	//接收地址
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
	
	//执行set
	Cmd(0x00);
	Data_in(s2);
	Clk(0x02);
	
	//read接收指令
	Cmd(0x01);
	Data_in(0x08);
	Clk(0x02);
	
	//执行读取
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
}


void Set_Bl_single(uint16_t Row, uint8_t GBL_num)  //Row是多少行，Col是多少列，s1是地址，s2是值, GBL_num是连接哪个GBL（0-浮空,1-GBL2，2-GBL1，3-GBL3）
{
	uint8_t s1;
	uint8_t s2;
	
	s1 = Row/4;
	s2 = (pow(4,(Row%4)))*GBL_num;
	
	Rstn(0x01);
	
	//接收指令
	Cmd(0x01);
	Data_in(0x07);
	Clk(0x02);
	
	//接收地址
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
	
	//执行set
	Cmd(0x00);
	Data_in(s2);
	Clk(0x02);
	
	//read接收指令
	Cmd(0x01);
	Data_in(0x0A);
	Clk(0x02);
	
	//执行读取
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
}

void Set_Wl_single(uint16_t Col, uint8_t GWL_num)  //Col是多少列, s1是地址，s2是值，GWL_num是连接哪个GWL（0-GWL1,1-GWL2）
{
	uint8_t s1;
	uint8_t s2;
	
	s1 = Col/8;
	s2 = (pow(2,(Col%8)))*GWL_num;
	
	Rstn(0x01);
	
	//接收指令
	Cmd(0x01);
	Data_in(0x06);
	Clk(0x02);
	
	//接收地址
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
	
	//执行set
	Cmd(0x00);
	Data_in(s2);
	Clk(0x02);
	
	//read接收指令
	Cmd(0x01);
	Data_in(0x09);
	Clk(0x02);
	
	//执行读取
	Cmd(0x00);
	Data_in(s1);
	Clk(0x02);
}


void GBL_GWL_GSL_Init(void)
{
	HAL_GPIO_WritePin(GBL3_EN_GPIO_Port, GBL3_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GBL2_EN_GPIO_Port, GBL2_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GBL1_EN_GPIO_Port, GBL1_EN_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GWL2_EN_GPIO_Port, GWL2_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GWL1_EN_GPIO_Port, GWL1_EN_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GSL3_EN_GPIO_Port, GSL3_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GSL2_EN_GPIO_Port, GSL2_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GSL1_EN_GPIO_Port, GSL1_EN_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GWL1_A0_GPIO_Port, GWL1_A0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GWL2_A0_GPIO_Port, GWL2_A0_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GBL1_A0_GPIO_Port, GBL1_A0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GBL2_A0_GPIO_Port, GBL2_A0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GBL3_A0_GPIO_Port, GBL3_A0_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GSL1_A0_GPIO_Port, GSL1_A0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GSL2_A0_GPIO_Port, GSL2_A0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GSL3_A0_GPIO_Port, GSL3_A0_Pin, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GWL1_A1_GPIO_Port, GWL1_A1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GWL2_A1_GPIO_Port, GWL2_A1_Pin, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GBL1_A1_GPIO_Port, GBL1_A1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GBL2_A1_GPIO_Port, GBL2_A1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GBL3_A1_GPIO_Port, GBL3_A1_Pin, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GSL1_A1_GPIO_Port, GSL1_A1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GSL2_A1_GPIO_Port, GSL2_A1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GSL3_A1_GPIO_Port, GSL3_A1_Pin, GPIO_PIN_RESET);
}

void GBL_select(uint8_t num) //num为选择哪个GBL 1-GBL1，2-GBL2，3-GBL3
{
	if(num == 1)
	{
		HAL_GPIO_WritePin(GBL1_A0_GPIO_Port, GBL1_A0_Pin, GPIO_PIN_RESET);
	}
	else if(num == 2)
	{
		HAL_GPIO_WritePin(GBL2_A0_GPIO_Port, GBL2_A0_Pin, GPIO_PIN_RESET);
	}
	else if(num == 3)
	{
		HAL_GPIO_WritePin(GBL3_A0_GPIO_Port, GBL3_A0_Pin, GPIO_PIN_RESET);
	}
	
}


void GWL_select(uint8_t num) //num为选择哪个GWL 1-GWL1，2-GWL2
{
	if(num == 1)
	{
		HAL_GPIO_WritePin(GWL1_A0_GPIO_Port, GWL1_A0_Pin, GPIO_PIN_RESET);
	}
	else if(num == 2)
	{
		HAL_GPIO_WritePin(GWL2_A0_GPIO_Port, GWL2_A0_Pin, GPIO_PIN_RESET);
	}
	
}


void GSL_select(uint8_t num, uint8_t model) //1模式GSL输出电压模式ADC检测，2模式GSL通过DAC输入电压
{
		if(model == 1) //GSL输出电压模式
		{
			if(num == 1)
			{
				HAL_GPIO_WritePin(GSL1_A1_GPIO_Port, GSL1_A1_Pin, GPIO_PIN_SET);
			}
			else if(num == 2)
			{
				HAL_GPIO_WritePin(GSL2_A1_GPIO_Port, GSL2_A1_Pin, GPIO_PIN_SET);
			}
			else if(num == 3)
			{
				HAL_GPIO_WritePin(GSL3_A1_GPIO_Port, GSL3_A1_Pin, GPIO_PIN_SET);
			}			
		}
		else if(model == 2)//GSL通过DAC输入电压
		{
			if(num == 1)
			{
				HAL_GPIO_WritePin(GSL1_A0_GPIO_Port, GSL1_A0_Pin, GPIO_PIN_RESET);
			}
			else if(num == 2)
			{
				HAL_GPIO_WritePin(GSL2_A0_GPIO_Port, GSL2_A0_Pin, GPIO_PIN_RESET);
			}
			else if(num == 3)
			{
				HAL_GPIO_WritePin(GSL3_A0_GPIO_Port, GSL3_A0_Pin, GPIO_PIN_RESET);
			}
		}
}


uint8_t Rec_data[1024] = {0};
uint8_t BL_Reg_data[256] = {0};


void Set_Bl_single_rows(uint16_t Row_start, uint16_t Row) //Row_start起始行, Row共多少行
{
	 int row = Row_start;    // 起始行
   int row_s = Row;  			 // 总行数
	 
	 for (int i = 0; i < row_s; i++) {
				// 1. 计算当前行号
				int current_row = row + i;
				
				// 2. 提取当前行对应的控制位（bit0→初始行，bit1→初始行+1…）
				uint8_t ctrl_bit;
				if (i >= 0 && i < 64) 
				{
					// 计算i对应的字节索引（0~7）和该字节内的位索引（0~7）
					int byte_idx = i / 8;    // 等价于 i >> 3（位运算更高效）
					int bit_idx = i % 8;     // 等价于 i & 0x07（位运算更高效）
					
					// 提取Rec_data[byte_idx]的第bit_idx位（bit0为最低位）
					ctrl_bit = (Rec_data[Dispose_index+byte_idx] >> bit_idx) & 0x01;
		    }
				

				// 3. 根据控制位确定该行的两位配置值
				uint8_t row_bits;
				if (ctrl_bit == 0) {
						row_bits = 0x01; // 0→配置为01（高位0，低位1）
				} else {
						row_bits = 0x02; // 1→配置为10（高位1，低位0）
				}

				// 4. 计算当前行对应的寄存器索引和位偏移（每行占2位）
				int reg_idx = current_row / 4;          // 行1→0，行5→1，行9→2
				int bit_offset = (current_row % 4) * 2; // 行1→2(bit2/3)，行9→2(bit2/3)

				// 5. 关键操作：先清除目标位，再写入配置值（避免影响其他位）
				BL_Reg_data[reg_idx] &= ~(0x03 << bit_offset); // 0x03=00000011，清除两位
				BL_Reg_data[reg_idx] |= (row_bits << bit_offset); // 写入配置值


    }
		
		int Reg_start = row / 4;
    int Reg_end = (row + row_s - 1) / 4;
    for (int i = Reg_start; i <= Reg_end; i++) {
       Set_Bl_Reg(i, BL_Reg_data[i]);
    }
		Dispose_index+=Valid_data_byte;
}


void Set_Bl_Reg(uint8_t Reg_num, uint8_t single)
{
	Rstn(0x01);
	
	//接收指令
	Cmd(0x01);
	Data_in(0x07);
	Clk(0x02);
	
	//接收地址
	Cmd(0x00);
	Data_in(Reg_num);
	Clk(0x02);
	
	//执行set
	Cmd(0x00);
	Data_in(single);
	Clk(0x02);
	
}

void Set_Mux_Reg(uint8_t Mux_num)
{
	Rstn(0x01);
	
	//接收指令
	Cmd(0x01);
	Data_in(0x11);
	Clk(0x02);
	
	//接收地址
	Cmd(0x00);
	Data_in(Mux_num);
	Clk(0x02);
}
