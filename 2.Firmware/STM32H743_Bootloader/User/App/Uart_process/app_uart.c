/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:36
 * @LastEditTime: 2022-07-18 20:57:55
 */
#include "App_Uart.h"

/*相关句柄*/
extern TIM_HandleTypeDef htim13;

/*内部使用变量*/
#define UART_BUFFER_LEN 512 //串口接收缓存长度
#define UART_BUFFER_HALF (uint16_t)(UART_BUFFER_LEN / 2)
uint8_t Uart_Buffer[UART_BUFFER_LEN]; //串口接收缓存

 
static uint8_t Receive_Flag = 0; //接收到数据标志

static uint16_t Timeout_Max = 10; //串口接收超时最大时间   5ms * Timeout_Max = Timeout
static uint8_t Timeout_Flag = 0; //串口接收超时标志

static uint16_t RX_Size_Min = 0; //串口接收数据最小长度
static uint16_t RX_Size_Half = 0;
static uint16_t RX_Size_Max = UART_BUFFER_LEN; //串口接收数据最大长度

/*串口接收状态*/
enum User_UART_RX_State
{
  Full,      //收到数据满一帧
  Wait_Full, //收到部分数据，等待后续数据
  None,      //没有数据
} RX_State = None;

/*外部使用变量*/
void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);
void (*User_UART_RX_Finished)(uint8_t *data, uint16_t len);

/*内部使用函数*/

/**
 * @brief 串口接收 数据处理状态机 循环调用
 * @return {*}
 */
void User_UART_RX_FSM(void)
{
  uint16_t RX_Reality_Size = Bsp_UART_Get_RX_Buff_Occupy(&huart1);

  if (RX_Reality_Size > 0 && RX_Reality_Size < 256)
  {
    RX_State = Wait_Full;
    Receive_Flag = 1;
    __HAL_TIM_CLEAR_FLAG(&htim13, TIM_FLAG_UPDATE);   //重置状态
    __HAL_TIM_SetCounter(&htim13, 0); 
    User_UART_Timer_Enable(1);                    //开启定时器
  }
  else if (RX_Reality_Size >= 256)
  {
    RX_State = Full;
    Receive_Flag = 1;
    User_UART_Timer_Enable(0);                    //关闭定时器
  }
  else
  {
    RX_State = None;

    if(Receive_Flag == 1)
    {
      User_UART_Timer_Enable(1);                        //开启定时器
    }
  }



  switch (RX_State)
  {
  case Wait_Full:
 
    if (User_UART_Query_Timeout() == 1) //查询若是超时
    {
      Receive_Flag = 0;
      uint16_t size = Bsp_UART_Read(&huart1, Uart_Buffer, 512);
      User_UART_RX_Finished(Uart_Buffer, size); //则调用完成函数
    }
    
  
    break;

  case Full:

    Receive_Flag = 0;
    uint16_t size = Bsp_UART_Read(&huart1, Uart_Buffer, 512);
    User_UART_RX_Fun(Uart_Buffer, size);    //则调用数据处理函数

    break;

  case None:

    if (User_UART_Query_Timeout() == 1) //查询若是超时
    {
      uint16_t size = Bsp_UART_Read(&huart1, Uart_Buffer, 512);
      
      User_UART_RX_Finished(Uart_Buffer, size); //则调用完成函数
    }
    

 
    break;

  default:
    break;
  }
}

/**
 * @brief 定时中断服务函数
 * @return {*}
 */
void User_UART_Timer(void)
{
  static uint16_t Count = 0;

  Count++;

  if (Count >= Timeout_Max)
  {
    Count = 0;
    Timeout_Flag = 1;

    HAL_TIM_Base_Stop_IT(&htim13);
    __HAL_TIM_SetCounter(&htim13, 0);
  }
}

/**
 * @brief 定时器使能
 * @param {uint8_t} Enable
 * @return {*}
 */
void User_UART_Timer_Enable(uint8_t Enable)
{
  static uint8_t Enable_Last = 0;

  if (Enable == Enable_Last)
  {
    return;
  }

  if (Enable)
  {
    __HAL_TIM_CLEAR_FLAG(&htim13, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&htim13);
  }
  else
  {
    HAL_TIM_Base_Stop_IT(&htim13);
    __HAL_TIM_SetCounter(&htim13, 0);
  }

  Enable_Last = Enable;
}

/**
 * @brief 循环查询是否超时
 * @return {uint8_t} 1:已超时     0:未超时
 */
uint8_t User_UART_Query_Timeout(void)
{
  if (Timeout_Flag == 1)
  {
    Timeout_Flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
 * @brief 设置从串口读取数据长度最大值
 * @param {uint16_t} Size
 * @return {*}
 */
// void User_UART_RX_Size_Max(uint16_t Size)
//{
//   if(Size < UART_BUFFER_LEN)
//   {
//     RX_Size_Max = Size;
//   }
//   else
//   {
//     RX_Size_Max = UART_BUFFER_LEN;
//   }
// }

/**
 * @brief 设置从串口读取数据长度最小值
 * @param {uint16_t} Size
 * @return {*}
 */
void User_UART_RX_Size_Min(uint16_t Size)
{
  if (Size < UART_BUFFER_LEN)
  {
    RX_Size_Min = Size;
  }
  else
  {
    RX_Size_Min = UART_BUFFER_LEN;
  }
}

/**
 * @brief 设置串口超时最大时间 默认5ms
 * @param {uint16_t} Timeout  最大超时时间    5ms * Timeout = Timeout_Max
 * @return {*}
 */
void User_Uart_RX_Timeout_Set(uint16_t Timeout)
{
  Timeout_Max = Timeout;
}

/**
 * @brief 串口回显 测试用
 * @param {uint8_t} *data
 * @param {uint16_t} len
 * @return {*}
 */
void User_UART_Echo(uint8_t *data, uint16_t len)
{
  Bsp_UART_Write(&huart1, data, len);
  //Bsp_UART_Poll_DMA_TX(&huart1);
}

/**
 * @brief 串口接收完成 测试用
 * @return {*}
 */
void User_UART_Finished_Demo(uint8_t *data, uint16_t len)
{
  //Bsp_UART_Write(&huart1, "Uart RX is finished!\r\n", 21);
  Bsp_UART_Poll_DMA_TX(&huart1);
}
