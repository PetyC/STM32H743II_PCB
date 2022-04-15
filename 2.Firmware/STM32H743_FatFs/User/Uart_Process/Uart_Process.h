/*
 * @Descripttion:串口接收数据处理 DMA+空闲中断+环形队列
 * @version: HAL STM32Cubemx
 * @Author: Pi
 * @Date: 2021-07-06 16:34:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-04-15 19:15:05
 */
#ifndef UART_PROCESS_H
#define UART_PROCESS_H

#include "main.h"
#include "usart.h"
#include "looplist.h"
#include "BufferManage.h"
#include "string.h"
#include "CRC.h"

#define Single_Buffer_Len 50 // DMA单次接收缓存长度
#define RX_Buffer_Len 200    //接收区数据缓存长度
#define RX_Len_Manage_LEN 20 //接收区数据每次收到数据长度数组 长度

#define TX_Buffer_Len 200    //接收区数据缓存长度
#define TX_Len_Manage_LEN 20 //接收区数据每次收到数据长度数组 长度


typedef struct
{
  Buff_Manage_Str List_Manage;              //环形队列管理变量
  uint8_t Single_Buffer[Single_Buffer_Len]; // DMA单次接收缓存
  uint8_t Buffer[RX_Buffer_Len];            //接收区数据缓存
  uint32_t RX_Len_Manage[RX_Len_Manage_LEN]; //接收区数据每次收到数据长度 (必须32位长度)
} Usart_RX_Data_Str;


typedef struct
{
  Buff_Manage_Str List_Manage;              //环形队列管理变量
  uint8_t Buffer[TX_Buffer_Len];            //接收区数据缓存
  uint32_t TX_Len_Manage[TX_Len_Manage_LEN]; //接收区数据每次收到数据长度 (必须32位长度)
} Usart_TX_Data_Str;


extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;


void User_UART_IRQHandler(UART_HandleTypeDef *huart);
void User_UART_IDLECallback(UART_HandleTypeDef *huart);
void User_UART_RX_Handle(void);

void User_UART_Init(void);

#endif
