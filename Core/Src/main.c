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
#include "mnist_nn.h"
#include <stdio.h>
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void preparer_fond(uint16_t largeur, uint16_t hauteur);
void effacer_texte(void);
void afficher_probabilites(const float proba[MNIST_CLASSES], uint16_t x_zone_texte);
void recuperer_image_mnist(uint8_t image[MNIST_PIXELS], uint16_t cote_zone_dessin);

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
  TS_StateTypeDef TS_State;
  GPIO_PinState bp1;
  GPIO_PinState bp1_avant;
  uint8_t image_mnist[MNIST_PIXELS];
  float probabilites[MNIST_CLASSES];
  uint16_t largeur_ecran;
  uint16_t hauteur_ecran;
  uint16_t cote_zone_dessin;
  uint16_t x_zone_texte;
  uint16_t x;
  uint16_t y;
  uint8_t touche_dessin = 0;
  const uint16_t rayon = 7;

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
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  largeur_ecran = BSP_LCD_GetXSize();
  hauteur_ecran = BSP_LCD_GetYSize();

  // À gauche on garde un carré noir pour dessiner.
  // Le reste de l'écran sert à afficher les probabilités.
  cote_zone_dessin = hauteur_ecran;
  x_zone_texte = cote_zone_dessin;

  preparer_fond(largeur_ecran, hauteur_ecran);
  effacer_texte();
  bp1_avant = HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    bp1 = HAL_GPIO_ReadPin(BP1_GPIO_Port, BP1_Pin);

    // BP1 efface le dessin et le texte.
    if (bp1 == GPIO_PIN_SET && bp1_avant == GPIO_PIN_RESET)
    {
      preparer_fond(largeur_ecran, hauteur_ecran);
      effacer_texte();
      touche_dessin = 0;
    }
    bp1_avant = bp1;

    BSP_TS_GetState(&TS_State);

    if (TS_State.touchDetected)
    {
      // Tant que le doigt est posé, on dessine seulement.
      if (TS_State.touchX[0] < cote_zone_dessin && TS_State.touchY[0] < hauteur_ecran)
      {
        touche_dessin = 1;
        x = TS_State.touchX[0];
        y = TS_State.touchY[0];

        if (x < rayon)
        {
          x = rayon;
        }
        if (x > cote_zone_dessin - rayon - 1)
        {
          x = cote_zone_dessin - rayon - 1;
        }
        if (y < rayon)
        {
          y = rayon;
        }
        if (y > hauteur_ecran - rayon - 1)
        {
          y = hauteur_ecran - rayon - 1;
        }

        BSP_LCD_SelectLayer(0);
        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        BSP_LCD_FillCircle(x, y, rayon);
      }
    }
    else if (touche_dessin)
    {
      // Quand le doigt est relevé, on transforme le dessin en image 28x28
      // puis on lance l'inférence.
      recuperer_image_mnist(image_mnist, cote_zone_dessin);
      mnist_predict_proba(image_mnist, probabilites);
      afficher_probabilites(probabilites, x_zone_texte);
      touche_dessin = 0;
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

void preparer_fond(uint16_t largeur, uint16_t hauteur)
{
  uint16_t cote_zone_dessin = hauteur;

  // La couche 0 contient le fond et le dessin.
  BSP_LCD_SelectLayer(0);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(0, 0, cote_zone_dessin, hauteur);

  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillRect(cote_zone_dessin, 0, largeur - cote_zone_dessin, hauteur);

  BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_FillRect(cote_zone_dessin, 0, 2, hauteur);
}

void effacer_texte(void)
{
  // La couche 1 sert juste pour le texte à droite.
  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0);
  BSP_LCD_SetBackColor(0);
  BSP_LCD_SetFont(&Font12);
}

void afficher_probabilites(const float proba[MNIST_CLASSES], uint16_t x_zone_texte)
{
  char texte[40];
  uint8_t i;
  uint8_t i_max = 0;
  uint16_t y;
  uint16_t pourcent_dixieme;

  for (i = 1; i < MNIST_CLASSES; i++)
  {
    if (proba[i] > proba[i_max])
    {
      i_max = i;
    }
  }

  effacer_texte();

  for (i = 0; i < MNIST_CLASSES; i++)
  {
    // On affiche une ligne par chiffre.
    y = 8 + i * 24;
    pourcent_dixieme = (uint16_t)(1000.0f * proba[i] + 0.5f);

    if (i == i_max)
    {
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      sprintf(texte, "> %d : %3u.%1u %%", i, pourcent_dixieme / 10, pourcent_dixieme % 10);
    }
    else
    {
      BSP_LCD_SetFont(&Font12);
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      sprintf(texte, "  %d : %3u.%1u %%", i, pourcent_dixieme / 10, pourcent_dixieme % 10);
    }

    BSP_LCD_DisplayStringAt(x_zone_texte + 10, y, (uint8_t *)texte, LEFT_MODE);
  }
}

void recuperer_image_mnist(uint8_t image[MNIST_PIXELS], uint16_t cote_zone_dessin)
{
  uint16_t x_debut;
  uint16_t x_fin;
  uint16_t y_debut;
  uint16_t y_fin;
  uint16_t x;
  uint16_t y;
  uint16_t ligne;
  uint16_t colonne;
  uint16_t somme;
  uint32_t nb_pixels_blancs;
  uint32_t nb_pixels_case;
  uint32_t pixel;

  // On relit ce qui a été dessiné sur la couche 0.
  BSP_LCD_SelectLayer(0);

  for (ligne = 0; ligne < MNIST_H; ligne++)
  {
    y_debut = (ligne * cote_zone_dessin) / MNIST_H;
    y_fin = ((ligne + 1) * cote_zone_dessin) / MNIST_H;

    for (colonne = 0; colonne < MNIST_W; colonne++)
    {
      x_debut = (colonne * cote_zone_dessin) / MNIST_W;
      x_fin = ((colonne + 1) * cote_zone_dessin) / MNIST_W;
      nb_pixels_blancs = 0;
      nb_pixels_case = (uint32_t)(x_fin - x_debut) * (uint32_t)(y_fin - y_debut);

      // On compte combien de pixels blancs sont presents dans chaque case.
      for (y = y_debut; y < y_fin; y++)
      {
        for (x = x_debut; x < x_fin; x++)
        {
          pixel = BSP_LCD_ReadPixel(x, y);
          somme = ((pixel >> 16) & 0xFF) + ((pixel >> 8) & 0xFF) + (pixel & 0xFF);

          if (somme > 0)
          {
            nb_pixels_blancs++;
          }
        }
      }

      image[ligne * MNIST_W + colonne] =
          (uint8_t)((255u * nb_pixels_blancs + nb_pixels_case / 2u) / nb_pixels_case);
    }
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
