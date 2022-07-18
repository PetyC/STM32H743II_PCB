/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:39
 * @LastEditTime: 2022-07-18 19:50:47
 */
#ifndef APP_UART_H
#define APP_UART_H

#include "Bsp_Uart.h"


extern void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);     //串口接收数据处理函数指针
extern void (*User_UART_RX_Finished)(uint8_t *data, uint16_t len);                      //串口接收数据处理完成函数指针
 

/*数据处理状态机 循环调用*/
void User_UART_RX_FSM(void);

/*设置从串口读取数据长度最大值*/
void User_UART_RX_Size_Max(uint16_t Size);

/*设置从串口读取数据长度最小值*/
void User_UART_RX_Size_Min(uint16_t Size);

/*设置串口超时最大时间 默认5ms*/
void User_Uart_RX_Timeout_Set(uint16_t Timeout);

/*定时中断服务函数*/
void User_UART_Timer(void);

/*定时器使能*/
void User_UART_Timer_Enable(uint8_t Enable);

/*查询是否超时*/
uint8_t User_UART_Query_Timeout(void);

/*串口回显功能 测试用*/
void User_UART_Echo(uint8_t *data , uint16_t len);

/*串口接收完成 测试用*/
void User_UART_Finished_Demo(uint8_t *data, uint16_t len);
#endif
