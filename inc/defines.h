/**
 *  Defines for your entire project at one place
 * 
 *	@author 	Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.com
 *	@version 	v1.0
 *	@ide		Keil uVision 5
 *	@license	GNU GPL v3
 *	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#ifndef TM_DEFINES_H
#define TM_DEFINES_H
#define TM_DISCO_STM32F4_DISCOVERY

/* Use SPI communication with SDCard */
#define	FATFS_USE_SDIO				0

/* Select your SPI settings */
#define FATFS_SPI				SPI1
#define FATFS_SPI_PINSPACK		TM_SPI_PinsPack_1

/* Custom CS pin for SPI communication */
#define FATFS_CS_PORT		GPIOB
#define FATFS_CS_PIN		GPIO_Pin_5

/* Use detect pin */
#define FATFS_USE_DETECT_PIN			1
/* Use writeprotect pin */
#define FATFS_USE_WRITEPROTECT_PIN		1

/* If you want to overwrite default CD pin, then change this settings */
#define FATFS_USE_DETECT_PIN_PORT		GPIOB
#define FATFS_USE_DETECT_PIN_PIN		GPIO_PIN_6

/* If you want to overwrite default WP pin, then change this settings */
#define FATFS_USE_WRITEPROTECT_PIN_PORT		GPIOB
#define FATFS_USE_WRITEPROTECT_PIN_PIN		GPIO_PIN_7

/* Put your global defines for all libraries here used in your project */
/* Control pins */
/* RS - Register select pin */
#define HD44780_RS_PORT			GPIOB
#define HD44780_RS_PIN			GPIO_Pin_2
/* E - Enable pin */
#define HD44780_E_PORT			GPIOB
#define HD44780_E_PIN			GPIO_Pin_7

/* D4 - Data 4 pin */
#define HD44780_D4_PORT			GPIOC
#define HD44780_D4_PIN			GPIO_Pin_12
/* D5 - Data 5 pin */
#define HD44780_D5_PORT			GPIOC
#define HD44780_D5_PIN			GPIO_Pin_13
/* D6 - Data 6 pin */
#define HD44780_D6_PORT			GPIOB
#define HD44780_D6_PIN			GPIO_Pin_12
/* D7 - Data 7 pin */
#define HD44780_D7_PORT			GPIOB
#define HD44780_D7_PIN			GPIO_Pin_10

#endif
