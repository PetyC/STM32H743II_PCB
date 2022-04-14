/*
 * @Descripttion:
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-07-06 16:34:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-04-14 20:31:33
 */
#ifndef UART_PROCESS_H
#define UART_PROCESS_H

#include "main.h"
#include "usart.h"
#include "looplist.h"
#include "BufferManage.h"
#include "string.h"
#include "CRC.h"

//串口错误标志
#define ERROR_UART_CMD 0XFF //串口数据错误返回命令
#define ERROR_OTHER 0X01    //数据格式错误
#define ERROR_PE 0X02       //奇偶错误
#define ERROR_NE 0X03       //噪声错误
#define ERROR_FE 0X04       //帧错误
#define ERROR_ORE 0X05      //溢出错误
#define ERROR_CRC 0X07      // CRC校验错误

//缓存区长度
#define UART_RX_LEN 10 //缓存区长度

extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;



void USER_UART_IRQHandler(UART_HandleTypeDef *huart);
void USER_UART_IDLECallback(UART_HandleTypeDef *huart);
void USER_UART_RX_Handle(void);



void USER_UART_Loop_List_Init(void);
#endif
