/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
osThreadId Detect_KEY_TaskHandle;
osThreadId Blink_LED_TaskHandle;
osThreadId LCD_TaskHandle;
osSemaphoreId KEY_Binary_SemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Start_Detect_KEY_Task(void const * argument);
void Start_Blink_LED_Task(void const * argument);
void Start_LCD_Task(void const * argument);

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
  xSemaphoreTake(KEY_Binary_SemHandle , portMAX_DELAY); 

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

  /* definition and creation of Detect_KEY_Task */
  osThreadDef(Detect_KEY_Task, Start_Detect_KEY_Task, osPriorityIdle, 0, 128);
  Detect_KEY_TaskHandle = osThreadCreate(osThread(Detect_KEY_Task), NULL);

  /* definition and creation of Blink_LED_Task */
  osThreadDef(Blink_LED_Task, Start_Blink_LED_Task, osPriorityIdle, 0, 128);
  Blink_LED_TaskHandle = osThreadCreate(osThread(Blink_LED_Task), NULL);

  /* definition and creation of LCD_Task */
  osThreadDef(LCD_Task, Start_LCD_Task, osPriorityNormal, 0, 128);
  LCD_TaskHandle = osThreadCreate(osThread(LCD_Task), NULL);

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

/* USER CODE BEGIN Header_Start_Detect_KEY_Task */
/**
* @brief Function implementing the Detect_KEY_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Detect_KEY_Task */
void Start_Detect_KEY_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Detect_KEY_Task */
  /* Infinite loop */
  for(;;)
  {

    uint8_t KEY_Stat = HAL_GPIO_ReadPin(KEY_GPIO_Port , KEY_Pin);

    if(KEY_Stat == 1)     //如果按键按下则获取信号量
    {
      xSemaphoreGive(KEY_Binary_SemHandle);    //释放掉信号量
    }
    else
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port , LED2_Pin , GPIO_PIN_SET);    
    }

    osDelay(1);
  }
  /* USER CODE END Start_Detect_KEY_Task */
}

/* USER CODE BEGIN Header_Start_Blink_LED_Task */
/**
* @brief Function implementing the Blink_LED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Blink_LED_Task */
void Start_Blink_LED_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Blink_LED_Task */
  /* Infinite loop */
  for(;;)
  {

    BaseType_t Binary_Sem = 0; 

    if(KEY_Binary_SemHandle != NULL)    //信号量不为空
    {
      Binary_Sem =  xSemaphoreTake(KEY_Binary_SemHandle , portMAX_DELAY);          //获取信号

      if(Binary_Sem == pdTRUE)      //如果获取到信号量
      {
        HAL_GPIO_WritePin(LED2_GPIO_Port , LED2_Pin , GPIO_PIN_RESET);    //点亮LED
      }
    }
    
    osDelay(1);
  }
  /* USER CODE END Start_Blink_LED_Task */
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_LCD_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
