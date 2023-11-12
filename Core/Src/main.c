/*
1:31:07 AM 10/19/2023

CAN transmission works on reset, but after one send it doesn't anymore.
So got it working with my code rn but only partly
find a way to make it loop
*/
#include "main.h"
#include "stdio.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

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
CAN_HandleTypeDef hcan1;
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
void CAN1_Tx(uint8_t data);
uint8_t CAN1_Rx(void);
uint8_t k = 0;
uint8_t rec = 0;
/* USER CODE BEGIN PFP */

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
  // MX_CAN1_Init();
  // for(;;){
  //   CAN1_Tx(k);
  //   rec = CAN1_Rx();
  //   k += 1;
  //   if (k>25) {
  //     k = 0;
  //   }
  //   HAL_Delay(1000);
  // }
  // return 0;
  HAL_Delay(3000);
  MX_USB_DEVICE_Init();
  MX_GPIO_Init();
  for (;;) {
    HAL_Delay(1000);
    int startTime = HAL_GetTick();
    int numSamples = 0;
    while (numSamples <= 20000) {
      HAL_ADC_Start(&hadc1);
      HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
      uint32_t readADC = HAL_ADC_GetValue(&hadc1);
      char txBuf[64];
      snprintf(txBuf, sizeof(txBuf), "%.6lu\r\n", readADC);
      CDC_Transmit_FS((uint8_t *)txBuf, strlen(txBuf));
      numSamples++;
    }
    int timeTook = HAL_GetTick() - startTime;
    char buf[64];
    snprintf(buf, sizeof(buf), "Took %d milliseconds to get %d samples\r\n", timeTook, numSamples);
    CDC_Transmit_FS((uint8_t *)buf, strlen(buf));
  }
  /* USER CODE END 3 */
}

void CAN1_Tx(uint8_t data) {
  CAN1->sTxMailBox[0].TDLR = data;
  CAN1->sTxMailBox[0].TIR |= 1;
}

uint8_t CAN1_Rx(){
    // Monitoring FIFO 0 message pending bits FMP0[1:0]
    while(!(CAN1->RF0R & 3)){}

    // Read the data from the FIFO 0 mailbox from Mailbox data low register
    uint8_t RxD = (CAN1->sFIFOMailBox[0].RDLR) & 0xFF;
    rec = RxD;

    // Releasing FIFO 0 output mailbox
    CAN1->RF0R |= 1<<5;

    return RxD;
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{
  /* 1. Setting Up the Baud Rate and Configuring CAN1 in
    * Loop Back Mode -------------------------------------------------------*/

  // Enable clock for CAN1
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

  // Entering CAN Initialization Mode and wait for acknowledgment
  CAN1->MCR |= CAN_MCR_INRQ;
  while (!(CAN1->MSR & CAN_MSR_INAK)){}

  //Set Loop back mode for CAN1
  CAN1->BTR |= CAN_BTR_LBKM;

  //Setting the Re synchronization jump width to 1
  CAN1->BTR &= ~CAN_BTR_SJW;

  //Setting the no. of time quanta for Time segment 2
  // TS2 = 4-1;
  CAN1->BTR &= ~(CAN_BTR_TS2);
  CAN1->BTR |= (CAN_BTR_TS2_1 | CAN_BTR_TS2_0);

  //Setting the no. of time quanta for Time segment 1
  // TS1 = 3-1;
  CAN1->BTR &= ~(CAN_BTR_TS1);
  CAN1->BTR |= (CAN_BTR_TS1_1);

  //Setting the Baud rate Pre-scalar for CAN1
  // BRP[9:0] = 16-1
  CAN1->BTR |= ((16-1)<<0);

  // Exit the Initialization mode for CAN1
  // Wait until the INAK bit is cleared by hardware
  CAN1->MCR &= ~CAN_MCR_INRQ;
  while (CAN1->MSR & CAN_MSR_INAK){}

  //Exit Sleep Mode
  CAN1->MCR &= ~CAN_MCR_SLEEP;
  while (CAN1->MSR & CAN_MSR_SLAK){}

  /* 2. Setting up the Transmission----------------------------------------*/

  CAN1->sTxMailBox[0].TIR = 0;

  //Setting up the Std. ID
  CAN1->sTxMailBox[0].TIR = (0x245<<21);
  CAN1->sTxMailBox[0].TDHR = 0;

  // Setting Data Length to 1 Byte.
  CAN1->sTxMailBox[0].TDTR = 1;

  /* 3. Configuring the Filters--------------------------------------------*/

  //Enter Filter Initialization mode to configure the filters
  CAN1->FMR |= CAN_FMR_FINIT;

  // Configuring the Number of Filters Reserved for CAN1
  // and also the start bank for CAN2
  CAN1->FMR |= 14<<8;

  // Select the single 32-bit scale configuration
  CAN1->FS1R |= CAN_FS1R_FSC13;

  // Set the receive ID
  CAN1->sFilterRegister[13].FR1 = 0x245<<21;

  // Select Identifier List mode
  CAN1->FM1R |= CAN_FM1R_FBM13;

  //Activating filter 14
  CAN1->FA1R |= CAN_FA1R_FACT13;

  //Exit filter Initialization Mode
  CAN1->FMR &= ~CAN_FMR_FINIT;
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER |= (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1);

  GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL8_3 | GPIO_AFRH_AFSEL8_0 |
                    GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_3);

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
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
