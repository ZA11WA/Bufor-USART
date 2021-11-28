/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

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

/* USER CODE BEGIN PV */

    /*BUFOR KOłOWY*/
#define USART_TXBUF_LEN 1512
#define USART_RXBUF_LEN 512
uint8_t USART_TxBuf[USART_TXBUF_LEN];
uint8_t USART_RxBuf[USART_RXBUF_LEN];

__IO int USART_TX_Empty=0;
__IO int USART_TX_Busy=0;
__IO int USART_RX_Empty=0;
__IO int USART_RX_Busy=0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t USART_kbhit(){
	if(USART_RX_Empty==USART_RX_Busy){
		return 0;
	}else{
		return 1;
	}
}//USART_kbhit
/*Funkcja pobierająca znak do bufora*/
uint8_t USART_getchar(){
uint8_t tmp;
	if(USART_RX_Empty!=USART_RX_Busy){
		tmp=USART_RxBuf[USART_RX_Busy];
		USART_RX_Busy++;
		if(USART_RX_Busy >= USART_RXBUF_LEN)USART_RX_Busy=0;
		return tmp;
	}else return 0;
}//USART_getchar

uint8_t USART_getline(char *buf){
static uint8_t bf[128];
static uint8_t idx=0;
int i;
uint8_t ret;
	while(USART_kbhit()){
		bf[idx]=USART_getchar();
		if(((bf[idx]==10)||(bf[idx]==13))){
			bf[idx]=0;
			for(i=0;i<=idx;i++){
				buf[i]=bf[i];//przekopiuj do bufora
			}
			ret=idx;
			idx=0;
			return ret;// odebrano linie
		}else{//jesli tekst dluzszy to zawijamy
			idx++;
			if(idx>=128)idx=0;
		}
	}
	return 0;
}//USART_getline
void Send(char* format, ...)
{
	char tmp_rs[128]; 					//tablica pomocnicza
	int i; 								//zmienna pomocnicza
	__IO int idx; 						//zmienna pomocnicza

	va_list arglist; 					//zmienna do obsługi formatowania argumentów przekazywanych w funkcji
	va_start(arglist,format);
	vsprintf(tmp_rs,format,arglist); 	//złożenie łańcucha z argumentami i przypisanie do tablicy pomocniczej
	va_end(arglist);

	idx=USART_TX_Empty; 				//przypisanie do zmiennej pomocniczej wskaźnika empty z bufora wysyłania

	for(i=0;i<strlen(tmp_rs);i++) 		//przejście po elementach tablicy pomocniczej, które nie są puste
	{
		USART_TxBuf[idx]=tmp_rs[i]; 	//przypisanie elementowi bufora wysyłania wartości elementu z tablicy pomocniczej
		idx++; 										//zwiększenie zmiennej pomocniczej, wskazującej na element bufora wysyłania
		if(idx >= USART_TXBUF_LEN) idx=0; 			//jeżeli zmienna wyszła poza zakres bufora przesunięcie jej na początek
	}
	__disable_irq(); 							//wyłączenie zapytań o przerwania

	if((USART_TX_Empty==USART_TX_Busy)&&(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TXE)==SET)) 	//jeżeli wskaźniki empty i busy są sobie
																						//równe oraz flaga bufora wysyłania jest ustawiona
	{
		USART_TX_Empty=idx; 						//ustawienie zmiennej pomocniczej jako wskaźnik empty bufora wysyłania
		uint8_t tmp=USART_TxBuf[USART_TX_Busy]; 				//przypisanie do zmiennej pomocniczej elementu, który ma zostać wysłany
		USART_TX_Busy++;							 				//zwiększenie wskaźnika busy
		if(USART_TX_Busy >= USART_TXBUF_LEN) USART_TX_Busy=0; 			//jeżeli wskaźnik busy wychodzi poza rozmiar bufora przesunięcie go na początek
		HAL_UART_Transmit_IT(&huart2, &tmp, 1); 					//wysłanie elementu przypisanego do zmiennej pomocniczej
	}
	else
	{
		USART_TX_Empty=idx; 				//ustawienie zmiennej pomocniczej jako wskaeaźnik empty bufora wysyłania
	}
	__enable_irq(); 					//włączenie zapytań o przerwania
}
/*Usart Callback Nadawanie*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart==&huart2){
		if(USART_TX_Empty!=USART_TX_Busy){
			uint8_t tmp = USART_TxBuf[USART_TX_Busy];
			USART_TX_Busy++;
			if(USART_TX_Busy>=USART_TXBUF_LEN) USART_TX_Busy=0;
			HAL_UART_Transmit_IT(&huart2,&tmp,1);
		}
	}
}
/*Usart Callback Odbiór*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart==&huart2){
		USART_RX_Empty++;
		if(USART_RX_Empty>=USART_RXBUF_LEN)USART_RX_Empty=0;
		HAL_UART_Receive_IT(&huart2,&USART_RxBuf[USART_RX_Empty],1);
	}
}

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UART_Receive_IT(&huart2,&USART_RxBuf[USART_RX_Empty],1);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
