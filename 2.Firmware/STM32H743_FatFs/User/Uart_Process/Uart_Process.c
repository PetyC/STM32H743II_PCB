
#include "Uart_Process.h"

//rb_t Uart_RX_Manage; //环形队列管理变量
//UART_RX_Str Uart_RX; //串口缓存结构体


Buff_Manage_Str Uart_RX_Manage;   //定义缓存管理变量

#define Usart1ReadBuffLen 150  //串口1接收数据数组大小
unsigned char Usart1ReadBuff[Usart1ReadBuffLen] = {0};                      //接收数据缓存


uint32_t  Usart1ReadCnt = 0;                                                //串口接收到的数据个数

#define Usart1BufferMemoryLen 1024     //串口接收缓存区大小
uint8_t   Usart1BufferMemory[Usart1BufferMemoryLen];                        //接收数据缓存区


#define Usart1BufferMemoryManageLen 20 //串口接收缓存区管理
uint32_t  Usart1BufferMemoryManage[Usart1BufferMemoryManageLen];            //管理缓存区


#define Usart1BufferMemoryCopyLen 150  //提取缓存区的数据的数组大小
uint8_t   Usart1BufferMemoryCopy[Usart1BufferMemoryCopyLen];                //提取缓存区的数据


void USER_UART_Loop_List_Init(void)
{
	
	__HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除空闲中断标志（否则会一直不断进入中断）
	
  /*创建缓存*/
  BufferManageCreate(&Uart_RX_Manage , Usart1BufferMemory , Usart1BufferMemoryLen , Usart1BufferMemoryManage , Usart1BufferMemoryManageLen*4);
  
  __HAL_UART_ENABLE_IT(&huart1 , UART_IT_IDLE);  //开启空闲中断

	HAL_UART_Receive_DMA(&huart1, Usart1ReadBuff, Usart1ReadBuffLen);   //不加这行不能收到第一次数据
	
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

    USER_UART_IDLECallback(huart);      //调用空闲中断数据处理函数
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
    
//    if (Uart_RX_Manage.ReadLen == 0) //防止刚上电就进空闲中断
//    {
//                                  // 标记接收结束
//      HAL_UART_Receive_DMA(&huart1, Usart1ReadBuff, Usart1ReadBuffLen); // 重新启动DMA接收
//      return;
//    }

 

    int value;

    Usart1ReadCnt = Usart1ReadBuffLen - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx); // 总数据量减去未接收到的数据量为已经接收到的数据量

    BufferManageWrite(&Uart_RX_Manage , Usart1ReadBuff , Usart1ReadCnt , &value);//把数据存入缓存

    HAL_UART_Receive_DMA(&huart1, Usart1ReadBuff, Usart1ReadBuffLen); // 重新启动DMA接收

  }



 


}

/**
 * @msg: 串口接收数据处理
 * @param {*}
 * @return {*}
 */
void USER_UART_RX_Handle(void)
{

/*提取缓存的数据*/
		BufferManageRead(&Uart_RX_Manage , Usart1BufferMemoryCopy , &Uart_RX_Manage.ReadLen);
		
		if(Uart_RX_Manage.ReadLen > 0)//有数据
		{
		
      HAL_UART_Transmit(&huart1 , Usart1BufferMemoryCopy , Uart_RX_Manage.ReadLen , 0x500);
		}


 
}

