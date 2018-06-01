#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "misc.h"
#include "delay.h"
#include "codec.h"
#include "List.h"
#include "../FatFS/ff.h"
#include <stdbool.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "tm_stm32f4_hd44780.h"
#include "ws2812b.h"
#include "delay.h"
#include "../FatFS/ff.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

FATFS fatfs;
FIL file;
u16 sample_buffer[2048];
volatile s8 num_of_switch = -1;
volatile u16 result_of_conversion = 0;
volatile u8 diode_state = 0;
volatile s8 change_song = 0;
volatile u8 error_state = 0;
volatile bool random_mode = 0;
bool pause = 0;
bool half_second = 0;
char song_name[13] = { };
int n = 0;
TCHAR long_name[32]={};

uint8_t customChar[] = {
		0x1F,	// xxx 11111
		0x11,	// xxx 10001
		0x11,	// xxx 10001
		0x11,	// xxx 10001
		0x11,	// xxx 10001
		0x11,	// xxx 10001
		0x11,	// xxx 10001
		0x1F	// xxx 11111
		};

void EXTI0_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		num_of_switch = 0;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
void EXTI9_5_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
		num_of_switch = 5;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line5);
	} else if (EXTI_GetITStatus(EXTI_Line7) != RESET) {
		num_of_switch = 7;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line7);
	} else if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
		num_of_switch = 8;
		TIM_Cmd(TIM5, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}
// TODO: ADC2 volume
//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
//	{
//		ADC_conversion();
//		Codec_VolumeCtrl(170);
//		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//	}
//}
void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {

		spin_diodes();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
void TIM5_IRQHandler(void) {
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		if (num_of_switch == 5)// next song
				{
			change_song = 1;
		} else if (num_of_switch == 7)	// pause/resume
				{
			if (pause == 0) {
				pause = 1;
				TIM_Cmd(TIM3, DISABLE);
				Codec_PauseResume(0);
				NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, ENABLE);
			} else {
				pause = 0;
				TIM_Cmd(TIM3, ENABLE);
				Codec_PauseResume(1);
				NVIC_SystemLPConfig(NVIC_LP_SLEEPONEXIT, DISABLE);
			}
		} else if (num_of_switch == 8)// previous song
				{
			change_song = -1;
		}
		num_of_switch = -1;
		TIM_Cmd(TIM5, DISABLE);
		TIM_SetCounter(TIM5, 0);
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	}
}
void JOINT_VIBRATION() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseInitTypeDef TIMER;
	/* Time base configuration */
	TIMER.TIM_Period = 8400 - 1;
	TIMER.TIM_Prescaler = 3000 - 1;
	TIMER.TIM_ClockDivision = TIM_CKD_DIV1;
	TIMER.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM5, &TIMER);
	TIM_Cmd(TIM5, DISABLE);

	// KONFIGURACJA PRZERWAN - TIMER/COUNTER
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	// numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;	// subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure);	// zapisz wypelniona strukture do rejestrow
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);	// wyczyszczenie przerwania od timera 5 (wystapilo przy konfiguracji timera)
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);// zezwolenie na przerwania od przepelnienia dla timera 5
}
void DIODES_INTERRUPT() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	// KONFIGURACJA PRZERWAN - TIMER/COUNTER
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	// numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;// priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;	// subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure);	// zapisz wypelniona strukture do rejestrow

	TIM_TimeBaseInitTypeDef TIMER_3;
	TIMER_3.TIM_Period = 48000 - 1;	// okres zliczania nie przekroczyc 2^16!
	TIMER_3.TIM_Prescaler = 1000 - 1;	// wartosc preskalera, tutaj bardzo mala
	TIMER_3.TIM_ClockDivision = TIM_CKD_DIV1;	// dzielnik zegara
	TIMER_3.TIM_CounterMode = TIM_CounterMode_Up;	// kierunek zliczania
	TIM_TimeBaseInit(TIM3, &TIMER_3);

	// UWAGA: uruchomienie zegara jest w przerwaniu
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	// wyczyszczenie przerwania od timera 3 (wystapilo przy konfiguracji timera)
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);// zezwolenie na przerwania od przepelnienia dla timera 3
}
void DIODES_init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitTypeDef DIODES;
	/* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
	DIODES.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	DIODES.GPIO_Mode = GPIO_Mode_OUT;	// tryb wyprowadzenia, wyjcie binarne
	DIODES.GPIO_OType = GPIO_OType_PP;	// wyjcie komplementarne
	DIODES.GPIO_Speed = GPIO_Speed_100MHz;	// max. V przelaczania wyprowadzen
	DIODES.GPIO_PuPd = GPIO_PuPd_NOPULL;	// brak podciagania wyprowadzenia
	GPIO_Init(GPIOD, &DIODES);
}
void spin_diodes() {

	if (random_mode == 0) {
		GPIO_ResetBits(GPIOD,
				GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	} else {
		GPIO_SetBits(GPIOD,
				GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	}

	if (diode_state == 3) {
		if (random_mode == 0) {
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
		} else {
			GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		}
		diode_state = 0;
	} else if (diode_state == 2) {
		if (random_mode == 0) {
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
		} else {
			GPIO_ResetBits(GPIOD, GPIO_Pin_13);
		}
		diode_state = 3;
	} else if (diode_state == 1) {
		if (random_mode == 0) {
			GPIO_SetBits(GPIOD, GPIO_Pin_14);
		} else {
			GPIO_ResetBits(GPIOD, GPIO_Pin_14);
		}
		diode_state = 2;
	} else if (diode_state == 0) {
		if (random_mode == 0) {
			GPIO_SetBits(GPIOD, GPIO_Pin_15);
		} else {
			GPIO_ResetBits(GPIOD, GPIO_Pin_15);
		}
		diode_state = 1;
	}
}
void BUTTON_init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	/*
	 5 - przewijanie wstecz
	 6 - start/stop
	 7 - przewijanie do przodu*/
	GPIO_InitTypeDef USER_BUTTON;

	USER_BUTTON.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8;
	USER_BUTTON.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &USER_BUTTON);
}
void INTERRUPT_init() {
	// KONFIGURACJA KONTROLERA PRZERWAN
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; // numer przerwania
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // priorytet glowny
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; // subpriorytet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // uruchom dany kanal
	NVIC_Init(&NVIC_InitStructure); // zapisz wypelniona strukture do rejestrow

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; // wybor numeru aktualnie konfigurowanej linii przerwan
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // wybor trybu - przerwanie badz zdarzenie
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // wybor zbocza, na ktore zareaguje przerwanie
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; // uruchom dana linie przerwan
	EXTI_Init(&EXTI_InitStructure); // zapisz strukture konfiguracyjna przerwan zewnetrznych do rejestrow

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	// KONFIGURACJA KONTROLERA PRZERWAN DLA SWITCH Pin_5, Pin_6, Pin_7
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	EXTI_InitStructure.EXTI_Line = EXTI_Line5 | EXTI_Line7 | EXTI_Line8;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_Init(&EXTI_InitStructure);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
}
void MY_DMA_initM2P() {
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0; // wybor kanalu DMA
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // ustalenie rodzaju transferu (memory2memory / peripheral2memory /memory2peripheral)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // tryb pracy - pojedynczy transfer badz powtarzany
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; // ustalenie priorytetu danego kanalu DMA
	DMA_InitStructure.DMA_BufferSize = 2048; // liczba danych do przeslania
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &sample_buffer; // adres zrodlowy
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) (&(SPI3->DR)); // adres docelowy
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // zezwolenie na inkrementacje adresu po kazdej przeslanej paczce danych
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // ustalenie rozmiaru przesylanych danych
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // ustalenie trybu pracy - jednorazwe przeslanie danych
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; // wylaczenie kolejki FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;

	DMA_Init(DMA1_Stream5, &DMA_InitStructure); // zapisanie wypelnionej struktury do rejestrow wybranego polaczenia DMA
	DMA_Cmd(DMA1_Stream5, ENABLE); // uruchomienie odpowiedniego polaczenia DMA

	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_Cmd(SPI3, ENABLE);
}
bool read_and_send(FRESULT fresult, int position, volatile ITStatus it_status,
		UINT read_bytes, uint32_t DMA_FLAG) {

	it_status = RESET;
	while (it_status == RESET) {
		it_status = DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG);
	}
	fresult = f_read(&file, &sample_buffer[position], 1024 * 2, &read_bytes);

	/*
	 * TODO: FFT HERE
	 * x - 20-300 Hz
	 * y - 300 - 3000 Hz
	 * z - 3000 Hz - 20KHz
	 *
	for(int j=0; j<8; j++)
		{
		WS2812B_SetRGB(0, j, x, 0, 0);
		}
	for(int j=8; j<16; j++)
			{
			WS2812B_SetRGB(0, j, 0, y,0);
			}
	for(int j=16; j<24; j++)
			{
			WS2812B_SetRGB(0, j, 0,0, z);
			}
	 */
	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG);

	if (fresult != FR_OK)
			{
		error_state = 2;
		return 0;
	}
	if (read_bytes < 1024 * 2 || change_song != 0) {
		return 0;
	}
	return 1;
}
void play_wav(struct List *song, FRESULT fresult) {
	struct List *temporary_song = song;
	UINT read_bytes;
	fresult = f_open(&file, temporary_song->file.fname, FA_READ);
	if (fresult == FR_OK) {
		fresult = f_lseek(&file, 44);
		volatile ITStatus it_status;
		change_song = 0;
		for (int i = 0; i < 13; i++) {
			song_name[i] = (char) temporary_song->file.fname[i];
		}

		I2S_Cmd(CODEC_I2S, DISABLE);

		TM_HD44780_Init(16, 2);
		TM_HD44780_Puts(0, 0, song_name);

		inits();
		WS2812B_Init();

		TIM_Cmd(TIM3, ENABLE);
		while (1) {
			if (read_and_send(fresult, 0, it_status, read_bytes, DMA_FLAG_HTIF5)
					== 0) {
				break;
			}
			if (read_and_send(fresult, 1024, it_status, read_bytes,
					DMA_FLAG_TCIF5) == 0) {
				break;
			}
		}
		diode_state = 0;
		TIM_Cmd(TIM3, DISABLE);
		fresult = f_close(&file);
		TM_HD44780_Clear();
	}
}
bool isWAV(FILINFO fileInfo) {
	int i = 0;
	for (i = 0; i < 13; i++) {
		if (fileInfo.fname[i] == '.') {
			if (fileInfo.fname[i + 1] == 'W' && fileInfo.fname[i + 2] == 'A'
					&& fileInfo.fname[i + 3] == 'V') {

					return 1;
			}
		}
	}
	return 0;
}

void inits() {
	codec_init();
	codec_ctrl_init();
	I2S_Cmd(CODEC_I2S, ENABLE);
	MY_DMA_initM2P();
	BUTTON_init();
	JOINT_VIBRATION();
	INTERRUPT_init();
	DIODES_INTERRUPT();
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}

int main(void) {
	SystemInit();
	DIODES_init();
	delay_init(80);
	SPI_SD_Init();

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	SysTick_Config(90000);

	TM_HD44780_Init(16, 2);
	TM_HD44780_CreateChar(0, &customChar[0]);
	TM_HD44780_Clear();
	TM_HD44780_Puts(2, 0, "DISCO PLAYER");
	Delayms(1000);
	TM_HD44780_Clear();
	WS2812B_Init();

	// SD CARD
	FRESULT fresult;
	DIR Dir;
	FILINFO fileInfo;

	struct List *first = 0, *last = 0, *pointer;

	disk_initialize(0);
	fresult = f_mount(&fatfs, 1, 1);
	if (fresult != FR_OK)
			{
		error_state = 1;
		TM_HD44780_Clear();
		TM_HD44780_Puts(2, 0, "CARD ERROR");
		TIM_Cmd(TIM4, ENABLE);
		for (;;) {
		}
	}
	fresult = f_opendir(&Dir, "\\");
	if (fresult != FR_OK) {
		return (fresult);
	}
	u32 number_of_songs = 0;
	for (;;) {
		fresult = f_readdir(&Dir, &fileInfo);
		if (fresult != FR_OK) {
			return (fresult);
		}
		if (!fileInfo.fname[0]) {
			break;
		}
		if (isWAV(fileInfo) == 1)
				{
			if (number_of_songs == 0) {
				first = last = add_last(last, fileInfo);
			} else {
				last = add_last(last, fileInfo);
			}
			number_of_songs++;
		}
	}
	if (first == 0)
			{
		error_state = 3;
		error_state = 3;
		TM_HD44780_Clear();

		TM_HD44780_Puts(2, 0, "NO .WAV");
		TIM_Cmd(TIM4, ENABLE);
		for (;;) {
		}
	}
	last->next = first;
	first->previous = last;
	pointer = first;

	inits();

	u32 rand_number = 0;
	u32 i_loop = 0;

	for (;;) {
		play_wav(pointer, fresult);
		if (error_state != 0) {
			break;
		}
		if (change_song >= 0)
				{
			if (random_mode == 1 && number_of_songs > 1) {
				rand_number = RNG_GetRandomNumber() % (number_of_songs - 1) + 1;
				for (i_loop = 1; i_loop <= rand_number; i_loop++) {
					pointer = pointer->next;
				}
			} else {
				pointer = pointer->next;
			}
		} else if (change_song == -1) {
			if (random_mode == 1 && number_of_songs > 1) {
				rand_number = RNG_GetRandomNumber() % (number_of_songs - 1) + 1;
				for (i_loop = 1; i_loop <= rand_number; i_loop++) {
					pointer = pointer->previous;
				}
			} else {
				pointer = pointer->previous;
			}
		}
	}
	GPIO_ResetBits(GPIOD,
			GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15
					| CODEC_RESET_PIN);

	I2S_Cmd(CODEC_I2S, DISABLE);
	TM_HD44780_Init(16, 2);
	TM_HD44780_Clear();
	TM_HD44780_Puts(0, 1, "NO CARD");

	TIM_Cmd(TIM2, DISABLE);
	TIM_Cmd(TIM4, ENABLE);
	for (;;) {
	}
	return 0;
}
