/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:36
 * @LastEditTime: 2022-07-05 02:01:19
 */
#include "App_Uart_process.h"

extern CRC_HandleTypeDef hcrc;


/**
 * @brief 串口数据处理
 * @return {*}
 */
void User_UART_Process(void)
{
  /*获得当前RX 缓存数*/
  uint16_t Buff_Occupy = User_UART_Get_RX_Buff_Occupy(&huart1);

  /*数据缓存区*/
  uint8_t Uart_Data[512];

  /*单次最少接收8个字节*/
  if(Buff_Occupy >= 8 )
  {
    uint16_t size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));

    uint8_t data_len = Uart_Data[1];

    
    if(User_UART_RX_CRC(Uart_Data ,data_len) == 1)
    {
      /*CRC校验错误业务*/
      return;
    }

    /*串口处理*/
    User_UART_CMD_Process(Uart_Data ,data_len);
    
    memset(Uart_Data , 0 , sizeof(Uart_Data));
  }
  else
  {
    /*数据错误*/
    memset(Uart_Data , 0 , sizeof(Uart_Data));
  }

  
}


/**
 * @brief 数据CRC校验
 * @param {uint8_t} *Data   原始数据
 * @param {uint8_t} Len     数据长度
 * @return {uint8_t} 0:正确   1:错误
 */
uint8_t User_UART_RX_CRC(uint8_t *Data , uint8_t Len)
{
  /*末尾不为0D 0A*/
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
void User_UART_CMD_Process(uint8_t *Data , uint8_t Len)
{
 switch (Data[0])
 {
 case REST_CMD:
  
  break;
 
 default:
  break;
 }

}