/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:36
 * @LastEditTime: 2022-07-11 19:20:03
 */
#include "App_Uart.h"
#include "Bootloader.h"

/*相关句柄*/
extern CRC_HandleTypeDef hcrc;
extern TIM_HandleTypeDef htim13;

/*内部使用变量*/
#define UART_BUFFER_LEN 512
#define UART_BUFFER_HALF (uint16_t)(UART_BUFFER_LEN / 2)
uint8_t Uart_Buffer[UART_BUFFER_LEN];
uint8_t UART_RX_Time_Out_Flag = 0;      //串口超时标志


/*内部使用函数*/
static void User_UART_RX_Process(uint8_t Finished_Flag);

/*外部使用变量*/
void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);
void (*User_UART_RX_Finished)(void);



/**
 * @brief 串口接收循环
 * @return {*}
 */
void User_UART_RX_Loop(void)
{
  /*获得当前RX 缓存数*/
  uint16_t Buff_Occupy = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

  /*RX缓存小于最大缓存数*/
  if (Buff_Occupy < UART_BUFFER_HALF && Buff_Occupy > 0)
  {
    //串口超时定时器开启
    if (UART_RX_Time_Out_Flag == 0)
    {
      HAL_TIM_Base_Start_IT(&htim13);
    }
    //定时器超时,则数据接收结束
    if (UART_RX_Time_Out_Flag == 1)
    {
      UART_RX_Time_Out_Flag = 0;
      User_UART_RX_Process(1);
    }
  }
  else if (Buff_Occupy >= UART_BUFFER_HALF)
  {

    User_UART_RX_Process(0);

    if (UART_RX_Time_Out_Flag == 1)
    {
      UART_RX_Time_Out_Flag = 0;
      HAL_TIM_Base_Stop(&htim13);
    }
  }
}

/**
 * @brief 串口数据处理，使用指针函数处理
 * @return {*}
 */
static void User_UART_RX_Process(uint8_t Finished_Flag)
{
  memset(Uart_Buffer, 0, UART_BUFFER_LEN);

  uint16_t size = Bsp_UART_Read(&huart1, Uart_Buffer, UART_BUFFER_LEN);

  if(User_UART_RX_Fun != NULL)
  {
    User_UART_RX_Fun(Uart_Buffer, size);
  }

  /*如果接收完成*/
  if(Finished_Flag == 1)
  {
    if(User_UART_RX_Finished != NULL)
    {
      User_UART_RX_Finished();
    }
  }
}



/**
 * @brief 串口回显
 * @param {uint8_t} *data
 * @param {uint16_t} len
 * @return {*}
 */
void User_UART_Echo(uint8_t *data , uint16_t len)
{
  Bsp_UART_Write(&huart1 , data , len);
  Bsp_UART_Poll_DMA_TX(&huart1);
}


