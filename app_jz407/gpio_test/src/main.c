#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS   1000
#define STACK_SIZE      1024
#define PRIORITY        7

static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios)
};

static const struct gpio_dt_spec buttons[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios)
};

void button_thread_entry(void *p1, void *p2, void *p3)
{
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
    }

    while (1) {
        for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
            if (gpio_pin_get_dt(&buttons[i]) > 0) {
                gpio_pin_toggle_dt(&leds[i]);
                
                k_msleep(50); 
            }
        }
        k_msleep(50);
    }
}

K_THREAD_DEFINE(button_tid, STACK_SIZE, button_thread_entry, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void)
{
    for (int i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!gpio_is_ready_dt(&leds[i])) return 0;
        gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
    }


    while (1) {
        for (int i = 0; i < ARRAY_SIZE(leds); i++) {
            int state = gpio_pin_get_dt(&leds[i]);
            printk("LED %d: %s\n", i, state ? "ON" : "OFF");
        }
        
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}