
#include "Uart_Process.h"

rb_t Uart_RX_Manage; //环形队列管理变量
UART_RX_Str Uart_RX; //串口缓存结构体


//Buff_Manage_Str buff_manage_struct_test;   //定义缓存管理变量


void USER_UART_Loop_List_Init()
{

}








/**
 * @msg: 用户自定义串口空闲中断函数
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void USER_UART_IRQHandler(UART_HandleTypeDef *huart)
{

  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) //判断是否是串口2空闲中断
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除空闲中断标志（否则会一直不断进入中断）

    HAL_UART_DMAStop(&huart1);          //停止本次DMA传输

    USER_UART_IDLECallback(huart); //调用中断处理函数
  }
}

/**
 * @msg: 用户自定义串口空闲中断服务函数
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void USER_UART_IDLECallback(UART_HandleTypeDef *huart)
{

  if (huart == &huart1)
  {
    
    if (Uart_RX.BUF_LEN == 0) //防止刚上电就进空闲中断
    {
      Uart_RX.Finish_Flag = 0;                             // 标记接收结束
      HAL_UART_Receive_DMA(&huart1, &Uart_RX.Once_Read, 1); // 重新启动DMA接收
      return;
    }

    Uart_RX.Finish_Flag = 1; // 标记接收结束

    HAL_UART_Receive_DMA(&huart1, &Uart_RX.Once_Read, 1); // 重新启动DMA接收
  }
}

/**
 * @msg: 串口接收数据处理
 * @param {*}
 * @return {*}
 */
void USER_UART_RX_Handle(void)
{

  if (Uart_RX.Finish_Flag != 1) //判断是否有新数据
  {
    return;
  }

  Uart_RX.Finish_Flag = 0; //复位标志



/*先原样发送回*/
  uint8_t Data;

  while(rbCanRead(&Uart_RX_Manage) > 0)
  {
    rbRead(&Uart_RX_Manage , &Data , 1);

    HAL_UART_Transmit(&huart1 , &Data , 1 , 0X500);
		
		
		
		Uart_RX.BUF_LEN--;
  }

}

/**
 * @msg: 串口接收完成中断函数
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    PutData(&Uart_RX_Manage, &Uart_RX.Once_Read, 1); //存入队列

    Uart_RX.BUF_LEN++;

    HAL_UART_Receive_DMA(&huart1, &Uart_RX.Once_Read, 1); // 重新启动DMA接收
  }
}

/**
 * @msg: 串口接收错误处理
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  uint8_t TxError[8] = {0XFF, 0x00, 0XFF, 0XFF, 0XFF, 0XFF, 0X0D, 0X0A}; //接收错误返回

  if (huart->Instance == USART1)
  {
    /* 解决错误接受数据后无法发送的问题 */
    switch (huart->ErrorCode)
    {
    case HAL_UART_ERROR_NONE:
      break;
    case HAL_UART_ERROR_PE: //奇偶错误

      TxError[1] = ERROR_PE;

      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);

      break;

    case HAL_UART_ERROR_NE: //噪声错误

      TxError[1] = ERROR_NE;

      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);

      break;

    case HAL_UART_ERROR_FE: //帧错误

      TxError[1] = ERROR_FE;

      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);

      break;

    case HAL_UART_ERROR_ORE: //溢出错误

      TxError[1] = ERROR_ORE;

      __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);
      break;

    default:
      TxError[1] = 0XFF;

      break;
    }

    __HAL_UNLOCK(huart); //执行HAL_UART_ErrorCallback时，还处于lock，需先unlock

    HAL_UART_Transmit(&huart1, (uint8_t *)TxError, sizeof(TxError), 0xff); //发送错误数据
  }
}
