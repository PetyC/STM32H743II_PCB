/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-06-09 18:54:44
 * @LastEditTime: 2022-06-14 21:48:21
 */
#ifndef  BSP_S7735_H
#define  BSP_S7735_H

#include <inttypes.h>
#include "main.h"

/*屏幕尺寸*/
#define defWIDTH   128
#define defHEIGHT  128
#define defXSTART  0
#define defYSTART  2

/*显示方向 0~3*/
#define USE_HORIZONTAL 2

//#define BUFFER     //全帧宽度*高度* 2字节大小。非常快
//#define BUFFER1        //实际上没有缓冲写，很慢，因为内存有限
#define HVBUFFER      //行缓冲区宽度* 2字节大小。用作行或列缓冲区。


/*IO操作*/
#define Pin_RES_Low()   LCD_RES_GPIO_Port->BSRR = (uint32_t)LCD_RES_Pin << 16U
#define Pin_RES_High()  LCD_RES_GPIO_Port->BSRR = LCD_RES_Pin

#define Pin_DC_Low()    LCD_DC_GPIO_Port->BSRR = (uint32_t)LCD_DC_Pin << 16U  
#define Pin_DC_High()   LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin                  

#define Pin_BLK_Low()   LCD_BLK_GPIO_Port->BSRR = (uint32_t)LCD_BLK_Pin << 16U
#define Pin_BLK_High()   LCD_BLK_GPIO_Port->BSRR = LCD_BLK_Pin



void Pin_BLK_Pct(uint8_t);

void SPI_Transmit(uint8_t *data , uint16_t len);
void SPI_TransmitCmd(uint8_t *data , uint16_t len);
void SPI_TransmitData(uint8_t *data , uint16_t len);

void _Delay(uint32_t d);



#endif
