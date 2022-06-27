/*
 * @Description: UART任务
 * @Autor: Pi
 * @Date: 2022-06-27 15:30:28
 * @LastEditTime: 2022-06-27 15:32:44
 */
#ifndef UART_TASK_H
#define UART_TASK_H

/*FreeRTOS相关头文件*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


/*BSP相关文件*/
#include "Dev_Uart.h"
#include "stdio.h"






void Usart_Task(void const * argument);



#endif
