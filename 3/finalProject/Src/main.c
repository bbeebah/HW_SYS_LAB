/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2559 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
 uint8_t receive;
// ------ var accelerometer ------ \\

 	 int8_t x,y,z;
	char xx[20],yy[20],zz[20];
// ------ var sound ------ \\

	uint8_t comSound[2];
	uint8_t play[2];
	uint16_t Istr[1];
	uint8_t key[] = { 0x0F,0x1F,0x2F,0x3F,0x4F,0x5F,0x6F,0x7F,0x8F,0x9F,0xAF,0xBF,0xCF,0xDF,0xEF,0xFF }; /*C D E F G A B*/
	uint8_t song1[] = {4,3,2,3,4,4,4,3,3,3,4,6,6,4,3,2,3,4,4,4,3,3,4,3,2}; // nu-ma-lee
	int ontime1[] = {3000, 1000, 2000, 2000, 2000, 2000, 4000, 2000, 2000, 4000, 2000, 2000, 2000,
			3000, 1000, 2000, 2000, 2000, 2000, 4000, 2000, 2000, 4000, 2000, 4000};
	uint8_t song2[]=  {1,1,1,2,3,3,3,2,1,2,3,1,3,3,3,4,5,5,4,3,4,5,3,1,1,1,2,3,3,3,2,1,2,3,1,1,1,1,2,3,3,3,3,1,2,3,1};// happybirthday

// ------ var mic ------- \\

	float float_abs(float in){
		return in < 0 ? -in : in;
	}

	#define PDM_BUFFER_SIZE 20
	#define PCM_BUFFER_SIZE 2500
	#define LEAKY_KEEP_RATE 0.95
	#define UART_DEBUG_TICK_RATE 100
	#define PDM_BLOCK_SIZE_BITS 16

	/* Microphone stuffs */

	uint16_t pdm_buffer[PDM_BUFFER_SIZE]; // Buffer for pdm value from hi2s2 (Mic)
	uint16_t pdm_value=0;
	uint8_t  pcm_value=0;                 // For keeping pcm value calculated from pdm_value
	                                      // value range is 0-16, 8-bit is chosen because it
	                                      // can store 0-255
	uint16_t pcm_count = 0;

	char uart_temp_display_buffer[100];

	float leaky_pcm_buffer = 0.0;         // Fast Estimation of moving average of PDM
	float leaky_amp_buffer = 0.0;         // Fast Estimation of moving average of abs(PCM)

	double pcm_square = 0;
	float max_amp = 0;
	int currentMicrophoneAmp = 0;
	int maxMicrophoneAmp = 0;

// ------ new game ------ \\

	#define STATUS_INIT  0
	#define STATUS_PLAYHARD  1
	#define STATUS_PLAYEASY  2
	#define STATUS_END  3
	int status = STATUS_INIT; // 0 - initial :: 1 - playing hard :: 2 - playing easy :: 3 - end game
	int score = 0;
	char** gameBoardh = {"\t<<==\t|\t\t|\t\t|\t\t|\t\n\r",
			"\t\t|\t<=\t|\t\t|\t\t|\t\n\r",
			"\t\t|\t\t|\t==\t|\t\t|\t\n\r",
			"\t\t|\t\t|\t\t|\t=>\t|\t\n\r",
			"\t\t|\t\t|\t\t|\t\t|\t==>>\n\r",};
	char** gameBoarde = {"\t<=\t|\t\t|\t\n\r",
			"\t\t|\t==\t|\t\n\r",
			"\t\t|\t\t|\t=>\n\r",};

//	int status = 5;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void playSound(int r, char c){
	if(r==100){
		if(c=='e'){
			// Off
			comSound[0] = 0x1E;
			comSound[1] = 0x20;
			HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

			// Change note

			play[0] = 0x1C;
			int  i;
			for(i=0; i<sizeof(song1); i++)
			{
				play[1] = key[song1[i]];
				HAL_I2C_Master_Transmit (&hi2c1, 0x94, play, 2, 50 );

				// On
				comSound[0] = 0x1E;
				comSound[1] = 0xE0;
				HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

				int j;
				for(j=0;j<ontime1[i];j++)
					HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );
			}
			return;
		}
		else if(c=='h'){
			// Off
			comSound[0] = 0x1E;
			comSound[1] = 0x20;
			HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

			// Change note

			play[0] = 0x1C;
			int  i;
			for(i=0; i<sizeof(song1); i++)
			{
				play[1] = key[song2[i]];
				HAL_I2C_Master_Transmit (&hi2c1, 0x94, play, 2, 50 );

				// On
				comSound[0] = 0x1E;
				comSound[1] = 0xE0;
				HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

				int ontime;
				if(r==4||r==11||r==16||r==17||r==22||r==27||r==34||r==39||r==46)ontime=6000;
				else ontime=4000;
				int j;
				for(j=0;j<ontime;j++)
					HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );
			}
			return;
		}
	}
	if(c=='e'){
		// Off
		comSound[0] = 0x1E;
		comSound[1] = 0x20;
		HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

		// Change note

		play[0] = 0x1C;
		play[1] = key[song1[r]];
		HAL_I2C_Master_Transmit (&hi2c1, 0x94, play, 2, 50 );

		// On
		comSound[0] = 0x1E;
		comSound[1] = 0xE0;
		HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

		int i;
		for(i=0;i<ontime1[r];i++)HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );

	}else if(c == 'h'){
		// Off
		comSound[0] = 0x1E;
		comSound[1] = 0x20;
		HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

		// Change note

		play[0] = 0x1C;
		play[1] = key[song2[r]];
		HAL_I2C_Master_Transmit (&hi2c1, 0x94, play, 2, 50 );

		// On
		comSound[0] = 0x1E;
		comSound[1] = 0xE0;
		HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

		int ontime;
		if(r==4||r==11||r==16||r==17||r==22||r==27||r==34||r==39||r==46)ontime=6000;
		else ontime=4000;
		int j;
		for(j=0;j<ontime;j++)
			HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );

	}
	/*
	if(r == 100)
			{
				// Off
					comSound[0] = 0x1E;
					comSound[1] = 0x20;
					HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

					// Change note

					play[0] = 0x1C;
				int  i;
				for(i=0; i<sizeof(song1); i++)
				{
					play[1] = key[song1[i]];
						HAL_I2C_Master_Transmit (&hi2c1, 0x94, play, 2, 50 );

						// On
						comSound[0] = 0x1E;
						comSound[1] = 0xE0;
						HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

						int j;
						for(j=0;j<ontime1[i];j++)
							HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );
				}
			}
	if(r < 0 || r > 16) return;
	// Off
	comSound[0] = 0x1E;
	comSound[1] = 0x20;
	HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	// Change note

	comSound[0] = 0x1C;
	comSound[1] = key[r];
	HAL_I2C_Master_Transmit (&hi2c1, 0x94, comSound, 2, 50 );

	// On
	comSound[0] = 0x1E;
	comSound[1] = 0xE0;
	HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	int i;
	for(i=0;i<1000;i++)HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 );*/

}

void GetMicrophone(){
	uint8_t i = 0;
	HAL_I2S_Receive(&hi2s2, pdm_buffer, PDM_BUFFER_SIZE, 200); // Receive PDM from Mic

	for (i = 0; i < PDM_BUFFER_SIZE; i++) {
		pcm_value = -PDM_BLOCK_SIZE_BITS / 2;
		pdm_value = pdm_buffer[i];
		// calculate PCM value
		while (pdm_value != 0)        // while pdm_value still have 1s in binary
		{
			pcm_value++;
			pdm_value ^= pdm_value & -pdm_value; // remove left most 1 in binary
		}
		leaky_pcm_buffer += pcm_value;
		leaky_pcm_buffer *= LEAKY_KEEP_RATE;
		leaky_amp_buffer += float_abs(leaky_pcm_buffer);
		leaky_amp_buffer *= LEAKY_KEEP_RATE;
	}
	pcm_count++;
	if (max_amp < leaky_amp_buffer)
		max_amp = leaky_amp_buffer;
	pcm_square += (leaky_amp_buffer / 2500) * leaky_amp_buffer;
	if (pcm_count >= 250) {
		currentMicrophoneAmp = max_amp;
		pcm_count = 0;
		pcm_square = 0;
		max_amp = 0;
	}


}

int random_number(int min, int max){
	return (rand()%(max-min+1))+min;
}


/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_I2S3_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  //------initial accelerometer------\\

  	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_RESET);
	uint8_t address = 0x20;
	HAL_SPI_Transmit(&hspi1,&address,1,50);

	uint8_t data = 0x67;
	HAL_SPI_Transmit(&hspi1,&data,1,50);
	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_3,GPIO_PIN_SET);

  //------initial sound------\\

	Istr[0] = 0;
	  if( HAL_I2S_Transmit (&hi2s3, Istr , 0x10, 10 ) != HAL_OK) //not sure if this is needed but i just put it here
	  {   }    ;

	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, 0);  //Reset is set down

	   //confirmation LED

	  HAL_Delay(500);

	  //Initialization sequence for CS43L22:

	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, 1); //Reset is set Up (Power CS43L22)
	  HAL_Delay(500);
	  comSound[0] = 0x00;
	  comSound[1] = 0x99;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);
	  comSound[0] = 0x47;
	  comSound[1] = 0x80;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	  comSound[0] = 0x32;
	  comSound[1] = 0x80; // 0xBB or 0x80
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	  comSound[0] = 0x32;
	  comSound[1] = 0x00; // 0x3B or 0x00
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	  comSound[0] = 0x00;
	  comSound[1] = 0x00;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);


	  comSound[0] = 0x1E;
	  comSound[1] = 0xC0;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);

	  comSound[0] = 0x02;
	  comSound[1] = 0x9E;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x94, comSound, 2, 50);


	  int i;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  srand(time(NULL));
	  char a[100];

	  HAL_UART_Transmit(&huart2, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\r", 45, 1000);

		  HAL_UART_Transmit(&huart2,"--------------------------------------------------\n\r", 52, 1000);
		  HAL_UART_Transmit(&huart2,"\t\t PIANO TIRED \n\r", 17, 1000);
		  HAL_UART_Transmit(&huart2,"--------------------------------------------------\n\r", 52, 1000);
		  HAL_Delay(1500);
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  if(status == STATUS_INIT){
		  char titlebegin[] = "\rThe louder you speak , the more difficult the game will be.\n\r";
		  HAL_UART_Transmit(&huart2, &titlebegin, sizeof(titlebegin), 1000);
		  HAL_Delay(1500);
		  char worn[] = "\n\n\n    s p e a k   n o w  ! ! \n\n\r";
		  HAL_UART_Transmit(&huart2, &worn, sizeof(worn), 1000);
		  //HAL_Delay(500);
		  int micCounter = 8000;
		  while(micCounter!=0){
			  GetMicrophone();
//			  sprintf(uart_temp_display_buffer, "Loudness : %d\n\r", (int) currentMicrophoneAmp);
//			  		  		HAL_UART_Transmit(&huart2, (uint8_t*) uart_temp_display_buffer,
//			  		  				strlen(uart_temp_display_buffer), 100);
			  if(currentMicrophoneAmp > maxMicrophoneAmp){
				  maxMicrophoneAmp = currentMicrophoneAmp;
			  }
			  micCounter--;
		  }

		  if(maxMicrophoneAmp > 55000){
			  char hard[] = "\n\nLEVEL : hard  (;p) \n\r";
			  HAL_UART_Transmit(&huart2, &hard, sizeof(hard), 1000);
			  status = STATUS_PLAYHARD;
		  }else{
			  char easy[] = "\n\nLEVEL : easy  ('.') \n\r";
			  HAL_UART_Transmit(&huart2, &easy, sizeof(easy), 1000);
			  status = STATUS_PLAYEASY;
		  }

	  }else if(status == STATUS_PLAYHARD){
		  char musich[] = "music : spider \n\r";
		  HAL_UART_Transmit(&huart2, &musich, sizeof(musich), 1000);

		  char boarde[] = "**********************************************************************************\n\r";
		  HAL_UART_Transmit(&huart2, &boarde, sizeof(boarde), 200);
		  char hboarde[] = "\t<<==\t|\t<=\t|\t==\t|\t=>\t|\t==>>\n\r";
		  HAL_UART_Transmit(&huart2, &hboarde, sizeof(hboarde), 200);
		  HAL_UART_Transmit(&huart2, &boarde, sizeof(boarde), 200);
		  for(i=0; i<sizeof(song1); i++){
			  int ran=song2[i]%5;
			  if(ran == 0) HAL_UART_Transmit(&huart2,  "\t<<==\t|\t\t|\t\t|\t\t|\t\n\r", 19, 200);
			  else if(ran==1) HAL_UART_Transmit(&huart2, "\t\t|\t<=\t|\t\t|\t\t|\t\n\r", 17, 200);
			  else if(ran==2) HAL_UART_Transmit(&huart2, "\t\t|\t\t|\t==\t|\t\t|\t\n\r", 17, 200);
			  else if(ran==3) HAL_UART_Transmit(&huart2, "\t\t|\t\t|\t\t|\t=>\t|\t\n\r", 17, 200);
			  else if(ran==4) HAL_UART_Transmit(&huart2, "\t\t|\t\t|\t\t|\t\t|\t==>>\n\r", 19, 200);

			  HAL_Delay(600);
			  int accCount = 18000;
			  while(accCount!=0){
				  accCount--;
				  //HAL_UART_Transmit(&huart2, "|",1,1000);
				  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
				  address = 0x29 + 0x80;
				  HAL_SPI_Transmit(&hspi1, &address, 1, 50);
				  HAL_SPI_Receive(&hspi1, &x, 1, 50);
				  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
				  if((ran==0 && x<-55) || (ran==1 && x < -35) || (ran==2 && x > -10 && x < 10) || (ran==3 && x > 35) || (ran==4 && x>55)){

					  score++;
					  playSound(i, 'h');
					  break;
				  }
			  }


		  }
		  HAL_UART_Transmit(&huart2,"---------------------------------------------------------------------------\n\r", 77, 1000);
		  playSound(100, 'h');

		  sprintf(a, "HIT RATE : %d %\n\r",(int) (score*100/sizeof(song2)));
		  status = STATUS_END;


	  }else if(status == STATUS_PLAYEASY){
		  // song1 , ontime1
		  //HAL_Delay(1000);
		  char musice[] = "music : nu ma lee \n\r";
		  HAL_UART_Transmit(&huart2, &musice, sizeof(musice), 200);
		  char boarde[] = "***************************************************\n\r";
		  HAL_UART_Transmit(&huart2, &boarde, sizeof(boarde), 200);
		  char hboarde[] = "\t<=\t|\t==\t|\t=>\n\r";
		  HAL_UART_Transmit(&huart2, &hboarde, sizeof(hboarde), 200);
		  HAL_UART_Transmit(&huart2, &boarde, sizeof(boarde), 200);
		  for(i=0; i<sizeof(song1); i++){
			  int ran=song1[i]%3;
			  if(ran == 0) HAL_UART_Transmit(&huart2, "\t<=\t|\t\t|\t\n\r", 11, 200);
			  else if(ran==1) HAL_UART_Transmit(&huart2, "\t\t|\t==\t|\t\n\r", 11, 200);
			  else HAL_UART_Transmit(&huart2, "\t\t|\t\t|\t=>\n\r", 11, 200);
			  HAL_Delay(600);
			  int accCount = 18000;
			  while(accCount!=0){
				  accCount--;
				  //HAL_UART_Transmit(&huart2, "|",1,1000);
				  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
				  address = 0x29 + 0x80;
				  HAL_SPI_Transmit(&hspi1, &address, 1, 50);
				  HAL_SPI_Receive(&hspi1, &x, 1, 50);
				  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
				  if((ran==0 && x < -40) || (ran==1 && x > -10 && x < 10) || (ran==2 && x > 40)){

					  score++;
					  playSound(i, 'e');
					  break;
				  }
			  }


		  }
		  HAL_UART_Transmit(&huart2,"--------------------------------------------------\n\r", 52, 1000);
		  playSound(100, 'e');

		  sprintf(a, "HIT RATE : %d %\n\r",(int) (score*100/sizeof(song1)));
		  status = STATUS_END;


	  }else if(status=STATUS_END){


		  HAL_UART_Transmit(&huart2,(uint8_t*) a, strlen(a), 100);
		  HAL_UART_Transmit(&huart2,"\n-------END GAME-------\n\n\n\r",25, 100);

		  char playagain[] = "\rPlay again? [Y/N]\n\r";
		  HAL_UART_Transmit(&huart2,&playagain, sizeof(playagain), 100);
		  char in;
		  while(1){
			  if(HAL_UART_Receive(&huart2, &in, 1,100) == HAL_OK){
				  HAL_UART_Transmit(&huart2, &in, 1,100);
				  if(in == 'Y' || in == 'y') {
					  HAL_UART_Transmit(&huart2, "\n\n\n\n\n", 5,100);
					  status = STATUS_INIT;
					  break;
				  }
				  else if(in == 'N' || in == 'n') return 0;
			  }
		  }
	  }
	  else{
		  GetMicrophone();
		  if(maxMicrophoneAmp != currentMicrophoneAmp){
		  		sprintf(uart_temp_display_buffer, "Loudness : %d\n\r", (int) currentMicrophoneAmp);
		  		HAL_UART_Transmit(&huart2, (uint8_t*) uart_temp_display_buffer,
		  				strlen(uart_temp_display_buffer), 100);
		  		maxMicrophoneAmp = currentMicrophoneAmp;
		  	}
	  }
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
   RCC_OscInitStruct.HSICalibrationValue = 16;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
   RCC_OscInitStruct.PLL.PLLM = 8;
   RCC_OscInitStruct.PLL.PLLN = 168;
   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ = 7;
   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
     Error_Handler();
   }

    /**Initializes the CPU, AHB and APB busses clocks 
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

   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
   PeriphClkInitStruct.PLLI2S.PLLI2SN = 88;
   PeriphClkInitStruct.PLLI2S.PLLI2SR = 4;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
   {
     Error_Handler();
   }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 50000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* I2S2 init function */
static void MX_I2S2_Init(void)
{

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_32K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }

}

/* I2S3 init function */
static void MX_I2S3_Init(void)
{

  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s3.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1680;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PA9   ------> USB_OTG_FS_VBUS
     PA10   ------> USB_OTG_FS_ID
     PA11   ------> USB_OTG_FS_DM
     PA12   ------> USB_OTG_FS_DP
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin 
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin 
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin 
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
