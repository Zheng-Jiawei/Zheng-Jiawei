#ifndef HRC_HOST_HAL_H
#define HRC_HOST_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  uint8_t id;
} GPIO_TypeDef;

typedef struct
{
  uint8_t unused;
} UART_HandleTypeDef;

typedef enum
{
  GPIO_PIN_RESET = 0U,
  GPIO_PIN_SET = 1U
} GPIO_PinState;

typedef enum
{
  HAL_OK = 0U,
  HAL_ERROR = 1U
} HAL_StatusTypeDef;

extern GPIO_TypeDef hrc_host_gpio_a;
extern GPIO_TypeDef hrc_host_gpio_b;
extern GPIO_TypeDef hrc_host_gpio_c;
extern GPIO_TypeDef hrc_host_gpio_d;
extern GPIO_TypeDef hrc_host_gpio_e;
extern GPIO_TypeDef hrc_host_gpio_f;
extern GPIO_TypeDef hrc_host_gpio_g;
extern UART_HandleTypeDef huart1;

#define GPIOA (&hrc_host_gpio_a)
#define GPIOB (&hrc_host_gpio_b)
#define GPIOC (&hrc_host_gpio_c)
#define GPIOD (&hrc_host_gpio_d)
#define GPIOE (&hrc_host_gpio_e)
#define GPIOF (&hrc_host_gpio_f)
#define GPIOG (&hrc_host_gpio_g)

#define GPIO_PIN_0  ((uint16_t)0x0001U)
#define GPIO_PIN_1  ((uint16_t)0x0002U)
#define GPIO_PIN_2  ((uint16_t)0x0004U)
#define GPIO_PIN_3  ((uint16_t)0x0008U)
#define GPIO_PIN_4  ((uint16_t)0x0010U)
#define GPIO_PIN_5  ((uint16_t)0x0020U)
#define GPIO_PIN_6  ((uint16_t)0x0040U)
#define GPIO_PIN_7  ((uint16_t)0x0080U)
#define GPIO_PIN_8  ((uint16_t)0x0100U)
#define GPIO_PIN_9  ((uint16_t)0x0200U)
#define GPIO_PIN_10 ((uint16_t)0x0400U)
#define GPIO_PIN_14 ((uint16_t)0x4000U)

#define HRC_DATA_IN0_Pin       GPIO_PIN_0
#define HRC_DATA_IN0_GPIO_Port GPIOE
#define HRC_DATA_IN1_Pin       GPIO_PIN_1
#define HRC_DATA_IN1_GPIO_Port GPIOE
#define HRC_DATA_IN2_Pin       GPIO_PIN_2
#define HRC_DATA_IN2_GPIO_Port GPIOE
#define HRC_DATA_IN3_Pin       GPIO_PIN_3
#define HRC_DATA_IN3_GPIO_Port GPIOE
#define HRC_DATA_IN4_Pin       GPIO_PIN_4
#define HRC_DATA_IN4_GPIO_Port GPIOE
#define HRC_DATA_IN5_Pin       GPIO_PIN_5
#define HRC_DATA_IN5_GPIO_Port GPIOE
#define HRC_DATA_IN6_Pin       GPIO_PIN_6
#define HRC_DATA_IN6_GPIO_Port GPIOE
#define HRC_DATA_IN7_Pin       GPIO_PIN_7
#define HRC_DATA_IN7_GPIO_Port GPIOE

#define HRC_RSTN_Pin           GPIO_PIN_8
#define HRC_RSTN_GPIO_Port     GPIOE
#define HRC_CMD_Pin            GPIO_PIN_9
#define HRC_CMD_GPIO_Port      GPIOE
#define HRC_CLK_Pin            GPIO_PIN_10
#define HRC_CLK_GPIO_Port      GPIOE

#define HRC_DATA_OUT0_Pin       GPIO_PIN_0
#define HRC_DATA_OUT0_GPIO_Port GPIOC
#define HRC_DATA_OUT1_Pin       GPIO_PIN_1
#define HRC_DATA_OUT1_GPIO_Port GPIOC
#define HRC_DATA_OUT2_Pin       GPIO_PIN_2
#define HRC_DATA_OUT2_GPIO_Port GPIOC
#define HRC_DATA_OUT3_Pin       GPIO_PIN_3
#define HRC_DATA_OUT3_GPIO_Port GPIOC
#define HRC_DATA_OUT4_Pin       GPIO_PIN_2
#define HRC_DATA_OUT4_GPIO_Port GPIOF
#define HRC_DATA_OUT5_Pin       GPIO_PIN_7
#define HRC_DATA_OUT5_GPIO_Port GPIOG
#define HRC_DATA_OUT6_Pin       GPIO_PIN_8
#define HRC_DATA_OUT6_GPIO_Port GPIOG
#define HRC_DATA_OUT7_Pin       GPIO_PIN_9
#define HRC_DATA_OUT7_GPIO_Port GPIOG
#define HRC_VALID_OUT_Pin       GPIO_PIN_9
#define HRC_VALID_OUT_GPIO_Port GPIOB

#define LED2_Pin                GPIO_PIN_14
#define LED2_GPIO_Port          GPIOF

void HAL_GPIO_WritePin(GPIO_TypeDef *port,
                       uint16_t pin,
                       GPIO_PinState state);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart,
                                    uint8_t *data,
                                    uint16_t len,
                                    uint32_t timeout_ms);
void delay_us(uint16_t us);
void SystemInit(void);

void HRC_Host_Init(void);
void HRC_Host_BeginPhase(const char *phase_name);
void HRC_Host_Check(uint8_t condition, const char *check_name);
uint32_t HRC_Host_GetChecks(void);
uint32_t HRC_Host_GetFailures(void);
uint8_t HRC_Host_LogContains(const char *needle);
void HRC_Host_RecordAdcResult(uint8_t raw, uint8_t code);
void HRC_Host_RecordOctdcResult(uint8_t value);
void HRC_Host_Finalize(void);

#ifdef __cplusplus
}
#endif

#endif /* HRC_HOST_HAL_H */
