/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:36
 * @LastEditTime: 2022-07-04 19:59:00
 */
#include "App_Uart_process.h"

extern CRC_HandleTypeDef hcrc;



//uint8_t Uart_RX_Loop()
//{
//  /*单次数据量*/
//  uint16_t RX_Buff_MAX = 512;

//  /*获得当前RX 缓存数*/
//  uint16_t Buff_Occupy = User_UART_Get_RX_Buff_Occupy(&huart1);

// 
//  if (Buff_Occupy < RX_Buff_MAX && Buff_Occupy > 0) //RX数据小于单次数据量
//  {
//    //串口超时定时器开启
//    if (UART_RX_Time_Out_Flag == 0)
//    {
//     HAL_TIM_Base_Start_IT(&htim13);
//    }
//    //定时器超时,则数据接收结束
//    if (UART_RX_Time_Out_Flag == 1)
//    {
//      UART_RX_Time_Out_Flag = 0;

//      uint8_t Uart_Data[512];
//      uint16_t size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));
//      /*
//      * 业务处理
//      */
//    }
//  }
//  else if (Buff_Occupy >= RX_Buff_MAX)      //RX数据大于等于单次数据量
//  {
//    uint8_t Uart_Data[512];
//    uint16_t size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));
//    
//    if (UART_RX_Time_Out_Flag == 1)
//    {
//      UART_RX_Time_Out_Flag = 0;
//      HAL_TIM_Base_Stop(&htim13);
//    }

//    /*
//    * 业务处理
//    */

//  }

//  return Flash_Error;
//}



void Demo2(void)
{
  /*获得当前RX 缓存数*/
  uint16_t Buff_Occupy = User_UART_Get_RX_Buff_Occupy(&huart1);

  uint8_t Uart_Data[512];

  /*单次最少接收8个字节*/
  if(Buff_Occupy >= 8 )
  {
    uint16_t size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));

    uint8_t data_len = Uart_Data[1];

    if(App_Uart_RX_CRC(Uart_Data ,data_len) == 1)
    {
      /*CRC校验错误业务*/
      return;
    }

    App_Uart_CMD_Process(Uart_Data ,data_len);

  }

  
}


/**
 * @brief 数据CRC校验
 * @param {uint8_t} *Data   原始数据
 * @param {uint8_t} Len     数据长度
 * @return {uint8_t} 0:正确   1:错误
 */
uint8_t App_Uart_RX_CRC(uint8_t *Data , uint8_t Len)
{
  /*末尾不为0*/
  if((Data[Len - 1 - 2] != 0X0D) || (Data[Len - 1 - 1] != 0X0A))
  {
    return 1;
  }

  uint16_t Ret = 0;

  Ret = HAL_CRC_Calculate(&hcrc , (uint32_t *)Data , Len - 4);    //删去OD OA 和 附带的CRC校验

  uint16_t Temp = Data[Len - 1 - 2 - 1];
  Temp = Temp << 8;
  Temp |= Data[Len - 1 - 2];

  if(Ret == Temp)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}



/**
 * @brief 串口命令解析
 * @param {uint8_t} *Data
 * @param {uint8_t} Len
 * @return {*}
 */
void App_Uart_CMD_Process(uint8_t *Data , uint8_t Len)
{
 switch (Data[0])
 {
 case REST_CMD:
  
  break;
 
 default:
  break;
 }

}