/*
 * This program turns on the 4 leds of the stm32f4 discovery board
 * with pressed user button in order.
 */

#include "gpio.h"

#define LEDn     4 // 4 LEDs

/* User Button. */
#define USER_BUTTON GPIO_Pin_0 // User Button connects to PA0
#define BUTTON_GPIO_PORT (GPIOA) // User Button connects to Port A

/* The array stores the led order used to switch them on and off. */
static uint16_t leds[LEDn] = {GREEN, ORANGE, RED, BLUE};

/* This is how long we wait in the delay function. */
#define LED_LONG    1000000L
#define PAUSE_SHORT 10000L

/* A simple time comsuming function. */
void delay(__IO uint32_t nCount) {
    while(nCount--)
        __asm("nop"); // do nothing
}

/* Initialize the GPIO port D for output LEDs. */
void setup_leds(void) {
    /* Structure storing the information of GPIO Port D. */
    static GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable the GPIOD peripheral clock. */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Pin numbers of LEDs are mapped to bits number. */
    GPIO_InitStructure.GPIO_Pin   = ALL_LEDS;
    /* Pins in output mode. */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    /* Clock speed rate for the selected pins. */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    /* Operating output type for the selected pins. */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    /* Operating Pull-up/Pull down for the selected pins. */
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    /* Write this data into memory at the address
     * mapped to GPIO device port D, where the led pins
     * are connected */
    GPIO_Init(LEDS_GPIO_PORT, &GPIO_InitStructure);
}

/* Initialize the GPIO port A for input User Button. */
static void setup_button(void) {
    /* Structure storing the information of GPIO Port A. */
    static GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable the GPIOA peripheral clock. */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* Pin number of User Button is mapped to a bit number. */
    GPIO_InitStructure.GPIO_Pin   = USER_BUTTON;
    /* Pin in input mode. */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    /* Clock speed rate for the selected pins. */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    /* Operating output type for the selected pins. */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    /* Operating Pull-up/Pull down for the selected pins. */
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    
    /* Write this data into memory at the address
     * mapped to GPIO device port A, where the led pins
     * are connected */
    GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStructure);
}

/* Get the status of User Button.
 * 0: Not pressed.
 * 1: Pressed.
 */
static uint8_t read_button(void) {
    return GPIO_ReadInputDataBit(BUTTON_GPIO_PORT, USER_BUTTON);    
}

/* Turn all leds on and off 4 times. */
static void flash_all_leds(void) {
    int i;
    for (i = 0; i < 4; i++)
    {
        /* Turn on all user leds */
        GPIO_SetBits(LEDS_GPIO_PORT, ALL_LEDS);
        /* Wait a short time */
        delay(LED_LONG);
        /* Turn off all leds */
        GPIO_ResetBits(LEDS_GPIO_PORT, ALL_LEDS);
        /* Wait again before looping */
        delay(LED_LONG);
    }
}
