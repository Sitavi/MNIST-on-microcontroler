/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "stdio.h"
#include "HorombeRGB565.h"
#include "train-bord-de-mer_0.h"
#include <string.h>
#include <stdlib.h>
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

/* USER CODE BEGIN PV */

//minicom -D /dev/ttyACM0

uint32_t potl,potr,joystick_h,joystick_v,bp1,bp2;
TS_StateTypeDef  TS_State;
ADC_ChannelConfTypeDef sConfig = {0};

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;
uint32_t date_time_format = RTC_FORMAT_BIN;
uint32_t drawing_color = 0;


GPIO_TypeDef* LED_PORT[8] = {
		LED18_GPIO_Port,
		LED17_GPIO_Port,
		LED16_GPIO_Port,
		LED15_GPIO_Port,
		LED14_GPIO_Port,
		LED13_GPIO_Port,
		LED12_GPIO_Port,
		LED11_GPIO_Port
	};

uint16_t LED_PIN[8] = {
		LED18_Pin,
		LED17_Pin,
		LED16_Pin,
		LED15_Pin,
		LED14_Pin,
		LED13_Pin,
		LED12_Pin,
		LED11_Pin
	};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void update_inputs(void);
void afficher_leds(uint16_t adc_value);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	char text[60]={};
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

	typedef enum {
	    ETAT_INIT,
		ETAT_1,
		ETAT_2
	} Etat_t;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_UART7_Init();
  /* USER CODE BEGIN 2 */
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+ BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4);
  BSP_LCD_DisplayOn();
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(LCD_COLOR_RED);
  BSP_LCD_DrawBitmap(0,0,(uint8_t*)train_bord_de_mer_0_bmp);
  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(00);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
  BSP_LCD_SetBackColor(00);

  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());


  Etat_t etat = ETAT_INIT;



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  switch(etat){

	  case ETAT_INIT:
		// Send
		sprintf(text,"Enter hours:\n\r");
		HAL_UART_Transmit(&huart1, (uint8_t*)text, strlen((char*)text), 100);

		// Init hours
		uint8_t hours[3];
		if(HAL_UART_Receive(&huart1, hours, 2, 30000)==HAL_OK){
		  hours[3] = '\0'; // null-terminate
		};
		sTime.Hours = atoi((char*)hours);

		// Send
		sprintf(text,"Enter minutes:\n\r");
		HAL_UART_Transmit(&huart1, (uint8_t*)text, strlen((char*)text), 100);

		// Init minutes
		uint8_t minutes[3];
		if(HAL_UART_Receive(&huart1, minutes, 2, 30000)==HAL_OK){
		  minutes[3] = '\0'; // null-terminate
		};
		sTime.Minutes = atoi((char*)minutes);

		sTime.Seconds = 0;

		HAL_RTC_SetTime(&hrtc, &sTime, date_time_format);

	  	update_inputs();

	  	etat = ETAT_1;
		break;

	  case ETAT_1:
		  update_inputs();

		  // Send
			HAL_RTC_GetDate(&hrtc, &sDate, date_time_format);
			HAL_RTC_GetTime(&hrtc, &sTime, date_time_format);

			sprintf(text,"Time : %d:%d:%d",sTime.Hours, sTime.Minutes, sTime.Seconds);
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_DisplayStringAtLine(1,(uint8_t*) text);
			sprintf(text,"Time : %d:%d:%d\n\r",sTime.Hours, sTime.Minutes, sTime.Seconds);
			HAL_UART_Transmit(&huart1, (uint8_t*)text, strlen((char*)text), 100);

			 afficher_leds(joystick_v);

			  if(!(bp1 || bp2)){
				  BSP_LCD_SelectLayer(0);
				  BSP_LCD_DrawBitmap(0,0,(uint8_t*)train_bord_de_mer_0_bmp);
				  BSP_LCD_SelectLayer(1);
			  }

			  if(TS_State.touchDetected){
				  //On vérifie que le cercle de rayon 4 dessiné autour du point sera bien sur l'écran
				  if((TS_State.touchX[0]>4) && (TS_State.touchX[0]<476) && (TS_State.touchY[0]>4) && (TS_State.touchY[0]<268)){
					  BSP_LCD_SelectLayer(0);
					  BSP_LCD_SetTextColor(~drawing_color);
					  BSP_LCD_FillEllipse(TS_State.touchX[0],TS_State.touchY[0], 4, 6);
					  BSP_LCD_SetTextColor(drawing_color);
					  BSP_LCD_FillCircle(TS_State.touchX[0],TS_State.touchY[0],2);
					  BSP_LCD_SelectLayer(1);
					  drawing_color+=10000;
				  }
			  }

		  if(TS_State.touchDetected && TS_State.touchX[0]>240){
			  BSP_LCD_Clear(00);
			  etat = ETAT_2;
		  }
		  break;

	  case ETAT_2:
		  update_inputs();

		  // Send
//		  HAL_GPIO_WritePin(LED13_GPIO_Port,LED13_Pin,bp1);
//		  HAL_GPIO_WritePin(LED14_GPIO_Port,LED14_Pin,bp2);
		  sprintf(text,"BP1 : %d BP2 : %d",bp1,bp2);
		  BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
		  BSP_LCD_DisplayStringAtLine(1,(uint8_t*) text);

		  sprintf(text,"POTL : %4u POTR : %4u",(uint16_t)potl,(uint16_t)potr);
		  BSP_LCD_SetTextColor(LCD_COLOR_RED);
		  BSP_LCD_DisplayStringAtLine(2,(uint8_t*) text);

		  sprintf(text,"joy_v : %4u joy_h : %4u",(uint16_t)joystick_v,(uint16_t)joystick_h);
		  BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
		  BSP_LCD_DisplayStringAtLine(3,(uint8_t*) text);

		  if(!(bp1 || bp2)){
			  BSP_LCD_SelectLayer(0);
			  BSP_LCD_DrawBitmap(0,0,(uint8_t*)train_bord_de_mer_0_bmp);
			  BSP_LCD_SelectLayer(1);
		  }

		  afficher_leds(joystick_h);

		  if(TS_State.touchDetected){
			  //On vérifie que le cercle de rayon 4 dessiné autour du point sera bien sur l'écran
			  if((TS_State.touchX[0]>4) && (TS_State.touchX[0]<476) && (TS_State.touchY[0]>4) && (TS_State.touchY[0]<268)){
				  BSP_LCD_SelectLayer(0);
				  BSP_LCD_SetTextColor(~drawing_color);
				  BSP_LCD_FillEllipse(TS_State.touchX[0],TS_State.touchY[0], 6, 4);
				  BSP_LCD_SetTextColor(drawing_color);
				  BSP_LCD_FillCircle(TS_State.touchX[0],TS_State.touchY[0],2);
				  BSP_LCD_SelectLayer(1);
				  drawing_color+=10000;
			  }
		  }

		  if(TS_State.touchDetected && TS_State.touchX[0]<241){
			  BSP_LCD_Clear(00);
			  etat = ETAT_1;
			}
		  break;
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void update_inputs(void){
	// Lecture des boutons
	bp1 = HAL_GPIO_ReadPin(BP1_GPIO_Port,BP1_Pin);
	bp2 = HAL_GPIO_ReadPin(BP2_GPIO_Port,BP2_Pin);

	// Lecture des entrées-sorties analogiques
	sConfig.Channel = ADC_CHANNEL_6;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	HAL_ADC_Start(&hadc3);
	while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
	potr = HAL_ADC_GetValue(&hadc3);

	sConfig.Channel = ADC_CHANNEL_7;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	HAL_ADC_Start(&hadc3);
	while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
	potl = HAL_ADC_GetValue(&hadc3);

	sConfig.Channel = ADC_CHANNEL_8;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
	HAL_ADC_Start(&hadc3);
	while(HAL_ADC_PollForConversion(&hadc3, 100)!=HAL_OK);
	joystick_v = HAL_ADC_GetValue(&hadc3);

	HAL_ADC_Start(&hadc1);
	while(HAL_ADC_PollForConversion(&hadc1, 100)!=HAL_OK);
	joystick_h = HAL_ADC_GetValue(&hadc1);

	// Lecture de l'écran tactile
	BSP_TS_GetState(&TS_State);
}

void afficher_leds(uint16_t adc_value)
{
    uint8_t nb_leds;
    uint8_t i;

    nb_leds = (adc_value * 8) / 4096 + 1;  // 0..12

    if (nb_leds > 8)
        nb_leds = 8;

    for (i = 0; i < 8; i++) {
        if (i < nb_leds)
            HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(LED_PORT[i], LED_PIN[i], GPIO_PIN_RESET);
    }
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
