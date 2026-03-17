#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <stdio.h>

int main(void)
{
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c_sw));

    if (!device_is_ready(i2c_dev)) {
        printk("Error: Software I2C bus not ready. Check pull-ups and pin configuration.\n");
        return 0;
    }
    printk("Starting I2C bus scan (Software Bit-bang)...\n");

    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        struct i2c_msg msgs[1];
        uint8_t dst;

        msgs[0].buf = &dst;
        msgs[0].len = 0;
        msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

        int ret = i2c_transfer(i2c_dev, msgs, 1, addr);

        if (ret == 0) {
            printk("  - Device found at address: 0x%02X\n", addr);
        }
    }
    printk("Scan finished.\n");

    while (1) {
        k_msleep(1000);
    }
}