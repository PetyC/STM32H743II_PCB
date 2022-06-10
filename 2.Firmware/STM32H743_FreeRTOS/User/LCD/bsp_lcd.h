/*
 * @Description: LCD 应用
 * @Autor: Pi
 * @Date: 2022-06-09 19:52:33
 * @LastEditTime: 2022-06-10 16:54:47
 */
#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "main.h"
#include <string.h>
#include "Fifo.h"
#include "bsp_ST7735S.h"

/*是否使用了FreeRTOS*/
#define USE_FreeRTOS  1


/* SPI设备数据结构 */
typedef struct
{
	uint8_t Status;		/* 发送状态 */
	_fifo_t fifo;	/* 发送fifo */
	uint8_t *dmatx_buf;	/* dma发送缓存 */
	uint16_t dmatx_buf_size;/* dma发送缓存大小 */

}SPI_Device_Str;


void User_LCD_Init(void);
uint16_t User_LCD_Write(SPI_HandleTypeDef *hspi, const uint8_t *buf, uint16_t size);
void User_LCD_Poll_DMA_TX(SPI_HandleTypeDef *hspi);


void User_LCD_Fill(uint16_t color);

void User_LCD_CPU_Show(void);
#endif
