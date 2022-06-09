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
#include "bsp_lcd.h"
#include "lcd.h"
#include "Dev_Uart.h"
#include "stdio.h"

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
osTimerId LED_TimerHandle;
osTimerId Uart_TimerHandle;
osTimerId LCD_TimerHandle;
osSemaphoreId KEY_Binary_SemHandle;
osSemaphoreId Uart_Time_Out_Binary_SemHandle;
osSemaphoreId LCD_Binary_SemHandle;
osSemaphoreId LCD_FPS_Binary_SemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);
void Start_KEY_Task(void const *argument);
void Start_LCD_Task(void const *argument);
void Start_Usart_Task(void const *argument);
void LED_Time_Callback(void const *argument);
void Uart_Timer_Callback(void const *argument);
void LCD_Timer_Callback(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
#if (configGENERATE_RUN_TIME_STATS == 1)
volatile uint32_t ulHighFrequencyTimerTicks;
#endif

__weak void configureTimerForRunTimeStats(void)
{
#if (configGENERATE_RUN_TIME_STATS == 1)
  ulHighFrequencyTimerTicks = 0;
#endif
}

__weak unsigned long getRunTimeCounterValue(void)
{
#if (configGENERATE_RUN_TIME_STATS == 1)
  return ulHighFrequencyTimerTicks;
#else
  return 0;
#endif
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
  called if a stack overflow is detected. */

  uint8_t Buff[50] = "Task:";
  strcat((char *)Buff, (char *)pcTaskName);
  strcat((char *)Buff, " Stack is overflow!\r\n");

  User_UART_Write(&huart1, Buff, sizeof(Buff));
  User_UART_Poll_DMA_TX(&huart1);
}
/* USER CODE END 4 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of KEY_Binary_Sem */
  osSemaphoreDef(KEY_Binary_Sem);
  KEY_Binary_SemHandle = osSemaphoreCreate(osSemaphore(KEY_Binary_Sem), 1);

  /* definition and creation of Uart_Time_Out_Binary_Sem */
  osSemaphoreDef(Uart_Time_Out_Binary_Sem);
  Uart_Time_Out_Binary_SemHandle = osSemaphoreCreate(osSemaphore(Uart_Time_Out_Binary_Sem), 1);

  /* definition and creation of LCD_Binary_Sem */
  osSemaphoreDef(LCD_Binary_Sem);
  LCD_Binary_SemHandle = osSemaphoreCreate(osSemaphore(LCD_Binary_Sem), 1);

  /* definition and creation of LCD_FPS_Binary_Sem */
  osSemaphoreDef(LCD_FPS_Binary_Sem);
  LCD_FPS_Binary_SemHandle = osSemaphoreCreate(osSemaphore(LCD_FPS_Binary_Sem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of LED_Timer */
  osTimerDef(LED_Timer, LED_Time_Callback);
  LED_TimerHandle = osTimerCreate(osTimer(LED_Timer), osTimerPeriodic, NULL);

  /* definition and creation of Uart_Timer */
  osTimerDef(Uart_Timer, Uart_Timer_Callback);
  Uart_TimerHandle = osTimerCreate(osTimer(Uart_Timer), osTimerOnce, NULL);

  /* definition and creation of LCD_Timer */
  osTimerDef(LCD_Timer, LCD_Timer_Callback);
  LCD_TimerHandle = osTimerCreate(osTimer(LCD_Timer), osTimerPeriodic, NULL);

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
  osThreadDef(KEY_Task, Start_KEY_Task, osPriorityAboveNormal, 0, 256);
  KEY_TaskHandle = osThreadCreate(osThread(KEY_Task), NULL);

  /* definition and creation of LCD_Task */
  osThreadDef(LCD_Task, Start_LCD_Task, osPriorityNormal, 0, 256);
  LCD_TaskHandle = osThreadCreate(osThread(LCD_Task), NULL);

  /* definition and creation of Usart_Task */
  osThreadDef(Usart_Task, Start_Usart_Task, osPriorityHigh, 0, 256);
  Usart_TaskHandle = osThreadCreate(osThread(Usart_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  /*开启LED定时器*/
  osTimerStart(LED_TimerHandle, 1000);
  /*开启LCD定时器*/
  osTimerStart(LCD_TimerHandle, 1000);
  /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for (;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
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
void Start_KEY_Task(void const *argument)
{
  /* USER CODE BEGIN Start_KEY_Task */
  char pcWriteBuffer[512];
  /* Infinite loop */
  for (;;)
  {
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == 1)
    {
      osDelay(50);

      if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == 1)
      {
#if (configUSE_TRACE_FACILITY == 1 && configUSE_STATS_FORMATTING_FUNCTIONS == 1)
        memset(pcWriteBuffer, 0, 512);
        sprintf((char *)pcWriteBuffer, "\r\n%s\r\n", "name  state  priority  residue_stack  Number");
        strcat((char *)pcWriteBuffer, "---------------------------------------------\r\n");
        vTaskList((char *)(pcWriteBuffer + strlen(pcWriteBuffer)));
        strcat((char *)pcWriteBuffer, "---------------------------------------------\r\n");
        strcat((char *)pcWriteBuffer, "B : Blocked, R : Ready, D : Deleted, S : Suspended\r\n");

        User_UART_Write(&huart1, (uint8_t *)pcWriteBuffer, strlen(pcWriteBuffer));
#endif
#if (configGENERATE_RUN_TIME_STATS == 1 && configUSE_STATS_FORMATTING_FUNCTIONS == 1)
        memset(pcWriteBuffer, 0, 512);
        strcat((char *)pcWriteBuffer, "\r\nName\t\tTime\t\tUsage rate\r\n");
        strcat((char *)pcWriteBuffer, "---------------------------------------------\r\n");
        vTaskGetRunTimeStats((char *)(pcWriteBuffer + strlen(pcWriteBuffer)));
        strcat((char *)pcWriteBuffer, "---------------------------------------------\r\n");

        User_UART_Write(&huart1, (uint8_t *)pcWriteBuffer, strlen(pcWriteBuffer));

#endif
      }
    }

    osDelay(20);
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
void Start_LCD_Task(void const *argument)
{
  /* USER CODE BEGIN Start_LCD_Task */

  LCD_Init();
  LCD_Fill(0, 0, 128, 128, BLACK);
  LCD_Fill(0, 0, 128, 128, GRED);
  char TX_FPS_Buff[50];
  int FPS = 0;

  TickType_t Last_Wake_Time = 0;
  Last_Wake_Time = osKernelSysTick();

  /* Infinite loop */
  for (;;)
  { 

    //User_LCD_Fill(RED);
    FPS++;

    if (osOK == osSemaphoreWait(LCD_FPS_Binary_SemHandle, 0))
    {
      sprintf((char *)TX_FPS_Buff, "LCD FPS:%d\r\n", FPS);
      User_UART_Write(&huart1, (uint8_t *)TX_FPS_Buff, strlen(TX_FPS_Buff));

      sprintf((char *)TX_FPS_Buff, "FPS:%d", FPS);

     // LCD_ShowString( 80 , 116 , (uint8_t *)TX_FPS_Buff , RED , BLACK ,12 , 0);
     
      FPS = 0;
      osTimerStart(LCD_TimerHandle, 1000);
    }
  
   
    osDelayUntil(&Last_Wake_Time, 20);
    
    //osDelayUntil(&Last_Wake_Time, 20);
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
void Start_Usart_Task(void const *argument)
{
  /* USER CODE BEGIN Start_Usart_Task */

  uint8_t Uart_Data[512];
  uint16_t size = 0;
  uint16_t TX_Buff_MAX = 400;
  bool Time_Out_Flag = 0; //串口超时标志

  /* Infinite loop */
  for (;;)
  {
    /*串口回显测试*/
    vTaskSuspendAll(); //打开调度锁 禁止调度

    size = User_UART_Read(&huart1, Uart_Data, sizeof(Uart_Data));

    if (size > 0)
    {
      User_UART_Write(&huart1, Uart_Data, size);
    }

    uint16_t Buff_Occupy = User_UART_Get_TX_Buff_Occupy(&huart1);

    if (Buff_Occupy < TX_Buff_MAX && Buff_Occupy > 0)
    {
      //串口超时定时器开启
      if (Time_Out_Flag == 0)
      {
        Time_Out_Flag = 1;
        osTimerStart(Uart_TimerHandle, 10);
      }

      if (osOK == osSemaphoreWait(Uart_Time_Out_Binary_SemHandle, 0))
      {
        Time_Out_Flag = 0;
        User_UART_Poll_DMA_TX(&huart1);
      }
    }
    else if (User_UART_Get_TX_Buff_Occupy(&huart1) > TX_Buff_MAX)
    {
      User_UART_Poll_DMA_TX(&huart1);

      if (Time_Out_Flag == 1)
      {
        Time_Out_Flag = 0;
        osTimerStop(Uart_TimerHandle);
      }
    }

    xTaskResumeAll(); //恢复调度

    osDelay(1);
  }

  /* USER CODE END Start_Usart_Task */
}

/* LED_Time_Callback function */
void LED_Time_Callback(void const *argument)
{
  /* USER CODE BEGIN LED_Time_Callback */
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  /* USER CODE END LED_Time_Callback */
}

/* Uart_Timer_Callback function */
void Uart_Timer_Callback(void const *argument)
{
  /* USER CODE BEGIN Uart_Timer_Callback */

  /*产生二值信号量*/
  osSemaphoreRelease(Uart_Time_Out_Binary_SemHandle);

  /* USER CODE END Uart_Timer_Callback */
}

/* LCD_Timer_Callback function */
void LCD_Timer_Callback(void const *argument)
{
  /* USER CODE BEGIN LCD_Timer_Callback */
  /*产生二值信号量*/
  osSemaphoreRelease(LCD_FPS_Binary_SemHandle);
  /* USER CODE END LCD_Timer_Callback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
