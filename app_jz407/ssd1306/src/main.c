#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stdio.h>

int main(void)
{
    while (1) {
        printk("Hello world!\n");
        k_sleep(K_SECONDS(1));
    }
}