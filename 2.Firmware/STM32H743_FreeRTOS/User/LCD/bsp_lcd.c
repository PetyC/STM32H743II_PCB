/*
 * @Description: LCD 应用
 * @Autor: Pi
 * @Date: 2022-06-09 19:52:29
 * @LastEditTime: 2022-06-10 00:47:43
 */
#include "bsp_lcd.h"
#include "BSP_ST7735S.h"
#include "lcd.h"
#include "stdio.h"

#if USE_FreeRTOS == 1
#include "FreeRTOS.h"
#include "task.h"
volatile static UBaseType_t uxSavedInterruptStatus = 0;
#endif

/* LCD 缓存 */
#define SRAMD4 __attribute__((section(".RAM_D3")))
#define LCD_BUFFER_LEN (uint16_t)(LCD_W * LCD_H * 2)
SRAMD4 uint8_t LCD_Buf[LCD_BUFFER_LEN];
SRAMD4 uint8_t LCD_DMA_Buf[LCD_BUFFER_LEN];

/* LCD设备实例 */
static SPI_Device_Str LCD_Dev = {0};

/*SPI句柄*/
extern SPI_HandleTypeDef hspi1;

/*内部使用函数*/
static void fifo_lock(void);
static void fifo_unlock(void);
static void User_SPI_TX_Cplt_Callback(SPI_HandleTypeDef *hspi);

/* fifo上锁函数 */
static void fifo_lock(void)
{
#if USE_FreeRTOS == 1
  uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
#else
  __disable_irq();
#endif
}

/* fifo解锁函数 */
static void fifo_unlock(void)
{
#if USE_FreeRTOS == 1
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
#else
  __enable_irq();
#endif
}

/**
 * @brief 用户定义的SPI DMA发送完成的回调函数.
 * @param {UART_HandleTypeDef} *huart
 * @return {*}
 */
static void User_SPI_TX_Cplt_Callback(SPI_HandleTypeDef *hspi)
{
  if (hspi == &hspi1)
  {
    LCD_Dev.Status = 0; /* DMA发送空闲 */
  }
}

/**
 * @brief 初始化LCD
 * @return {*}
 */
void User_LCD_Init(void)
{

  /* 配置串口1收发fifo */
  fifo_register(&LCD_Dev.fifo, &LCD_Buf[0], sizeof(LCD_Buf), fifo_lock, fifo_unlock);

  /* 配置串口1 DMA收发buf */
  LCD_Dev.dmatx_buf = &LCD_DMA_Buf[0];
  LCD_Dev.dmatx_buf_size = sizeof(LCD_Buf);

  /*初始化发送状态*/
  LCD_Dev.Status = 0;

  /*初始化屏幕*/
  LCD_Init();
}

/**
 * @brief SPI发送数据接口，实际是写入发送fifo，发送由dma处理
 * @param {uint8_t} uart_id
 * @param {uint8_t} *buf
 * @param {uint16_t} size
 * @return {*}
 */
uint16_t User_LCD_Write(SPI_HandleTypeDef *hspi, const uint8_t *buf, uint16_t size)
{
  if (hspi == &hspi1)
  {
    return fifo_write(&LCD_Dev.fifo, buf, size);
  }
  else
  {
    return 0;
  }
}

/**
 * @brief  循环从FIFO中读出数据并发送
 * @param
 * @retval
 */
void User_LCD_Poll_DMA_TX(SPI_HandleTypeDef *hspi)
{

  uint16_t size = 0;

  if (0x01 == LCD_Dev.Status)
  {
    return;
  }

  size = fifo_read(&LCD_Dev.fifo, LCD_Dev.dmatx_buf, LCD_Dev.dmatx_buf_size);

  if (size != 0)
  {
    LCD_Dev.Status = 0x01; /* DMA发送状态 */

    if (hspi == &hspi1)
    {
      HAL_SPI_Transmit_DMA(&hspi1, LCD_Dev.dmatx_buf, size);
    }
  }
}

/**
 * @brief HAL提供的SPI发送完成中断回调函数
 * @param {SPI_HandleTypeDef} *hspi
 * @return {*}
 */

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  User_SPI_TX_Cplt_Callback(hspi);
}

uint8_t LCD_Buffer0[LCD_BUFFER_LEN];
/**
 * @brief 全屏填充
 * @param {uint16_t} color
 * @return {*}
 */
void User_LCD_Fill(uint16_t color)
{

  for (uint16_t j = 0; j < LCD_BUFFER_LEN / 2; j++)
  {
    LCD_Buffer0[j * 2] = (uint8_t)(color >> 8);
    LCD_Buffer0[j * 2 + 1] = (uint8_t)(color);
  }

  User_LCD_Write(&hspi1, LCD_Buffer0, LCD_BUFFER_LEN);

  LCD_Address_Set(0, 0, 127, 127);
  Set_SPI_DATASIZE_8BIT();

  User_LCD_Poll_DMA_TX(&hspi1);
}


/**
 * @brief 显示MPU状态
 * @return {*}
 */
void User_LCD_CPU_Show(void)
{
  char pcWriteBuffer[512];

  uint8_t buff[] = "Name State Priority Stack Number";
 // LCD_ShowString( 0 , 0 , (uint8_t *)buff , RED , BLACK ,12 , 0);
  LCD_ShowString( 0 , 12 , "123456789012345678901" , RED , BLACK ,12 , 0);

/*
  memset(pcWriteBuffer, 0, 512);
  sprintf((char *)pcWriteBuffer, "%s", "name  state  priority  residue_stack  Number");


  vTaskList((char *)(pcWriteBuffer + strlen(pcWriteBuffer)));

  strcat((char *)pcWriteBuffer, "B:Blocked, R:Ready, D:Deleted, S:Suspended\r\n");

  LCD_ShowString( 80 , 116 , (uint8_t *)pcWriteBuffer , RED , BLACK ,12 , 0);
 */


  memset(pcWriteBuffer, 0, 512);
  uint8_t buff2[] = "Name Time Usage rate";
  LCD_ShowString( 0 , 36 , buff2 , RED , BLACK ,12 , 0);


  vTaskGetRunTimeStats((char *)pcWriteBuffer);

  uint8_t Buff[21] = {0};
  uint16_t pos_y;
  
  for(uint16_t i = 0 ,  pos_y = 48 ; i < strlen(pcWriteBuffer)-21 ; i+=21 , pos_y +=12)
  {
    memcpy(Buff , pcWriteBuffer+i , 21);
    LCD_ShowString( 0 , 48 , (uint8_t *)Buff , RED , BLACK ,12 , 0);
  }








}