/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:39
 * @LastEditTime: 2022-07-05 01:52:37
 */
#ifndef APP_UART_PROCESS_H
#define APP_UART_PROCESS_H


#include "Bsp_Uart.h"


//APP命令
#define REST_CMD              0X30           //重启



void User_UART_Process(void);
uint8_t User_UART_RX_CRC(uint8_t *Data , uint8_t Len);
void User_UART_CMD_Process(uint8_t *Data , uint8_t Len);

#endif
