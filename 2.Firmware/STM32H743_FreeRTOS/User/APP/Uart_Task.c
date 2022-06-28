/*
 * @Description: UART任务
 * @Autor: Pi
 * @Date: 2022-06-27 15:30:24
 * @LastEditTime: 2022-06-28 19:55:35
 */
#include "Uart_Task.h"

/*FreeRTOS相关变量*/
extern osTimerId Uart_TimerHandle;
extern osSemaphoreId Uart_Time_Out_Binary_SemHandle;


/*系统相关变量*/
extern TIM_HandleTypeDef htim13;


/**
 * @brief UART任务
 * @param {void*} argument
 * @return {*}
 */
void Usart_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Usart_Task */

  uint8_t Uart_Data[512];
  uint16_t size = 0;
  uint16_t TX_Buff_MAX = 400;
  bool Time_Out_Flag = 0; //串口超时标志

  /* Infinite loop */
  for (;;)
  {
    /*串口回显测试*/
    vTaskSuspendAll(); //打开调度锁 禁止调度

    size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));

    if (size > 0)
    {
      User_UART_Write(&huart1, Uart_Data, size);
    }

    uint16_t Buff_Occupy = User_UART_Get_TX_Buff_Occupy(&huart1);

    if (Buff_Occupy < TX_Buff_MAX && Buff_Occupy > 0)
    {
      //串口超时定时器开启
      if (Time_Out_Flag == 0)
      {
        Time_Out_Flag = 1;
        osTimerStart(Uart_TimerHandle, 10);
      }
      //定时器超时
      if (osOK == osSemaphoreWait(Uart_Time_Out_Binary_SemHandle, 0))
      {
        Time_Out_Flag = 0;
        User_UART_Poll_DMA_TX(&huart1);
      }
    }
    else if (User_UART_Get_TX_Buff_Occupy(&huart1) > TX_Buff_MAX)
    {
      User_UART_Poll_DMA_TX(&huart1);

      if (Time_Out_Flag == 1)
      {
        Time_Out_Flag = 0;
        osTimerStop(Uart_TimerHandle);
      }
    }

    xTaskResumeAll(); //恢复调度

    osDelay(1);
  }

  /* USER CODE END Start_Usart_Task */
}



/**
 * @brief 定时器回调函数
 * @param {TIM_HandleTypeDef} *htim
 * @return {*}
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if(htim == &htim13)
  {
    HAL_TIM_Base_Stop(&htim13);
    
  }
    
 

}