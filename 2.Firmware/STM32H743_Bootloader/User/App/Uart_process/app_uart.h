/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:39
 * @LastEditTime: 2022-07-11 19:20:11
 */
#ifndef APP_UART_H
#define APP_UART_H


#include "Bsp_Uart.h"


extern void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);     //串口接收数据处理函数指针
extern void (*User_UART_RX_Finished)(void);                       //串口接收数据处理完成函数指针
extern uint8_t UART_RX_Time_Out_Flag;                             //串口超时标志


void User_UART_Echo(uint8_t *data , uint16_t len);



void User_UART_RX_Loop(void);



 
 

#endif
