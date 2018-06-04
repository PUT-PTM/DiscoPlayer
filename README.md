# DISCO PLAYER

## Overview
Music player that plays songs from SD card with 3 channels disco lights and HD44780 display. 
## Description 
It's a music player that uses SD card to play songs. It has 3 RGB disco lights that pulse to the rythm of the song. It uses HD44780 to display the name of the song. It's made by three Poznan University of Technology's students: Robert Błaszyński, Dominik Błaszczyk and Przemysław Barłóg.
## Tools 
To make it work, we used STM32F4 board as a heart and brain of the project. Additional peripherals are display, SD card reader and buttons module. Code was written and compiled by System Workbench for STM32.
## How to run 
The easiest way to run our code is to use System Workbench. Here is how to connect everything properly:

* HD44780:
```
1 -> GND
2 -> 5V
3 -> ADC (potentiometer)
4 -> PB2
5 -> GND
6 -> PB7
[...]
11 -> PC12
12 -> PC13
13 -> PB12
14 -> PB10
15 -> 3V
16 -> GND
```

* SD Card Reader:
```
GND -> GND
3V3 -> 3V
CS -> PB11
MOSI -> PB15
SCK -> PB13
MISO -> PB14
GND -> GND
```

* Potentiometer:
```
VCC -> 5V
GND -> GND
ADC -> HD3
```

* LED:
```
DI -> PA0
5V -> AC/DC
GND -> GND
```

* Buttons:
```
G -> 3V
K6 -> PA5
K7 -> PA7
```

## How to compile 
Any compiler should do the job. We recommend using System Workbench as we did.
## Future improvements 
Our main goal for that moment is to achieve even better disco light implementation to make it work like a charm.
## Attributions
To make it all work, we used some external libraries:
* https://stm32f4-discovery.net/2014/06/library-16-interfacing-hd44780-lcd-controller-with-stm32f4/
* https://github.com/Tomasz-Mankowski/STM32F4xx-WS2812B-lib
* http://elm-chan.org/fsw/ff/00index_e.html

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
## Credits
The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.
Supervisor: Tomasz Mańkowski
