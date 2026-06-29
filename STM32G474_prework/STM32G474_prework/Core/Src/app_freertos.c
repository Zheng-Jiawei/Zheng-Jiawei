/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "M203.h"
#include "spi.h"
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
extern uint8_t Rec_data[1024];
extern uint8_t BL_Reg_data[256];

extern uint8_t	FPGA_rx_buff[256];
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;
osThreadId myTask04Handle;
osThreadId myTask05Handle;
osTimerId myTimer01Handle;
osTimerId myTimer02Handle;
osTimerId myTimer03Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);
void StartTask05(void const * argument);
void Callback01(void const * argument);
void Callback02(void const * argument);
void Callback03(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of myTimer01 */
  osTimerDef(myTimer01, Callback01);
  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);

  /* definition and creation of myTimer02 */
  osTimerDef(myTimer02, Callback02);
  myTimer02Handle = osTimerCreate(osTimer(myTimer02), osTimerPeriodic, NULL);

  /* definition and creation of myTimer03 */
  osTimerDef(myTimer03, Callback03);
  myTimer03Handle = osTimerCreate(osTimer(myTimer03), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, StartTask02, osPriorityNormal, 0, 256);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, StartTask03, osPriorityNormal, 0, 256);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* definition and creation of myTask04 */
  osThreadDef(myTask04, StartTask04, osPriorityIdle, 0, 256);
  myTask04Handle = osThreadCreate(osThread(myTask04), NULL);

  /* definition and creation of myTask05 */
  osThreadDef(myTask05, StartTask05, osPriorityNormal, 0, 256);
  myTask05Handle = osThreadCreate(osThread(myTask05), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		HAL_GPIO_WritePin(GPIOF, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_SET);
    osDelay(500);
		HAL_GPIO_WritePin(GPIOF, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);
		osDelay(500);
    
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
		if(FPGA_rx_flag == 1)
		{
			if(FPGA_buff[0] == 0x55 && FPGA_buff[1] == 0xAA) //�ж�֡ͷ
			{
				if(FPGA_buff[18] == 0xAA && FPGA_buff[19] == 0x55) //�ж�֡β
				{
					if((FPGA_buff[16] == ((CRC_Check(FPGA_buff+2, 14) >> 8) & 0xFF) ) && (FPGA_buff[17] == (CRC_Check(FPGA_buff+2, 14) & 0xFF)))//�ж�CRC16����
					{
						if(FPGA_buff[3] == 0x01) //�жϹ����� //01��ʾ˵����ʼ�к͹�����
						{
							FPGA_Row_start = (FPGA_buff[4] << 8 | FPGA_buff[5]);
							FPGA_RowS = (FPGA_buff[6] << 8 | FPGA_buff[7]);
							
							FPGA_Col_start = (FPGA_buff[8] << 8 | FPGA_buff[9]);
							FPGA_ColS = (FPGA_buff[10] << 8 | FPGA_buff[11]);
							
							Rows_and_cols_flag = 0;
						  GSL_select(1, 1);	//ѡ��GSL1
							HAL_GPIO_WritePin(GSL1_ADC_X0_SEL_GPIO_Port, GSL1_ADC_X0_SEL_Pin, GPIO_PIN_RESET);  //��ģ�⿪�ؽӵ�X0
							HAL_GPIO_WritePin(GSL2_ADC_X1_SEL_GPIO_Port, GSL2_ADC_X1_SEL_Pin, GPIO_PIN_RESET);	//��ģ�⿪�ؽӵ�X1
							HAL_GPIO_WritePin(GSL3_ADC_X2_SEL_GPIO_Port, GSL3_ADC_X2_SEL_Pin, GPIO_PIN_RESET);	//��ģ�⿪�ؽӵ�X2
							
							Set_Sl_single(FPGA_Col_start, 2);
							Set_Wl_single(FPGA_Col_start, 1);
							
							Valid_data_byte = (FPGA_RowS + 7) / 8;
							
//							FPGA_Row_Reg_start = FPGA_Row_start / 4;
//							FPGA_Row_Reg_end = (FPGA_Row_start + FPGA_RowS - 1) / 4;
						}
						else if(FPGA_buff[3] == 0x02)  //02 ��ʾ��������1����0
						{
							if(FPGA_buff[2] == Valid_data_byte) //�жϴ������Ч�ֽ����Ƿ���ȷ
							{
								uint8_t valid_len = Valid_data_byte;
								for(uint8_t i=0; i<valid_len; i++)
								{
										Rec_data[rec_index + i] = FPGA_buff[4 + i];
								}
								rec_index += valid_len;
//								Set_Bl_single_rows(FPGA_Row_start, FPGA_RowS);
							}	
						}
						else if(FPGA_buff[3] == 0x03) 	//��ʾ���ݴ������
						{
							Dispose_read_flag = 1;
							
						}
						else if(FPGA_buff[3] == 0x04) 	//��ʾ������һ��ѭ��
						{
//							Reset_flag = 1;
								Rst(0);
				
								Dispose_read_flag = 0;
							
								FPGA_Row_start = 0;
								FPGA_RowS = 0;
								
								FPGA_Col_start = 0;
								FPGA_ColS = 0;
								
								Dispose_index = 0;
								Valid_data_byte = 0;
								
								rec_index = 0;
								
								
								FPGA_Col_Card = 0;
								FPGA_Read_num = 0;
								Rows_and_cols_flag = 0;
								
								memset(Rec_data, 0, 1024);
								memset(ADC_data, 0, sizeof(ADC_data));//2048
								memset(BL_Reg_data, 0, 256);
								memset(FPGA_tx_buff, 0, 1024);
								
								HAL_GPIO_WritePin(FPGA_IO_GPIO_Port, FPGA_IO_Pin, GPIO_PIN_RESET);
						}
						else if(FPGA_buff[3] == 0x05) 	//���ж���ģʽ
						{
							FPGA_Row_start = (FPGA_buff[4] << 8 | FPGA_buff[5]);
							FPGA_RowS = (FPGA_buff[6] << 8 | FPGA_buff[7]);
							
							FPGA_Col_Card = FPGA_buff[8];
							FPGA_Read_num = FPGA_buff[9];
							Set_Mux_Reg(FPGA_Col_Card);
							
							HAL_GPIO_WritePin(GSL1_A0_GPIO_Port, GSL1_A0_Pin, GPIO_PIN_SET);	 //��GSL1��GND
							HAL_GPIO_WritePin(GSL1_A1_GPIO_Port, GSL1_A1_Pin, GPIO_PIN_RESET); //��GSL1��GND
							
							HAL_GPIO_WritePin(GSL1_ADC_X0_SEL_GPIO_Port, GSL1_ADC_X0_SEL_Pin, GPIO_PIN_SET);  //��ģ�⿪�ؽӵ�X0
							HAL_GPIO_WritePin(GSL2_ADC_X1_SEL_GPIO_Port, GSL2_ADC_X1_SEL_Pin, GPIO_PIN_SET);	//��ģ�⿪�ؽӵ�X1
							HAL_GPIO_WritePin(GSL3_ADC_X2_SEL_GPIO_Port, GSL3_ADC_X2_SEL_Pin, GPIO_PIN_SET);	//��ģ�⿪�ؽӵ�X2
							
							Valid_data_byte = (FPGA_RowS + 7) / 8;
							
							for(uint8_t n = 0; n < 16; n++)
							{
									int FPGA_Col_start1 = FPGA_Col_Card + n * 64;
									Set_Wl_single(FPGA_Col_start1, 1);
									
							}
							
							osDelay(1);
							Rows_and_cols_flag = 1;
						}
					}
					
				}
			}
			FPGA_rx_flag = 0;
		}	
    osDelay(5);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
		if(Dispose_read_flag == 1)
		{
			if(Rows_and_cols_flag == 0)
			{
				for(uint16_t i=0; i<(rec_index / Valid_data_byte); i++)
				{
					Set_Bl_single_rows(FPGA_Row_start, FPGA_RowS);
					osDelay(10);
					ADC_data[i] = ADC_Read_data(0);
				}
				Dispose_read_flag = 2; //��ʾ�����üĴ�����ȡADC���
			}
			else if(Rows_and_cols_flag == 1) //���ж��б�־
			{
				uint16_t read_times = rec_index / Valid_data_byte;  // �����ٴ�
        uint16_t store_index = 0;                           // ADC_data ���λ������
				
				// ѭ����ȡ
        for(uint16_t i=0; i < read_times; i++)
        {
          uint16_t temp_adc[16];  // ��ʱ���һ�ζ�����16·ADC

          Set_Bl_single_rows(FPGA_Row_start, FPGA_RowS);
          osDelay(10);
          ADC_Read_AllChannels(temp_adc);  // һ�ζ���16·����ʱ����

          // �ؼ���ֻ����ǰ FPGA_Col_Card �����ݵ�����ADC_data
          for(uint8_t j=0; j < FPGA_Read_num; j++)
          {
            ADC_data[store_index++] = temp_adc[j];
          }
        }
				Dispose_read_flag = 2; //��ʾ�����üĴ�����ȡADC���
			}
			
		}
		if(Dispose_read_flag == 2)
		{
//			uint8_t buff_pos = 4; //��FPGA_buff[4]��ʼ������
//			FPGA_tx_buff[0] = 0x55;
//			FPGA_tx_buff[1] = 0xAA;
//			FPGA_tx_buff[2] = (rec_index / Valid_data_byte) * 2;
//			FPGA_tx_buff[3] = 0x03;
//			for(uint16_t i=0; i<(rec_index / Valid_data_byte); i++)
//			{
//				uint16_t adc_val = ADC_data[i];
//				
//				// ���ֽ� ���� FPGA_buff
//				FPGA_tx_buff[buff_pos++] = (adc_val >> 8) & 0xFF;

//				// ���ֽ� ���� FPGA_buff
//				FPGA_tx_buff[buff_pos++] = adc_val & 0xFF;
//			}
//			
//			FPGA_tx_buff[buff_pos++] = ((CRC_Check(FPGA_tx_buff+2, 14) >> 8) & 0xFF);
//			FPGA_tx_buff[buff_pos++] = (CRC_Check(FPGA_tx_buff+2, 14) & 0xFF);
//			FPGA_tx_buff[buff_pos++] = 0xAA;
//			FPGA_tx_buff[buff_pos++] = 0x55;
//			Dispose_read_flag = 3; //�ѽ�ADC����Ԥ���
//			
//			HAL_SPI_TransmitReceive_DMA(&hspi3,FPGA_tx_buff,FPGA_rx_buff,8+(rec_index / Valid_data_byte) * 2);
			uint8_t buff_pos = 4; //��FPGA_buff[4]��ʼ������
      uint16_t read_times = rec_index / Valid_data_byte;
      uint16_t total_adc_points;

      // ���������ݳ��ȣ����е���=ÿ��1�����ݣ����ж���=ÿ��16������
      if(Rows_and_cols_flag == 0)
      {
        total_adc_points = read_times;   // 1·/��
      }
      else
      {
        total_adc_points = read_times * FPGA_Read_num; 
      }

      // ֡ͷ֡����
      FPGA_tx_buff[0] = 0x55;
      FPGA_tx_buff[1] = 0xAA;
      FPGA_tx_buff[2] = total_adc_points * 2;  // �������ֽ�����16bit��
      FPGA_tx_buff[3] = 0x03;

      // ѭ���������ADC����
      for(uint16_t i=0; i < total_adc_points; i++)
      {
        uint16_t adc_val = ADC_data[i];
        FPGA_tx_buff[buff_pos++] = (adc_val >> 8) & 0xFF;
        FPGA_tx_buff[buff_pos++] = adc_val & 0xFF;
      }

      // CRC У�飺�� buff[2] ��ʼ������ = ��ǰλ�� - 2
      uint16_t crc_val = CRC_Check(FPGA_tx_buff + 2, buff_pos - 2);
      // д��CRC
      FPGA_tx_buff[buff_pos++] = (crc_val >> 8) & 0xFF;
      FPGA_tx_buff[buff_pos++] = crc_val & 0xFF;
      // д��֡β
      FPGA_tx_buff[buff_pos++] = 0xAA;
      FPGA_tx_buff[buff_pos++] = 0x55;

      Dispose_read_flag = 3;

      // DMA ���ͳ��� = �������õ��ܳ���
      HAL_SPI_TransmitReceive_DMA(&hspi3, FPGA_tx_buff, FPGA_rx_buff, buff_pos);
		}
		if(Dispose_read_flag == 3)
		{
			//��PF9 ����֪ͨFPGA
			HAL_GPIO_WritePin(FPGA_IO_GPIO_Port, FPGA_IO_Pin, GPIO_PIN_SET);
			Dispose_read_flag = 4;//��֪ͨFPGA��ȡ����
		}
//		if(Dispose_read_flag == 4)
//		{
//			if(Reset_flag == 1)
//			{
//				Reset_flag = 0;
//				Rst(0);
//				
//				Dispose_read_flag = 0;
//			
//				FPGA_Row_start = 0;
//				FPGA_RowS = 0;
//				
//				FPGA_Col_start = 0;
//				FPGA_ColS = 0;
//				
//				Dispose_index = 0;
//				Valid_data_byte = 0;
//				
//				rec_index = 0;
//				
//				
//				memset(Rec_data, 0, 256);
//				memset(ADC_data, 0, 256);
//				memset(BL_Reg_data, 0, 256);
//				memset(FPGA_tx_buff, 0, 256);
//				
//				HAL_GPIO_WritePin(FPGA_IO_GPIO_Port, FPGA_IO_Pin, GPIO_PIN_RESET);
//			}
//		}
    osDelay(5);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the myTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(30);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief Function implementing the myTask05 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void const * argument)
{
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(30);
  }
  /* USER CODE END StartTask05 */
}

/* Callback01 function */
void Callback01(void const * argument)
{
  /* USER CODE BEGIN Callback01 */

  /* USER CODE END Callback01 */
}

/* Callback02 function */
void Callback02(void const * argument)
{
  /* USER CODE BEGIN Callback02 */

  /* USER CODE END Callback02 */
}

/* Callback03 function */
void Callback03(void const * argument)
{
  /* USER CODE BEGIN Callback03 */

  /* USER CODE END Callback03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

