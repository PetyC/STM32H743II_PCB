/*
 * @Description: ST7735S BSP支持包
 * @Autor: Pi
 * @Date: 2022-06-09 19:09:25
 * @LastEditTime: 2022-06-09 19:55:33
 */
#include "BSP_ST7735S.h"

extern SPI_HandleTypeDef hspi1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*相关命令参数*/
uint8_t Set_Frame_Rate_CMD0[] = {0xB1, 0x02, 0x35, 0x36};                   // Frame rate 80Hz   全色正常模式
uint8_t Set_Frame_Rate_CMD1[] = {0xB2, 0x02, 0x35, 0x36};                   // Frame rate 80Hz   空闲模式8色
uint8_t Set_Frame_Rate_CMD2[] = {0xB3, 0x02, 0x35, 0x36, 0x02, 0x35, 0x36}; // Frame rate 80Hz   局部模式全色

uint8_t Set_Display_Inversion_CMD[] = {0xB4, 0x03}; //开启反转

//------------------------------------ST7735S Power Sequence-----------------------------------------//
uint8_t Set_Power_Sequence_CMD0[] = {0xC0, 0xA2, 0x02, 0x84}; //电源控制1
uint8_t Set_Power_Sequence_CMD1[] = {0xC1, 0xC5};             //电源控制2
uint8_t Set_Power_Sequence_CMD2[] = {0xC2, 0x0D, 0x00};       //电源控制3  正常全色模式
uint8_t Set_Power_Sequence_CMD3[] = {0xC3, 0x8D, 0x2A};       //电源控制3  正常全色模式
uint8_t Set_Power_Sequence_CMD4[] = {0xC4, 0x8D, 0xEE};       //电源控制3  正常全色模式
//---------------------------------End ST7735S Power Sequence---------------------------------------//

// VCOM
uint8_t Set_VCOM_CMD[] = {0xC5, 0x0a};

//内存数据访问控制  通过设置此选项来改变屏幕方向
#if (USE_HORIZONTAL == 0)
uint8_t Set_Memory_Data_Access_CMD[] = {0x36, 0x08}; // 0x08 0xC8 0x78 0xA8
#elif (USE_HORIZONTAL == 1)
uint8_t Set_Memory_Data_Access_CMD[] = {0x36, 0xC8}; // 0x08 0xC8 0x78 0xA8
#elif (USE_HORIZONTAL == 2)
uint8_t Set_Memory_Data_Access_CMD[] = {0x36, 0x78}; // 0x08 0xC8 0x78 0xA8
#else
uint8_t Set_Memory_Data_Access_CMD[] = {0x36, 0xA8}; // 0x08 0xC8 0x78 0xA8
#endif

//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
uint8_t Set_Gamma_Sequence_CMD0[] = {0XE0, 0x12, 0x1C, 0x10, 0x18, 0x33, 0x2C, 0x25, 0x28, 0x28, 0x27, 0x2F, 0x3C, 0x00, 0x03, 0x03, 0x10}; //伽马 + 矫正
uint8_t Set_Gamma_Sequence_CMD1[] = {0XE1, 0x12, 0x1C, 0x10, 0x18, 0x2D, 0x28, 0x23, 0x28, 0x28, 0x26, 0x2F, 0x3B, 0x00, 0x03, 0x03, 0x10}; //伽马 - 矫正
//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//

// 65k mode
uint8_t Set_Interface_Pixel_CMD[] = {
    0x3A,
    0x05,
}; // 16-Bit
// Display on
uint8_t Set_Display_on_CMD[] = {0x29};
// Sleep out
uint8_t Set_Sleep_Out_CMD[] = {0x11};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*只在内部使用的函数*/
static void Set_SPI_DATASIZE_16BIT(void);
static void Set_SPI_DATASIZE_8BIT(void);

/**
 * @brief 设置SPI传输数据长度16位
 * @param {*}
 * @return {*}
 */
static void Set_SPI_DATASIZE_16BIT(void)
{
  SPI1->CFG1 &= ~(0x0000001f);              //置0
  SPI1->CFG1 |= SPI_DATASIZE_16BIT;         //再赋值
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT; // 此处赋值为了正确使用HAL_SPI_Transmit() 函数
}

/**
 * @brief 设置SPI传输数据长度8位
 * @param {*}
 * @return {*}
 */
static void Set_SPI_DATASIZE_8BIT(void)
{
  SPI1->CFG1 &= ~(0x0000001f);             //置0
  SPI1->CFG1 |= SPI_DATASIZE_8BIT;         //再赋值
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT; //(0x00000007UL)
}

/**
 * @brief LCD写入8位数据
 * @param {uint8_t} dat 写入的数据
 * @return {*}
 */
void LCD_WR_DATA8(uint8_t dat)
{
  Set_SPI_DATASIZE_8BIT();
  HAL_SPI_Transmit(&hspi1, &dat, 1, 0Xff);
}

/**
 * @brief LCD写入16位数据
 * @param {uint16_t} dat  写入的数据
 * @return {*}
 */
void LCD_WR_DATA16(uint16_t dat)
{
  Set_SPI_DATASIZE_16BIT();
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&dat, 1, 0Xff);
}

/**
 * @brief 写入命令
 * @param {uint8_t} CMD         命令
 * @param {uint8_t} *Parameter  参数数组
 * @param {uint8_t} Len         参数长度
 * @return {*}
 */
void LCD_WR_CMD(uint8_t *CMD, uint8_t Len)
{
  LCD_DC_Clr();
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&CMD[0], 1, 0Xff);
  LCD_DC_Set();

  if (Len <= 1)
  {
    return;
  }
  else
  {
    HAL_SPI_Transmit(&hspi1, &CMD[1], (Len - 1), 0Xff);
  }
}

/**
 * @brief 设置写入地址
 * @param {uint16_t} x1
 * @param {uint16_t} y1
 * @param {uint16_t} x2
 * @param {uint16_t} y2
 * @return {*}
 */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  uint8_t CMD = 0X2A;
  uint16_t Data[2];

  switch (USE_HORIZONTAL)
  {
  case 0:
    x1 += 2;
    x2 += 2;
    y1 += 1;
    y1 += 1;
    break;
  case 1:
    x1 += 2;
    x2 += 2;
    y1 += 3;
    y2 += 3;
    break;
  case 2:
    x1 += 1;
    x1 += 1;
    y1 += 2;
    y2 += 2;
    break;
  case 3:
    x1 += 3;
    x2 += 3;
    y1 += 2;
    y2 += 2;
    break;

  default:
    return;
  }

  LCD_DC_Clr(); //写命令
  Set_SPI_DATASIZE_8BIT();
  HAL_SPI_Transmit(&hspi1, &CMD, 1, 0Xff);
  LCD_DC_Set(); //写数据
  Set_SPI_DATASIZE_16BIT();
  Data[0] = x1;
  Data[1] = x2;
  HAL_SPI_Transmit(&hspi1, (uint8_t *)Data, 2, 0Xff);

  CMD = 0x2b;
  LCD_DC_Clr(); //写命令
  Set_SPI_DATASIZE_8BIT();
  HAL_SPI_Transmit(&hspi1, &CMD, 1, 0Xff);
  LCD_DC_Set(); //写数据
  Set_SPI_DATASIZE_16BIT();
  Data[0] = y1;
  Data[1] = y2;
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&Data, 2, 0Xff);

  CMD = 0x2c;
  LCD_DC_Clr(); //写命令
  Set_SPI_DATASIZE_8BIT();
  HAL_SPI_Transmit(&hspi1, &CMD, 1, 0Xff);
  LCD_DC_Set(); //写数据
}

/**
 * @brief LCD初始化工作
 * @param {*}
 * @return {*}
 */
void LCD_Init(void)
{

  LCD_RES_Clr(); //复位
  HAL_Delay(100);
  LCD_RES_Set();
  HAL_Delay(100);
  LCD_BLK_Set(); //打开背光
  HAL_Delay(100);

  LCD_WR_CMD(Set_Sleep_Out_CMD, sizeof(Set_Sleep_Out_CMD));
  HAL_Delay(120); //

  LCD_WR_CMD(Set_Frame_Rate_CMD0, sizeof(Set_Frame_Rate_CMD0));
  LCD_WR_CMD(Set_Frame_Rate_CMD1, sizeof(Set_Frame_Rate_CMD1));
  LCD_WR_CMD(Set_Frame_Rate_CMD2, sizeof(Set_Frame_Rate_CMD2));
  LCD_WR_CMD(Set_Display_Inversion_CMD, sizeof(Set_Display_Inversion_CMD));
  LCD_WR_CMD(Set_Power_Sequence_CMD0, sizeof(Set_Power_Sequence_CMD0));
  LCD_WR_CMD(Set_Power_Sequence_CMD1, sizeof(Set_Power_Sequence_CMD1));
  LCD_WR_CMD(Set_Power_Sequence_CMD2, sizeof(Set_Power_Sequence_CMD2));
  LCD_WR_CMD(Set_Power_Sequence_CMD3, sizeof(Set_Power_Sequence_CMD3));
  LCD_WR_CMD(Set_Power_Sequence_CMD4, sizeof(Set_Power_Sequence_CMD4));
  LCD_WR_CMD(Set_VCOM_CMD, sizeof(Set_VCOM_CMD));
  LCD_WR_CMD(Set_Memory_Data_Access_CMD, sizeof(Set_Memory_Data_Access_CMD));
  LCD_WR_CMD(Set_Gamma_Sequence_CMD0, sizeof(Set_Gamma_Sequence_CMD0));
  LCD_WR_CMD(Set_Gamma_Sequence_CMD1, sizeof(Set_Gamma_Sequence_CMD1));
  LCD_WR_CMD(Set_Interface_Pixel_CMD, sizeof(Set_Interface_Pixel_CMD));
  LCD_WR_CMD(Set_Display_on_CMD, sizeof(Set_Display_on_CMD));
}
