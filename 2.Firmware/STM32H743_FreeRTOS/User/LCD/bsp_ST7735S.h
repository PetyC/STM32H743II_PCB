/*
 * @Description: ST7735S BSP支持包
 * @Autor: Pi
 * @Date: 2022-06-09 19:09:07
 * @LastEditTime: 2022-06-10 16:53:48
 */
#ifndef BSP_ST7735S_H
#define BSP_ST7735S_H

#include "main.h"
#include <string.h>
 
/*屏幕分辨率*/
#define LCD_W 128
#define LCD_H 128
/*屏幕方向 0~3*/
#define USE_HORIZONTAL 3

/*GPIO操作*/
#define LCD_RES_Clr() LCD_RES_GPIO_Port->BSRR = (uint32_t)LCD_RES_Pin << 16U
#define LCD_RES_Set() LCD_RES_GPIO_Port->BSRR = LCD_RES_Pin

#define LCD_DC_Clr() LCD_DC_GPIO_Port->BSRR = (uint32_t)LCD_DC_Pin << 16U 
#define LCD_DC_Set() LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin                  

#define LCD_BLK_Clr() LCD_BLK_GPIO_Port->BSRR = (uint32_t)LCD_BLK_Pin << 16U 
#define LCD_BLK_Set() LCD_BLK_GPIO_Port->BSRR = LCD_BLK_Pin                  


void LCD_Init(void);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA16(uint16_t dat);
void LCD_WR_CMD(uint8_t *CMD, uint8_t Len);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);



/*暂时暴露出接口*/
void Set_SPI_DATASIZE_16BIT(void);
void Set_SPI_DATASIZE_8BIT(void);
#endif
