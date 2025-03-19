#include <stdio.h>
#include "cvi1822.h"
#include "chip_cv1822.h"
#include "dw_gpio.h"
#include "dw_uart.h"
#include "dw_ictl.h"
#include "cv_i2c.h"
#include "pwr_wdt.h"
#include "cvi_gpio.h"

void device_sleep_wake(uint8_t sleep_state);

#define BUTTON_GPIO_BASE GPIO1_BASE
#define BUTTON_GPIO_PIN 11 // pin B11

void delay_us(uint8_t time)
{
    uint8_t i = 0;
    while (time--)
    {
        i = 10;
        while (i--)
            ;
    }
}

uint8_t prev_button_state = GPIO_LEVEL_HIGH; // assume pin is pulled up by default

void main(void)
{
    uint8_t current_button_state;

#ifdef CONFIG_PUTCHAR_DW_UART
    dw_uart_init();
#endif

    // i think set led state (what?)
    mmio_write_32(RTC_INFO0, 0x8051);

    // initialize interrupt controller (not sure if needed)
    dw_ictl_init();
    ap2rtc_irq_init();
    irq_enable();

    // configure button GPIO as input (confident correct)
    set_gpio_direction(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN, GPIO_DIRECTION_IN);

    // LED GPIO for status indication (confident correct)
    set_gpio_direction(GPIO2_BASE, 24, GPIO_DIRECTION_OUT);
    set_gpio_level(GPIO2_BASE, 24, GPIO_LEVEL_HIGH); // Turn LED on when awake

    printf("GPIO Sleep/Wake monitoring started\n");

    while (1)
    {
        current_button_state = get_gpio_level(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN);

        // detect transition from HIGH to LOW if using pull-up
        if (prev_button_state == GPIO_LEVEL_HIGH && current_button_state == GPIO_LEVEL_LOW)
        {
            // wait
            delay_us(20);

            // read again to confirm hold and not noise
            current_button_state = get_gpio_level(BUTTON_GPIO_BASE, BUTTON_GPIO_PIN);

            if (current_button_state == GPIO_LEVEL_LOW)
            {
                printf("Button pressed (toggling sleep state)\n");

                // use LED state as sleep/wake state (SUPER stupid lol)
                uint8_t led_state = get_gpio_level(GPIO2_BASE, 24);

                if (led_state == GPIO_LEVEL_HIGH)
                {
                    // awake --> go to sleep
                    set_gpio_level(GPIO2_BASE, 24, GPIO_LEVEL_LOW); // LED off
                    device_sleep_wake(1);                           // 1 = sleep
                }
                else
                {
                    // asleep --> wake up
                    set_gpio_level(GPIO2_BASE, 24, GPIO_LEVEL_HIGH); // LED on
                    device_sleep_wake(0);                            // 0 = wake
                }
            }
        }

        // set state
        prev_button_state = current_button_state;

        // wait
        delay_us(10);
    }
}

#define CTRL_REG_BASE 0x05025000
// in the ctrl register
#define RTC_IP_ISO_CTRL_OFFSET 0x084
#define REG_MCU_ISO_EN_BIT 1
// also in the ctrl register
#define RTC_IP_PWR_REQ_OFFSET 0x080
#define REG_MCU_PWR_REQ_BIT 2
#define REG_MCU_PWR_ACK_BIT 18

#define RTCSYS_UART_BASE 0x05022000
// in the uart register
#define SRR_REG_OFFSET 0x088
#define SRR_BIT_0 0
#define SRR_BIT_1 1


void set_bit_at_addr(volatile uint32_t *addr_ptr, uint8_t bit, uint8_t value)
{
    uint32_t reg_val = *addr_ptr;
    reg_val = (reg_val & ~(1 << bit)) | ((value & 1) << bit);
    *addr_ptr = reg_val;
}


// sleep_state: 1 -> is currently awake -> sleep, 0 -> is currently asleep -> wake up
void device_sleep_wake(uint8_t sleep_state)
{
    uint32_t srr_addr = RTCSYS_UART_BASE + SRR_REG_OFFSET;
    uint32_t iso_ctrl_addr = CTRL_REG_BASE + RTC_IP_ISO_CTRL_OFFSET;
    uint32_t pwr_ctrl_addr = CTRL_REG_BASE + RTC_IP_PWR_REQ_OFFSET;

    if (sleep_state)
    {
        printf("Device entering sleep mode...\n");
        // soft reset register 0
        set_bit_at_addr(&srr_addr, 0, 0);
        set_bit_at_addr(&srr_addr, 1, 0);
        // reg_mcu_iso_en 1
        set_bit_at_addr(&iso_ctrl_addr, REG_MCU_ISO_EN_BIT, 1);
        // reg_mcu_pwr_req 0
        set_bit_at_addr(&pwr_ctrl_addr, REG_MCU_PWR_REQ_BIT, 0);       
    }
    else
    {
        printf("Device waking up...\n");
        // reg_mcu_pwr_req 1
        set_bit_at_addr(&pwr_ctrl_addr, REG_MCU_PWR_REQ_BIT, 1);
        // reg_mcu_pwr_ack 1
        set_bit_at_addr(&pwr_ctrl_addr, REG_MCU_PWR_ACK_BIT, 1);
        // reg_mcu_iso_en 0
        set_bit_at_addr(&iso_ctrl_addr, REG_MCU_ISO_EN_BIT, 0);
        // soft reset register 1
        set_bit_at_addr(&srr_addr, SRR_BIT_0, 1);
        set_bit_at_addr(&srr_addr, SRR_BIT_1, 1);
    }
}