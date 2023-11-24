/* USER CODE BEGIN Header */
/**
  -24 bit i2s formatı destekliyo, normal mod CHIPEN yüksek olunca 2 üssü 17 saat nabzı kadar devam ediyo
  VDD'den beslemnezken WS ve SCK pinlerinin kullanılması uzun vadede mikrofona zarar veriyo
  mikrofon uyandıktan sonra 2 üssü 18 saat nabzı kadar çıktı vermiyo
  **64 saat nabzı kadar WS çerçevesi sürmeli veya 23 SCK saat nabzı kadar kelime tipi veri uzunluğu geçmeli.
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define SAMPLE_RATE 8000  // Adjust as needed
#define FLASH_START_ADDR 0x08000000 //F429 elkitapçığı(datasheet) sayfa 89'da burada yazan adrese ulaşılabilir

I2S_HandleTypeDef hi2s2;


void SystemClock_Config(void);
static void GPIO_Init(void);
static void MX_I2S2_Init(void);

void RecordAndSaveToFlash(void);

int main(void) {
    HAL_Init();

    SystemClock_Config();

    GPIO_Init();

    MX_I2S2_Init();

    while (1) {
        RecordAndSaveToFlash();
    }
}

void RecordAndSaveToFlash(void) {
    // Buffer for sound samples
    uint16_t soundBuffer[SAMPLE_RATE];

    // Start I2S communication
    HAL_I2S_Receive(&hi2s2, soundBuffer, SAMPLE_RATE, HAL_MAX_DELAY);

    // Save sound data to flash memory
    HAL_FLASH_Unlock();

    for (int i = 0; i < SAMPLE_RATE; ++i) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_START_ADDR + (i * 2), soundBuffer[i]);
        //i*2 ile her işlemde 2 bayt kadar alanda ilerlemiş oluyo
        //HAL_FLASH fınksiyonlarında IRQRequest ile IT arasındaki fark ne?
        //half word ile i2s tutarlı mı? Tutarlı, STMf4 lerde Halfword = 16 bit
    }

    HAL_FLASH_Lock();

}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


  __HAL_RCC_PLL_PLLM_CONFIG(8);


  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

}

static void MX_I2S2_Init(void)
{

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;


}

static void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  //Buradaki error handler ile hard fault oluşmadı ve ses kaydedilmedi
/*
  if(HAL_GPIO_Init(GPIOC, &GPIO_InitStruct) != HAL_OK){
	  Error_Handler();
  }
*/

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  //Buradaki error handler ile hard fault oluşmadı ve ses kaydedilmedi
  //Error_Handler();

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);



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
