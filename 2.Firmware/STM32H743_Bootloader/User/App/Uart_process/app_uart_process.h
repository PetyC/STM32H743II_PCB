/*
 * @Description: 串口数据处理
 * @Autor: Pi
 * @Date: 2022-07-04 19:10:39
 * @LastEditTime: 2022-07-05 14:52:33
 */
#ifndef APP_UART_PROCESS_H
#define APP_UART_PROCESS_H


#include "Bsp_Uart.h"


//APP命令
#define KEY_CMD               0X30          //按键
#define REST_CMD              0X32          //复位
#define WIFI_SSID_CMD         0X34          //wifi SSID
#define WIFI_PAW_CMD          0X36          //wifi PAW


//#define APP_UPDATA_CMD        0X38          //APP 升级标志
#define APP_BIN_CMD           0X40          //APP Bin文件大小
#define APP_START_CMD         0X42          //APP 开始传输
#define APP_CHECK_CMD         0X44          //查询APP是否需要升级



void User_UART_Process(void);
uint8_t User_UART_RX_CRC(uint8_t *Data , uint8_t Len);
void User_UART_CMD_Process(uint8_t *Data , uint8_t Len);

#endif
