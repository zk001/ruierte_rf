#ifndef _BOARD_H_
#define _BOARD_H_
#include "../../common.h"
#include "key.h"

#define ROW0				GPIO_PA1
#define ROW1				GPIO_PD7
#define ROW2				GPIO_PA0
#define ROW3				GPIO_PD4

#define COL0				GPIO_PC6
#define COL1				GPIO_PC0
#define COL2				GPIO_PB7
#define COL3				GPIO_PB6

#define DCDC_CE				GPIO_PB1

#define LED1				GPIO_PB4
#define LED2				GPIO_PB5
#define LED3				GPIO_PC1
#define LED4				GPIO_PC4
#define LED5				GPIO_PC5

#define HAL_LED_ALL   (HAL_LED_1 | HAL_LED_2 | HAL_LED_3 | HAL_LED_4 | HAL_LED_5)
#define MAX_LEDS 5

#define MAX_GPIO_KEYS 13
#define MAX_GPIO_LEDS 5
#define MAX_KEYS (MAX_GPIO_KEYS)
#endif
