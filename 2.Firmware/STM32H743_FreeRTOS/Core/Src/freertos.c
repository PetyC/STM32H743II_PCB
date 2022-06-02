/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_init.h"
#include "lcd.h"
#include "Dev_Uart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId KEY_TaskHandle;
osThreadId LCD_TaskHandle;
osThreadId Usart_TaskHandle;
osSemaphoreId KEY_Binary_SemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Start_KEY_Task(void const * argument);
void Start_LCD_Task(void const * argument);
void Start_Usart_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of KEY_Binary_Sem */
  osSemaphoreDef(KEY_Binary_Sem);
  KEY_Binary_SemHandle = osSemaphoreCreate(osSemaphore(KEY_Binary_Sem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of KEY_Task */
  osThreadDef(KEY_Task, Start_KEY_Task, osPriorityLow, 0, 128);
  KEY_TaskHandle = osThreadCreate(osThread(KEY_Task), NULL);

  /* definition and creation of LCD_Task */
  osThreadDef(LCD_Task, Start_LCD_Task, osPriorityNormal, 0, 128);
  LCD_TaskHandle = osThreadCreate(osThread(LCD_Task), NULL);

  /* definition and creation of Usart_Task */
  osThreadDef(Usart_Task, Start_Usart_Task, osPriorityAboveNormal, 0, 128);
  Usart_TaskHandle = osThreadCreate(osThread(Usart_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port , LED1_Pin);
    osDelay(800);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Start_KEY_Task */
/**
* @brief Function implementing the KEY_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_KEY_Task */
void Start_KEY_Task(void const * argument)
{
  /* USER CODE BEGIN Start_KEY_Task */
 
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_KEY_Task */
}

/* USER CODE BEGIN Header_Start_LCD_Task */
/**
* @brief Function implementing the LCD_Task thread.
* @param argument: Not used
* @retval None
*/ 
/* USER CODE END Header_Start_LCD_Task */
void Start_LCD_Task(void const * argument)
{
  /* USER CODE BEGIN Start_LCD_Task */

  LCD_Init();
  LCD_Fill(0,0 , 128 , 128 , BLACK);
  
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_LCD_Task */
}

/* USER CODE BEGIN Header_Start_Usart_Task */
/**
* @brief Function implementing the Usart_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Usart_Task */
void Start_Usart_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Usart_Task */
  uint8_t Uart_Data[255];
  /* Infinite loop */
  for(;;)
  {
   
    uint8_t size = User_UART_Read(&huart1 , Uart_Data , 255);

    if(size !=0)
    {
      User_UART_Write(&huart1 , Uart_Data , size);
      User_UART_Poll_DMA_TX(&huart1);
      memset(Uart_Data , 0 , 255);
    }
    
  }
  /* USER CODE END Start_Usart_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
