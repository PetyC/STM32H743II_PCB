/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:36
 * @LastEditTime: 2022-07-15 19:45:23
 */
#include "App_Uart.h"

/*相关句柄*/
extern CRC_HandleTypeDef hcrc;
extern TIM_HandleTypeDef htim13;

/*内部使用变量*/
#define UART_BUFFER_LEN 256
#define UART_BUFFER_HALF (uint16_t)(UART_BUFFER_LEN / 2)
uint8_t Uart_Buffer[UART_BUFFER_LEN];


/*内部使用函数*/



/*外部使用变量*/
void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);
void (*User_UART_RX_Finished)(void);

/**
 * @brief 串口回显
 * @param {uint8_t} *data
 * @param {uint16_t} len
 * @return {*}
 */
void User_UART_Echo(uint8_t *data, uint16_t len)
{
  Bsp_UART_Write(&huart1, data, len);
  Bsp_UART_Poll_DMA_TX(&huart1);
}

 

/**
 * @brief 串口超时时间选择
 * @param {uint8_t} sw 0:5ms  1:3S
 * @return {*}
 */
void User_UART_Time_Out_Sw(uint8_t sw)
{
  switch (sw)
  {
  case 0: // 5MS超时
    TIM13->ARR = 5000 - 1;
    TIM13->PSC = 240 - 1;
    break;

  case 1: // 3S超时
    TIM13->ARR = 60000 - 1;
    TIM13->PSC = 12000 - 1;
    break;

  case 2:  //5S超时
    TIM13->ARR = 60000 - 1;
    TIM13->PSC = 20000 - 1;
    break;

  default:
    TIM13->ARR = 5000 - 1;
    TIM13->PSC = 240 - 1;
    break;
  }
}
