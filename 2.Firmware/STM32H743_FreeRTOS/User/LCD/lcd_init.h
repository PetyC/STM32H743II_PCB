/*
 * @Description:LCD 驱动层相关函数
 * @Autor: Pi
 * @Date: 2022-01-14 16:06:56
 * @LastEditTime: 2022-06-09 19:30:21
 */
#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "main.h"
#include <string.h>
#include "Fifo.h"

#define USE_HORIZONTAL 3

#define LCD_W 128
#define LCD_H 128


#define LCD_RES_Clr() LCD_RES_GPIO_Port->BSRR = (uint32_t)LCD_RES_Pin << 16U
#define LCD_RES_Set() LCD_RES_GPIO_Port->BSRR = LCD_RES_Pin

#define LCD_DC_Clr() LCD_DC_GPIO_Port->BSRR = (uint32_t)LCD_DC_Pin << 16U 
#define LCD_DC_Set() LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin                  

#define LCD_BLK_Clr() LCD_BLK_GPIO_Port->BSRR = (uint32_t)LCD_BLK_Pin << 16U 
#define LCD_BLK_Set() LCD_BLK_GPIO_Port->BSRR = LCD_BLK_Pin        




void LCD_Init(void);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void Set_SPI_DATASIZE_16BIT(void);
void Set_SPI_DATASIZE_8BIT(void);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


void User_LCD_Fill(uint16_t color);
void User_LCD_ShowPicture(const uint8_t pic[], uint32_t len);
void User_LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);





#endif
