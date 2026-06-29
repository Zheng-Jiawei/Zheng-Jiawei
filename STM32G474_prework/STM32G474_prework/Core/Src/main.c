/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "M203.h"
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint8_t FPGA_tx_buff[1024] = {0x00};
uint8_t	FPGA_rx_buff[256] = {0x00};

uint8_t FPGA_buff[256] = {0x00};
uint16_t ADC_data[1024] = {0x0000};

uint8_t FPGA_rx_flag = 0;

uint16_t FPGA_Row_start = 0;
uint16_t FPGA_RowS = 0;

uint16_t FPGA_Col_start = 0;
uint16_t FPGA_ColS = 0;

uint8_t FPGA_Col_Card = 0;
uint8_t Rows_and_cols_flag = 0;

uint8_t FPGA_Row_Reg_start = 0;
uint8_t FPGA_Row_Reg_end = 0;

uint8_t Valid_data_byte = 0;
uint8_t FPGA_Read_num = 0;

uint16_t rec_index = 0;
uint16_t Dispose_index = 0;

uint8_t Dispose_read_flag = 0;

uint8_t Reset_flag = 0;
//uint8_t FPGA_tx_buff1[4] = {0x55, 0xAA, 0x11, 0x12};
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int16_t ADC_data1 = 0;
int16_t ADC_data2 = 0;

int16_t ADC_data_A0 = 0;
int16_t ADC_data_A1 = 0;
int16_t ADC_data_A2 = 0;
int16_t ADC_data_A3 = 0;
int16_t ADC_data_A4 = 0;
int16_t ADC_data_A5 = 0;
int16_t ADC_data_A6 = 0;
int16_t ADC_data_A7 = 0;

int16_t ADC_data_B0 = 0;
int16_t ADC_data_B1 = 0;
int16_t ADC_data_B2 = 0;
int16_t ADC_data_B3 = 0;
int16_t ADC_data_B4 = 0;
int16_t ADC_data_B5 = 0;
int16_t ADC_data_B6 = 0;
int16_t ADC_data_B7 = 0;

void ADC_Init();
void DAC_update(uint8_t CS_num, uint8_t Ch, uint16_t data);
void delay_us(uint16_t nus);
int16_t Read_16BitData_Direct(void);


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI3_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(5);
	HAL_SPI_TransmitReceive_DMA(&hspi3,FPGA_tx_buff,FPGA_rx_buff,20);
//HAL_SPI_Receive_DMA(&hspi3,FPGA_rx_buff,4);

//	FPGA_tx_buff[0] = 0x55;
//	FPGA_tx_buff[1] = 0xAA;
//	FPGA_tx_buff[2] = 0xA1;
//	FPGA_tx_buff[3] = 0xA2;
//	FPGA_tx_buff[4] = 0xA3;
	
//	HAL_SPI_TransmitReceive_DMA(&hspi3,FPGA_tx_buff,FPGA_rx_buff,20);
//	HAL_SPI_Transmit_DMA(&hspi3, FPGA_tx_buff, 4);
//	HAL_SPI_Receive_DMA(&hspi3, FPGA_rx_buff, 4);

//	HAL_GPIO_WritePin(GPIOE, DAC_CS1_Pin, GPIO_PIN_RESET);
//	DAC_update(2,4095);

//	ADC_Init();
//	HAL_Delay(1);
//  ADC_Read_data(0);

//		HAL_GPIO_WritePin(M203_RSTN_GPIO_Port, M203_RSTN_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(EN_1_1V_GPIO_Port, EN_1_1V_Pin, GPIO_PIN_SET);
		
//		HAL_GPIO_WritePin(EN_2_5V_GPIO_Port, EN_2_5V_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M203_RSTN_GPIO_Port, M203_RSTN_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M203_CMD_GPIO_Port, M203_CMD_Pin, GPIO_PIN_SET);
//		HAL_GPIO_WritePin(M203_CLK_GPIO_Port, M203_CLK_Pin, GPIO_PIN_SET);
//		Data_in(0xFF);

		HAL_Delay(2000);
		M203_Init();
		GBL_GWL_GSL_Init();
		HAL_Delay(5);
		GBL_select(1); 		//ѡ��GBL1
		GWL_select(2); 		//ѡ��GWL2
//		GSL_select(1, 1);	//ѡ��GSL1
		HAL_Delay(5);
//		Set_Bl_single(504, 2);
//		Set_Sl_single(505, 2);
//		Set_Wl_single(505, 1);
		DAC_update(1, 1, 901); //�״����ò���Ч
		DAC_update(1, 1, 901); //����WLΪ1.1V GWL1��GWL2��GBL1��GBL2�ڵ�һ��DAC�� 00 01 02 03���δ��� GWL1��GWL2��GBL1��GBL2
		DAC_update(1, 2, 164); //����BLΪ0.2V GWL1��GWL2��GBL1��GBL2��һ��DAC�� 00 01 02 03���δ��� GWL1��GWL2��GBL1��GBL2
		//GBL3��GSL1��GSL2��GSL3���ڵڶ���DAC�� 00 01 02 03���δ��� GBL3��GSL1��GSL2��GSL3
		
		ADC_Init();
		
//		HAL_Delay(50);
//		while(1)
//		{
//			ADC_Read_data(0);
//			HAL_Delay(1000);
//		}
		
//		HAL_Delay(5);
//		ADC_Read_data(0);
		
	
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	if(hspi->Instance == SPI3)
//	{
//		HAL_SPI_TransmitReceive_DMA(&hspi3,FPGA_tx_buff,FPGA_rx_buff,4);
//	}
//}

void DAC_update(uint8_t CS_num, uint8_t Ch, uint16_t data) //CS_num��ʾƬѡ�ڼ���DAC��Ch��ʾ��Ҫ��һ��ͨ����data�ǵ�ѹ���� 
{
	if(CS_num == 1)
	{

		uint8_t Tx_buf[2] = {0x00, 0x00};
		uint16_t cmd = 0x8000 | (Ch << 12) | (data & 0x0FFF);
		Tx_buf[0] = (uint8_t)(cmd >> 8);
		Tx_buf[1] = (uint8_t)(cmd & 0xFF);

		HAL_GPIO_WritePin(GPIOE, DAC_CS1_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, Tx_buf, 2, 0xFFF);
		HAL_GPIO_WritePin(GPIOE, DAC_CS1_Pin, GPIO_PIN_SET);
	}
	else if(CS_num == 2)
	{

		uint8_t Tx_buf[2] = {0x00, 0x00};
		uint16_t cmd = 0x8000 | (Ch << 12) | (data & 0x0FFF);
		Tx_buf[0] = (uint8_t)(cmd >> 8);
		Tx_buf[1] = (uint8_t)(cmd & 0xFF);

		HAL_GPIO_WritePin(GPIOE, DAC_CS2_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, Tx_buf, 2, 0xFFF);
		HAL_GPIO_WritePin(GPIOE, DAC_CS2_Pin, GPIO_PIN_SET);
	}
	else if(CS_num == 3)
	{

		uint8_t Tx_buf[2] = {0x00, 0x00};
		uint16_t cmd = 0x8000 | (Ch << 12) | (data & 0x0FFF);
		Tx_buf[0] = (uint8_t)(cmd >> 8);
		Tx_buf[1] = (uint8_t)(cmd & 0xFF);

		HAL_GPIO_WritePin(GPIOE, DAC_CS3_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, Tx_buf, 2, 0xFFF);
		HAL_GPIO_WritePin(GPIOE, DAC_CS3_Pin, GPIO_PIN_SET);
	}

}

void ADC_Init()
{
	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);  // ��λ ��1.25us����ʱ1ms�㹻

	HAL_GPIO_WritePin(GPIOC, HW_RNGSEL0_Pin|HW_RNGSEL1_Pin, GPIO_PIN_SET);	//Ӳ��ģʽ+-10V
	HAL_GPIO_WritePin(SEQEN_GPIO_Port, SEQEN_Pin, GPIO_PIN_RESET); 					//�������
	HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);        //ת����ʼ�ӿ�����
	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);          //CSƬѡ����
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);          				//RD����
	
	HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, GPIO_PIN_RESET);  //0-7ͨ���ɼ�
	HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, GPIO_PIN_RESET);  //0-7ͨ���ɼ�
	HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, GPIO_PIN_RESET); 	//0-7ͨ���ɼ�
	
	HAL_GPIO_WritePin(ADC_RESET_GPIO_Port, ADC_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(4);  // ��λ��ȴ� ��3.7ms �ȶ�
}

void ADC_SwitchChannel(uint8_t channel)
{
    // ����ͨ����Χ 0~7
    if(channel > 7) channel = 0;

    // ����ͨ�������� CHSEL2 / CHSEL1 / CHSEL0
    // �����ƣ�CH2 CH1 CH0 �� ��Ӧ 000(0) ~ 111(7)

   // CHSEL2
    HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, (channel & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // CHSEL1
    HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, (channel & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // CHSEL0
    HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, (channel & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

//    // �����һС����ʱ��ȷ��ͨ����ƽ�ȶ���CM2249 Ҫ��
//    HAL_Delay(1);
}

int16_t ADC_Read_data(uint8_t Ch)
{
	HAL_GPIO_WritePin(ADC_CHSEL0_GPIO_Port, ADC_CHSEL0_Pin, (Ch & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL1_GPIO_Port, ADC_CHSEL1_Pin, (Ch & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ADC_CHSEL2_GPIO_Port, ADC_CHSEL2_Pin, (Ch & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	
	delay_us(1); // ͨ������ʱ�� ��50ns
	
		// 2. ���ؼ����� dummy ת������BUSY�½���������ͨ��
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

    // 3. �ȴ� BUSY �½��� �� ��ʱоƬ������������ͨ����
    while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);
	
	// ===================== 2. ����ת�� CONVST ������ =====================
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
    delay_us(1);   // ��50ns ����
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);


    // ===================== 3. �ȴ�ת����� BUSY ��� =====================
    while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);


    // ===================== 4. ��ʼ�����ݣ�CS ���� =====================
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);


    // ===================== 5. ��ȡ A ͨ������һ�� RD ���壩 =====================
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
    delay_us(1);  // �����ȶ�ʱ�� ��30ns
    ADC_data1 = Read_16BitData_Direct();  // ��Aͨ��
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
    delay_us(1);


    // ===================== 6. ��ȡ B ͨ�����ڶ��� RD ���壩 =====================
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
    delay_us(1);
    ADC_data2 = Read_16BitData_Direct();  // ��Bͨ��
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);


    // ===================== 7. ������ȡ =====================
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

    return ADC_data1;  // ����Aͨ��
	
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);        //CSƬѡ����ѡ��
//	HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);	//ת����ʼ�ӿ�����
//	delay_us(10);
//	HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);				//ת����ʼ�ӿ�����
//							
//	while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);      //�ȴ�BUSY��������

//	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);        //RD��һ�����Ͷ�ȡAͨ��
//	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);          //RD����
//	ADC_data1 = Read_16BitData_Direct();	//Aͨ��ADCֵ
//	
//	delay_us(10);
//	
//	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);        //RD�ڶ������Ͷ�ȡBͨ��
//	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);          //RD����
//	ADC_data2 = Read_16BitData_Direct();														//Bͨ��ADCֵ
//	HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);  //CSƬѡ����
//	
//	return ADC_data1;
}

void ADC_Read_AB(uint8_t Ch, int16_t *pA, int16_t *pB)
{
	  // �л�ͨ��
    ADC_SwitchChannel(Ch);
		delay_us(1);
		
		// 2. ���ؼ����� dummy ת������BUSY�½���������ͨ��
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

    // 3. �ȴ� BUSY �½��� �� ��ʱоƬ������������ͨ����
    while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

		// ===================== 2. ����ת�� CONVST ������ =====================
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
    delay_us(1);   // ��50ns ����
    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);


    // ===================== 3. �ȴ�ת����� BUSY ��� =====================
    while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);


    // ===================== 4. ��ʼ�����ݣ�CS ���� =====================
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);


    // ===================== 5. ��ȡ A ͨ������һ�� RD ���壩 =====================
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
    delay_us(1);  // �����ȶ�ʱ�� ��30ns
    *pA = Read_16BitData_Direct();  // ��Aͨ��
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
    delay_us(1);


    // ===================== 6. ��ȡ B ͨ�����ڶ��� RD ���壩 =====================
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
    delay_us(1);
    *pB = Read_16BitData_Direct();  // ��Bͨ��
    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);


    // ===================== 7. ������ȡ =====================
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

//    // �л�ͨ��
//    ADC_SwitchChannel(Ch);
//		delay_us(1);

//    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);

//    // �ȴ� BUSY ���ͣ���ԭ��д���� CONVST���Ұ�������������
//    while(HAL_GPIO_ReadPin(BUSY_GPIO_Port, BUSY_Pin) == GPIO_PIN_SET);

//    // �� A ͨ��
//    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
//    *pA = Read_16BitData_Direct();

//    delay_us(2);

//    // �� B ͨ��
//    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
//    *pB = Read_16BitData_Direct();

//    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
}

void ADC_Read_AllChannels(uint16_t *adc_result)
{
    int16_t A, B;
    uint8_t ch;

    for(ch = 0; ch < 8; ch++)
    {
        ADC_Read_AB(ch, &A, &B);

        adc_result[ch]      = A;  			 // A0~A7 �� 0~7
        adc_result[15 - ch] = B;         // B0��15, B1��14 ... B7��8
				
    }
}

int16_t Read_16BitData_Direct(void)
{
    int32_t port_value = GPIOD->IDR;  // ��ȡ�����˿�D��ֵ
    
    // ��������λ��
    int16_t data = 0;
    
    // ���ֽڣ�PD7-PD0 -> D7-D0
    data |= ((port_value >> 7) & 0x01) << 0;  // PD7 -> D0
    data |= ((port_value >> 6) & 0x01) << 1;  // PD6 -> D1
    data |= ((port_value >> 5) & 0x01) << 2;  // PD5 -> D2
    data |= ((port_value >> 4) & 0x01) << 3;  // PD4 -> D3
    data |= ((port_value >> 3) & 0x01) << 4;  // PD3 -> D4
    data |= ((port_value >> 2) & 0x01) << 5;  // PD2 -> D5
    data |= ((port_value >> 1) & 0x01) << 6;  // PD1 -> D6
    data |= ((port_value >> 0) & 0x01) << 7;  // PD0 -> D7
    
    // ���ֽڣ�PD15-PD8 -> D15-D8
    data |= ((port_value >> 15) & 0x01) << 8;   // PD15 -> D8
    data |= ((port_value >> 14) & 0x01) << 9;   // PD14 -> D9
    data |= ((port_value >> 13) & 0x01) << 10;  // PD13 -> D10
    data |= ((port_value >> 12) & 0x01) << 11;  // PD12 -> D11
    data |= ((port_value >> 11) & 0x01) << 12;  // PD11 -> D12
    data |= ((port_value >> 10) & 0x01) << 13;  // PD10 -> D13
    data |= ((port_value >> 9) & 0x01) << 14;   // PD9 -> D14
    data |= ((port_value >> 8) & 0x01) << 15;   // PD8 -> D15
    
    return data;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI3)
	{
//		if(FPGA_rx_buff[0] == 0x11)
//		{
//			HAL_SPI_Transmit_DMA(&hspi3, FPGA_tx_buff1, 4);
//		}
//		else if(FPGA_rx_buff[0] == 0x22)
//		{
//			FPGA_tx_buff1[0] = 0x66;
//			HAL_SPI_Transmit_DMA(&hspi3, FPGA_tx_buff1, 4);
//		}
//		HAL_SPI_Receive_DMA(&hspi3, FPGA_rx_buff, 4);
		 memcpy(FPGA_buff, FPGA_rx_buff, 20);
		  
//		 if(FPGA_buff[3] == 0x04)
//		 {
//			SPI3_DMA_Reset();
//		 }
		 if(FPGA_buff[3] != 0x03)
		 {
				HAL_SPI_TransmitReceive_DMA(&hspi3,FPGA_tx_buff,FPGA_rx_buff,20);
		 }
		 
		 FPGA_rx_flag = 1;
	}
}

///* �����ġ�SPI3 + DMA ������λ���������˾͵��ã����̻�ԭ */
//void SPI3_DMA_Reset(void)
//{
//    // 1. ǿ��ֹͣ DMA ���� & ����
//    HAL_SPI_DMAStop(&hspi3);

//    // 2. ʧ�� SPI ����
//    __HAL_SPI_DISABLE(&hspi3);

//    // 3. �����λ SPI3 ���裨Ӳ���Ĵ������㣩
//    __HAL_RCC_SPI3_FORCE_RESET();
//    __HAL_RCC_SPI3_RELEASE_RESET();

//    // 4. ���³�ʼ�� SPI3���ص���CubeMX���õĳ�ʼ״̬��
////    HAL_SPI_Init(&hspi3);
//		MX_SPI3_Init();

//    // 5. ��ս��ջ���������ֹ���������ݣ�
//    memset(FPGA_rx_buff, 0x00, 256);
//    memset(FPGA_buff, 0x00, 256);

//}

//void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
//{
//    if (hspi->Instance == SPI3)
//    {
//        /* �������������� (OVR) ������������������־ */
//        if (hspi->ErrorCode != HAL_SPI_ERROR_NONE)
//        {
//            /* ���Ӳ�� OVR��FRE��MODF ��־�� */
//            __HAL_SPI_CLEAR_OVRFLAG(hspi);
//            __HAL_SPI_CLEAR_FREFLAG(hspi);
//            __HAL_SPI_CLEAR_MODFFLAG(hspi);
//            
//            /* �ָ� SPI ״̬��Ϊ READY���Ա���������ٴο��� DMA ���� */
//            hspi->State = HAL_SPI_STATE_READY;
//            
////						HAL_SPI_TransmitReceive_DMA(&hspi3, FPGA_tx_buff, FPGA_rx_buff, 20); 
//						
//            /* �����Ҫ�Զ��������գ��������������µ��� DMA ���պ��������磺
//               HAL_SPI_TransmitReceive_DMA(&hspi3, FPGA_tx_buff, FPGA_rx_buff, 14); 
//               ����ֱ�ӵ�����д�õĸ�λ������
//               SPI3_DMA_Reset();
//            */
//        }
//    }
//}

void delay_us(uint16_t nus)
{    
   uint16_t i=0;  
   while(nus--)
   {
      i=30;  //�Լ�����
      while(i--) ;    
   }
}


/*
    * @name   CRC_Check
    * @brief  ����CRCУ��
    * @param  CRC_Ptr->����ָ�룬LEN->����
    * @retval CRCУ��ֵ      
*/
uint16_t CRC_Check(uint8_t *CRC_Ptr,uint8_t LEN)
{
    uint16_t CRC_Value = 0;
    uint8_t  i         = 0;
    uint8_t  j         = 0;

    CRC_Value = 0xffff;
    for(i=0;i<LEN;i++)  //LENΪ���鳤��
    {
        CRC_Value ^= *(CRC_Ptr+i);
        for(j=0;j<8;j++)
        {
            if(CRC_Value & 0x00001)
                CRC_Value = (CRC_Value >> 1) ^ 0xA001;
            else
                CRC_Value = (CRC_Value >> 1);
        }
    }
    CRC_Value = ((CRC_Value >> 8) +  (CRC_Value << 8)); //�����ߵ��ֽ�

    return CRC_Value;
		
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
