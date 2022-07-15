/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:39
 * @LastEditTime: 2022-07-15 18:44:39
 */
#ifndef APP_UART_H
#define APP_UART_H


#include "Bsp_Uart.h"


extern void (*User_UART_RX_Fun)(uint8_t *data, uint16_t len);     //串口接收数据处理函数指针
extern void (*User_UART_RX_Finished)(void);                       //串口接收数据处理完成函数指针
 

/*串口回显功能*/
void User_UART_Echo(uint8_t *data , uint16_t len);

/*循环读取*/
void User_UART_RX_Loop(void);

/*设置单次从串口FIFO中读取数目*/
void User_UART_RX_Read_Len(uint32_t size);

/*超时时间选择*/
void User_UART_Time_Out_Sw(uint8_t sw);

/*超时服务函数*/
void User_UART_Timer(void);

void User_UART_Timer_Reset(void);


extern uint8_t Updata_Flag;
#endif
