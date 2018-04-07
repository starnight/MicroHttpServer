#ifndef __GPIO_H__
#define __GPIO_H__

/* Include STM32F4 and standard peripherals configuration headers. */
#include <stm32f4xx.h>
#include "stm32f4xx_conf.h"

/* Define the readable hardware memory address, according to the
 * schematic of STM32F4-Discovery.
 */
/* LEDs. */
#define GREEN    	GPIO_Pin_12 // Green LED connects to PD12
#define ORANGE   	GPIO_Pin_13 // Orange LED connects to PD13
#define RED      	GPIO_Pin_14 // Red LED connects to PD14
#define BLUE     	GPIO_Pin_15 // Blue LED connects to PD15
#define ALL_LEDS	(GREEN | ORANGE | RED | BLUE) // all leds
#define LEDS_GPIO_PORT	(GPIOD) // LEDs connect to Port D

void setup_leds(void);
void delay(__IO uint32_t nCount);

#endif
