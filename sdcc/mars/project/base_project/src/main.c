#include "chip_cv1822.h"
#include "cvi_gpio.h"

#define BUTTON_GPIO_BASE GPIO1_BASE
#define BUTTON_GPIO_PIN 11

// using pin B11 as LED output, using B9 for ground

void delay_us(uint8_t time)
{
	uint8_t i=0;

	while(time--) {
		i=10;
		while(i--) ;
	}
}

void main(void)
{
    set_gpio_direction(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN, GPIO_DIRECTION_OUT);

    while (1) {
        set_gpio_level(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN, GPIO_LEVEL_HIGH);
        delay_us(500000); // 0.5s
        set_gpio_level(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN, GPIO_LEVEL_LOW);
        delay_us(500000);
    }
}
