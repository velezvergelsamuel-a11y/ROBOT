/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Un pin de salida: puerto + máscara */
typedef struct {
    GPIO_TypeDef *puerto;
    uint16_t      mascara;
} Salida_t;

/* Estado posible de una rueda (también sirve como índice de las tablas) */
typedef enum {
    RUEDA_ALTO = 0,
    RUEDA_FRENTE,
    RUEDA_ATRAS,
    RUEDA_ESTADOS
} EstadoRueda_t;

/* Un paso de la ruta: qué hace cada rueda y por cuánto tiempo */
typedef struct {
    EstadoRueda_t izq;
    EstadoRueda_t der;
    uint32_t      duracion_ms;
} Paso_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TIEMPO_GIRO_MS    470U
#define TIEMPO_AVANCE_MS  1500U
#define TIEMPO_RETRO_MS   1500U
#define TIEMPO_REPOSO_MS  300U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Pines de cada rueda, del bit menos significativo (A) al más significativo (C) */
static const Salida_t pines_izq[3] = {
    { PIN1_GPIO_Port, PIN1_Pin },
    { PIN2_GPIO_Port, PIN2_Pin },
    { PIN3_GPIO_Port, PIN3_Pin }
};

static const Salida_t pines_der[3] = {
    { PIN4_GPIO_Port, PIN4_Pin },
    { PIN5_GPIO_Port, PIN5_Pin },
    { PIN6_GPIO_Port, PIN6_Pin }
};

/* Código de 3 bits para cada estado: { ALTO, FRENTE, ATRAS }
   La rueda derecha va espejada, por eso FRENTE y ATRAS están intercambiados */
static const uint8_t codigo_izq[RUEDA_ESTADOS] = { 0x07U, 0x06U, 0x05U };
static const uint8_t codigo_der[RUEDA_ESTADOS] = { 0x07U, 0x05U, 0x06U };

/* Ruta completa del robot */
static const Paso_t ruta[] = {
    { RUEDA_FRENTE, RUEDA_ATRAS,  TIEMPO_GIRO_MS   },  /* giro a la derecha   */
    { RUEDA_FRENTE, RUEDA_FRENTE, TIEMPO_AVANCE_MS },  /* hacia adelante      */
    { RUEDA_ATRAS,  RUEDA_FRENTE, TIEMPO_GIRO_MS   },  /* giro a la izquierda */
    { RUEDA_ATRAS,  RUEDA_ATRAS,  TIEMPO_RETRO_MS  }   /* hacia atrás         */
};

#define NUM_PASOS  (sizeof(ruta) / sizeof(ruta[0]))

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static void escribir_rueda(const Salida_t pines[3], uint8_t codigo);
static void aplicar_estados(EstadoRueda_t izq, EstadoRueda_t der);
static void ejecutar_paso(const Paso_t *paso);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Recorre los 3 pines de una rueda y escribe el bit correspondiente */
static void escribir_rueda(const Salida_t pines[3], uint8_t codigo)
{
    for (uint8_t bit = 0; bit < 3; bit++)
    {
        GPIO_PinState nivel = ((codigo >> bit) & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(pines[bit].puerto, pines[bit].mascara, nivel);
    }
}

/* Traduce los estados de cada rueda a su código y los aplica */
static void aplicar_estados(EstadoRueda_t izq, EstadoRueda_t der)
{
    escribir_rueda(pines_izq, codigo_izq[izq]);
    escribir_rueda(pines_der, codigo_der[der]);
}

/* Ejecuta un paso de la ruta y deja el robot en reposo al terminar */
static void ejecutar_paso(const Paso_t *paso)
{
    aplicar_estados(paso->izq, paso->der);
    HAL_Delay(paso->duracion_ms);

    aplicar_estados(RUEDA_ALTO, RUEDA_ALTO);
    HAL_Delay(TIEMPO_REPOSO_MS);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  /* USER CODE BEGIN 2 */

  /* Empieza con ambas ruedas detenidas */
  aplicar_estados(RUEDA_ALTO, RUEDA_ALTO);
  HAL_Delay(500);

  /* Recorre la ruta una sola vez */
  for (uint32_t i = 0; i < NUM_PASOS; i++)
  {
      ejecutar_paso(&ruta[i]);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PIN6_GPIO_Port, PIN6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, PIN5_Pin|PIN4_Pin|PIN3_Pin|PIN2_Pin
                          |PIN1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PIN6_Pin */
  GPIO_InitStruct.Pin = PIN6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PIN6_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PIN5_Pin PIN4_Pin PIN3_Pin PIN2_Pin
                           PIN1_Pin */
  GPIO_InitStruct.Pin = PIN5_Pin|PIN4_Pin|PIN3_Pin|PIN2_Pin
                          |PIN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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