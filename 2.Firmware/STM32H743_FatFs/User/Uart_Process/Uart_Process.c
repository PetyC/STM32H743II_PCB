/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-04-14 16:11:43
 * @LastEditTime: 2022-04-15 19:15:00
 */
#include "Uart_Process.h"

Usart_RX_Data_Str RX_Data = {0};        //串口收到数据结构体
Usart_TX_Data_Str TX_Data = {0};        //串口发送数据结构体

/*发送缓存*/
#define Usart1SendBufferMemoryCopyLen 150  //提取缓存区的数据的数组大小
uint8_t  Usart1SendBufferMemoryCopy[Usart1SendBufferMemoryCopyLen];//提取缓存区的数据

/*接收缓存*/
#define Usart1ReadBufferMemoryCopyLen 150                      //提取缓存区的数据的数组大小
uint8_t Usart1ReadBufferMemoryCopy[Usart1ReadBufferMemoryCopyLen]; //提取缓存区的数据




/**
 * @brief 空闲中断初始化
 * @param {*}
 * @return {*}
 */
void User_UART_Init(void)
{

  /*创建缓存*/
  BufferManageCreate(&RX_Data.List_Manage, RX_Data.Buffer, RX_Buffer_Len, RX_Data.RX_Len_Manage , RX_Len_Manage_LEN * 4);
	BufferManageCreate(&TX_Data.List_Manage , TX_Data.Buffer , TX_Buffer_Len , TX_Data.TX_Len_Manage ,  TX_Len_Manage_LEN * 4);
  
	

  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); //开启空闲中断

  HAL_UART_Receive_DMA(&huart1, RX_Data.Single_Buffer, Single_Buffer_Len); //使能DMA接收

  __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除空闲中断标志 否者上电会进入中断
}

/**
 * @brief 用户自定义串口中断
 * @param {UART_HandleTypeDef} *huart 串口句柄
 * @return {*}
 */
void User_UART_IRQHandler(UART_HandleTypeDef *huart)
{

  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) //判断是否是串口2空闲中断
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除空闲中断标志（否则会一直不断进入中断）

    HAL_UART_DMAStop(&huart1); //停止本次DMA传输

    User_UART_IDLECallback(huart); //调用空闲中断数据处理函数
  }
}

/**
 * @brief 用户自定义串口空闲中断服务函数
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
void User_UART_IDLECallback(UART_HandleTypeDef *huart)
{

  if (huart == &huart1)
  {

    int value;

    uint32_t Usart1ReadCnt = Single_Buffer_Len - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx); // 总数据量减去未接收到的数据量为已经接收到的数据量

    if (Usart1ReadCnt == 0) //上电就进入空闲中断 (怀疑TX 需要硬件上拉)
    {
      HAL_UART_Receive_DMA(&huart1, RX_Data.Single_Buffer, Single_Buffer_Len); // 重新启动DMA接收
      return;
    }

    BufferManageWrite(&RX_Data.List_Manage, RX_Data.Single_Buffer, Usart1ReadCnt, &value); //把数据存入缓存

    HAL_UART_Receive_DMA(&huart1, RX_Data.Single_Buffer, Single_Buffer_Len); // 重新启动DMA接收
  }
}




/**
 * @brief 串口接收数据处理
 * @param {*}
 * @return {*}
 */
void User_UART_RX_Handle(void)
{

  int value;

  /*提取收到的缓存的数据*/
  BufferManageRead(&RX_Data.List_Manage, Usart1ReadBufferMemoryCopy, &RX_Data.List_Manage.ReadLen);

  if (RX_Data.List_Manage.ReadLen > 0) //有数据
  {
		/*写入发送缓存*/
		BufferManageWrite(&TX_Data.List_Manage , Usart1ReadBufferMemoryCopy , RX_Data.List_Manage.ReadLen,  &value);
  }
	
  if( __HAL_DMA_GET_COUNTER(&hdma_usart1_tx) == 0 && HAL_DMA_GetState(&hdma_usart1_tx) == HAL_DMA_STATE_READY)    //如果DMA发送完成
  {
    /*取出缓存的数据*/
		BufferManageRead(&TX_Data.List_Manage, Usart1SendBufferMemoryCopy, &TX_Data.List_Manage.SendLen);
   
    if(TX_Data.List_Manage.SendLen >0)//有数据
		{
				
			HAL_UART_Transmit_DMA(&huart1, Usart1SendBufferMemoryCopy, TX_Data.List_Manage.SendLen);
				
		}
    
  }

  
}
