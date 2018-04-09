/**
 *	Keil project example for HD44780 LCD driver
 *
 *	@author 	Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.net
 *	@ide		Keil uVision 5
 */
#include "defines.h"
#include "stm32f4xx.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_hd44780.h"
#include "ws2812b.h"
//#include "tm_stm32f4_disco.h"
//#include "tm_stm32f4_fatfs.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main()
{

	 //Rectangle for custom character
		//xxx means doesn't care, lower 5 bits are important for LCD
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
		//Initialize system
		SystemInit();

		//Initialize LCD 16 cols x 2 rows
		TM_HD44780_Init(16, 2);

		//Save custom character on location 0 in LCD
		TM_HD44780_CreateChar(0, &customChar[0]);

		TM_HD44780_Clear();
		TM_HD44780_Puts(2, 0, "DISCO PLAYER");
		Delayms(3000);
		TM_HD44780_Puts(2, 1, "Done in 90% ");


		WS2812B_Init();
				while(1)
				{
							int r = rand() % 10;
							int g = rand() % 10;
							int b = rand() % 10;
							int x = rand() % 24;
							WS2812B_SetRGB(0, x, r, g, b);
						for(int j=0; j<100000; j++){};
				}
}





/*
 * 	//Rectangle for custom character
	//xxx means doesn't care, lower 5 bits are important for LCD
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
	//Initialize system
	SystemInit();

	//Initialize LCD 20 cols x 4 rows
	TM_HD44780_Init(16, 2);

	//Save custom character on location 0 in LCD
	TM_HD44780_CreateChar(0, &customChar[0]);

	//Put string to LCD
//	TM_HD44780_Puts(0, 0, "STM32F4/29 Discovery");
//	TM_HD44780_Puts(2, 1, "20x4 HD44780 LCD");
//	TM_HD44780_Puts(0, 2, "stm32f429-\n\r       discovery.com");

	//Wait a little
	//Delayms(3000);

	//Clear LCD
	TM_HD44780_Clear();

	//Show cursor
	//TM_HD44780_CursorOn();

	//Write new text
	TM_HD44780_Puts(0, 0, "DISCO PLAYER");
	TM_HD44780_Puts(0,1, "Done in 90% ");

	//Wait a little
	//Delayms(1000);

	//Enable cursor blinking
	//TM_HD44780_BlinkOn();

	//Show custom character at x = 1, y = 2 from RAM location 0
	//TM_HD44780_PutCustom(1, 2, 0);

	//TM_HD44780_Clear();

	while (1) {

	}
 */
